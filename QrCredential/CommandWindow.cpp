//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif
#include "CommandWindow.h"
#include <strsafe.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include<string.h>
#include <locale.h>
#include<fstream>
#include <string>
#include <iostream>
#include <winuser.h>
#include <dbt.h>

#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>

#include <atlstr.h>
#include <winioctl.h>


#include <stdio.h>
#include <Setupapi.h>
#include <winioctl.h>
#include <winioctl.h>
#include <cfgmgr32.h>


#ifdef USE_BLOCK_CIPHER
// defines for RC2 block cipher
#define ENCRYPT_ALGORITHM   CALG_RC2
#define ENCRYPT_BLOCK_SIZE  8
#else
// defines for RC4 stream cipher
#define ENCRYPT_ALGORITHM   CALG_RC4
#define ENCRYPT_BLOCK_SIZE  1
#endif
#define KEYLENGTH 0x00800000

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT 5444  //Oturum açma için mesajlaþma portu
#define DEFAULT_BUFLEN 512
// Custom messages for managing the behavior of the window thread.
#define WM_EXIT_THREAD              WM_USER + 1
#define WM_TOGGLE_CONNECTED_STATUS  WM_USER + 2

const WCHAR c_szClassName[] = L"EventWindow";
const WCHAR c_szConnected[] = L"Connected";
const WCHAR c_szDisconnected[] = L"Disconnected";

char FirstDriveFromMask(ULONG unitmask);  //prototype
static BOOL CAPIDecryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword);
DWORD GetPhysicalDriveSerialNumber(char usbDrive, UINT nDriveNumber, CString& strSerialNumber);
DEVINST GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType, char* szDosDeviceName, char* szDeviceId);
CCommandWindow::CCommandWindow() : _hWnd(NULL), _hInst(NULL), _fConnected(FALSE), _pProvider(NULL)
{

}

CCommandWindow::~CCommandWindow()
{
    // If we have an active window, we want to post it an exit message.
    if (_hWnd != NULL)
    {
        PostMessage(_hWnd, WM_EXIT_THREAD, 0, 0);
        _hWnd = NULL;
    }

    // We'll also make sure to release any reference we have to the provider.
    if (_pProvider != NULL)
    {
        _pProvider->Release();
        _pProvider = NULL;
    }
}

// Performs the work required to spin off our message so we can listen for events.
HRESULT CCommandWindow::Initialize(__in CSampleProvider *pProvider)
{
    HRESULT hr = S_OK;

	if (_pProvider != NULL)
	{
    // Be sure to add a release any existing provider we might have, then add a reference
    // to the provider we're working with now.

        _pProvider->Release();
    }
    _pProvider = pProvider;
    _pProvider->AddRef();

	HANDLE hThread20 = CreateThread(NULL, 0, _ThreadProc2, this, 0, NULL);

    // Create and launch the window thread.
    HANDLE hThread = CreateThread(NULL, 0, _ThreadProc, this, 0, NULL);
    if (hThread == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;

}
DWORD WINAPI CCommandWindow::_ThreadProc2(__in LPVOID lpParameter)
{

	CCommandWindow *pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
	//SC_HANDLE serviceDbHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	//SC_HANDLE serviceHandle = OpenService(serviceDbHandle, L"EtService", SC_MANAGER_ALL_ACCESS);

	if (pCommandWindow == NULL)
	{

		// TODO: What's the best way to raise this error?
		//return 0;
	}

	
	HRESULT hr = S_OK;
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char mesaj[256];
	setlocale(LC_ALL, "Turkish");
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}


	//Listen to incoming connections
	listen(s, 3);
	//Accept and incoming connection

	c = sizeof(struct sockaddr_in);
	new_socket = accept(s, (struct sockaddr *)&client, &c);
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}
	//MessageBoxA(NULL, "Accept", "Tamam",MB_OK);

	//	puts("Connection accepted");



	recv(new_socket, mesaj, sizeof(mesaj), 0);
	//	MessageBoxA(NULL, mesaj, "Tamam", MB_OK);
	if (mesaj[0] == 'l') {

		HKEY hKey = NULL;
		LONG lResult;
		LPTSTR szVal;
		//DWORD dwVal;
		//HRESULT hr;
		//wchar_t buffer[MAX_PATH];
		char usbDriveS[2];

		//DWORD size = sizeof(buffer);
		// Open the registry key...
		lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\ET", 0, KEY_ALL_ACCESS, &hKey);

		if (lResult != ERROR_SUCCESS) {
			MessageBoxW(NULL, L"ET Anahtar okunamadý!!!", L"Hata", MB_OK | MB_ICONERROR);
			return 0;
		}else {
			//MessageBoxA(NULL, deviceID, "jhjh", MB_OK);
			LPCTSTR value = TEXT("surucu");
			usbDriveS[0] = '0';
			usbDriveS[1] = NULL;
			LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (BYTE *)usbDriveS, strlen(usbDriveS) * sizeof(char));

		}

		WinExec("netsh interface set interface Ethernet admin=enable", SW_HIDE);
		PostMessage(pCommandWindow->_hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
		

	}

	//Reply to client
	//	message = "Hello Client , I have received your connection. But I have to go now, bye\n";
	//	send(new_socket, message, strlen(message), 0);

	//	getchar();

	closesocket(s);
	WSACleanup();

	// Start it 
	/*
	SERVICE_STATUS_PROCESS status;
	DWORD bytesNeeded;
	QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);
	yourfile << "servisi kapatan thread çalýþtý\n";
	if (status.dwCurrentState == SERVICE_STOP) {
		BOOL b = StartService(serviceHandle, NULL, NULL);
		if (b) {
			//MessageBox(NULL, L"Service baþladý", L"Device change", 0);
			yourfile << "servis baþladý\n";
		}
		else {
			//MessageBox(NULL, L"Service baþlatýlamadý", L"Device change", 0); 
			yourfile << "servis baþlayamadý\n";
		}
	}
	CloseServiceHandle(serviceHandle);
	CloseServiceHandle(serviceDbHandle);*/
	return hr;
}

