/* **************** Python-dynload.h **************** */
#ifndef PYTHON_DYNLOAD_H
#define PYTHON_DYNLOAD_H
#include <assert.h>
#include <windows.h>
typedef int Py_ssize_t;


typedef struct {
	int cf_flags;  /* bitmask of CO_xxx flags relevant to future */
} PyCompilerFlags;

typedef struct _object {
	//struct _object *_ob_next;this only for debug dll.
	//struct _object *_ob_prev;
	Py_ssize_t ob_refcnt;
	struct _typeobject *ob_type;
} PyObject;

typedef struct {
	PyObject ob_base;
	Py_ssize_t ob_size; /* Number of items in variable part */
} PyVarObject;

#define PyObject_VAR_HEAD      PyVarObject ob_base;
typedef void(*destructor)(PyObject *);

typedef struct _typeobject {
	PyObject_VAR_HEAD
		const char *tp_name; /* For printing, in format "<module>.<name>" */
	Py_ssize_t tp_basicsize, tp_itemsize; /* For allocation */

										  /* Methods to implement standard operations */

	destructor tp_dealloc;
	void * tp_print;
	void * tp_getattr;
	void * tp_setattr;
	void *tp_reserved; /* formerly known as tp_compare */
	void * tp_repr;

	/* Method suites for standard classes */

	void *tp_as_number;
	void *tp_as_sequence;
	void *tp_as_mapping;

	/* More standard operations (here for binary compatibility) */

	void * tp_hash;
	void * tp_call;
	void * tp_str;
	void * tp_getattro;
	void * tp_setattro;

	/* Functions to access object as input/output buffer */
	void *tp_as_buffer;

	/* Flags to define presence of optional/expanded features */
	unsigned long tp_flags;

	const char *tp_doc; /* Documentation string */

						/* Assigned meaning in release 2.0 */
						/* call function for all accessible objects */
	void * tp_traverse;

	/* delete references to contained objects */
	void * tp_clear;

	/* Assigned meaning in release 2.1 */
	/* rich comparisons */
	void * tp_richcompare;

	/* weak reference enabler */
	Py_ssize_t tp_weaklistoffset;

	/* Iterators */
	void * tp_iter;
	void * tp_iternext;

	/* Attribute descriptor and subclassing stuff */
	struct PyMethodDef *tp_methods;
	struct PyMemberDef *tp_members;
	struct PyGetSetDef *tp_getset;
	struct _typeobject *tp_base;
	PyObject *tp_dict;
	void * tp_descr_get;
	void * tp_descr_set;
	Py_ssize_t tp_dictoffset;
	void * tp_init;
	void * tp_alloc;
	void * tp_new;
	void * tp_free; /* Low-level free-memory routine */
	void * tp_is_gc; /* For PyObject_IS_GC */
	PyObject *tp_bases;
	PyObject *tp_mro; /* method resolution order */
	PyObject *tp_cache;
	PyObject *tp_subclasses;
	PyObject *tp_weaklist;
	void * tp_del;

	/* Type attribute cache version tag. Added in version 2.6 */
	unsigned int tp_version_tag;

	void * tp_finalize;
} PyTypeObject;

typedef struct {
	struct _object *_ob_next;
	struct _object *_ob_prev;
	int co_argcount;		/* #arguments, except *args */
	int co_kwonlyargcount;	/* #keyword only arguments */
	int co_nlocals;		/* #local variables */
	int co_stacksize;		/* #entries needed for evaluation stack */
	int co_flags;		/* CO_..., see below */
	PyObject *co_code;		/* instruction opcodes */
	PyObject *co_consts;	/* list (constants used) */
	PyObject *co_names;		/* list of strings (names used) */
	PyObject *co_varnames;	/* tuple of strings (local variable names) */
	PyObject *co_freevars;	/* tuple of strings (free variable names) */
	PyObject *co_cellvars;      /* tuple of strings (cell variable names) */
								/* The rest doesn't count for hash or comparisons */
	unsigned char *co_cell2arg; /* Maps cell vars which are arguments. */
	PyObject *co_filename;	/* unicode (where it was loaded from) */
	PyObject *co_name;		/* unicode (name, for reference) */
	int co_firstlineno;		/* first source line number */
	PyObject *co_lnotab;	/* string (encoding addr<->lineno mapping) See
							Objects/lnotab_notes.txt for details. */
	void *co_zombieframe;     /* for optimization only (see frameobject.c) */
	PyObject *co_weakreflist;   /* to support weakrefs to code objects */
} PyCodeObject;

typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);

typedef
enum { PyGILState_LOCKED, PyGILState_UNLOCKED }
PyGILState_STATE;

struct PyMethodDef {
	char        *ml_name;
	PyCFunction  ml_meth;
	int                  ml_flags;
	char        *ml_doc;
};

#define _PyObject_EXTRA_INIT 

#define PyObject_HEAD_INIT(type)        \
    { _PyObject_EXTRA_INIT              \
    1, type },

#define PyModuleDef_HEAD_INIT { \
    PyObject_HEAD_INIT(NULL)    \
    NULL, /* m_init */          \
    0,    /* m_index */         \
    NULL, /* m_copy */          \
  }

typedef struct {
	PyObject ob_base;
	PyObject *(*m_init)();
	Py_ssize_t m_index;
	PyObject* m_copy;
} PyModuleDef_Base;

typedef struct PyModuleDef {
	PyModuleDef_Base m_base;
	const char* m_name;
	const char* m_doc;
	Py_ssize_t m_size;
	struct PyMethodDef *m_methods;
	void * m_reload;
	void * m_traverse;
	void * m_clear;
	void * m_free;
}PyModuleDef;

typedef struct {
	PyObject_VAR_HEAD
		Py_ssize_t ob_shash;
	char ob_sval[1];

	/* Invariants:
	*     ob_sval contains space for 'ob_size+1' elements.
	*     ob_sval[ob_size] == 0.
	*     ob_shash is the hash of the string or -1 if not computed yet.
	*/
} PyBytesObject;

#define Py_TYPE(ob)             (((PyObject*)(ob))->ob_type)
#define PyType_HasFeature(t,f)  (((t)->tp_flags & (f)) != 0)
#define PyType_FastSubclass(t,f)  PyType_HasFeature(t,f)
#define Py_TPFLAGS_UNICODE_SUBCLASS     (1UL << 28)
#define Py_TPFLAGS_BYTES_SUBCLASS       (1UL << 27)

#define PyUnicode_Check(op) \
                 PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_UNICODE_SUBCLASS)
#define PyBytes_Check(op) \
                 PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_BYTES_SUBCLASS)
#define PyBytes_AS_STRING(op) (assert(PyBytes_Check(op)), \
                                (((PyBytesObject *)(op))->ob_sval))
#define _Py_Dealloc(op2) (                               \
    (*Py_TYPE(op2)->tp_dealloc)((PyObject *)(op2)))

#define Py_INCREF(op) (                         \
    ((PyObject *)(op))->ob_refcnt++)

#define Py_DECREF(op)                                   \
    do {                                                \
        PyObject *_py_decref_tmp = (PyObject *)(op);    \
        if (--(_py_decref_tmp)->ob_refcnt == 0)             \
			_Py_Dealloc(_py_decref_tmp);                    \
    } while (0)

#define Py_XINCREF(op)                                \
    do {                                              \
        PyObject *_py_xincref_tmp = (PyObject *)(op); \
        if (_py_xincref_tmp != NULL)                  \
            Py_INCREF(_py_xincref_tmp);               \
    } while (0)

#define Py_XDECREF(op)                                \
    do {                                              \
        PyObject *_py_xdecref_tmp = (PyObject *)(op); \
        if (_py_xdecref_tmp != NULL)                  \
            Py_DECREF(_py_xdecref_tmp);               \
    } while (0)


#define METH_OLDARGS  0x0000
#define METH_VARARGS  0x0001
#define METH_KEYWORDS 0x0002
/* METH_NOARGS and METH_O must not be combined with the flags above. */
#define METH_NOARGS   0x0004
#define METH_O        0x0008

/* METH_CLASS and METH_STATIC are a little different; these control
the construction of methods for a class.  These cannot be used for
functions in modules. */
#define METH_CLASS    0x0010
#define METH_STATIC   0x0020


#define Py_None (&_Py_NoneStruct)
#define PyObject_TypeCheck(ob, tp) (Py_TYPE(ob) == (tp) || PyType_IsSubtype(Py_TYPE(ob), (tp)))


struct IMPORT {
	char *name;
	void(*proc)();
};
int _load_python_FromFile(wchar_t *dllname);
extern struct IMPORT imports[];
#include "import-tab.h"

#endif PYTHON_DYNLOAD_H