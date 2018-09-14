#include "include\python_dynload.h"
#include "include\debug.h"
#include <windows.h>
#include <stdio.h>
#include <Winbase.h>

#define DYNLOG "[DYNLOG] "

struct IMPORT imports[] = {
	{ "_Py_PackageContext", NULL },
	{ "Py_IsInitialized", NULL },
	{ "Py_InitializeEx", NULL },
	{ "Py_Finalize", NULL },
	{ "PyEval_InitThreads", NULL },
	{ "Py_GetPath", NULL },
	{ "Py_IgnoreEnvironmentFlag", NULL },
	{ "Py_NoSiteFlag", NULL },
	{ "PyGILState_Ensure", NULL },
	{ "PyRun_SimpleStringFlags", NULL },
	{ "PyGILState_Release", NULL },
	{ "PyObject_CallFunction", NULL },
	{ "PyUnicode_AsEncodedString", NULL },
	{ "PyArg_ParseTuple", NULL },
	{ "PyExc_ImportError", NULL },
	{ "PyErr_Occurred", NULL },
	{ "PyImport_ImportModule", NULL },
	{ "PyTuple_New", NULL },
	{ "PyTuple_SetItem", NULL },
	{ "Py_BuildValue", NULL },
	{ "PyErr_Format", NULL },
	{ "PyModule_Create2", NULL },
	{ "PyErr_Clear", NULL },
	{ "Py_SetPath", NULL },
	{ "PyImport_AppendInittab", NULL },
	{ "PyImport_AddModule", NULL },
	{ "PyBool_FromLong", NULL },
	{ "PyModule_GetDict", NULL },
	{ "PyModuleDef_Type", NULL },
	{ "PyModule_FromDefAndSpec2", NULL },
	{ "PyModule_GetDef", NULL },
	{ "PyModule_ExecDef", NULL },
	{ "PyType_IsSubtype", NULL },
	{ "PySys_SetArgvEx", NULL },
	{ "Py_DecodeLocale", NULL },
	{ "Py_SetProgramName", NULL },
	{ "PyMem_RawFree", NULL},
	{ NULL, NULL }, /* sentinel */
};


int _load_python_FromFile(wchar_t *dllname)
{
	int i;
	struct IMPORT *p = imports;
	HMODULE hmod;

	hmod = LoadLibraryW(dllname);
	if (hmod == NULL) {
		dprint(DYNLOG"Can't Load python36:%ls",dllname);
		return 0;
	}

	for (i = 0; p->name; ++i, ++p) {
		p->proc = (void(*)())GetProcAddress(hmod, p->name);
		if (p->proc == NULL) {
			dprint(DYNLOG"undefined symbol %s -> exit(-1)", p->name);
			return 0;
		}
	}

	return 1;
}