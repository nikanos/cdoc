#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>

/*Tell the linker we need the following libraries*/
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"winmm.lib")

void ShowUsage(TCHAR* ProgName);
BOOL ParseArgs(int argc, TCHAR** argv);
BOOL CdOpenClose(BOOL bOpen, TCHAR* drive);
int CdOpenCloseAll(BOOL bOpen);
void mciHandleError(TCHAR* drive, MCIERROR mciErrorCode);

BOOL bOpenDrive = TRUE, bAll = FALSE, bUsage = FALSE;

int _tmain(int argc, TCHAR** argv)
{
	int i;
	int numDrivesSuccess = 0;
	int numArgErrors = 0;
	if (argc == 1)
	{
		ShowUsage(argv[0]);
		return EXIT_FAILURE;
	}
	if (!ParseArgs(argc, argv))
	{
		ShowUsage(argv[0]);
		return EXIT_FAILURE;
	}
	if (bUsage)
	{
		ShowUsage(argv[0]);
		return EXIT_SUCCESS;
	}
	if (bAll)
		numDrivesSuccess = CdOpenCloseAll(bOpenDrive);
	else
	{
		for (i = 1; i < argc; i++)
		{
			if (lstrlen(argv[i]) > 0 && argv[i][0] == '-')
				continue;

			if (!PathIsRoot(argv[i]))
			{
				_ftprintf(stderr, TEXT("Argument #%d (%s) is not a root drive path \n"), i, argv[i]);
				numArgErrors++;
			}
			if (numArgErrors > 0)
				return EXIT_FAILURE;
		}

		for (i = 1; i < argc; i++)
		{
			if (lstrlen(argv[i]) > 0 && argv[i][0] == '-')
				continue;

			numDrivesSuccess += (CdOpenClose(bOpenDrive, argv[i]) == TRUE ? 1 : 0);
		}
	}
	if (numDrivesSuccess > 0)
		_tprintf(TEXT("%s command was successfully sent to %d drive(s)\n"), (bOpenDrive ? "Open" : "Close"), numDrivesSuccess);
	else
		_tprintf(TEXT("No command was sent to any drive\n"));
	return EXIT_SUCCESS;
}

BOOL ParseArgs(int argc, TCHAR** argv)
{
	BOOL parseSuccess = TRUE;
	for (int i = 1; i < argc; i++)
	{
		if (lstrlen(argv[i]) > 0 && argv[i][0] == '-')
		{
			if (!lstrcmp(argv[i], TEXT("-c")) || !lstrcmp(argv[i], TEXT("--close")))
			{
				bOpenDrive = FALSE;
				continue;
			}
			if (!lstrcmp(argv[i], TEXT("-o")) || !lstrcmp(argv[i], TEXT("--open")))
			{
				bOpenDrive = TRUE;
				continue;
			}
			if (!lstrcmp(argv[i], TEXT("-a")) || !lstrcmp(argv[i], TEXT("--all")))
			{
				bAll = TRUE;
				continue;
			}
			if (!lstrcmp(argv[i], TEXT("-h")) || !lstrcmp(argv[i], TEXT("--help")))
			{
				bUsage = TRUE;
				continue;
			}
			_ftprintf(stderr, TEXT("Unsupported argument: %s\n"), argv[i]);
			parseSuccess = FALSE;
		}
	}
	return parseSuccess;
}

void ShowUsage(TCHAR* progName)
{
	_tprintf(TEXT("Cd Open-Close v1.0\n"));
	_tprintf(TEXT("Usage:%s [(drives)] [-h] [--help] [-a] [--all] [-c] [--close] [-o] [--open]\n"), progName);
	_tprintf(TEXT("%s -o or --open: Sets action to (open) (Default: open)\n"), progName);
	_tprintf(TEXT("%s -c or --close: Sets action to (close) (Default: open)\n"), progName);
	_tprintf(TEXT("%s -h or --help: Displays this help screen\n"), progName);
	_tprintf(TEXT("%s -a or --all: Opens all Cd-Rom Drives of the system\n"), progName);
	_tprintf(TEXT("%s e:\\ d:\\ Opens drive e:\\ and d:\\\n"), progName);
}

BOOL CdOpenClose(BOOL bOpen, TCHAR* drive)
{
	MCI_OPEN_PARMS mop;
	DWORD mciOpenFlags = MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT;
	ZeroMemory(&mop, sizeof(MCI_OPEN_PARMS));
	mop.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
	mop.lpstrElementName = drive;

	MCIERROR mciOpenError = mciSendCommand(0, MCI_OPEN, mciOpenFlags, (DWORD_PTR)&mop);
	if (!mciOpenError)
	{
		DWORD mciSetFlags;
		if (bOpen)
			mciSetFlags = MCI_SET_DOOR_OPEN;
		else
			mciSetFlags = MCI_SET_DOOR_CLOSED;
		mciSendCommand(mop.wDeviceID, MCI_SET, mciSetFlags, 0);
		mciSendCommand(mop.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);
		return TRUE;
	}
	else
	{
		mciHandleError(drive, mciOpenError);
		return FALSE;
	}
}

int CdOpenCloseAll(BOOL bOpen)
{
	TCHAR drive[4];
	DWORD logicalDrives;
	int numDrivesSuccess = 0;
	logicalDrives = GetLogicalDrives();
	for (int i = 0; i < sizeof(DWORD) * 8; i++)
	{
		if ((1 << i) & logicalDrives)
		{
			wsprintf(drive, TEXT("%c:\\"), 'A' + i);
			if (GetDriveType(drive) == DRIVE_CDROM)
				numDrivesSuccess += (CdOpenClose(bOpen, drive) == TRUE ? 1 : 0);

		}
	}
	return numDrivesSuccess;
}

void mciHandleError(TCHAR* drive, MCIERROR mciErrorCode)
{
	TCHAR errorMessage[512];
	mciGetErrorString(mciErrorCode, errorMessage, 512);
	_ftprintf(stderr, TEXT("(%s)%s\n"), drive, errorMessage);
}