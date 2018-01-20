#include <iostream>
#include <string.h>
#ifdef _WIN32
#include <Winsock2.h>
#endif
#pragma comment(lib, "ws2_32.lib")
int main()
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);
    /* ... */
    WSACleanup();
    return 0;
}