// Wraps our internal connected status so callers can easily access it.
BOOL CCommandWindow::GetConnectedStatus()
{
    return _fConnected;
}

//
//  FUNCTION: _MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
HRESULT CCommandWindow::_MyRegisterClass()
{
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc      = _WndProc;
    wcex.hInstance        = _hInst;
    wcex.hIcon            = NULL;
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName    = c_szClassName;

    return RegisterClassEx(&wcex) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

//
//   FUNCTION: _InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HRESULT CCommandWindow::_InitInstance()
{
    HRESULT hr = S_OK;

    // Create our window to receive events.
    // 
    // This dialog is for demonstration purposes only.  It is not recommended to create 
    // dialogs that are visible even before a credential enumerated by this credential 
    // provider is selected.  Additionally, any dialogs that are created by a credential
    // provider should not have a NULL hwndParent, but should be parented to the HWND
    // returned by ICredentialProviderCredentialEvents::OnCreatingWindow.
    _hWnd = CreateWindowEx(
        WS_EX_TOPMOST, 
        c_szClassName, 
        c_szDisconnected, 
        WS_DLGFRAME,
        200, 200, 200, 80, 
        NULL,
        NULL, _hInst, NULL);
    if (_hWnd == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // Add a button to the window.
        _hWndButton = CreateWindow(L"Button", L"Press to connect", 
                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                             10, 10, 180, 30, 
                             _hWnd, 
                             NULL,
                             _hInst,
                             NULL);
        if (_hWndButton == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (SUCCEEDED(hr))
        {
            // Show and update the window.
            if (!ShowWindow(_hWnd, SW_HIDE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            if (SUCCEEDED(hr))
            {
                if (!UpdateWindow(_hWnd))
                {
                   hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
		
    }

    return hr;
}

// Called from the separate thread to process the next message in the message queue. If
// there are no messages, it'll wait for one.
BOOL CCommandWindow::_ProcessNextMessage()
{
    // Grab, translate, and process the message.
    MSG msg;
    GetMessage(&(msg), _hWnd, 0, 0);
    TranslateMessage(&(msg));
    DispatchMessage(&(msg));

    // This section performs some "post-processing" of the message. It's easier to do these
    // things here because we have the handles to the window, its button, and the provider
    // handy.
    switch (msg.message)
    {
    // Return to the thread loop and let it know to exit.
    case WM_EXIT_THREAD: return FALSE;

    // Toggle the connection status, which also involves updating the UI.
    case WM_TOGGLE_CONNECTED_STATUS:
        _fConnected = !_fConnected;
        if (_fConnected)
        {
            SetWindowText(_hWnd, c_szConnected);
            SetWindowText(_hWndButton, L"Press to disconnect");
        }
        else
        {
            SetWindowText(_hWnd, c_szDisconnected);
            SetWindowText(_hWndButton, L"Press to connect");
        }
        _pProvider->OnConnectStatusChanged();
        break;

    }
    return TRUE;
}

// Manages window messages on the window thread.
LRESULT CALLBACK CCommandWindow::_WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
	TCHAR szMsg[80];
	char usbDrive;
	char usbDriveS[2];
	char szSource[] = " :\\anahtar.txt";

	UINT nDriveNumber = 1;
	CString strSerialNumber;
	switch (message)
    {
    // Originally we were going to work with USB keys being inserted and removed, but it
    // seems as though these events don't get to us on the secure desktop. However, you
    // might see some messageboxi in CredUI.
    // TODO: Remove if we can't use from LogonUI.
    case WM_DEVICECHANGE:
       // MessageBox(NULL, L"Device change", L"Device change", 0);
			switch (wParam)
			{
			case DBT_DEVICEARRIVAL:
				//MessageBoxA(NULL, "aygýt takýldý", "kjkj", MB_OK);
				// Check whether a CD or DVD was inserted into a drive.
				if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
				{
					PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
					usbDrive = FirstDriveFromMask(lpdbv->dbcv_unitmask);

					szSource[0] = usbDrive;
					StringCchPrintf(szMsg, sizeof(szMsg) / sizeof(szMsg[0]),
						TEXT("Drive %s: Media has arrived.\n"),
						szSource);



					char DriveLetter = usbDrive;
					DriveLetter &= ~0x20; // uppercase

					if (DriveLetter < 'A' || DriveLetter > 'Z') {
						return 1;
					}

					char szRootPath[] = "X:\\";   // "X:\"  -> for GetDriveType
					szRootPath[0] = DriveLetter;

					char szDevicePath[] = "X:";   // "X:"   -> for QueryDosDevice
					szDevicePath[0] = DriveLetter;

					char szVolumeAccessPath[] = "\\\\.\\X:";   // "\\.\X:"  -> to open the volume
					szVolumeAccessPath[4] = DriveLetter;

					long DeviceNumber = -1;

					// open the storage volume
					HANDLE hVolume = CreateFileA(szVolumeAccessPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
					if (hVolume == INVALID_HANDLE_VALUE) {
						return 1;
					}

					// get the volume's device number
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
					if (res) {
						DeviceNumber = sdn.DeviceNumber;
					}
					CloseHandle(hVolume);

					if (DeviceNumber == -1) {
						return 1;
					}

					// get the drive type which is required to match the device numbers correctely
					UINT DriveType = GetDriveTypeA(szRootPath);

					// get the dos device name (like \device\floppy0) to decide if it's a floppy or not - who knows a better way?
					char szDosDeviceName[MAX_PATH];
					res = QueryDosDeviceA(szDevicePath, szDosDeviceName, MAX_PATH);
					if (!res) {
						return 1;
					}
					char deviceID[MAX_PATH];
					// get the device instance handle of the storage volume by means of a SetupDi enum and matching the device number
					DEVINST DevInst = GetDrivesDevInstByDeviceNumber(DeviceNumber, DriveType, szDosDeviceName, deviceID);

/*

					DWORD dwResult = GetPhysicalDriveSerialNumber(usbDrive, nDriveNumber, strSerialNumber);
					char zSerial[500];
					wcstombs(zSerial, strSerialNumber.GetBuffer(MAX_PATH), 500);
					strSerialNumber.ReleaseBuffer();
*/				
					HKEY hKey = NULL;
					LONG lResult;
					LPTSTR szVal;
					DWORD dwVal;
					HRESULT hr;
					wchar_t buffer[MAX_PATH];
					char x[MAX_PATH];
					
					DWORD size = sizeof(buffer);
					// Open the registry key...
					lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\ET", 0, KEY_ALL_ACCESS, &hKey);

					if (lResult != ERROR_SUCCESS) {
						MessageBoxW(NULL, L"ET Anahtar okunamadý!!!", L"Hata", MB_OK|MB_ICONERROR);
						return 0;
					}

					LONG result = RegQueryValueEx(hKey, L"EtKey", 0, NULL, (BYTE*)buffer, &size);
					if (ERROR_SUCCESS == result) {
						
						wcstombs(x, buffer, MAX_PATH);
						//MessageBoxA(NULL, x, "kjkj", MB_OK);
					}
					else {
						MessageBoxW(NULL, L"EtKey Anahtar okunamadý!!!", L"Hata", MB_OK | MB_ICONERROR);
						return 0;
					}
						

					if (!CAPIDecryptFile(szSource, deviceID, x)){
					 	MessageBoxA(NULL, "Geçersiz flash bellek!!!", "HATA!!!",MB_OK|MB_ICONERROR);
					   //printf("Error encrypting file!\n");
						//
					}
					else {
						//MessageBoxA(NULL, deviceID, "jhjh", MB_OK);
						LPCTSTR value = TEXT("surucu");
						usbDriveS[0] = usbDrive;
						usbDriveS[1] = NULL;
						LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (BYTE *)usbDriveS, strlen(usbDriveS) * sizeof(char));
						WinExec("netsh interface set interface Ethernet admin=enable", SW_HIDE);
						PostMessage(hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
						

					}
				}
				break;

			case DBT_DEVICEREMOVECOMPLETE:
				//std::cout << "device disconnected: " << path << "\n";
				break;
			}
		

        break;

    // We assume this was the button being clicked.
    case WM_COMMAND:
       // PostMessage(hWnd, WM_TOGGLE_CONNECTED_STATUS, 0, 0);
        break;

    // To play it safe, we hide the window when "closed" and post a message telling the 
    // thread to exit.
    case WM_CLOSE:
       // ShowWindow(hWnd, SW_HIDE);
      //  PostMessage(hWnd, WM_EXIT_THREAD, 0, 0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

DWORD GetPhysicalDriveSerialNumber(char usbDrive, UINT nDriveNumber, CString& strSerialNumber)
{
	DWORD dwResult = NO_ERROR;
	strSerialNumber.Empty();

	// Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
	CString strDrivePath;
	strDrivePath.Format(_T("\\\\.\\%c:"), usbDrive);
	// call CreateFile to get a handle to physical drive
	HANDLE hDevice = ::CreateFile(strDrivePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == hDevice)
		return ::GetLastError();

	// set the input STORAGE_PROPERTY_QUERY data structure
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	// get the necessary output buffer size
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
	DWORD dwBytesReturned = 0;
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		::CloseHandle(hDevice);
		return dwResult;
	}

	// allocate the necessary memory for the output buffer
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
	ZeroMemory(pOutBuffer, dwOutBufferSize);

	// get the storage device descriptor
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer, dwOutBufferSize,
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		delete[]pOutBuffer;
		::CloseHandle(hDevice);
		return dwResult;
	}

	// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
	// followed by additional info like vendor ID, product ID, serial number, and so on.
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset != 0)
	{
		// finally, get the serial number
		strSerialNumber = CString(pOutBuffer + dwSerialNumberOffset);
	}

	// perform cleanup and return
	delete[]pOutBuffer;
	::CloseHandle(hDevice);
	return dwResult;
}


DEVINST GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType, char* szDosDeviceName, char* szDeviceId)
{
	bool IsFloppy = (strstr(szDosDeviceName, "\\Floppy") != NULL); // who knows a better way?

	GUID* guid;

	switch (DriveType) {
	case DRIVE_REMOVABLE:
		if (IsFloppy) {
			guid = (GUID*)&GUID_DEVINTERFACE_FLOPPY;
		}
		else {
			guid = (GUID*)&GUID_DEVINTERFACE_DISK;
		}
		break;
	case DRIVE_FIXED:
		guid = (GUID*)&GUID_DEVINTERFACE_DISK;
		break;
	case DRIVE_CDROM:
		guid = (GUID*)&GUID_DEVINTERFACE_CDROM;
		break;
	default:
		return 0;
	}

	// Get device interface info set handle for all devices attached to system
	HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE) {
		return 0;
	}

	// Retrieve a context structure for a device interface of a device information set
	DWORD dwIndex = 0;
	long res;

	BYTE Buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;
	SP_DEVICE_INTERFACE_DATA         spdid;
	SP_DEVINFO_DATA                  spdd;
	DWORD                            dwSize;

	spdid.cbSize = sizeof(spdid);

	while (true) {
		res = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &spdid);
		if (!res) {
			break;
		}

		dwSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL); // check the buffer size

		if (dwSize != 0 && dwSize <= sizeof(Buf)) {

			pspdidd->cbSize = sizeof(*pspdidd); // 5 Bytes!

			ZeroMemory(&spdd, sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
			if (res) {

				// in case you are interested in the USB serial number:
				// the device id string contains the serial number if the device has one,
				// otherwise a generated id that contains the '&' char...

				DEVINST DevInstParent = 0;
				CM_Get_Parent(&DevInstParent, spdd.DevInst, 0);
				char szDeviceIdString[MAX_PATH];
				CM_Get_Device_IDA(DevInstParent, szDeviceIdString, MAX_PATH, 0);



				// open the disk or cdrom or floppy
				HANDLE hDrive = CreateFile(pspdidd->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				if (hDrive != INVALID_HANDLE_VALUE) {
					// get its device number
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
					if (res) {
						if (DeviceNumber == (long)sdn.DeviceNumber) {  // match the given device number with the one of the current device
																	   //MessageBoxA(NULL, szDeviceIdString, "khkj", MB_OK);
							strcpy(szDeviceId, szDeviceIdString);
							//getch();
							CloseHandle(hDrive);
							SetupDiDestroyDeviceInfoList(hDevInfo);
							return spdd.DevInst;
						}
					}
					CloseHandle(hDrive);
				}
			}
		}
		dwIndex++;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}



// Our thread procedure. We actually do a lot of work here that could be put back on the 
// main thread, such as setting up the window, etc.
static BOOL CAPIDecryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword)
{

    FILE *hSource      = NULL;
    FILE *hDestination = NULL;
	errno_t err;
    INT eof = 0;

    HCRYPTPROV hProv   = 0;
    HCRYPTKEY hKey     = 0;
    HCRYPTHASH hHash   = 0;

    PBYTE pbKeyBlob = NULL;
    DWORD dwKeyBlobLen;

    PBYTE pbBuffer = NULL;
    DWORD dwBlockLen;
    DWORD dwBufferLen;
    DWORD dwCount;
	char x[MAX_PATH];
    BOOL status = FALSE;

    // Open source file.
	DWORD attr = GetFileAttributesA(szSource);
	SetFileAttributesA(szSource, attr - FILE_ATTRIBUTE_HIDDEN);
	err = fopen_s(&hSource,szSource,"rb");
    if(err !=0) {
        printf("Error opening Ciphertext file!\n");
		MessageBoxA(NULL, "dosya açýlamadý", "lkl", MB_OK);
        goto done;
    }

    // Open destination file.
	/*
	err = fopen_s(&hDestination,szDestination,"wb");
    if(err != 0) {
        printf("Error opening Plaintext file!\n");
        goto done;
    }
*/
    // Get handle to the default provider.
    if(!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        printf("Error %x during CryptAcquireContext!\n", GetLastError());
        goto done;
    }

    if(szPassword == NULL) {
        // Decrypt the file with the saved session key.

        // Read key blob length from source file and allocate memory.
        fread(&dwKeyBlobLen, sizeof(DWORD), 1, hSource);
        if(ferror(hSource) || feof(hSource)) {
            printf("Error reading file header!\n");
            goto done;
        }
        if((pbKeyBlob = (PBYTE)malloc(dwKeyBlobLen)) == NULL) {
            printf("Out of memory or improperly formatted source file!\n");
            goto done;
        }

        // Read key blob from source file.
        fread(pbKeyBlob, 1, dwKeyBlobLen, hSource);
        if(ferror(hSource) || feof(hSource)) {
            printf("Error reading file header!\n");
            goto done;
        }

        // Import key blob into CSP.
        if(!CryptImportKey(hProv, pbKeyBlob, dwKeyBlobLen, 0, 0, &hKey)) {
            printf("Error %x during CryptImportKey!\n", GetLastError());
            goto done;
        }
    } else {
        // Decrypt the file with a session key derived from a password.

        // Create a hash object.
        if(!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
            printf("Error %x during CryptCreateHash!\n", GetLastError());
            goto done;
        }

        // Hash in the password data.
        if(!CryptHashData(hHash, (PBYTE)szPassword, (DWORD)strlen(szPassword), 0)) {
            printf("Error %x during CryptHashData!\n", GetLastError());
            goto done;
        }

        // Derive a session key from the hash object.
        if(!CryptDeriveKey(hProv, ENCRYPT_ALGORITHM, hHash, KEYLENGTH, &hKey)) {
            printf("Error %x during CryptDeriveKey!\n", GetLastError());
            goto done;
        }

        // Destroy the hash object.
        CryptDestroyHash(hHash);
        hHash = 0;
    }

    // Determine number of bytes to decrypt at a time. This must be a multiple
    // of ENCRYPT_BLOCK_SIZE.
    dwBlockLen = MAX_PATH - MAX_PATH % ENCRYPT_BLOCK_SIZE;
    dwBufferLen = dwBlockLen;

    // Allocate memory.
    if((pbBuffer = (PBYTE)malloc(dwBufferLen)) == NULL) {
        printf("Out of memory!\n");
        goto done;
    }

    // Decrypt source file and write to destination file.
    do {
        // Read up to 'dwBlockLen' bytes from source file.
        dwCount = (DWORD)fread(pbBuffer, 1, dwBlockLen, hSource);
        if(ferror(hSource)) {
            printf("Error reading Ciphertext!\n");
            goto done;
        }
        eof = feof(hSource);

        // Decrypt data
        if(!CryptDecrypt(hKey, 0, eof, 0, pbBuffer, &dwCount)) {
            printf("Error %x during CryptDecrypt!\n", GetLastError());
            goto done;
        }
		

        // Write data to destination file.
		/*
        fwrite(pbBuffer, 1, dwCount, hDestination);
        if(ferror(hDestination)) {
            printf("Error writing Plaintext!\n");
            goto done;
        }*/

    } while(!feof(hSource));
	strncpy(x, (char *)pbBuffer,strlen(szDestination));
	x[strlen(szDestination)] = NULL;
	//MessageBoxA(NULL, x, szDestination, MB_OK);
	if (strcmp(szDestination, x) == 0) {
		status = TRUE;
	} else MessageBoxA(NULL, "Þifre çözülemiyor!!!", "HATA!!!", MB_OK|MB_ICONERROR);
    done:
    // Close files.
    if(hSource) fclose(hSource);
    if(hDestination) fclose(hDestination);
	DWORD attr2 = GetFileAttributesA(szSource);
	SetFileAttributesA(szSource, attr2 + FILE_ATTRIBUTE_HIDDEN);
    // Free memory.
    if(pbKeyBlob) free(pbKeyBlob);
    if(pbBuffer) free(pbBuffer);

    // Destroy session key.
    if(hKey) CryptDestroyKey(hKey);

    // Destroy hash object.
    if(hHash) CryptDestroyHash(hHash);

    // Release provider handle.
    if(hProv) CryptReleaseContext(hProv, 0);

    return(status);
}
char FirstDriveFromMask(ULONG unitmask)
{
	char i;

	for (i = 0; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}

	return (i + 'A');
}
DWORD WINAPI CCommandWindow::_ThreadProc(__in LPVOID lpParameter)
{
    CCommandWindow *pCommandWindow = static_cast<CCommandWindow *>(lpParameter);

    if (pCommandWindow == NULL)
    {
        // TODO: What's the best way to raise this error?
        return 0;
    }

    HRESULT hr = S_OK;

    // Create the window.
    pCommandWindow->_hInst = GetModuleHandle(NULL);
    if (pCommandWindow->_hInst != NULL)
    {            
        hr = pCommandWindow->_MyRegisterClass();
        if (SUCCEEDED(hr))
        {
            hr = pCommandWindow->_InitInstance();
			
			
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }




    // ProcessNextMessage will pump our message pump and return false if it comes across
    // a message telling us to exit the thread.
    if (SUCCEEDED(hr))
    {        
        while (pCommandWindow->_ProcessNextMessage()) 
        {
			
        }
    }
    else
    {
        if (pCommandWindow->_hWnd != NULL)
        {
            pCommandWindow->_hWnd = NULL;
        }
    }

    return 0;
}

