#include <Python.h>
#include <statican.h>
#include <Python-ast.h>
#include <structmember.h>

#include <setobject.h>

/* Data */
typedef struct Ctxt_s {
	PyObject_HEAD
	PyObject *exp_defs; /*< A set containing all the variables defined explicitly in this context until now. */
	PyObject *imp_defs; /*< A set containing all the variables defined implicitly in this context until now.
	                        NULL, if implicit definitions are not meant to be protocolled. */
	PyObject *used;     /*< A set containing all the variables used in this context until now. */
	PyObject *name;     /*< The name of this context. */
	int is_func;        /*< Tells whether this is a function context. */
} Ctxt;
typedef struct StrictVisitor_s {
	PyObject_HEAD
	PyStaticAn_Visior_FIELDS
	PyObject *ctxts;        /*< A stack of contexts (implemented by a list of Ctxt).
	                            Must at least contain two entries:
	                              - the builtin context (containing builtin function/class/var names),
	                              - the 'global' context of this module. */
	Ctxt *cur;              /*< The current ctxt (last element of ctxts) */
	int entering_funcdef;   /*< Tells whether we are about to enter a function definition.
	                            The enter_block method fetches/resets this flag. */
	int in_function;        /*< Tells whether we are currently in a function. */
	unsigned strict_flags;  /*< The given strict options. */
} StrictVisitor;

#define STRICT_NO_UNUSED_IMP_VARS   1
#define STRICT_VARS                 2


/* Utility */

#define FOREACH_IN_SEQ(self, seq, f) PyStaticAn_foreach_in_asdl_seq((self), (seq), (PyStaticAn_visit_asdl_elt_t)(f))

/* An identifier is explicitly declared, if it is
 * - global,
 * - nonlocal,
 * - a parameter,
 * - an import,
 * - a function name,
 * - a class name,
 * - or annotated with a certain type.
 *
 * An identifier may shadow an identifier of an outer scope, if it is
 * - global
 * - or nonlocal.
*/
#define SHADOW_FLAG   (DEF_GLOBAL | DEF_NONLOCAL)

static void ctxt_add_exp_def(Ctxt *ctxt, PyObject *name)
{
	PyObject *exp_defs = ctxt->exp_defs;
	if (!exp_defs) {
		exp_defs = PySet_New(0);
		ctxt->exp_defs = exp_defs;
		if (!exp_defs) {
			Py_FatalError("StrictVisitor: Could not allocate internal set.");
		}
	}
	PySet_Add(exp_defs, name);
}
static int set_contains(PyObject *anyset, PyObject *key)
{
	if (!anyset) return 0;
	return PySet_Contains(anyset, key);
}
/* Find the context which explicitly declares the respective name. */
static Ctxt *find_exp_def_ctxt(StrictVisitor *self, PyObject *name)
{
	Py_ssize_t i, size;
	PyObject *ctxts;
	ctxts = self->ctxts;
	size = PyList_Size(ctxts);
	for (i = 0; i < size; i++) {
		Ctxt *ctxt = (Ctxt*)PyList_GetItem(ctxts, i);
		if (set_contains(ctxt->exp_defs, name)) {
			return ctxt;
		}
	}
	return NULL;
}

static void add_exp_def(PyStaticAn_Visitor *self_, PyObject *name, int flag)
{
	StrictVisitor *self = (StrictVisitor*)self_;

	if (self->in_function && ((flag & SHADOW_FLAG) == 0) && (self->strict_flags & STRICT_VARS)) {
		Ctxt *ctxt = find_exp_def_ctxt(self, name);
		if (ctxt != NULL) {
			/* TODO More detailed info. */
			PyErr_Format(PyExc_SyntaxError,
				"Item '%R' in context '%R' shadows previously declared item in %s context '%R'",
				name,
				self->cur->name,
				(ctxt == cur)? "same" : "outer",
				ctxt->name
			);
			PyErr_SyntaxLocationObject(self->filename, 0, 0);
			self->abort = 1;
		}
	}
	ctxt_add_exp_def(self->cur, name);
}
static void add_imp_def(PyStaticAn_Visitor *self_, PyObject *name)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	PyObject *imp_defs = self->imp_defs;
	Ctxt *found = NULL;
	if (!imp_defs) return; /* Not meant to be collected. */
	if (!find_exp_def_ctxt(self, name)) {
		PySet_Add(imp_defs, name);
	}
}

