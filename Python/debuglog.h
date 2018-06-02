
#ifndef DEBUGLOG_H
#define DEBUGLOG_H 1

#ifndef USE_DEBUG
#ifdef Py_DEBUG
#define USE_DEBUG 1
#else
#define USE_DEBUG 0
#endif
#endif

#if USE_DEBUG
#define DEBUG(x) x
#include <stdio.h>
extern void _PyUnicode_Dump(PyObject *op);
#define DBGLOG_FD stdout /*stderr*/
#else
#define DEBUG(x) do { /* nothing */ } while(0)
#endif

#define DBGLOG(...) DEBUG(fprintf(DBGLOG_FD, __VA_ARGS__))



#if USE_DEBUG
#include <abstract.h>
#include <unicodeobject.h>
static void dbglog_dump_object(PyObject *ob)
{
	if (ob == NULL) {
		DBGLOG("null");
	} else {
		PyObject *str = PyObject_Str(ob);
		_PyUnicode_Dump(str);
		DBGLOG("\t\tstr: %s", PyUnicode_1BYTE_DATA(str));
		/* DBGLOG("\n\t\trepr: %s", PyUnicode_AsUTF8(PyObject_Repr(ob)));*/
	}
}
#endif

#define DBGLOGVARO(var) DEBUG( DBGLOG("\t" #var ": "); dbglog_dump_object(var); DBGLOG("\n"); )
#define DBGLOGVARD(var) DBGLOG("\t" #var ": %d\n", var)
#define DBGLOGFIELDD(pobj,field) DBGLOGVARD(pobj->field)
#define DBGLOGFIELDO(pobj,field) DBGLOGVARO(pobj->field)

#define DBGLOG_FBEG() DEBUG( DBGLOG(">>> "); DBGLOG(__FUNCTION__); DBGLOG(" ("); DBGLOG(__FILE__); DBGLOG(")\n"); )

#endif /* _H */

