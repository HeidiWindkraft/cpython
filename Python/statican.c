#include <Python.h>
#include <statican.h>
#include <Python-ast.h>
#include <structmember.h>

#include <stdio.h>
#define DBG printf

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#  define UNLIKELY(value) __builtin_expect((value), 0)
#else
#  define UNLIKELY(value) (value)
#endif

#define TAB_ENTRY(TYPE, KIND) [KIND##_kind] = PyStaticAn_visit_##TYPE##_dfs_##KIND

static const PyStaticAn_visit_mod_t PyStaticAn_mod_dfs_tab[PY_STATICAN_N_MOD_KINDS] =
{
	[PY_STATICAN_MOD_DEFAULT] = PyStaticAn_report_unexpected_mod,
	TAB_ENTRY(mod, Module),
	TAB_ENTRY(mod, Interactive),
	TAB_ENTRY(mod, Expression),
	TAB_ENTRY(mod, Suite)
};
static const PyStaticAn_visit_stmt_t PyStaticAn_stmt_dfs_tab[PY_STATICAN_N_STMT_KINDS] =
{
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
};
static const PyStaticAn_visit_expr_t PyStaticAn_expr_dfs_tab[PY_STATICAN_N_EXPR_KINDS] =
{
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
};

static void stralloc_panic(void)
{
	Py_FatalError("Could not allocate internal string.");
}

/* Note that the order of evalutation of array initializers is indeterminately sequenced. */
static PyObject *join_arr(PyStaticAn_Visitor *self, PyObject *arr[], unsigned len)
{
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;
	unsigned i;

	join = self->vt->join;
	for (i = 0; i < len; i++) {
		res = join(self, res, arr[i]);
	}
	return res;
}
#define JOINARR(self, arr) join_arr((self), arr, sizeof(arr)/sizeof(arr[0]))

static PyObject *dispatch_visit_mod(
	const PyStaticAn_visit_mod_t *table,
	PyStaticAn_Visitor *self,
	mod_ty mod,
	PyStaticAn_visit_mod_t fallback)
{
	unsigned kindidx;
	PyStaticAn_visit_mod_t f;

	kindidx = (unsigned)(mod->kind);
	if (UNLIKELY(kindidx > PY_STATICAN_N_MOD_KINDS)) {
		return fallback(self, mod);
	}
	f = table[kindidx];
	if (UNLIKELY(!f)) {
		return fallback(self, mod);
	}
	return f(self, mod);
}
static PyObject *dispatch_visit_stmt(
	const PyStaticAn_visit_stmt_t *table,
	PyStaticAn_Visitor *self,
	stmt_ty stmt,
	PyStaticAn_visit_stmt_t fallback)
{
	unsigned kindidx;
	PyStaticAn_visit_stmt_t f;

	kindidx = (unsigned)(stmt->kind);
	if (UNLIKELY(kindidx > PY_STATICAN_N_STMT_KINDS)) {
		return fallback(self, stmt);
	}
	f = table[kindidx];
	if (UNLIKELY(!f)) {
		return fallback(self, stmt);
	}
	return f(self, stmt);
}
static PyObject *dispatch_visit_expr(
	const PyStaticAn_visit_expr_t *table,
	PyStaticAn_Visitor *self,
	expr_ty expr,
	PyStaticAn_visit_expr_t fallback)
{
	unsigned kindidx;
	PyStaticAn_visit_expr_t f;

	kindidx = (unsigned)(expr->kind);
	if (UNLIKELY(kindidx > PY_STATICAN_N_EXPR_KINDS)) {
		return fallback(self, expr);
	}
	f = table[kindidx];
	if (UNLIKELY(!f)) {
		return fallback(self, expr);
	}
	return f(self, expr);
}