static void check_if_decl(StrictVisitor *self, PyObject *name, int lineno, int col_offset)
{
	if (find_exp_def_ctxt(self, name)) return;
	PyErr_Format(PyExc_SyntaxError,
		"strict_vars does not allow to use undeclared identifier '%R'",
		name
	);
	PyErr_SyntaxLocationObject(self->filename, lineno, col_offset);
	self->abort = 1;
}

/* MOD - Visit it the default DFS way. */

/* DFS STMT */
static void visit_def(PyStaticAn_Visitor *self_, PyObject *name, int flag)
{
	/* Everything which is visited here is declared explicitly. */
	add_exp_def(self_, name, flag);
}

static PyObject *visit_stmt_FunctionDef(PyStaticAn_Visitor *self_, stmt_ty s)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	/* The strict imports must happen **textually** before the first function. */
	if (!self->strict_flags) {
		self->abort = 1;
		return;
	}
	self->entering_funcdef = 1;
	return PyStaticAn_visit_stmt_dfs_FunctionDef(self_, s);
}
static PyObject *visit_stmt_AsyncFunctionDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	if (!self->strict_flags) {
		self->abort = 1;
		return;
	}
	self->entering_funcdef = 1;
	return PyStaticAn_visit_stmt_dfs_AsyncFunctionDef(self_, s);
}
#define visit_stmt_ClassDef PyStaticAn_visit_stmt_dfs_ClassDef
#define visit_stmt_Return PyStaticAn_visit_stmt_dfs_Return
#define visit_stmt_Delete PyStaticAn_visit_stmt_dfs_Delete

static PyObject *PyStaticAn_visit_stmt_dfs_Assign(PyStaticAn_Visitor *self_, stmt_ty s)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	unsigned strict_flags = self->strict_flags;
	asdl_seq *targets = s->v.Assign.targets;
	unsigned i, len;

	if (!self->strict_flags) {
		self->abort = 1;
		return NULL;
	}

	len = asdl_seq_LEN(seq);
	for (i = 0; i < len; i++) {
		expr_ty target = (expr_ty)asdl_seq_GET(seq, i);
		if (target->kind == Name_kind) {
			if (in_function && (strict_flags & STRICT_VARS)) {
				check_if_decl(self, target->v.Name.id, target->lineno, target->col_offset);
			} else {
				add_imp_def(self, target->v.Name.id);
			}
		} else {
			/* visit it normally. */
			PyStaticAn_accept_expr(self_, target);
		}
	}

	/* visit the rhs value normally. */
	return PyStaticAn_accept_expr(self_, s->v.Assign.value);
}
PyObject *PyStaticAn_visit_stmt_dfs_AugAssign(PyStaticAn_Visitor *self, stmt_ty s)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	unsigned strict_flags = self->strict_flags;
	expr_ty target = s->v.AugAssign.target;

	if (!self->strict_flags) {
		self->abort = 1;
		return NULL;
	}

	if (target->kind == Name_kind) {
		if (strict_flags & STRICT_VARS) {
			add_exp_def(self, e_name->v.Name.id, 0);
		} else {
			add_imp_def(self, target->v.Name.id);
		}
	} else {
		/* visit it normally. */
		PyStaticAn_accept_expr(self_, target);
	}

	/* visit the rhs value normally. */
	return PyStaticAn_accept_expr(self_, s->v.AugAssign.value);
}

#define visit_stmt_AnnAssign PyStaticAn_visit_stmt_dfs_AnnAssign

static void add_stmt_for_iter(StrictVisitor *self, PyObject *name)
{
	if (find_exp_def_ctxt(self, name)) return;
	add_exp_def(self, name, 0);
}

