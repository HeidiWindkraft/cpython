#ifndef Py_LIMITED_API
#ifndef Py_STATICAN_H
#define Py_STATICAN_H

#include "Python.h"
#include <object.h>
#include <asdl.h>

/* TODO find an easy-to-merge way to give Python-ast.h an include guard. */
typedef struct _mod *mod_ty;
typedef struct _stmt *stmt_ty;
typedef struct _expr *expr_ty;
typedef struct _withitem *withitem_ty;
typedef struct _excepthandler *excepthandler_ty;
typedef struct _comprehension *comprehension_ty;

#include <symtable.h>


#ifdef __cplusplus
extern "C" {
#endif

void PyStaticAn_Analyze(mod_ty mod, PyObject *filename);

typedef struct PyStaticAn_visitor_s PyStaticAn_Visitor;

/* See _mod_kind. Note that zero is unused. */
#define PY_STATICAN_N_MOD_KINDS 5
#define PY_STATICAN_MOD_DEFAULT 0
typedef PyObject *(*PyStaticAn_visit_mod_t)(PyStaticAn_Visitor *self, mod_ty mod);
PyObject *PyStaticAn_visit_mod_dfs(PyStaticAn_Visitor *self, mod_ty stmt);
PyObject *PyStaticAn_visit_mod_dfs_Module(PyStaticAn_Visitor *self, mod_ty mod);
PyObject *PyStaticAn_visit_mod_dfs_Interactive(PyStaticAn_Visitor *self, mod_ty mod);
PyObject *PyStaticAn_visit_mod_dfs_Expression(PyStaticAn_Visitor *self, mod_ty mod);
PyObject *PyStaticAn_visit_mod_dfs_Suite(PyStaticAn_Visitor *self, mod_ty mod);
PyObject *PyStaticAn_report_unexpected_mod(PyStaticAn_Visitor *self, mod_ty mod);

/* See _stmt_kind. Note that zero is unused. */
#define PY_STATICAN_N_STMT_KINDS 26
#define PY_STATICAN_STMT_DEFAULT 0
typedef PyObject *(*PyStaticAn_visit_stmt_t)(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_FunctionDef(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_AsyncFunctionDef(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_ClassDef(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Return(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Delete(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Assign(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_AugAssign(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_AnnAssign(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_For(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_AsyncFor(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_While(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_If(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_With(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_AsyncWith(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Raise(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Try(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Assert(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Import(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_ImportFrom(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Global(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Nonlocal(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Expr(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Pass(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Break(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_visit_stmt_dfs_Continue(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_report_unexpected_stmt(PyStaticAn_Visitor *self, stmt_ty stmt);

/* See _expr_kind. Note that zero is unused. */
#define PY_STATICAN_N_EXPR_KINDS 31
#define PY_STATICAN_EXPR_DEFAULT 0
typedef PyObject *(*PyStaticAn_visit_expr_t)(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_BoolOp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_BinOp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_UnaryOp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Lambda(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_IfExp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Dict(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Set(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_ListComp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_SetComp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_DictComp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_GeneratorExp(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Await(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Yield(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_YieldFrom(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Compare(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Call(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Num(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Str(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_FormattedValue(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_JoinedStr(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Bytes(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_NameConstant(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Ellipsis(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Constant(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Attribute(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Subscript(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Starred(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Name(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_List(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_visit_expr_dfs_Tuple(PyStaticAn_Visitor *self, expr_ty ex);
PyObject *PyStaticAn_report_unexpected_expr(PyStaticAn_Visitor *self, expr_ty ex);


typedef PyObject *(*PyStaticAn_visit_withitem_t)(PyStaticAn_Visitor *self, withitem_ty item);
PyObject *PyStaticAn_visit_withitem_dfs(PyStaticAn_Visitor *self, withitem_ty item);

typedef PyObject *(*PyStaticAn_visit_excepthandler_t)(PyStaticAn_Visitor *self, excepthandler_ty eh);
PyObject *PyStaticAn_visit_excepthandler_dfs(PyStaticAn_Visitor *self, excepthandler_ty item);

typedef PyObject *(*PyStaticAn_visit_comprehension_generator_t)(PyStaticAn_Visitor *self, comprehension_ty cg);
PyObject *PyStaticAn_visit_comprehension_generator_dfs(PyStaticAn_Visitor *self, comprehension_ty cg);

typedef PyObject *(*PyStaticAn_join_visit_results_t)(PyStaticAn_Visitor *self, PyObject *a, PyObject *b);
typedef void (*PyStaticAn_enter_block_dfs_t)(PyStaticAn_Visitor *self, identifier name, _Py_block_ty block);
typedef void (*PyStaticAn_leave_block_dfs_t)(PyStaticAn_Visitor *self);
typedef void (*PyStaticAn_enter_subblock_dfs_t)(PyStaticAn_Visitor *self);
typedef void (*PyStaticAn_leave_subblock_dfs_t)(PyStaticAn_Visitor *self);
typedef void (*PyStaticAn_visit_def_t)(PyStaticAn_Visitor *self, PyObject *name, int flag);

typedef struct PyStaticAn_visitorvt_s {
	PyStaticAn_visit_mod_t  visit_mod [PY_STATICAN_N_MOD_KINDS ];
	PyStaticAn_visit_stmt_t visit_stmt[PY_STATICAN_N_STMT_KINDS];
	PyStaticAn_visit_expr_t visit_expr[PY_STATICAN_N_EXPR_KINDS];
	PyStaticAn_join_visit_results_t join;
	PyStaticAn_enter_block_dfs_t enter_block;
	PyStaticAn_leave_block_dfs_t leave_block;
	PyStaticAn_enter_subblock_dfs_t enter_subblock;
	PyStaticAn_leave_subblock_dfs_t leave_subblock;
	PyStaticAn_visit_def_t visit_def;
	PyStaticAn_visit_withitem_t visit_withitem;
	PyStaticAn_visit_excepthandler_t visit_excepthandler;
	PyStaticAn_visit_comprehension_generator_t visit_comprehension_generator;
	const char *class_name;
} PyStaticAn_visitorvt_t;

#define PyStaticAn_Visior_FIELDS \
	const PyStaticAn_visitorvt_t *vt; \
	int abort; \
	PyObject *filename;

struct PyStaticAn_visitor_s {
	PyObject_HEAD
	PyStaticAn_Visior_FIELDS
};

void PyStaticAn_Visitor_destroy(PyObject *);
PyObject *PyStaticAn_Visitor_repr(PyObject *);

PyAPI_DATA(PyTypeObject) PyStaticAn_Visitor_Type;

PyObject *PyStaticAn_join_null(PyStaticAn_Visitor *self, PyObject *a, PyObject *b);

PyObject *PyStaticAn_accept_mod(PyStaticAn_Visitor *self, mod_ty mod);
PyObject *PyStaticAn_accept_stmt(PyStaticAn_Visitor *self, stmt_ty stmt);
PyObject *PyStaticAn_accept_expr(PyStaticAn_Visitor *self, expr_ty expr);
typedef PyObject *(*PyStaticAn_visit_asdl_elt_t)(PyStaticAn_Visitor *self, void * elt);
PyObject *PyStaticAn_foreach_in_asdl_seq(PyStaticAn_Visitor *self, asdl_seq *seq, PyStaticAn_visit_asdl_elt_t f);


#ifdef __cplusplus
}
#endif

#endif /* _H */
#endif /* LIMITED */


