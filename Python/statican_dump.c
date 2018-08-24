#include <Python.h>
#include <statican.h>
#include <Python-ast.h>
#include <structmember.h>
#include <stdio.h>
#include <traceback.h>


typedef struct DumpVisitor_s {
	PyObject_HEAD
	PyStaticAn_Visior_FIELDS
	int depth;
} DumpVisitor;

static void nl(DumpVisitor const *v) {
	int i;
	printf("\n");
	for (i = 0; i < v->depth; i++) {
		printf(" ");
	}
}

static PyObject *vmod(PyStaticAn_Visitor *self_, mod_ty mod, const char *msg)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	nl(self);
	printf("mod %s", msg);
	fflush(stdout);
	self->depth++;
	PyStaticAn_visit_mod_dfs(self_, mod);
	self->depth--;
	printf("\n");
	return NULL;
}
static PyObject *vstmt(PyStaticAn_Visitor *self_, stmt_ty stmt, const char *msg)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	nl(self);
	printf("stmt %s", msg);
	fflush(stdout);
	self->depth++;
	PyStaticAn_visit_stmt_dfs(self_, stmt);
	self->depth--;
	return NULL;
}
static PyObject *vexpr(PyStaticAn_Visitor *self_, expr_ty expr, const char *msg)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	nl(self);
	printf("expr %s", msg);
	fflush(stdout);
	self->depth++;
	PyStaticAn_visit_expr_dfs(self_, expr);
	self->depth--;
	return NULL;
}

/* MOD */
static PyObject *visit_mod_Module(PyStaticAn_Visitor *self, mod_ty mod)
{
	return vmod(self, mod, "Module");
}
static PyObject *visit_mod_Interactive(PyStaticAn_Visitor *self, mod_ty mod)
{
	return vmod(self, mod, "Interactive");
}
static PyObject *visit_mod_Expression(PyStaticAn_Visitor *self, mod_ty mod)
{
	return vmod(self, mod, "Expression");
}
static PyObject *visit_mod_Suite(PyStaticAn_Visitor *self, mod_ty mod)
{
	return vmod(self, mod, "Suite");
}

/* STMT */
static PyObject *visit_stmt_FunctionDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "FunctionDef");
}
static PyObject *visit_stmt_AsyncFunctionDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "AsyncFunctionDef");
}
static PyObject *visit_stmt_ClassDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "ClassDef");
}
static PyObject *visit_stmt_Return(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Return");
}
static PyObject *visit_stmt_Delete(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Delete");
}
static PyObject *visit_stmt_Assign(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Assign");
}
static PyObject *visit_stmt_AugAssign(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "AugAssign");
}
static PyObject *visit_stmt_AnnAssign(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "AnnAssign");
}
static PyObject *visit_stmt_For(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "For");
}
static PyObject *visit_stmt_AsyncFor(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "AsyncAssign");
}
static PyObject *visit_stmt_While(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "While");
}
static PyObject *visit_stmt_If(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "If");
}
static PyObject *visit_stmt_With(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "With");
}
static PyObject *visit_stmt_AsyncWith(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "AsyncWhile");
}
static PyObject *visit_stmt_Raise(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Raise");
}
static PyObject *visit_stmt_Try(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Try");
}
static PyObject *visit_stmt_Assert(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Assert");
}
static PyObject *visit_stmt_Import(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Import");
}
static PyObject *visit_stmt_ImportFrom(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "ImportFrom");
}
static PyObject *visit_stmt_Global(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Global");
}
static PyObject *visit_stmt_Nonlocal(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Nonlocal");
}
static PyObject *visit_stmt_Expr(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Expr");
}
static PyObject *visit_stmt_Pass(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Pass");
}
static PyObject *visit_stmt_Break(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Break");
}
static PyObject *visit_stmt_Continue(PyStaticAn_Visitor *self, stmt_ty s)
{
	return vstmt(self, s, "Continue");
}

