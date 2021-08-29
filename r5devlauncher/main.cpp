#include <string>
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <detours.h>

/*-----------------------------------------------------------------------------
 * _main.cpp
 *-----------------------------------------------------------------------------*/

 //----------------------------------------------------------------------------
 // Print the error message to the console if any
 //----------------------------------------------------------------------------
void PrintLastError()
{
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return;
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    printf("ERROR: %s\n", messageBuffer);
    LocalFree(messageBuffer);
}

//-----------------------------------------------------------------------------
// Purpose case switch:
// * Launch the game in user specified mode and state.
// * Load specified command line arguments from a file on the disk.
// * Format the file paths for the game exe and specified hook dll.
//-----------------------------------------------------------------------------
bool LaunchR5Apex()
{
    BOOL result;

    FILE* sLaunchParams;
    CHAR sArgumentBuffer[2048] = { 0 };
    CHAR sCommandDirectory[MAX_PATH];
    LPSTR sCommandLine = sCommandDirectory;

    CHAR sDevDll[MAX_PATH];
    CHAR sGameExe[MAX_PATH];
    CHAR sGameDirectory[MAX_PATH];

    STARTUPINFO StartupInfo = { 0 };
    PROCESS_INFORMATION ProcInfo = { 0 };

    ///////////////////////////////////////////////////////////////////////////
    // Initialize the startup info structure.
    StartupInfo.cb = sizeof(STARTUPINFO);

    GetCurrentDirectory(MAX_PATH, sGameDirectory);
    fopen_s(&sLaunchParams, "platform\\cfg\\startup_debug.cfg", "r");
    if (sLaunchParams)
    {
        while (fread(sArgumentBuffer, sizeof(sArgumentBuffer), 1, sLaunchParams) != NULL)
        {
            fclose(sLaunchParams);
        }
    }

    snprintf(sGameExe, sizeof(sGameExe), "%s\\r5apex.exe", sGameDirectory);
    snprintf(sDevDll, sizeof(sDevDll), "%s\\r5dev.dll", sGameDirectory);
    snprintf(sCommandLine, sizeof(sCommandDirectory), "%s\\r5apex.exe %s", sGameDirectory, sArgumentBuffer);
    printf("*** LAUNCHING GAME [DEBUG] ***\n");

    ///////////////////////////////////////////////////////////////////////////
    // Print the filepaths and arguments.
    printf(" - CWD: %s\n", sGameDirectory);
    printf(" - EXE: %s\n", sGameExe);
    printf(" - DLL: %s\n", sDevDll);
    printf(" - CLI: %s\n", sCommandLine);

    ///////////////////////////////////////////////////////////////////////////
    // Build our list of dlls to inject.
    LPCSTR DllsToInject[1] =
    {
        sDevDll
    };

    ///////////////////////////////////////////////////////////////////////////
    // Create the game process in a suspended state with our dll.
    result = DetourCreateProcessWithDllsA(
        sGameExe,                 // lpApplicationName
        sCommandLine,             // lpCommandLine
        NULL,                     // lpProcessAttributes
        NULL,                     // lpThreadAttributes
        FALSE,                    // bInheritHandles
        CREATE_SUSPENDED,         // dwCreationFlags
        NULL,                     // lpEnvironment
        sGameDirectory,           // lpCurrentDirectory
        &StartupInfo,             // lpStartupInfo
        &ProcInfo,                // lpProcessInformation
        1,                        // nDlls
        DllsToInject,             // rlpDlls
        NULL                      // pfCreateProcessA
    );

    ///////////////////////////////////////////////////////////////////////////
    // Failed to create the process.
    if (!result)
    {
        PrintLastError();
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Resume the process.
    ResumeThread(ProcInfo.hThread);

    ///////////////////////////////////////////////////////////////////////////
    // Close the process and thread handles.
    CloseHandle(ProcInfo.hProcess);
    CloseHandle(ProcInfo.hThread);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Entrypoint.
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[], char* envp[])
{
    LaunchR5Apex();
    Sleep(2000);
    return EXIT_SUCCESS;
}