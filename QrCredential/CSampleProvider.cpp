//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CSampleProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// This sample illustrates processing asynchronous external events and 
// using them to provide the user with an appropriate set of credentials.
// In this sample, we provide two credentials: one for when the system
// is "connected" and one for when it isn't. When it's "connected", the
// tile provides the user with a field to log in as the administrator.
// Otherwise, the tile asks the user to connect first.
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif
#pragma once
#include <credentialprovider.h>
#include "CSampleCredential.h"
#include "CommandWindow.h"
#include "guid.h"
#include <QrCodeGeneratorDemo.h>
#include <iostream>
#include <fstream>
using std::ofstream;
// CSampleProvider ////////////////////////////////////////////////////////
HRESULT CSampleProvider::Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders)
{
	switch (cpus)
	{
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
		for (DWORD i = 0; i < cProviders; i++)
		{
			//rgbAllow[i] = TRUE;
			if (i < dwFlags)
			{
			}
			// Kendi saðlayýcýmýz dýþýndaki oturumlarý gizleme
			if (IsEqualGUID(rgclsidProviders[i], CLSID_CSample)) {
				rgbAllow[i] = TRUE;
			}
			else {
				rgbAllow[i] =FALSE;
			}
		}

		return S_OK;
		break;
	case CPUS_CHANGE_PASSWORD:
	case CPUS_CREDUI:
		return E_NOTIMPL;
	default:
		return E_INVALIDARG;
	}
}

HRESULT CSampleProvider::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsOut)
{
	UNREFERENCED_PARAMETER(pcpsIn);
	UNREFERENCED_PARAMETER(pcpcsOut);
	return E_NOTIMPL;
}
DWORD WINAPI CSampleProvider::_ThreadProc3(__in LPVOID lpParameter)
{
	
	SC_HANDLE serviceDbHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE serviceHandle = OpenService(serviceDbHandle, L"EtService", SC_MANAGER_ALL_ACCESS);
	SERVICE_STATUS_PROCESS status;
	DWORD bytesNeeded;
	QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);

	if (status.dwCurrentState == SERVICE_RUNNING)
	{// Stop it
		BOOL b = ControlService(serviceHandle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&status);
		if (b)
		{
			//MessageBox(NULL, L"Service Durdu", L"Device change", 0);

		}
		else {
			//MessageBox(NULL, L"Service durdurulamadý", L"Device change", 0);
		
		}
	}
	CloseServiceHandle(serviceHandle);
	CloseServiceHandle(serviceDbHandle);
	return 0;
}
CSampleProvider::CSampleProvider() :
	_cRef(1),
	_pCredential(nullptr),
	_pCredProviderUserArray(nullptr)
{
	DllAddRef();
	_pcpe = NULL;
	_pCommandWindow = NULL;
	_pCredential = NULL;
	_pMessageCredential = NULL;

	//qrcode oluþturma bölümü
	const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;  // Error correction level
	WSAData d;
	if (WSAStartup(MAKEWORD(2, 2), &d) != 0) {
		//	return -1;
	}
	Adapter *adapter = new Adapter();
	char text[512];
	//text = adapter->adapterAdress;
	strcpy_s(text, adapter->adapterAdress);
	this->CryptText(text);
	WSACleanup();
	// Make and print the QR Code symbol
	const QrCode qr = QrCode::encodeText(text, errCorLvl);
	CreateImage(qr);
}
void CSampleProvider::CryptText(char *text)
{
	for (int i = 0; text[i] != NULL; i++) {
		if (text[i] != '.') {
			switch (57 - text[i])
			{
			case 0:text[i] = 'y'; break;
			case 1:text[i] = 'U'; break;
			case 2:text[i] = 's'; break;
			case 3:text[i] = 'u'; break;
			case 4:text[i] = 'F'; break;
			case 5:text[i] = 'b'; break;
			case 6:text[i] = 'A'; break;
			case 7:text[i] = 'd'; break;
			case 8:text[i] = 'E'; break;
			case 9:text[i] = 'k'; break;
			}
		}
		else text[i] = 'Z';
	}

}
CSampleProvider::~CSampleProvider()
{
	if (_pCredential != NULL)
	{
		_pCredential->Release();
		_pCredential = NULL;

	}

	if (_pCommandWindow != NULL)
	{
		delete _pCommandWindow;
	}
	if (_pCredProviderUserArray != NULL)
	{
		_pCredProviderUserArray->Release();
		_pCredProviderUserArray = NULL;
	}
	DllRelease();
}

// This method acts as a callback for the hardware emulator. When it's called, it simply
// tells the infrastructure that it needs to re-enumerate the credentials.
void CSampleProvider::OnConnectStatusChanged()
{

	if (_pcpe != NULL)
	{
		_pcpe->CredentialsChanged(_upAdviseContext);
	}
}