static void visit_stmt_for_target(StrictVisitor *self, expr_ty target)
{
	PyStaticAn_Visitor *self_ = (PyStaticAn_Visitor *)self;

	if (!self->strict_flags) {
		self->abort = 1;
		return;
	}

	if (target->kind == Name_kind) {
		add_stmt_for_iter(self, target->Name.id);
	} else if (target->kind == Tuple_kind) {
		unsigned i, len;
		asdl_seq *seq;

		seq = target->Tuple.elts;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			expr_ty elt = (expr_ty)asdl_seq_GET(seq, i);
			if (elt->kind == Name_kind) {
				add_stmt_for_iter(self, elt->Name.id);
			} else {
				/* visit it normally. */
				PyStaticAn_accept_expr(self_, elt);
			}
		}
	} else {
		/* visit it normally. */
		PyStaticAn_accept_expr(self_, target);
	}
}

PyObject *PyStaticAn_visit_stmt_dfs_For(PyStaticAn_Visitor *self_, stmt_ty s)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	unsigned strict_flags = self->strict_flags;

	if ((strict_flags & STRICT_VARS) == 0) {
		return PyStaticAn_visit_stmt_dfs_For(self_, s);
	}

	self->vt->enter_subblock(self);
	visit_stmt_for_target(self, s->v.For.target);

	/* visit the iteration expression and the block of this statement normally. */
	accept_expr(self, s->v.For.iter);
	PyStaticAn_visit_stmt_dfs_ForBlock(self_, s);

	/* close the additional sub-block. */
	self->vt->leave_subblock(self);

	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_AsyncFor(PyStaticAn_Visitor *self, stmt_ty s)
{
	/* TODO DRY */
	StrictVisitor *self = (StrictVisitor*)self_;
	unsigned strict_flags = self->strict_flags;

	if ((strict_flags & STRICT_VARS) == 0) {
		return PyStaticAn_visit_stmt_dfs_AsyncFor(self_, s);
	}

	self->vt->enter_subblock(self);
	visit_stmt_for_target(self, s->v.AsyncFor.target);

	/* visit the iteration expression and the block of this statement normally. */
	accept_expr(self, s->v.AsyncFor.iter);
	PyStaticAn_visit_stmt_dfs_AsyncForBlock(self_, s);

	/* close the additional sub-block. */
	self->vt->leave_subblock(self);

	return NULL;
}

#define visit_stmt_While PyStaticAn_visit_stmt_dfs_While
#define visit_stmt_If PyStaticAn_visit_stmt_dfs_If


static PyObject *visit_withitem(PyStaticAn_Visitor *self_, withitem_ty item)
{
	StrictVisitor *self = (StrictVisitor*)self_;
	unsigned strict_flags = self->strict_flags;
	expr_ty optv;

	PyStaticAn_accept_expr(self_, item->context_expr);
	optv = item->optional_vars;
	if (optv) {
		if (optv->kind == Name_kind) {
			if ((strict_flags & STRICT_VARS) == 0) {
				add_exp_def(self, optv->Name.id, 0);
			} else {
				add_imp_def(self, optv->Name.id);
			}
		} else {
			PyStaticAn_accept_expr(self, item->optional_vars);
		}
	}
	return NULL;
}
PyObject *visit_stmt_With(PyStaticAn_Visitor *self_, stmt_ty s)
{
	self->vt->enter_subblock(self);
	PyStaticAn_visit_stmt_dfs_With(self_, s);
	self->vt->leave_subblock(self);
	return NULL;
}
PyObject *visit_stmt_AsyncWith(PyStaticAn_Visitor *self, stmt_ty s)
{
	/* TODO DRY */
	self->vt->enter_subblock(self);
	PyStaticAn_visit_stmt_dfs_AsyncWith(self_, s);
	self->vt->leave_subblock(self);
	return NULL;
}
#define visit_stmt_Raise PyStaticAn_visit_stmt_dfs_Raise
#define visit_stmt_Try PyStaticAn_visit_stmt_dfs_Try
#define visit_stmt_Assert PyStaticAn_visit_stmt_dfs_Assert