/* DFS EXPR */
static PyObject *visit_expr_BoolOp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "BoolOp");
}
static PyObject *visit_expr_BinOp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "BinOp");
}
static PyObject *visit_expr_UnaryOp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "UnaryOp");
}

static PyObject *visit_expr_Lambda(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Lambda");
}
static PyObject *visit_expr_IfExp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "IfExp");
}
static PyObject *visit_expr_Dict(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Dict");
}
static PyObject *visit_expr_Set(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Set");
}
static PyObject *visit_expr_ListComp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "ListComp");
}

static PyObject *visit_expr_SetComp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "SetComp");
}
static PyObject *visit_expr_DictComp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "DictComp");
}
static PyObject *visit_expr_GeneratorExp(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "GeneratorExp");
}
static PyObject *visit_expr_Await(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Await");
}
static PyObject *visit_expr_Yield(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Yield");
}
static PyObject *visit_expr_YieldFrom(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "YieldFrom");
}
static PyObject *visit_expr_Compare(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Compare");
}
static PyObject *visit_expr_Call(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Call");
}
static PyObject *visit_expr_Num(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Num");
}
static PyObject *visit_expr_Str(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Str");
}
static PyObject *visit_expr_FormattedValue(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "FormattedValue");
}
static PyObject *visit_expr_JoinedStr(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "JoinedStr");
}
static PyObject *visit_expr_Bytes(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Bytes");
}
static PyObject *visit_expr_NameConstant(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "NameConstant");
}
static PyObject *visit_expr_Ellipsis(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Ellipsis");
}
static PyObject *visit_expr_Constant(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Constant");
}
static PyObject *visit_expr_Attribute(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Attribute");
}
static PyObject *visit_expr_Subscript(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Subscript");
}
static PyObject *visit_expr_Starred(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Starred");
}
static PyObject *visit_expr_Name(PyStaticAn_Visitor *self_, expr_ty e)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	vexpr(self_, e, "Name");
	self->depth++;
	nl(self);
	self->depth--;
	fflush(stdout);
	_Py_DumpASCII(0, e->v.Name.id);
	return NULL;
}
static PyObject *visit_expr_List(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "List");
}
static PyObject *visit_expr_Tuple(PyStaticAn_Visitor *self, expr_ty e)
{
	return vexpr(self, e, "Tuple");
}

static void enter_block(PyStaticAn_Visitor *self_, identifier name, _Py_block_ty block)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	nl(self);
	printf("block ");
	fflush(stdout);
	_Py_DumpASCII(0,name);
	printf(" {");
	self->depth++;
}
static void leave_block(PyStaticAn_Visitor *self_)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	self->depth--;
	nl(self);
	printf("}");
}
static void enter_subblock(PyStaticAn_Visitor *self_)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	nl(self);
	printf("subblock {");
	self->depth++;
}
static void leave_subblock(PyStaticAn_Visitor *self_)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	self->depth--;
	nl(self);
	printf("}");
}
void visit_def(PyStaticAn_Visitor *self_, PyObject *name, int flag)
{
	DumpVisitor *self = (DumpVisitor*)self_;
	nl(self);
	printf("definition ");
	fflush(stdout);
	_Py_DumpASCII(0,name);
	printf(" %x", flag);
}

#define TAB_ENTRY(TYPE, KIND) [KIND##_kind] = visit_##TYPE##_##KIND

