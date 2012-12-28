#include <iostream>

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include <vector>

HANDLE hProcess = NULL;
DWORD dwProcessId = 0;
HWND hProcesswnd = NULL;

using namespace std;

bool inject(DWORD pID)
{
	TCHAR Buffer[MAX_PATH];
	DWORD dwRet;

	dwRet = GetCurrentDirectory(MAX_PATH, Buffer);

	std::string path(Buffer);

	path.append("/bypassdll.dll");

	DWORD dwAttr = GetFileAttributesA(path.c_str());

	if(dwAttr == 0xFFFFFFFF)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			cout << "bypassdll.dll not found!" << endl; 
		else
			cout << "Wild unknown error appeared!";

		return false;
	}

	HANDLE Proc; 
	LPVOID RemoteString, LoadLibAddy; 
	if(!pID) 
		return false; 
	Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); 
	if(!Proc) 
	{ 
		cout << "OpenProcess() failed: " << GetLastError() << endl; 
		return false; 
	} 
	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"); 
	// Allocate space in the process for our DLL
	RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(path.c_str()), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); 
	// Write the string name of our DLL in the memory allocated 
	WriteProcessMemory(Proc, (LPVOID)RemoteString, path.c_str(), strlen(path.c_str()), NULL); 
	// Load our DLL 
	CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL); 
	CloseHandle(Proc); 

	cout << "Injection succeed!" << endl; 
	return true; 
}

bool StartProcess()
{
	TCHAR Buffer[MAX_PATH];
	DWORD dwRet;

	dwRet = GetCurrentDirectory(MAX_PATH, Buffer);

	std::string path(Buffer);

	path.append("/MapleStory.exe");

	STARTUPINFO si = { sizeof(si) };

	PROCESS_INFORMATION  pi;

	ZeroMemory(&si, sizeof(si));

	if(!CreateProcess(NULL, (LPSTR)path.c_str(),0, 0, FALSE, 0, 0, 0, &si, &pi))
		return false;

	WaitForInputIdle(pi.hProcess, INFINITE);

	hProcess = pi.hProcess;
	dwProcessId = pi.dwProcessId;
	return true;
}

bool FindProcess()
{

	TCHAR szBuffer[200];
	DWORD dwTemp;

	for (HWND hWnd = GetTopWindow(NULL); hWnd != NULL; hWnd = GetNextWindow(hWnd, GW_HWNDNEXT))
	{
		GetWindowThreadProcessId(hWnd, &dwTemp);

		if (dwTemp != dwProcessId){	continue; }

		if (!GetClassName(hWnd, szBuffer, sizeof(szBuffer) / sizeof(TCHAR))){	continue;	}

		if (!strcmp(szBuffer,"StartUpDlgClass"))
		{
			cout << "Found StartUp dialog! Injecting now...";
			bool valid = inject(dwProcessId);			
			Sleep(1000);
			::PostMessage(hWnd,WM_KEYDOWN, VK_ESCAPE , ::MapVirtualKey(VK_ESCAPE, 0) << 16);
			return valid;
		}
	}
	return false;
}

int main()
{
	cout << "Bypass injector:" << endl;
	cout << "    If you're reading this, you're as lazy as me xD" << endl;
	cout << "                                           \"TheFox\"" << endl << endl ;
	if(StartProcess())
	{
		cout << "MapleStory started!" << endl;

		if(FindProcess())
			cout << "Done!" << endl;
		else
		{
			cout << "Dam! Something's wrong!" << endl;
			system("pause");
		}
	}
	else
	{
		cout << "Can't start MapleStory Y.Y!" << endl;
		system("pause");
	}
	return 1;
}

