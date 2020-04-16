#pragma once
#include <iostream>
#include <WinSock2.h>
#include <stdio.h>
#include <string.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
namespace IpTespit {
	class Adapter {
	private:
		char buf[BUFSIZ];
		DWORD rv, size;
		PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
		PIP_ADAPTER_UNICAST_ADDRESS ua;
		void PrintAdapter(PIP_ADAPTER_ADDRESSES aa);
		void PrintAddr(PIP_ADAPTER_UNICAST_ADDRESS ua);
		bool PrintIpAdress();
		char *adapterName;
		void Refresh();
		

	public:
		Adapter();
		char *getIpAdress();
		char adapterAdress[512];
	};
}