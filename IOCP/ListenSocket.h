#pragma once
//https://learn.microsoft.com/en-us/windows/win32/api/mswsock/nf-mswsock-acceptex

#include "CompletionPort.h"

class ListenSocket {
public:
    ListenSocket(CompletionPort& completionPort) : completionPort(completionPort){
        listenSocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (listenSocketHandle == INVALID_SOCKET) {
            throw makeWinsockException("Unable to create listen socket");
        }

        loadAcceptExFunction();

        completionPort.AssociateSocket(listenSocketHandle, (ULONG_PTR)this);
    }

    void bind(int port) {
        sockaddr_in service;

        service.sin_family = AF_INET;
        service.sin_addr.S_un.S_un_b = { 0, 0, 0, 0 }; //Any source ip
        service.sin_port = htons(port);

        if (::bind(listenSocketHandle, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
            throw makeWinsockException("Unable to bind socket");
        }
    }

    void listen() {
        if (::listen(listenSocketHandle, 100) == SOCKET_ERROR) {
            throw makeWinsockException("Unable to listen");
        }
    }

    void accept(SOCKET acceptingSocketHandle) {
        memset(&acceptOverlapped, 0, sizeof(acceptOverlapped));
        DWORD dwBytes;

        BOOL hasSucceded = acceptEx(listenSocketHandle, acceptingSocketHandle, &acceptBuffer,
            0,
            sizeof(PaddedAddress),
            sizeof(PaddedAddress),
            &dwBytes, &acceptOverlapped);

        if (hasSucceded == FALSE) {
            throwIfNotIoPending("Accept failed");
        }
    }

    ~ListenSocket() {
        closesocket(listenSocketHandle);
    }
    
private:
    void loadAcceptExFunction() {
        GUID GuidAcceptEx = WSAID_ACCEPTEX;
        DWORD dwBytes;
        if (WSAIoctl(listenSocketHandle, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &GuidAcceptEx, sizeof(GuidAcceptEx),
            &acceptEx, sizeof(acceptEx),
            &dwBytes, NULL, NULL) == SOCKET_ERROR) {

            throw makeWinsockException("Unable to load AcceptEx function");
        }
    }
    CompletionPort& completionPort;
    LPFN_ACCEPTEX acceptEx = NULL;
    SOCKET listenSocketHandle = INVALID_SOCKET;
    WSAOVERLAPPED acceptOverlapped;
    

    struct PaddedAddress {
        sockaddr_in address;
        char padding[16];
    };

    struct AcceptBuffer {
        PaddedAddress local;
        PaddedAddress remote;
    };

    AcceptBuffer acceptBuffer;
};