void CSampleProvider::_ReleaseEnumeratedCredentials()
{
	if (_pCredential != nullptr)
	{
		_pCredential->Release();
		_pCredential = nullptr;
	}
	if (_pMessageCredential != NULL)
	{
		_pMessageCredential->Release();
		_pMessageCredential = NULL;
	}
}

void CSampleProvider::_CreateEnumeratedCredentials()
{
	

	switch (_cpus)
	{
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
	{
		// HANDLE hThread3 = CreateThread(NULL, 0, _ThreadProc3, this, 0, NULL);
		_EnumerateCredentials();
	
		break;
	}
	default:
		break;
	}
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.

HRESULT CSampleProvider::SetUsageScenario(
	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	DWORD dwFlags)
{
	HRESULT hr;

	// Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
	// that we're not designed for that scenario.
	switch (cpus)
	{
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
		// The reason why we need _fRecreateEnumeratedCredentials is because ICredentialProviderSetUserArray::SetUserArray() is called after ICredentialProvider::SetUsageScenario(),
		// while we need the ICredentialProviderUserArray during enumeration in ICredentialProvider::GetCredentialCount()
		_cpus = cpus;
		_fRecreateEnumeratedCredentials = true;
		hr = S_OK;
		break;

	case CPUS_CHANGE_PASSWORD:
	case CPUS_CREDUI:
		hr = E_NOTIMPL;
		break;

	default:
		hr = E_INVALIDARG;
		break;
	}

	return hr;
}
HRESULT CSampleProvider::_EnumerateCredentials()

{
	setlocale(LC_ALL, "Turkish");

	//  UNREFERENCED_PARAMETER(dwFlags);

	//HRESULT hr;

	// Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
	// that we're not designed for that scenario.

	HRESULT hr = E_UNEXPECTED;
	if (_pCredProviderUserArray != nullptr)
	{

		DWORD dwUserCount,defaultUser=0;
		_pCredProviderUserArray->GetCount(&dwUserCount);
		if (dwUserCount > 0)
		{
	
			for (DWORD i = 0; i < dwUserCount; i++) {
				ICredentialProviderUser *pCredUsr = NULL;
				hr = _pCredProviderUserArray->GetAt(i, &pCredUsr);
				LPWSTR userName = NULL;
				hr = pCredUsr->GetStringValue(PKEY_Identity_UserName, &userName);
				if (wcscmp(userName, L"Etkileþimli Tahta") == 0) {
					defaultUser = i; 
					break;
				}
			//	hr = _EnumerateOneCredential(i, userName, pCredUser);
			}

			ICredentialProviderUser *pCredUser;
			hr = _pCredProviderUserArray->GetAt(defaultUser, &pCredUser);
			if (SUCCEEDED(hr))
				if (!_pCredential && !_pMessageCredential && !_pCommandWindow)
				{
					// For the locked case, a more advanced credprov might only enumerate tiles for the 
					// user whose owns the locked session, since those are the only creds that will work

					_pCredential = new CSampleCredential();
					if (_pCredential != NULL)
					{
						_pMessageCredential = new CMessageCredential();
						if (_pMessageCredential)
						{
							_pCommandWindow = new CCommandWindow();
							if (_pCommandWindow != NULL)
							{
								// Initialize each of the object we've just created. 
								// - The CCommandWindow needs a pointer to us so it can let us know 
								// when to re-enumerate credentials.
								// - The CSampleCredential needs field descriptors.
								// - The CMessageCredential needs field descriptors and a message.
								hr = _pCommandWindow->Initialize(this);
								
								if (SUCCEEDED(hr))
								{

									hr = _pCredential->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, pCredUser);
									if (SUCCEEDED(hr))
									{
										hr = _pMessageCredential->Initialize(s_rgMessageCredProvFieldDescriptors, s_rgMessageFieldStatePairs, L"Lütfen karekodu okutun...", this);

									}
								}
							}
							else
							{
								hr = E_OUTOFMEMORY;
							}
						}
						else
						{
							hr = E_OUTOFMEMORY;
						}
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}
					// If anything failed, clean up.
					if (FAILED(hr))
					{
						if (_pCommandWindow != NULL)
						{
							delete _pCommandWindow;
							_pCommandWindow = NULL;
						}
						if (_pCredential != NULL)
						{
							_pCredential->Release();
							_pCredential = NULL;
						}
						if (_pMessageCredential != NULL)
						{
							_pMessageCredential->Release();
							_pMessageCredential = NULL;
						}
					}
				}
				else
				{
					//everything's already all set up
					hr = S_OK;
				}
		}

		}
	return hr;
}

