

#include"adapter.h"
using namespace IpTespit;
void Adapter::Refresh()
{
	WinExec("netsh interface set interface Ethernet admin=disable",SW_HIDE);
	::Sleep(500);
	WinExec("netsh wlan connect ssid=f@tih name=f@tih",SW_HIDE);
	//system("timeout /t 2");
	::Sleep(1000);
	PrintIpAdress();
	WinExec("netsh interface set interface Ethernet admin=enable",SW_HIDE);
}
Adapter::Adapter()
{
	Refresh();
 //   PrintIpAdress();
}
void Adapter::PrintAdapter(PIP_ADAPTER_ADDRESSES aa)

{

	memset(buf, 0, BUFSIZ);
	WideCharToMultiByte(CP_ACP, 0, aa->FriendlyName, wcslen(aa->FriendlyName), buf, BUFSIZ, NULL, NULL);
	adapterName = buf;
//	printf("adapter_name:%s\n", adapterName);
}
void Adapter::PrintAddr(PIP_ADAPTER_UNICAST_ADDRESS ua)

{


		memset(buf, 0, BUFSIZ);
		int family = ua->Address.lpSockaddr->sa_family;
		//printf("\t%s ", family == AF_INET ? "IPv4" : "IPv6");
		getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
		if (family == AF_INET) {
			if (strlen(buf) > 0) {
				strcpy_s(adapterAdress, buf);
			}
			else {
				Refresh();
			}

			//adapterAdress = buf;
		//	printf("%s\n", adapterAdress);
		}

}
bool Adapter::PrintIpAdress()

{
	static int i = 0;
	bool adapterWifi = false;
	memset(buf, 0, BUFSIZ);
	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);
	if (rv != ERROR_BUFFER_OVERFLOW) {
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		return false;
	}
	adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);
	if (rv != ERROR_SUCCESS) {
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		free(adapter_addresses);
		return false;
	}

	for (aa = adapter_addresses; aa != NULL; aa = aa->Next) {
		PrintAdapter(aa);
		if (_stricmp("Wi-Fi", adapterName)==0) {
			for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
				int family = ua->Address.lpSockaddr->sa_family;
				if (family == AF_INET) {
					PrintAddr(ua);
						 break;
				}
			}
			break;
		} 
	}/*
	i++;
	if (adapterWifi != true && i<20) {
		free(adapter_addresses);
		Refresh();
		
	}*/
	free(adapter_addresses);
	return true;
}
char* Adapter::getIpAdress()
{
	return adapterAdress;
}
/*
int main(int argc, char *argv[])
{
	WSAData d;
	if (WSAStartup(MAKEWORD(2, 2), &d) != 0) {
		return -1;
	}
	Adapter *adapter = new Adapter();
	printf("%s\n", adapter->getIpAdress());
	WSACleanup();
	system("pause");
	return 0;
}
*/