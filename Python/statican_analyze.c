#include <Python.h>
#include <statican.h>
#include <statican_dump.h>

static void run(PyObject *visitor_, mod_ty mod, PyObject *filename)
{
	PyStaticAn_Visitor *visitor = (PyStaticAn_Visitor *)visitor_;
	Py_INCREF(filename);
	visitor->filename = filename;
	(void) PyStaticAn_Visitor_accept_mod((PyObject *)visitor, mod);
	Py_DECREF(visitor);
}

void PyStaticAn_Analyze(mod_ty mod, PyObject *filename)
{
	/* run(PyStaticAn_DumpVisitorFactory(0), mod, filename); */
}
