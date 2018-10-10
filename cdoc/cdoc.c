#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>

/*Tell the linker we need the following libraries*/
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"winmm.lib")

void Usage(TCHAR* ProgName);
void ParseArgs(int argc, TCHAR** argv);
void CdOpenClose(BOOL bOpen, TCHAR* drive);
void CdOpenCloseAll(BOOL bOpen);
void mciHandleError(TCHAR* drive, MCIERROR mciErrorCode);

BOOL bOpenDrive = TRUE, bAll = FALSE, bUsage = FALSE;

int _tmain(int argc, TCHAR** argv)
{
	if (argc == 1)
		Usage(argv[0]);
	ParseArgs(argc, argv);
	if (bUsage)
		Usage(argv[0]);
	if (bAll)
		CdOpenCloseAll(bOpenDrive);
	else
	{
		for (int i = 1; i < argc; i++)
			if (PathIsRoot(argv[i]))
				CdOpenClose(bOpenDrive, argv[i]);
	}
	return 0;
}

void ParseArgs(int argc, TCHAR** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (!lstrcmp(argv[i], TEXT("-c")) || !lstrcmp(argv[i], TEXT("-close")))
			bOpenDrive = FALSE;
		else
			if (!lstrcmp(argv[i], TEXT("-a")) || !lstrcmp(argv[i], TEXT("-all")))
				bAll = TRUE;
			else
				if (!lstrcmp(argv[i], TEXT("-h")) || !lstrcmp(argv[i], TEXT("-help")))
					bUsage = TRUE;
	}
}

void Usage(TCHAR* progName)
{
	_tprintf(TEXT("Cd Open-Close v1.0\n"));
	_tprintf(TEXT("Usage:%s [(drives)] [-h] [-help] [-a] [-all] [-close] [-c]\n"), progName);
	_tprintf(TEXT("%s -c or -close:Changes default action(Open) to (Close)\n"), progName);
	_tprintf(TEXT("%s -h or -help:Displays this help screen\n"), progName);
	_tprintf(TEXT("%s -a or -all:Opens all Cd-Rom Drives of the system\n"), progName);
	_tprintf(TEXT("%s e:\\ d:\\ Opens drive e:\\ and d:\\\n"), progName);
	exit(-1);
}

void CdOpenClose(BOOL bOpen, TCHAR* drive)
{
	MCI_OPEN_PARMS mop;
	DWORD mciOpenFlags = MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT;
	ZeroMemory(&mop, sizeof(MCI_OPEN_PARMS));
	mop.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
	mop.lpstrElementName = drive;

	MCIERROR mciOpenError = mciSendCommand(0, MCI_OPEN, mciOpenFlags, (DWORD)&mop);
	if (!mciOpenError)
	{
		DWORD mciSetFlags;
		if (bOpen)
			mciSetFlags = MCI_SET_DOOR_OPEN;
		else
			mciSetFlags = MCI_SET_DOOR_CLOSED;
		mciSendCommand(mop.wDeviceID, MCI_SET, mciSetFlags, 0);
		mciSendCommand(mop.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);
	}
	else
		mciHandleError(drive, mciOpenError);
}

void CdOpenCloseAll(BOOL bOpen)
{
	TCHAR drive[4];
	DWORD logicalDrives;
	logicalDrives = GetLogicalDrives();
	for (int i = 0; i < sizeof(DWORD) * 8; i++)
	{
		if ((1 << i)&logicalDrives)
		{
			wsprintf(drive, TEXT("%c:\\"), 'A' + i);
			if (GetDriveType(drive) == DRIVE_CDROM)
				CdOpenClose(bOpen, drive);

		}
	}
}

void mciHandleError(TCHAR* drive, MCIERROR mciErrorCode)
{
	TCHAR errorMessage[512];
	mciGetErrorString(mciErrorCode, errorMessage, 512);
	_tprintf(TEXT("(%s)%s\n"), drive, errorMessage);
}