static PyObject *visit_alias(StrictVisitor *self, alias_ty a)
{
	/* Compute store_name, the name actually bound by the import
	   operation. It is different from a->name when a->name is a
	   dotted package name (e.g. spam.eggs)
	*/
	PyObject *store_name;
	PyObject *name = (a->asname == NULL) ? a->name : a->asname;
	Py_ssize_t dot = PyUnicode_FindChar(name, '.', 0, PyUnicode_GET_LENGTH(name), 1);
	if (dot != -1) {
		store_name = PyUnicode_Substring(name, 0, dot);
		if (!store_name) {
			Py_FatalError("StrictPythonVisitor: Out of memory");
		}
	} else {
		store_name = name;
		Py_INCREF(store_name);
	}

	if (_PyUnicode_EqualToASCIIString(name, "strict_no_unused_vars")) {
		self->strict_flags |= STRICT_NO_UNUSED_IMP_VARS;
	} else if (_PyUnicode_EqualToASCIIString(name, "strict_vars")) {
		self->strict_flags |= STRICT_VARS;
	}

	if (!_PyUnicode_EqualToASCIIString(name, "*")) {
		add_exp_def(self, store_name, DEF_IMPORT);
	}

	Py_DECREF(store_name);
	return NULL;
}
PyObject *visit_stmt_Import(PyStaticAn_Visitor *self, stmt_ty s)
{
	return FOREACH_IN_SEQ(self, s->v.Import.names, visit_alias);
}
PyObject *PyStaticAn_visit_stmt_dfs_ImportFrom(PyStaticAn_Visitor *self, stmt_ty s)
{
	return FOREACH_IN_SEQ(self, s->v.ImportFrom.names, visit_alias);
}
#define visit_stmt_Global PyStaticAn_visit_stmt_dfs_Global
#define visit_stmt_Nonlocal PyStaticAn_visit_stmt_dfs_Nonlocal
#define visit_stmt_Expr PyStaticAn_visit_stmt_dfs_Expr
#define visit_stmt_Pass PyStaticAn_visit_stmt_dfs_Pass
#define visit_stmt_Break PyStaticAn_visit_stmt_dfs_Break
#define visit_stmt_Continue PyStaticAn_visit_stmt_dfs_Continue

/* DFS EXPR */

#define visit_expr_BoolOp PyStaticAn_visit_expr_dfs_BoolOp
#define visit_expr_BinOp PyStaticAn_visit_expr_dfs_BinOp
#define visit_expr_UnaryOp PyStaticAn_visit_expr_dfs_UnaryOp
#define visit_expr_Lambda PyStaticAn_visit_expr_dfs_Lambda
#define visit_expr_IfExp PyStaticAn_visit_expr_dfs_IfExp
#define visit_expr_Dict PyStaticAn_visit_expr_dfs_Dict
#define visit_expr_Set PyStaticAn_visit_expr_dfs_Set

static PyObject *visit_comprehension_generator(PyStaticAn_Visitor *self_, comprehension_ty cg)
{
	StrictVisitor *self = (StrictVisitor *)self_;
	expr_ty target = cg->target;

	if (!self->strict_flags) {
		self->abort = 1;
		return NULL;
	}

	if (target->kind == Name_kind) {
		add_exp_def(self, target->Name.id);
	} else if (target->kind == Tuple_kind) {
		unsigned i, len;
		asdl_seq *seq;

		seq = target->Tuple.elts;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			expr_ty elt = (expr_ty)asdl_seq_GET(seq, i);
			if (elt->kind == Name_kind) {
				add_exp_def(self, elt->Name.id);
			} else {
				/* visit it normally. */
				PyStaticAn_accept_expr(self_, elt);
			}
		}
	} else {
		/* visit it normally. */
		PyStaticAn_accept_expr(self_, target);
	}
	PyStaticAn_accept_expr(self_, cg->iter);
	PyStaticAn_accept_expr_seq(self_, cg->ifs);

	return NULL;
}

#define visit_expr_ListComp PyStaticAn_visit_expr_dfs_ListComp
#define visit_expr_SetComp PyStaticAn_visit_expr_dfs_SetComp
#define visit_expr_DictComp PyStaticAn_visit_expr_dfs_DictComp
#define visit_expr_GeneratorExp PyStaticAn_visit_expr_dfs_GeneratorExp