static PyObject *accept_mod(PyStaticAn_Visitor *self, mod_ty mod)
{
	const PyStaticAn_visit_mod_t *tab = self->vt->visit_mod;
	if (self->abort) return NULL;
	if (!mod) return NULL;
	return dispatch_visit_mod(tab, self, mod, tab[0]);
}
static PyObject *accept_stmt(PyStaticAn_Visitor *self, stmt_ty stmt)
{
	const PyStaticAn_visit_stmt_t *tab = self->vt->visit_stmt;
	if (self->abort) return NULL;
	if (!stmt) return NULL;
	return dispatch_visit_stmt(tab, self, stmt, tab[0]);
}
static PyObject *accept_expr(PyStaticAn_Visitor *self, expr_ty expr)
{
	const PyStaticAn_visit_expr_t *tab = self->vt->visit_expr;
	if (self->abort) return NULL;
	if (!expr) return NULL;
	return dispatch_visit_expr(tab, self, expr, tab[0]);
}
static PyObject *accept_expr_seq(PyStaticAn_Visitor *self, asdl_seq *seq)
{
	int i, len;
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;

	if (self->abort) return NULL;
	if (seq) {
		join = self->vt->join;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			expr_ty elt = (expr_ty)asdl_seq_GET(seq, i);
			if (!elt) continue;
			res = join(self, res, accept_expr(self, elt));
		}
	}
	return res;
}
static PyObject *accept_keyword_seq(PyStaticAn_Visitor *self, asdl_seq *seq)
{
	int i, len;
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;

	if (self->abort) return NULL;
	if (seq) {
		join = self->vt->join;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			keyword_ty elt = (keyword_ty)asdl_seq_GET(seq, i);
			if (!elt) continue;
			res = join(self, res, accept_expr(self, elt->value));
		}
	}
	return res;
}
static void accept_def(PyStaticAn_Visitor *self, PyObject *name, int flag)
{
	if (self->abort) return;
	if (self->vt->visit_def) {
		self->vt->visit_def(self, name, flag);
	}
}
static PyObject *accept_stmt_seq(PyStaticAn_Visitor *self, asdl_seq *seq)
{
	int i, len;
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;

	if (self->abort) return NULL;
	if (seq) {
		join = self->vt->join;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			stmt_ty elt = (stmt_ty)asdl_seq_GET(seq, i);
			if (!elt) continue;
			res = join(self, res, accept_stmt(self, elt));
		}
	}
	return res;
}
static PyObject *visit_slice(PyStaticAn_Visitor *self, slice_ty s);
static PyObject *accept_slice_seq(PyStaticAn_Visitor *self, asdl_seq *seq)
{
	int i, len;
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;

	if (self->abort) return NULL;
	if (seq) {
		join = self->vt->join;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			slice_ty elt = (slice_ty)asdl_seq_GET(seq, i);
			if (!elt) continue;
			res = join(self, res, visit_slice(self, elt));
		}
	}
	return res;
}
static PyObject *visit_slice(PyStaticAn_Visitor *self, slice_ty s)
{
	PyObject *res = NULL;
	switch (s->kind) {
	case Slice_kind:
	{
		PyObject *a, *b, *c;
		a = accept_expr(self, s->v.Slice.lower);
		b = accept_expr(self, s->v.Slice.upper);
		c = accept_expr(self, s->v.Slice.step);
		{
			PyObject *xs[] = { a, b, c };
			res = JOINARR(self, xs);
		}
		break;
	}
	case ExtSlice_kind:
		res = accept_slice_seq(self, s->v.ExtSlice.dims);
		break;
	case Index_kind:
		res = accept_expr(self, s->v.Index.value);
		break;
	default:
		PyErr_Format(PyExc_SyntaxError,
			"Internal error: Visitor '%s' encountered unexpected slice expression (integer value of kind: %d)",
			self->vt->class_name,
			(unsigned)(s->kind)
		);
		PyErr_SyntaxLocationObject(self->filename, 0, 0);
	}
	return res;
}




/* DFS MOD */
PyObject *PyStaticAn_visit_mod_dfs(PyStaticAn_Visitor *self, mod_ty mod)
{
	return dispatch_visit_mod(PyStaticAn_mod_dfs_tab, self, mod, PyStaticAn_report_unexpected_mod);
}
PyObject *PyStaticAn_visit_mod_dfs_Module(PyStaticAn_Visitor *self, mod_ty mod)
{
	return accept_stmt_seq(self, mod->v.Module.body);
}
PyObject *PyStaticAn_visit_mod_dfs_Interactive(PyStaticAn_Visitor *self, mod_ty mod)
{
	return accept_stmt_seq(self, mod->v.Interactive.body);
}
PyObject *PyStaticAn_visit_mod_dfs_Expression(PyStaticAn_Visitor *self, mod_ty mod)
{
	return accept_expr(self, mod->v.Expression.body);
}
PyObject *PyStaticAn_visit_mod_dfs_Suite(PyStaticAn_Visitor *self, mod_ty mod)
{
	return PyStaticAn_report_unexpected_mod(self, mod);
}

