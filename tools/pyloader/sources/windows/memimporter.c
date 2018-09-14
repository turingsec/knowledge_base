#include "include\python_dynload.h"
#include "include\debug.h"
#include "include\aes.h"
#include <Shlwapi.h>

#define PYTHON_API_VERSION 1013

static PyObject *
Py_import_module(PyObject *self, PyObject *args)
//import_module(self.contents, initname, fullname, path)
{
	char *initfuncname;
	char *modname;
	char *cPathname;
	wchar_t wPathname[MAX_PATH] = { 0 };
	//HMEMORYMODULE hmem;
	HMODULE hmem;
	PyObject *(*do_init)();
	PyObject *mod;
	PyModuleDef *def;
	PyObject *spec;
	int is_moduledef;

	ULONG_PTR cookie = 0;
	char *oldcontext;

	/* code, initfuncname, fqmodulename, path */
	if (!PyArg_ParseTuple(args, "sssO:import_module",
		&initfuncname, &modname, &cPathname, &spec))
		return NULL;
	int ret = MultiByteToWideChar(CP_UTF8, 0, cPathname, strlen(cPathname), wPathname, MAX_PATH);
	if (!ret) {
		dprint(IMPORTERLOG"Can't switch encoding to wide char!Error:%d.",GetLastError());
	}
	if (!PathFileExistsW(wPathname)) {
		PyErr_Format(PyExc_ImportError,
			IMPORTERLOG"No such file %s", cPathname);
		return NULL;
	}
	hmem = LoadLibraryW(wPathname);
	if (!hmem) {
		PyErr_Format(PyExc_ImportError,
			IMPORTERLOG"MemoryLoadLibrary failed loading %ls", wPathname);
		return NULL;
	}
	do_init = (PyObject *(*)())GetProcAddress(hmem, initfuncname);
	if (!do_init) {
		FreeLibrary(hmem);
		PyErr_Format(PyExc_ImportError,
			IMPORTERLOG"Could not find function %s", initfuncname);
		return NULL;
	}

	oldcontext = _Py_PackageContext;
	_Py_PackageContext = modname;
	PyGILState_STATE gstate = PyGILState_Ensure();
	mod = do_init();
	PyGILState_Release(gstate);
	_Py_PackageContext = oldcontext;
	if (!mod) {
		if (!PyErr_Occurred())
			PyErr_Format(PyExc_ImportError,
				IMPORTERLOG"Error occurred during %s execution", initfuncname);
		return NULL;
	}
	else if (PyErr_Occurred()) {
		return NULL;
	}
	if (mod->ob_type == NULL) {
		// This can happen when a PyModuleDef is returned without calling PyModuleDef_Init on it
		PyErr_Format(PyExc_ImportError,
			IMPORTERLOG"Init function %s returned uninitialized object", initfuncname);
		return NULL;
	}

	is_moduledef = PyObject_TypeCheck(mod, PyModuleDef_Type);// notice
	if (is_moduledef) {
		mod = PyModule_FromDefAndSpec2((PyModuleDef*)mod, spec, PYTHON_API_VERSION);
	}

	def = PyModule_GetDef(mod);
	if (!def) {
		PyErr_Format(PyExc_ImportError,
			IMPORTERLOG"Init function %s did not return an extension", initfuncname);
		Py_XDECREF(mod);
		return NULL;
	}

	if (is_moduledef)
		PyModule_ExecDef(mod, def);
	else
		def->m_base.m_init = do_init;

	return mod;
}

static PyObject *
Py_messagebox(PyObject *self, PyObject *args) {
	char *msg;
	int len;
	if (!PyArg_ParseTuple(args, "s#:messagebox", &msg, &len))
		return NULL;
	MessageBoxA(NULL, msg, "´íÎó", MB_OK);
	return PyBool_FromLong(1);
}

static PyObject *
Py_aes_cbc_decrypt(PyObject *self, PyObject *args) {
	char *data;
	char *keyData;
	char *iv;
	int ivlen;
	int keylen;
	int datalen;
	if (!PyArg_ParseTuple(args, "s#s#s#:aes_cbc_decrypt", &data, &datalen, &keyData, &keylen, &iv, &ivlen))
		return NULL;

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, keyData, iv);
	AES_CBC_decrypt_buffer(&ctx, data, datalen);
	return Py_BuildValue("y#", data, datalen);
}

static PyObject *
Py_LoadLibraryW(PyObject *self, PyObject *args) {
	char *cPathname;
	wchar_t wPathname[MAX_PATH] = { 0 };
	long ret;
	if (!PyArg_ParseTuple(args, "s:LoadLibraryW", &cPathname))
		return NULL;
	ret = MultiByteToWideChar(CP_UTF8, 0, cPathname, strlen(cPathname), wPathname, MAX_PATH);
	if (!ret) {
		dprint(IMPORTERLOG"LoadLibrary can't switch encoding to wide char!Error:%d.", GetLastError());
	}
	ret = (long)LoadLibraryW(wPathname);
	return PyBool_FromLong(ret);
}

static struct PyMethodDef methods[] = {
	{ "import_module", Py_import_module, METH_VARARGS,
	"import_module(initfuncname, modname, path) -> module" },
	{ "messagebox", Py_messagebox, METH_VARARGS,
	"messagebox(msg) -> NULL" },
	{ "aes_cbc_decrypt", Py_aes_cbc_decrypt, METH_VARARGS,
	"aes_cbc_decrypt(data,key,iv) -> data(bytes)" },
	{ "LoadLibraryW", Py_LoadLibraryW, METH_VARARGS,
	"LoadLibraryW(path) -> handle" },
	{ NULL, NULL },		/* Sentinel */
};

static struct PyModuleDef moduleDef = {
	PyModuleDef_HEAD_INIT,
	"_memimporter",
	NULL,
	-1,
	methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyObject* PyInit__memimporter(void) {
	return PyModule_Create2(&moduleDef, PYTHON_API_VERSION);
}