/*
* Project: Windows API Serial port programming
* Reading NEO-7M GPS NMEA protocol data
*/
#include "tchar.h"
#include "windows.h"
#include "stdio.h"
#include "winbase.h"
#include <iostream>

using namespace std;

	HANDLE hMutex;
	HANDLE m_hComm;
	/*
	* Open serial port: CreateFile funciton
	*/
	DWORD WINAPI Read(LPVOID lpParamter)
	{
		m_hComm = CreateFile(						//"\\\\.\\COM4"
			"COM4",									// communicaiton port string (COMX)
			GENERIC_READ | GENERIC_WRITE,			// read or write types
			0,										// communication devices must be opened with exclusive access
			NULL,									// no security attributes
			OPEN_EXISTING,							// communication devices must be use OPEN_EXISTING
			0,										// FILE_FLAG_OVERLAPPED-0: Async I/O 
			NULL									// Template must be 0 for communication devices
		);

		/*
		* If execute successfully, return serial port handler, otherwise print error
		*/
		if (m_hComm == INVALID_HANDLE_VALUE)
		{
			printf("Open Serial Fail!");
			return FALSE;
		}

		/*
		* Setting buffer area, easy to lose data if small
		*/
		SetupComm(m_hComm,							// Serial port handler
			2000,									// input buffer bytes
			2000);									// output bufffer bytes

		/*
		* Setting ports writing/reading timeout
		*/
		COMMTIMEOUTS TimeOuts;
		GetCommTimeouts(m_hComm, &TimeOuts);
		//TimeOuts.ReadIntervalTimeout = MAXDWORD;	// reading interval time
		//reading total time = reading time factor * reading bytes + time constant
		TimeOuts.ReadIntervalTimeout = 0;			// reading interval time
		TimeOuts.ReadTotalTimeoutMultiplier = 0;	// reading time factor
		TimeOuts.ReadTotalTimeoutConstant = 1;		// reading time constant

		//TimeOuts.WriteTotalTimeoutMultiplier = 0; // writing time factor
		//TimeOuts.WriteTotalTimeoutConstant = 0;	// writing time constant
		SetCommTimeouts(m_hComm, &TimeOuts);

		/*
		* Setting serial port states
		*/
		DCB dcb;									// structure
		GetCommState(m_hComm, &dcb);				// feed dcb as object t
		dcb.BaudRate = 9600;						// Baudrateo modify
		dcb.ByteSize = 8;							// data bits
		dcb.Parity = NOPARITY;						// no odd or even
		dcb.StopBits = ONESTOPBIT;					// stop bit
		SetCommState(m_hComm, &dcb);				// finish modified objects as parameters

		/*
		* Clear buffer, if any junk left before TX/RX
		*/
		PurgeComm(m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

		char buffer[2000];
		//char sendbuf[32] = "test\n";
		DWORD readsize, dwError;
		BOOL bReadStatus;
		COMSTAT cs;

		/*
		* Clear error information before communicaiton
		*/
		while (TRUE)
		{
			WaitForSingleObject(hMutex, INFINITE);
			readsize = 0;
			ClearCommError(m_hComm, &dwError, &cs);		// get state
			if (cs.cbInQue>sizeof(buffer))				// data is greater than buffer
			{
				PurgeComm(m_hComm, PURGE_RXCLEAR);		// clear communication port data
				return 0;
			}
			memset(buffer, 0, sizeof(buffer));
			bReadStatus = ReadFile(m_hComm, buffer, sizeof(buffer), &readsize, 0);
			if (readsize>0)
			{
				std::cout << buffer;
			}

			ReleaseMutex(hMutex);
		}
		CloseHandle(m_hComm);
	}

	void main()
	{
		HANDLE hThread = CreateThread(NULL, 0, Read, NULL, 0, NULL);
		hMutex = CreateMutex(NULL, FALSE, NULL);
		CloseHandle(hThread);
		while (1);
	}