/* DFS STMT */
PyObject *PyStaticAn_visit_stmt_dfs(PyStaticAn_Visitor *self, stmt_ty stmt)
{
	return dispatch_visit_stmt(PyStaticAn_stmt_dfs_tab, self, stmt, PyStaticAn_report_unexpected_stmt);
}

static PyObject *visit_argannotations(PyStaticAn_Visitor *self, asdl_seq *args)
{
	int i, len;
	PyObject *res;
	PyStaticAn_join_visit_results_t join;

	res = NULL;
	join = self->vt->join;
	len = asdl_seq_LEN(args);
	for (i = 0; i < len; i++) {
		arg_ty arg = (arg_ty)asdl_seq_GET(args, i);
		if (arg->annotation) {
			res = join(self, res, accept_expr(self, arg->annotation));
		}
	}
	return res;
}
static void visit_params(PyStaticAn_Visitor *self, asdl_seq *args)
{
	int i, len;

	if (self->vt->visit_def) {
		len = asdl_seq_LEN(args);
		for (i = 0; i < len; i++) {
			arg_ty arg = (arg_ty)asdl_seq_GET(args, i);
			accept_def(self, arg->arg, DEF_PARAM);
		}
	}
}

static void visit_arguments(PyStaticAn_Visitor *self, arguments_ty a)
{
	if (a->args) {
		visit_params(self, a->args);
	}
	if (a->kwonlyargs) {
		visit_params(self, a->kwonlyargs);
	}
	if (a->vararg) {
		accept_def(self, a->vararg->arg, DEF_PARAM);
	}
	if (a->kwarg) {
		accept_def(self, a->kwarg->arg, DEF_PARAM);
	}
}