static const PyStaticAn_visitorvt_t DumpVisitorVT = {
	.class_name = "DumpVisitor",
	.join = PyStaticAn_join_null,
	.visit_mod = {
		[PY_STATICAN_MOD_DEFAULT] = PyStaticAn_report_unexpected_mod,
		TAB_ENTRY(mod, Module),
		TAB_ENTRY(mod, Interactive),
		TAB_ENTRY(mod, Expression),
		TAB_ENTRY(mod, Suite)
	},
	.visit_stmt = {
		[PY_STATICAN_STMT_DEFAULT] = PyStaticAn_report_unexpected_stmt,
		TAB_ENTRY(stmt, FunctionDef),
		TAB_ENTRY(stmt, AsyncFunctionDef),
		TAB_ENTRY(stmt, ClassDef),
		TAB_ENTRY(stmt, Return),
		TAB_ENTRY(stmt, Delete),
		TAB_ENTRY(stmt, Assign),
		TAB_ENTRY(stmt, AugAssign),
		TAB_ENTRY(stmt, AnnAssign),
		TAB_ENTRY(stmt, For),
		TAB_ENTRY(stmt, AsyncFor),
		TAB_ENTRY(stmt, While),
		TAB_ENTRY(stmt, If),
		TAB_ENTRY(stmt, With),
		TAB_ENTRY(stmt, AsyncWith),
		TAB_ENTRY(stmt, Raise),
		TAB_ENTRY(stmt, Try),
		TAB_ENTRY(stmt, Assert),
		TAB_ENTRY(stmt, Import),
		TAB_ENTRY(stmt, ImportFrom),
		TAB_ENTRY(stmt, Global),
		TAB_ENTRY(stmt, Nonlocal),
		TAB_ENTRY(stmt, Expr),
		TAB_ENTRY(stmt, Pass),
		TAB_ENTRY(stmt, Break),
		TAB_ENTRY(stmt, Continue)
	},
	.visit_expr = {
		[PY_STATICAN_EXPR_DEFAULT] = PyStaticAn_report_unexpected_expr,
		TAB_ENTRY(expr, BoolOp),
		TAB_ENTRY(expr, BinOp),
		TAB_ENTRY(expr, UnaryOp),
		TAB_ENTRY(expr, Lambda),
		TAB_ENTRY(expr, IfExp),
		TAB_ENTRY(expr, Dict),
		TAB_ENTRY(expr, Set),
		TAB_ENTRY(expr, ListComp),
		TAB_ENTRY(expr, SetComp),
		TAB_ENTRY(expr, DictComp),
		TAB_ENTRY(expr, GeneratorExp),
		TAB_ENTRY(expr, Await),
		TAB_ENTRY(expr, Yield),
		TAB_ENTRY(expr, YieldFrom),
		TAB_ENTRY(expr, Compare),
		TAB_ENTRY(expr, Call),
		TAB_ENTRY(expr, Num),
		TAB_ENTRY(expr, Str),
		TAB_ENTRY(expr, FormattedValue),
		TAB_ENTRY(expr, JoinedStr),
		TAB_ENTRY(expr, Bytes),
		TAB_ENTRY(expr, NameConstant),
		TAB_ENTRY(expr, Ellipsis),
		TAB_ENTRY(expr, Constant),
		TAB_ENTRY(expr, Attribute),
		TAB_ENTRY(expr, Subscript),
		TAB_ENTRY(expr, Starred),
		TAB_ENTRY(expr, Name),
		TAB_ENTRY(expr, List),
		TAB_ENTRY(expr, Tuple)
	},
	.enter_block = enter_block,
	.leave_block = leave_block,
	.enter_subblock = enter_subblock,
	.leave_subblock = leave_subblock,
	.visit_def = visit_def,
	.visit_withitem = 0,
	.visit_excepthandler = 0,
	.visit_comprehension_generator = 0
};

static PyTypeObject DumpVisitor_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "PyStaticAn_DumpVisitor",
    sizeof(DumpVisitor),
    0,
    (destructor)PyStaticAn_Visitor_destroy, /* tp_dealloc */
    0,                                      /* tp_print */
    0,                                         /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc)PyStaticAn_Visitor_repr,          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    0,                                          /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    &PyStaticAn_Visitor_Type,                   /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};

PyObject *PyStaticAn_DumpVisitorFactory(unsigned i)
{
	DumpVisitor *v = PyObject_New(DumpVisitor, &DumpVisitor_Type);
    if (!v) {
    	Py_FatalError("Could not allocate visitor.");
    }
    v->vt = &DumpVisitorVT;
    return (PyObject *)v;
}

