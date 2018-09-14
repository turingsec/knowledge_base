#include "include\Python_dynload.h"
#include "startPy.h"
#include "include\debug.h"
#include "include\hashlib.h"
#include "include\memimporter.h"
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <Psapi.h>

#define PyPARSE_IGNORE_COOKIE 0x0010

typedef enum enumSYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
}SYSTEM_INFORMATION_CLASS;

typedef struct tagPROCESS_BASIC_INFORMATION
{
	DWORD ExitStatus;
	DWORD PebBaseAddress;
	DWORD AffinityMask;
	DWORD BasePriority;
	ULONG UniqueProcessId;
	ULONG InheritedFromUniqueProcessId;
}PROCESS_BASIC_INFORMATION;

typedef LONG(WINAPI *PNTQUERYINFORMATIONPROCESS)(HANDLE, UINT, PVOID, ULONG, PULONG);
PNTQUERYINFORMATIONPROCESS  NtQueryInformationProcess = NULL;

DWORD GetParentProcessID()
{
	LONG                      status;
	DWORD                     dwParentPID = 0;
	HANDLE                    hProcess;
	PROCESS_BASIC_INFORMATION pbi;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	if (!hProcess)
		return -1;
	NtQueryInformationProcess = (PNTQUERYINFORMATIONPROCESS)GetProcAddress(LoadLibraryA("ntdll.dll"), "NtQueryInformationProcess");
	if (NtQueryInformationProcess == NULL) {
		dprint(LOADERLOG"Can't get NtQueryInfomationProcess pointer,%d", GetLastError());
		return -1;
	}
	status = NtQueryInformationProcess(hProcess, SystemBasicInformation, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (!status)
		dwParentPID = pbi.InheritedFromUniqueProcessId;

	CloseHandle(hProcess);
	return dwParentPID;
}

static HRESULT NormalizeNTPath(wchar_t* pszPath, size_t nMax)
// Normalizes the path returned by GetProcessImageFileName
{
	wchar_t* pszSlash = wcschr(&pszPath[1], L'\\');
	if (pszSlash) pszSlash = wcschr(pszSlash + 1, L'\\');
	if (!pszSlash)
		return E_FAIL;
	wchar_t cSave = *pszSlash;
	*pszSlash = 0;

	wchar_t szNTPath[_MAX_PATH];
	wchar_t szDrive[_MAX_PATH] = L"A:";
	// We'll need to query the NT device names for the drives to find a match with pszPath
	for (wchar_t cDrive = L'A'; cDrive < L'Z'; ++cDrive)
	{
		szDrive[0] = cDrive;
		szNTPath[0] = 0;
		if (0 != QueryDosDeviceW(szDrive, szNTPath, _MAX_PATH) &&
			0 == _wcsicmp(szNTPath, pszPath))
		{
			// Match
			wcscat_s(szDrive, _MAX_PATH, L"\\");
			wcscat_s(szDrive, _MAX_PATH, pszSlash + 1);
			wcscpy_s(pszPath, nMax, szDrive);
			return S_OK;
		}
	}
	*pszSlash = cSave;
	return E_FAIL;
}

int GetParrentModulePath(wchar_t *parentPath) {
	HANDLE hProcess;
	DWORD pid = GetParentProcessID();
	if (pid < 1) {
		dprint(LOADERLOG"Get parent pid fail");
		return -1;
	}

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hProcess) {
		dprint(LOADERLOG"open parent process fail");
		return -1;
	}

	if (!GetProcessImageFileNameW(hProcess, parentPath, MAX_PATH)) {
		dprint(LOADERLOG"Get parent process path fail,%d", GetLastError());
		CloseHandle(hProcess);
		return -1;
	}
	if (NormalizeNTPath(parentPath, MAX_PATH)) {
		CloseHandle(hProcess);
		return -1;
	}
	CloseHandle(hProcess);
	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	wchar_t parentPath[MAX_PATH] = { 0 };
	wchar_t baselibPath[MAX_PATH] = { 0 };
	wchar_t exePath[MAX_PATH] = { 0 };
	wchar_t libpath[MAX_PATH] = { 0 };
	wchar_t **args = NULL;

	HANDLE mutex = NULL;

	PyCompilerFlags flags;
	PyGILState_STATE restore_state;

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	DEBUG_EVENT dbg_event = { 0 };

	if (InitCurrentPath() < 0) {
		return -1;
	}

	if (!GetModuleFileNameW(NULL, exePath, MAX_PATH)) {
		dprint(LOADERLOG"can't get module name");
		ret = -1;
		goto EXIT;
	}

	if (GetParrentModulePath(parentPath)) {
		dprint(LOADERLOG"can't get parent path!");
		//ret = -1;
		//goto EXIT;
	}

#ifdef DEBUG_LOADER
	if (0) {
#else
	if (argc == 1) {
#endif
		dprint(LOADERLOG"Start debugger");
		if (!_wcsicmp(exePath, parentPath)) {// I shouldn't start by myself without cmd;
			dprint(LOADERLOG"I shouldn't start by myself without cmd");
			ret = -1;
			goto EXIT;
		}
		wcscat_s(exePath, MAX_PATH, L" worknow");

		do {
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));

			si.cb = sizeof(si);
			if (CreateProcessW(NULL, exePath, NULL, NULL, FALSE,
				DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,
				NULL, NULL, &si, &pi) == FALSE) {

				dprint(LOADERLOG"CreateProcess Failed! 0x%llx", GetLastError());
				ret = -1;
				goto EXIT;
			}

			ret = 0;
			while (1) {
				if (!WaitForDebugEvent(&dbg_event, INFINITE))
					goto DBGEXIT;
				switch (dbg_event.dwDebugEventCode) {
				case OUTPUT_DEBUG_STRING_EVENT:
					dprint(LOADERLOG"Debug String:%s", dbg_event.u.DebugString.lpDebugStringData);
					if (!strcmp(dbg_event.u.DebugString.lpDebugStringData, "restart")) {
						ret = 1;
						goto DBGEXIT;
					}
					break;
				case EXIT_PROCESS_DEBUG_EVENT:
					ret = 0;
					dprint(LOADERLOG"exitcode :%d", dbg_event.u.ExitProcess.dwExitCode);
					goto DBGEXIT;
				default:
					break;
				}
				ContinueDebugEvent(dbg_event.dwProcessId, dbg_event.dwThreadId, DBG_CONTINUE);
			}
		DBGEXIT:
			DebugActiveProcessStop(pi.dwProcessId);
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		} while (ret);
		dprint(LOADERLOG"exit debug!");
	}
	else {
		dprint(LOADERLOG"Start child!");
		dprint(LOADERLOG"parent path :%ls", parentPath);

#ifdef DEBUG_LOADER
		if (0) {
#else
		if (_wcsicmp(exePath, parentPath)) {// I must start by myself with this cmd.
#endif
			dprint(LOADERLOG"I must start by myself with worknow.");
			//ret = -1;
			//goto EXIT;
		}
		if (argc == 1) {// if no args,create mutex
			mutex = CreateMutexW(NULL, FALSE, L"marsnakeLoader");
			if (!mutex) {
				dprint(LOADERLOG"create mutex fail!");
				ret = -1;
				goto EXIT;
			}
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				ret = -1;
				goto EXIT;
			}
		}

		lstrcpynW(libpath, Current_Path, MAX_PATH);
		ret = _load_python_FromFile(lstrcatW(libpath, L"python36.dll"));
		if (!ret) {
			dprint(LOADERLOG"loading python36.dll from memory failed");
			ret = -1;
			goto EXIT;
		}

		Py_SetProgramName(exePath);// this API doesn't save string,don't change path string in the following code.
		lstrcpynW(baselibPath, Current_Path, MAX_PATH);
		wcscat_s(baselibPath, MAX_PATH, L"baselib.zip");

#ifdef NEED_CHECKHASH
		if (CheckSHA1Hash(baselibPath) < 0) {
			ret = -1;
			MessageBoxA(NULL, "项目文件已损坏,请重新安装!", "严重错误", MB_OK);
			goto EXIT;
		}
#endif
		PyImport_AppendInittab("_memimporter", PyInit__memimporter);
		Py_IgnoreEnvironmentFlag = 1;
		Py_NoSiteFlag = 1;
		Py_SetPath(baselibPath);
		Py_InitializeEx(0);
		if (!Py_IsInitialized()) {
			dprint(LOADERLOG"Py_Initialize() fail!");
			ret = -1;
			goto EXIT;
		}
		restore_state = PyGILState_Ensure();

		flags.cf_flags = PyPARSE_IGNORE_COOKIE;
		args = malloc((argc + 1) * sizeof(wchar_t *));
		memset(args, 0, (argc + 1) * sizeof(wchar_t *));
		for (int i = 0; i < argc; i++) {
			args[i] = Py_DecodeLocale(argv[i], NULL);// we need to free this by PyMem_RawFree().
		}
		PySys_SetArgvEx(argc, args, 1);


#ifdef DEBUG_FROMFILE
		FILE *fp;
		if (fopen_s(&fp, "..\\initialization_tiny.py", "rb")) {
			dprint(LOADERLOG "Can't Get initialization!");
			ret = -1;
			goto EXIT;
		}
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		char *data = malloc(size + 10);
		memset(data, 0, size + 10);
		fseek(fp, 0, SEEK_SET);
		fread(data, size, 1, fp);
		fclose(fp);

		ret = PyRun_SimpleStringFlags(data, &flags);
#else
		ret = PyRun_SimpleStringFlags(Cmd, &flags);
#endif	
		PyGILState_Release(restore_state);
		Py_Finalize();// notice,this will stuck process.
		dprint(LOADERLOG" finished python");
		}
EXIT:
	ReleaseCurrentPath();

	if (mutex) {
		CloseHandle(mutex);
	}

	if (args) {
		for (int i = 0; args[i]; i++) {
			PyMem_RawFree(args[i]);
		}
		free(args);
	}

	//if (!ret) {
	//	OutputDebugStringA("restart");
	//}

#ifdef DEBUG_PRINT
	_getch();
#endif
	return ret;
	}
//mainCRTStartup