PyObject *PyStaticAn_visit_stmt_dfs_FunctionDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject *res;
	PyStaticAn_join_visit_results_t join;
	arguments_ty a;
	expr_ty returns;

	join = self->vt->join;
	res = NULL;

	accept_def(self, s->v.FunctionDef.name, DEF_LOCAL);
	
	a = s->v.FunctionDef.args;
	returns = s->v.FunctionDef.returns;
	{
		PyObject *ress[] = {
			accept_expr_seq(self, a->defaults),
			accept_expr_seq(self, a->kw_defaults),
			(a->args)? visit_argannotations(self, a->args) : NULL,
			(a->vararg && a->vararg->annotation)? accept_expr(self, a->vararg->annotation) : NULL,
			(a->kwarg && a->kwarg->annotation)? accept_expr(self, a->kwarg->annotation) : NULL,
			(a->kwonlyargs)? visit_argannotations(self, a->kwonlyargs) : NULL,
			(returns)? accept_expr(self, returns) : NULL
		};
		res = join(self, res, JOINARR(self, ress));
	}

	if (s->v.FunctionDef.decorator_list) {
		res = join(self, res, accept_expr_seq(self, s->v.FunctionDef.decorator_list));
	}
	self->vt->enter_block(self, s->v.FunctionDef.name, FunctionBlock);

	visit_arguments(self, a);
	res = join(self, res, accept_stmt_seq(self, s->v.FunctionDef.body));

	self->vt->leave_block(self);
	return res;
}
PyObject *PyStaticAn_visit_stmt_dfs_AsyncFunctionDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	/* TODO this is basically a duplicate of FunctionDef */
	PyObject *res;
	PyStaticAn_join_visit_results_t join;
	arguments_ty a;
	expr_ty returns;

	join = self->vt->join;
	res = NULL;

	accept_def(self, s->v.AsyncFunctionDef.name, DEF_LOCAL);
	
	if (s->v.AsyncFunctionDef.args->defaults) {
		res = join(self, res, accept_expr_seq(self, s->v.AsyncFunctionDef.args->defaults));
	}
	if (s->v.AsyncFunctionDef.args->kw_defaults) {
		res = join(self, res, accept_expr_seq(self, s->v.AsyncFunctionDef.args->kw_defaults));
	}

	a = s->v.AsyncFunctionDef.args;
	if (a->args) {
		res = join(self, res, visit_argannotations(self, a->args));
	}
	if (a->vararg && a->vararg->annotation) {
		res = join(self, res, accept_expr(self, a->vararg->annotation));
	}
	if (a->kwarg && a->kwarg->annotation) {
		res = join(self, res, accept_expr(self, a->kwarg->annotation));
	}
	if (a->kwonlyargs) {
		res = join(self, res, visit_argannotations(self, a->kwonlyargs));
	}
	returns = s->v.AsyncFunctionDef.returns;
	if (returns) {
		res = join(self, res, accept_expr(self, returns));
	}

	if (s->v.AsyncFunctionDef.decorator_list) {
		res = join(self, res, accept_expr_seq(self, s->v.AsyncFunctionDef.decorator_list));
	}
	self->vt->enter_block(self, s->v.AsyncFunctionDef.name, FunctionBlock);

	visit_arguments(self, a);
	res = join(self, res, accept_stmt_seq(self, s->v.AsyncFunctionDef.body));

	self->vt->leave_block(self);
	return res;
}
PyObject *PyStaticAn_visit_stmt_dfs_ClassDef(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;

	join = self->vt->join;
	accept_def(self, s->v.ClassDef.name, DEF_LOCAL);
	res = accept_expr_seq(self, s->v.ClassDef.bases);
	res = join(self, res, accept_keyword_seq(self, s->v.ClassDef.keywords));
	res = join(self, res, accept_expr_seq(self, s->v.ClassDef.decorator_list));
	self->vt->enter_block(self, s->v.ClassDef.name, ClassBlock);
	res = join(self, res, accept_stmt_seq(self, s->v.ClassDef.body));
	self->vt->leave_block(self);
	return res;
}
PyObject *PyStaticAn_visit_stmt_dfs_Return(PyStaticAn_Visitor *self, stmt_ty s)
{
	if (s->v.Return.value) {
		return accept_expr(self, s->v.Return.value);
	}
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_Delete(PyStaticAn_Visitor *self, stmt_ty s)
{
	return accept_expr_seq(self, s->v.Delete.targets);
}
PyObject *PyStaticAn_visit_stmt_dfs_Assign(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject *a, *b;
	a = accept_expr_seq(self, s->v.Assign.targets);
	b = accept_expr(self, s->v.Assign.value);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_stmt_dfs_AugAssign(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject	*a = accept_expr(self, s->v.AugAssign.target),
				*b = accept_expr(self, s->v.AugAssign.value);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_stmt_dfs_AnnAssign(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject	*a = accept_expr(self, s->v.AnnAssign.target),
				*b = accept_expr(self, s->v.AnnAssign.annotation),
				*c = accept_expr(self, s->v.AnnAssign.value);
	PyObject *xs[] = { a, b, c };
	return JOINARR(self, xs);
}
PyObject *PyStaticAn_visit_stmt_dfs_For(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject	*a = accept_expr(self, s->v.For.target),
				*b = accept_expr(self, s->v.For.iter);
	self->vt->enter_subblock(self);
	PyObject	*c = accept_stmt_seq(self, s->v.For.body);
	self->vt->leave_subblock(self);
	self->vt->enter_subblock(self);
	PyObject	*d = accept_stmt_seq(self, s->v.For.orelse);
	self->vt->leave_subblock(self);
	PyObject *xs[] = { a, b, c, d };
	return JOINARR(self, xs);
}
PyObject *PyStaticAn_visit_stmt_dfs_AsyncFor(PyStaticAn_Visitor *self, stmt_ty s)
{
	/* TODO DRY */
	PyObject	*a = accept_expr(self, s->v.For.target),
				*b = accept_expr(self, s->v.For.iter);
	self->vt->enter_subblock(self);
	PyObject	*c = accept_stmt_seq(self, s->v.For.body);
	self->vt->leave_subblock(self);
	self->vt->enter_subblock(self);
	PyObject	*d = accept_stmt_seq(self, s->v.For.orelse);
	self->vt->leave_subblock(self);
	PyObject *xs[] = { a, b, c, d };
	return JOINARR(self, xs);
}
PyObject *PyStaticAn_visit_stmt_dfs_While(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject	*a = accept_expr(self, s->v.While.test);
	self->vt->enter_subblock(self);
	PyObject	*b = accept_stmt_seq(self, s->v.While.body);
	self->vt->leave_subblock(self);
	self->vt->enter_subblock(self);
	PyObject	*c = accept_stmt_seq(self, s->v.While.orelse);
	self->vt->leave_subblock(self);
	PyObject *xs[] = { a, b, c };
	return JOINARR(self, xs);
}
PyObject *PyStaticAn_visit_stmt_dfs_If(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject	*a = accept_expr(self, s->v.If.test);
	self->vt->enter_subblock(self);
	PyObject	*b = accept_stmt_seq(self, s->v.If.body);
	self->vt->leave_subblock(self);
	self->vt->enter_subblock(self);
	PyObject	*c = accept_stmt_seq(self, s->v.If.orelse);
	self->vt->leave_subblock(self);
	PyObject *xs[] = { a, b, c };
	return JOINARR(self, xs);
}
static PyObject *visit_withitem(PyStaticAn_Visitor *self, withitem_ty item)
{
	PyObject	*a = accept_expr(self, item->context_expr),
				*b = item->optional_vars? accept_expr(self, item->optional_vars) : NULL;
	return self->vt->join(self, a, b);
}
static PyObject *accept_withitem_seq(PyStaticAn_Visitor *self, asdl_seq *seq)
{
	int i, len;
	PyObject *res = NULL;
	PyStaticAn_join_visit_results_t join;

	if (self->abort) return NULL;
	if (seq) {
		join = self->vt->join;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			withitem_ty elt = (withitem_ty)asdl_seq_GET(seq, i);
			if (!elt) continue;
			res = join(self, res, visit_withitem(self, elt));
		}
	}
	return res;
}
PyObject *PyStaticAn_visit_stmt_dfs_With(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject	*a = accept_withitem_seq(self, s->v.With.items);
	self->vt->enter_subblock(self);
	PyObject	*b = accept_stmt_seq(self, s->v.With.body);
	self->vt->leave_subblock(self);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_stmt_dfs_AsyncWith(PyStaticAn_Visitor *self, stmt_ty s)
{
	/* TODO DRY */
	PyObject	*a = accept_withitem_seq(self, s->v.AsyncWith.items);
	self->vt->enter_subblock(self);
	PyObject	*b = accept_stmt_seq(self, s->v.AsyncWith.body);
	self->vt->leave_subblock(self);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_stmt_dfs_Raise(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject *a = NULL, *b = NULL;
	if (s->v.Raise.exc) {
		a = accept_expr(self, s->v.Raise.exc);
		if (s->v.Raise.cause) {
			b = accept_expr(self, s->v.Raise.cause);
		}
	}
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_stmt_dfs_Try(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject *a, *b, *c = NULL, *d;

	self->vt->enter_subblock(self);
	a = accept_stmt_seq(self, s->v.Try.body);
	self->vt->leave_subblock(self);
	self->vt->enter_subblock(self);
	b = accept_stmt_seq(self, s->v.Try.orelse);
	self->vt->leave_subblock(self);

	if (s->v.Try.handlers) {
		unsigned i, len;
		PyStaticAn_join_visit_results_t join;
		asdl_seq *seq;

		seq = s->v.Try.handlers;
		join = self->vt->join;
		len = asdl_seq_LEN(seq);
		for (i = 0; i < len; i++) {
			excepthandler_ty eh = (excepthandler_ty)asdl_seq_GET(seq, i);
			if (eh) {
				PyObject *x = NULL, *y;
				self->vt->enter_subblock(self);
				if (eh->v.ExceptHandler.type) {
					x = accept_expr(self, eh->v.ExceptHandler.type);
				}
				if (eh->v.ExceptHandler.name) {
					accept_def(self, eh->v.ExceptHandler.name, DEF_LOCAL);
				}
				self->vt->enter_subblock(self);
				y = accept_stmt_seq(self, eh->v.ExceptHandler.body);
				self->vt->leave_subblock(self);
				c = join(self, c, x);
				c = join(self, c, y);
				self->vt->leave_subblock(self);
			}
		}
	}

	self->vt->enter_subblock(self);
	d = accept_stmt_seq(self, s->v.Try.finalbody);
	self->vt->leave_subblock(self);

	{
		PyObject *xs[] = { a, b, c, d };
		return JOINARR(self, xs);
	}
}
PyObject *PyStaticAn_visit_stmt_dfs_Assert(PyStaticAn_Visitor *self, stmt_ty s)
{
	PyObject *a, *b = NULL;

	a = accept_expr(self, s->v.Assert.test);
	if (s->v.Assert.msg) {
		b = accept_expr(self, s->v.Assert.msg);
	}
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_stmt_dfs_Import(PyStaticAn_Visitor *self, stmt_ty s)
{
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_ImportFrom(PyStaticAn_Visitor *self, stmt_ty s)
{
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_Global(PyStaticAn_Visitor *self, stmt_ty s)
{
	int i, len;
	asdl_seq *seq = s->v.Global.names;
	len = asdl_seq_LEN(seq);
	for (i = 0; i < len; i++) {
		identifier name = (identifier)asdl_seq_GET(seq, i);
		accept_def(self, name, DEF_GLOBAL);
	}
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_Nonlocal(PyStaticAn_Visitor *self, stmt_ty s)
{
	int i, len;
	asdl_seq *seq = s->v.Nonlocal.names;
	len = asdl_seq_LEN(seq);
	for (i = 0; i < len; i++) {
		identifier name = (identifier)asdl_seq_GET(seq, i);
		accept_def(self, name, DEF_NONLOCAL);
	}
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_Expr(PyStaticAn_Visitor *self, stmt_ty s)
{
	return accept_expr(self, s->v.Expr.value);
}
PyObject *PyStaticAn_visit_stmt_dfs_Pass(PyStaticAn_Visitor *self, stmt_ty stmt)
{
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_Break(PyStaticAn_Visitor *self, stmt_ty stmt)
{
	return NULL;
}
PyObject *PyStaticAn_visit_stmt_dfs_Continue(PyStaticAn_Visitor *self, stmt_ty stmt)
{
	return NULL;
}

/* DFS EXPR */
PyObject *PyStaticAn_visit_expr_dfs(PyStaticAn_Visitor *self, expr_ty ex)
{
	return dispatch_visit_expr(PyStaticAn_expr_dfs_tab, self, ex, PyStaticAn_report_unexpected_expr);
}
PyObject *PyStaticAn_visit_expr_dfs_BoolOp(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr_seq(self, e->v.BoolOp.values);
}
PyObject *PyStaticAn_visit_expr_dfs_BinOp(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b;
	a = accept_expr(self, e->v.BinOp.left);
	b = accept_expr(self, e->v.BinOp.right);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_expr_dfs_UnaryOp(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr(self, e->v.UnaryOp.operand);
}

static identifier get_lazy_id(identifier *var, const char *cstr)
{
	if (*var == NULL) {
		*var = PyUnicode_InternFromString(cstr);
		if (!*var) {
			stralloc_panic();
		}
	}
	return *var;
}
#define DEF_LAZY_STATIC_ID(str) \
static identifier s_str_lazy_##str = NULL; \
static identifier get_##str##_id(void) { return get_lazy_id(&s_str_lazy_##str, #str); }
DEF_LAZY_STATIC_ID(lambda)

PyObject *PyStaticAn_visit_expr_dfs_Lambda(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b, *c;
	
	a = accept_expr_seq(self, e->v.Lambda.args->defaults);
	b = accept_expr_seq(self, e->v.Lambda.args->kw_defaults);
	self->vt->enter_block(self, get_lambda_id(), FunctionBlock);
	visit_arguments(self, e->v.Lambda.args);
	c = accept_expr(self, e->v.Lambda.body);
	self->vt->leave_block(self);
	{
		PyObject *xs[] = { a, b, c };
		return JOINARR(self, xs);
	}
}
PyObject *PyStaticAn_visit_expr_dfs_IfExp(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b, *c;

	a = accept_expr(self, e->v.IfExp.test);
	b = accept_expr(self, e->v.IfExp.body);
	c = accept_expr(self, e->v.IfExp.orelse);
	{
		PyObject *xs[] = { a, b, c };
		return JOINARR(self, xs);
	}
}
PyObject *PyStaticAn_visit_expr_dfs_Dict(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b;
	a = accept_expr_seq(self, e->v.Dict.keys);
	b = accept_expr_seq(self, e->v.Dict.values);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_expr_dfs_Set(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr_seq(self, e->v.Set.elts);
}
static void accept_implicit_arg(PyStaticAn_Visitor *self, int pos)
{
	PyObject *id = PyUnicode_FromFormat(".%d", pos);
	if (!id) {
		stralloc_panic();
	}
	accept_def(self, id, DEF_PARAM);
	Py_DECREF(id); /* don't leak. */
}
static PyObject *visit_comprehension(
	PyStaticAn_Visitor *self,
	expr_ty e,
	identifier scope_name,
	asdl_seq *generators,
	expr_ty elt,
	expr_ty value)
{
	PyObject *resa, *resb, *resc, *resd, *rese, *resf;
	comprehension_ty outermost = ((comprehension_ty)
									asdl_seq_GET(generators, 0));
	/* Outermost iterator is evaluated in current scope */
	resa = accept_expr(self, outermost->iter);
	/* Create comprehension scope for the rest */
	self->vt->enter_block(self, scope_name, FunctionBlock);
	/* Outermost iter is received as an argument ? */
	accept_implicit_arg(self, 0);
	resb = accept_expr(self, outermost->target);
	resc = accept_expr_seq(self, outermost->ifs);
	
	resd = NULL;
	{
		unsigned i, len;
		PyStaticAn_join_visit_results_t join;
		
		join = self->vt->join;
		len = asdl_seq_LEN(generators);
		for (i = 1; i < len; i++) {
			comprehension_ty elt = (comprehension_ty)asdl_seq_GET(generators, i);
			if (elt) {
				PyObject *x, *y, *z;
				x = accept_expr(self, elt->target);
				y = accept_expr(self, elt->iter);
				z = accept_expr_seq(self, elt->ifs);
				{
					PyObject *xs[] = { x, y, z };
					resd = join(self, resd, JOINARR(self, xs));
				}
			}
		}
	}

	rese = NULL;
	if (value) {
		rese = accept_expr(self, value);
	}
	resf = accept_expr(self, elt);
	self->vt->leave_block(self);

	{
		PyObject *ress[] = { resa, resb, resc, resd, rese, resf };
		return JOINARR(self, ress);
	}
}
DEF_LAZY_STATIC_ID(listcomp);
DEF_LAZY_STATIC_ID(setcomp);
DEF_LAZY_STATIC_ID(dictcomp);
DEF_LAZY_STATIC_ID(genexpr);

PyObject *PyStaticAn_visit_expr_dfs_ListComp(PyStaticAn_Visitor *self, expr_ty e)
{
	return visit_comprehension(self, e, get_listcomp_id(), e->v.ListComp.generators, e->v.ListComp.elt, NULL);
}

PyObject *PyStaticAn_visit_expr_dfs_SetComp(PyStaticAn_Visitor *self, expr_ty e)
{
	return visit_comprehension(self, e, get_setcomp_id(), e->v.SetComp.generators, e->v.SetComp.elt, NULL);
}
PyObject *PyStaticAn_visit_expr_dfs_DictComp(PyStaticAn_Visitor *self, expr_ty e)
{
	return visit_comprehension(
		self,
		e,
		get_dictcomp_id(),
		e->v.DictComp.generators,
		e->v.DictComp.key,
		e->v.DictComp.value);
}
PyObject *PyStaticAn_visit_expr_dfs_GeneratorExp(PyStaticAn_Visitor *self, expr_ty e)
{
	return visit_comprehension(self, e, get_genexpr_id(),
										 e->v.GeneratorExp.generators,
										 e->v.GeneratorExp.elt, NULL);
}
PyObject *PyStaticAn_visit_expr_dfs_Await(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr(self, e->v.Await.value);
}
PyObject *PyStaticAn_visit_expr_dfs_Yield(PyStaticAn_Visitor *self, expr_ty e)
{
	if (e->v.Yield.value) {
		return accept_expr(self, e->v.Yield.value);
	}
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_YieldFrom(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr(self, e->v.YieldFrom.value);
}
PyObject *PyStaticAn_visit_expr_dfs_Compare(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b;

	a = accept_expr(self, e->v.Compare.left);
	b = accept_expr_seq(self, e->v.Compare.comparators);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_expr_dfs_Call(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b, *c;

	a = accept_expr(self, e->v.Call.func);
	b = accept_expr_seq(self, e->v.Call.args);
	c = accept_keyword_seq(self, e->v.Call.keywords);
	{
		PyObject *xs[] = { a, b, c };
		return JOINARR(self, xs);
	}
}
PyObject *PyStaticAn_visit_expr_dfs_Num(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_Str(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_FormattedValue(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b = NULL;
	a = accept_expr(self, e->v.FormattedValue.value);
	if (e->v.FormattedValue.format_spec) {
		b = accept_expr(self, e->v.FormattedValue.format_spec);
	}
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_expr_dfs_JoinedStr(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr_seq(self, e->v.JoinedStr.values);
}
PyObject *PyStaticAn_visit_expr_dfs_Bytes(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_NameConstant(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_Ellipsis(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_Constant(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_Attribute(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr(self, e->v.Attribute.value);
}
PyObject *PyStaticAn_visit_expr_dfs_Subscript(PyStaticAn_Visitor *self, expr_ty e)
{
	PyObject *a, *b;
	a = accept_expr(self, e->v.Subscript.value);
	b = visit_slice(self, e->v.Subscript.slice);
	return self->vt->join(self, a, b);
}
PyObject *PyStaticAn_visit_expr_dfs_Starred(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr(self, e->v.Starred.value);
}
PyObject *PyStaticAn_visit_expr_dfs_Name(PyStaticAn_Visitor *self, expr_ty e)
{
	return NULL;
}
PyObject *PyStaticAn_visit_expr_dfs_List(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr_seq(self, e->v.List.elts);
}
PyObject *PyStaticAn_visit_expr_dfs_Tuple(PyStaticAn_Visitor *self, expr_ty e)
{
	return accept_expr_seq(self, e->v.Tuple.elts);
}

PyObject *PyStaticAn_report_unexpected_mod(PyStaticAn_Visitor *self, mod_ty mod)
{
	PyErr_Format(PyExc_SyntaxError,
		"Internal error: Visitor '%s' encountered unexpected mod (integer value of kind: %d)",
		self->vt->class_name,
		(unsigned)(mod->kind)
	);
	PyErr_SyntaxLocationObject(self->filename, 0, 0);
	self->abort = 1;
	return NULL;
}
PyObject *PyStaticAn_report_unexpected_stmt(PyStaticAn_Visitor *self, stmt_ty stmt)
{
	PyErr_Format(PyExc_SyntaxError,
		"Internal error: Visitor '%s' encountered unexpected statement (integer value of kind: %d)",
		self->vt->class_name,
		(unsigned)(stmt->kind)
	);
	PyErr_SyntaxLocationObject(
		self->filename,
		stmt->lineno,
		stmt->col_offset);
	self->abort = 1;
	return NULL;
}
PyObject *PyStaticAn_report_unexpected_expr(PyStaticAn_Visitor *self, expr_ty expr)
{
	PyErr_Format(PyExc_SyntaxError,
		"Internal error: Visitor '%s' encountered unexpected expression (integer value of kind: %d)",
		self->vt->class_name,
		(unsigned)(expr->kind)
	);
	PyErr_SyntaxLocationObject(
		self->filename,
		expr->lineno,
		expr->col_offset);
	self->abort = 1;
	return NULL;
}

void PyStaticAn_Visitor_destroy(PyObject *self_)
{
	PyStaticAn_Visitor *self = (PyStaticAn_Visitor *)self_;
	if (self->filename) {
		Py_DECREF(self->filename);
		self->filename = NULL;
	}
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


#define MAX_FACTORIES 32
static unsigned s_next_factory_idx = 0;
static PyStaticAn_factory_t s_factories[MAX_FACTORIES];
int PyStaticAn_RegisterFactory(PyStaticAn_factory_t factory)
{
	unsigned idx;

	if (s_next_factory_idx == MAX_FACTORIES) {
		return -1;
	}
	idx = s_next_factory_idx++;
	s_factories[idx] = factory;
	return idx;
}

void PyStaticAn_Analyze(mod_ty mod, PyObject *filename)
{
	unsigned i;
	for (i = 0; i < s_next_factory_idx; i++) {
		PyObject *visitor_ = s_factories[i](i);
		PyStaticAn_Visitor *visitor = (PyStaticAn_Visitor *)visitor_;
		Py_INCREF(filename);
		visitor->filename = filename;
		(void) accept_mod(visitor, mod);
		Py_DECREF(visitor);
	}
}

PyObject *PyStaticAn_join_null(PyStaticAn_Visitor *self, PyObject *a, PyObject *b)
{
	return NULL;
}