#define visit_expr_Await PyStaticAn_visit_expr_dfs_Await
#define visit_expr_Yield PyStaticAn_visit_expr_dfs_Yield
#define visit_expr_YieldFrom PyStaticAn_visit_expr_dfs_YieldFrom
#define visit_expr_Compare PyStaticAn_visit_expr_dfs_Compare
#define visit_expr_Call PyStaticAn_visit_expr_dfs_Call
#define visit_expr_Num PyStaticAn_visit_expr_dfs_Num
#define visit_expr_Str PyStaticAn_visit_expr_dfs_Str
#define visit_expr_FormattedValue PyStaticAn_visit_expr_dfs_FormattedValue
#define visit_expr_JoinedStr PyStaticAn_visit_expr_dfs_JoinedStr
#define visit_expr_Bytes PyStaticAn_visit_expr_dfs_Bytes
#define visit_expr_NameConstant PyStaticAn_visit_expr_dfs_NameConstant
#define visit_expr_Ellipsis PyStaticAn_visit_expr_dfs_Ellipsis
#define visit_expr_Constant PyStaticAn_visit_expr_dfs_Constant

/* Note: We don't visit the id of attribute fields - We don't need to check them
         and we can't because they are not local variables, but might stem from
         other packages. */
#define visit_expr_Attribute PyStaticAn_visit_expr_dfs_Attribute

#define visit_expr_Subscript PyStaticAn_visit_expr_dfs_Subscript
#define visit_expr_Starred PyStaticAn_visit_expr_dfs_Starred

PyObject *PyStaticAn_visit_expr_dfs_Name(PyStaticAn_Visitor *self, expr_ty e)
{
	/* TODO what about function names and imported names?
	        ignore all names following a '.' - visit_expr_Attribute. */
	/* TODO check whether declared, if strict_vars.
	        add to used names, if strict_no_unused_vars. */
	return NULL;
}
#define visit_expr_List PyStaticAn_visit_expr_dfs_List
#define visit_expr_Tuple PyStaticAn_visit_expr_dfs_Tuple


/* TODO Blocks: enter/leave, function block ... */


static void PyStaticAn_StrictVisitor_destroy(PyObject *self_)
{
	StrictVisitor *self = (StrictVisitor *)self_;

	PyStaticAn_Visitor_destroy(self_);
	/* TODO destroy the ctxt objects. */
#if 0
	if (self->filename) {
		Py_DECREF(self->filename);
		self->filename = NULL;
	}
#endif
}


static PyMemberDef PyStaticAn_Visitor_memberlist[] = {
	{"filename", T_OBJECT, offsetof(PyStaticAn_Visitor, filename), READONLY},
	{"abort",	T_INT,	offsetof(PyStaticAn_Visitor, abort), READONLY},
	{NULL}
};
PyObject *PyStaticAn_Visitor_repr(PyObject *self_)
{
	PyStaticAn_Visitor *self = (PyStaticAn_Visitor *)self_;
	return PyUnicode_FromFormat("<(PyStaticAn_Visitor %s at %p in %R>",
								self->vt->class_name,
								self,
								self->filename);
}


PyTypeObject PyStaticAn_Visitor_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"PyStaticAn_Visitor",
	sizeof(PyStaticAn_Visitor),
	0,
	(destructor)PyStaticAn_Visitor_destroy, /* tp_dealloc */
	0, /* tp_print */
	0, /* tp_getattr */
	0, /* tp_setattr */
	0, /* tp_reserved */
	(reprfunc)PyStaticAn_Visitor_repr, /* tp_repr */
	0, /* tp_as_number */
	0, /* tp_as_sequence */
	0, /* tp_as_mapping */
	0, /* tp_hash */
	0, /* tp_call */
	0, /* tp_str */
	PyObject_GenericGetAttr, /* tp_getattro */
	0, /* tp_setattro */
	0, /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT, /* tp_flags */
	0,  /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	0, /* tp_methods */
	PyStaticAn_Visitor_memberlist, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};