// This function will be called by LogonUI after SetUsageScenario succeeds.
// Sets the User Array with the list of users to be enumerated on the logon screen.
HRESULT CSampleProvider::SetUserArray(_In_ ICredentialProviderUserArray *users)
{

	if (_pCredProviderUserArray)
	{
		_pCredProviderUserArray->Release();
	}
	_pCredProviderUserArray = users;
	
	_pCredProviderUserArray->AddRef();
	return S_OK;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// If you wish to see an example of SetSerialization, please see either the SampleCredentialProvider
// sample or the SampleCredUICredentialProvider sample.  [The logonUI team says, "The original sample that
// this was built on top of didn't have SetSerialization.  And when we decided SetSerialization was
// important enough to have in the sample, it ended up being a non-trivial amount of work to integrate
// it into the main sample.  We felt it was more important to get these samples out to you quickly than to
// hold them in order to do the work to integrate the SetSerialization changes from SampleCredentialProvider 
// into this sample.]
HRESULT CSampleProvider::SetSerialization(
	__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION const * /*pcpcs*/
)
{
	//UNREFERENCED_PARAMETER(pcpcs);
	return E_NOTIMPL;
}

// Called by LogonUI to give you a callback. Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated

HRESULT CSampleProvider::Advise(
	__in ICredentialProviderEvents* pcpe,
	__in UINT_PTR upAdviseContext
)
{

	if (_pcpe != NULL)
	{
		_pcpe->Release();

	}
	_pcpe = pcpe;
	_pcpe->AddRef();
	_upAdviseContext = upAdviseContext;
	return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CSampleProvider::UnAdvise()
{

	if (_pcpe != NULL)
	{

		_pcpe->Release();
		_pcpe = NULL;
	}
	return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles. We return the number
// of fields to be displayed on our active tile, which depends on our connected state. The
// "connected" CSampleCredential has SFI_NUM_FIELDS fields, whereas the "disconnected" 
// CMessageCredential has SMFI_NUM_FIELDS fields.
HRESULT CSampleProvider::GetFieldDescriptorCount(
	__out DWORD* pdwCount
)
{

	if (_pCommandWindow->GetConnectedStatus())
	{
		*pdwCount = SFI_NUM_FIELDS;
	}
	else
	{
		*pdwCount = SMFI_NUM_FIELDS;
	}

	return S_OK;
}

// Gets the field descriptor for a particular field. Note that we need to determine which
// tile to use based on the "connected" status.
HRESULT CSampleProvider::GetFieldDescriptorAt(
	__in DWORD dwIndex,
	__deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
)
{
	HRESULT hr;

	if (_pCommandWindow->GetConnectedStatus())
	{
		// Verify dwIndex is a valid field.
		if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
		{
			hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	else
	{
		// Verify dwIndex is a valid field.
		if ((dwIndex < SMFI_NUM_FIELDS) && ppcpfd)
		{
			hr = FieldDescriptorCoAllocCopy(s_rgMessageCredProvFieldDescriptors[dwIndex], ppcpfd);
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}

	return hr;
}

// We only use one tile at any given time since the system can either be "connected" or 
// "disconnected". If we decided that there were multiple valid ways to be connected with
// different sets of credentials, we would provide a combobox in the "connected" tile so
// that the user could pick which one they want to use.
// The last cred prov used gets to select the default user tile
HRESULT CSampleProvider::GetCredentialCount(
	__out DWORD* pdwCount,
	__out_range(< , *pdwCount) DWORD* pdwDefault,
	__out BOOL* pbAutoLogonWithDefault
)
{

	*pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
	*pbAutoLogonWithDefault = FALSE;

	if (_fRecreateEnumeratedCredentials)
	{
		_fRecreateEnumeratedCredentials = false;
		_ReleaseEnumeratedCredentials();
		_CreateEnumeratedCredentials();
	}

	*pdwCount = 1;

	return S_OK;
}

// Returns the credential at the index specified by dwIndex. This function is called
// to enumerate the tiles. Note that we need to return the right credential, which depends
// on whether we're connected or not.
HRESULT CSampleProvider::GetCredentialAt(
	__in DWORD dwIndex,
	__deref_out ICredentialProviderCredential** ppcpc
)
{

	HRESULT hr;

	// Make sure the parameters are valid.
	if (ppcpc)//(dwIndex == 0) && 
	{
		if (_pCommandWindow->GetConnectedStatus())
		{
			hr = _pCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));

		}
		else
		{
			hr = _pMessageCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));

		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Boilerplate method to create an instance of our provider. 
HRESULT CSample_CreateInstance(__in REFIID riid, __in void** ppv)
{
	HRESULT hr;

	CSampleProvider* pProvider = new CSampleProvider();

	if (pProvider)
	{
		hr = pProvider->QueryInterface(riid, ppv);
		pProvider->Release();

	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}
