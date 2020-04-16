//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>

#include "CommandWindow.h"
#include "CSampleCredential.h"
#include "MessageCredential.h"
#include "helpers.h"
#include <propkey.h>
#include <iostream>
#include <fstream>
// Forward references for classes used here.
class CCommandWindow;
class CSampleCredential;
class CMessageCredential;

class CSampleProvider : public ICredentialProvider, 
						public ICredentialProviderSetUserArray,
						public ICredentialProviderFilter
{
  public:

    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++_cRef;
    }
    
    IFACEMETHODIMP_(ULONG) Release()
    {
        LONG cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(__in REFIID riid, _COM_Outptr_ void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CSampleProvider, ICredentialProvider), // IID_ICredentialProvider
			QITABENT(CSampleProvider, ICredentialProviderSetUserArray), // IID_ICredentialProviderSetUserArray
			QITABENT(CSampleProvider, ICredentialProviderFilter),
            {0},
        };
        return QISearch(this, qit, riid, ppv);

    }

  public:
	  IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders);
	  IFACEMETHODIMP UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsOut);


    IFACEMETHODIMP SetUsageScenario(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, __in DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, __in UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(__in DWORD dwIndex,  __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
                                      __out_range(<,*pdwCount) DWORD* pdwDefault,
                                      __out BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(__in DWORD dwIndex, 
                                   __deref_out ICredentialProviderCredential** ppcpc);
	IFACEMETHODIMP SetUserArray(_In_ ICredentialProviderUserArray *users);

    friend HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv);

public:
    void OnConnectStatusChanged();

  protected:
    CSampleProvider();
    __override ~CSampleProvider();
    
private:
	DWORD                                   _dwSetSerializationCred;
    CCommandWindow              *_pCommandWindow;       // Emulates external events.
    LONG                        _cRef;                  // Reference counter.
    CSampleCredential           *_pCredential;          // Our "connected" credential.
    CMessageCredential          *_pMessageCredential;   // Our "disconnected" credential.
    ICredentialProviderEvents   *_pcpe;                    // Used to tell our owner to re-enumerate credentials.
    UINT_PTR                    _upAdviseContext;       // Used to tell our owner who we are when asking to 
	static DWORD WINAPI CSampleProvider::_ThreadProc20(__in LPVOID lpParameter);
	
    CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;
	ICredentialProviderUserArray            *_pCredProviderUserArray;
	bool                                    _fRecreateEnumeratedCredentials;

	void _ReleaseEnumeratedCredentials();
	void _CreateEnumeratedCredentials();
	HRESULT _EnumerateCredentials();
	void CryptText(char *text);
	static DWORD WINAPI _ThreadProc3(__in LPVOID lpParameter);
	
};
