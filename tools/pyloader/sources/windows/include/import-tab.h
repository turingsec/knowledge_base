#define _Py_PackageContext (*(char *(*))imports[0].proc)
#define Py_IsInitialized ((int(*)(void))imports[1].proc)
#define Py_InitializeEx ((void(*)(int))imports[2].proc)
#define Py_Finalize ((void(*)(void))imports[3].proc)
#define PyEval_InitThreads ((void(*)(void))imports[4].proc)
#define Py_GetPath ((wchar_t *(*)(void))imports[5].proc)
#define Py_IgnoreEnvironmentFlag (*(int(*))imports[6].proc)
#define Py_NoSiteFlag (*(int(*))imports[7].proc)
#define PyGILState_Ensure ((PyGILState_STATE(*)(void))imports[8].proc)
#define PyRun_SimpleStringFlags ((int(*)(const char *, PyCompilerFlags *))imports[9].proc)
#define PyGILState_Release ((void(*)(PyGILState_STATE))imports[10].proc)
#define PyObject_CallFunction ((PyObject *(*)(PyObject *, const char *, ...))imports[11].proc)
#define PyUnicode_AsEncodedString ((PyObject* (*)(PyObject *,const char *,const char *))imports[12].proc)
#define PyArg_ParseTuple ((int(*)(PyObject *, char *, ...))imports[13].proc)
#define PyExc_ImportError (*(PyObject *(*))imports[14].proc)
#define PyErr_Occurred ((PyObject *(*)(void))imports[15].proc)
#define PyImport_ImportModule ((PyObject *(*)(char *))imports[16].proc)
#define PyTuple_New ((PyObject *(*)(Py_ssize_t))imports[17].proc)
#define PyTuple_SetItem ((int(*)(PyObject*, Py_ssize_t, PyObject *))imports[18].proc)
#define Py_BuildValue ((PyObject *(*)(char *, ...))imports[19].proc)
#define PyErr_Format ((PyObject *(*)(PyObject *, const char *, ...))imports[20].proc)
#define PyModule_Create2 ((PyObject *(*)(struct PyModuleDef*, int))imports[21].proc)
#define PyErr_Clear ((void(*)(void))imports[22].proc)
#define Py_SetPath ((void(*)(const wchar_t *))imports[23].proc)
#define PyImport_AppendInittab ((int(*)(const char *name, PyObject* (*)(void)))imports[24].proc)
#define PyImport_AddModule ((PyObject* (*)(char *name))imports[25].proc)
#define PyBool_FromLong ((PyObject *(*)(long))imports[26].proc)
#define PyModule_GetDict ((PyObject *(*)(PyObject *))imports[27].proc)
#define PyModuleDef_Type ((void*)imports[28].proc)
#define PyModule_FromDefAndSpec2 ((PyObject *(*)(PyModuleDef*, PyObject*, int))imports[29].proc)
#define PyModule_GetDef ((PyModuleDef *(*)(PyObject*))imports[30].proc)
#define PyModule_ExecDef ((int(*)(PyObject *, PyModuleDef *))imports[31].proc)
#define PyType_IsSubtype ((int(*)(void*, void*))imports[32].proc)
#define PySys_SetArgvEx ((void(*)(int, wchar_t**, int))imports[33].proc)
#define Py_DecodeLocale ((wchar_t*(*)(char *, size_t *))imports[34].proc)
#define Py_SetProgramName ((void (*)(wchar_t *name))imports[35].proc)
#define PyMem_RawFree ((void (*)(void *p))imports[36].proc)