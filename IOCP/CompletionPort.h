#pragma once
#include "ErrorMessage.h"

class CompletionPort {
public:
	CompletionPort() {
		iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (iocpHandle == nullptr) {
            throw makeWinsockException("Unable to create competion port");
        }
	}

    ~CompletionPort() {        
         CloseHandle(iocpHandle);        
    }
    
    void AssociateSocket(SOCKET socket, ULONG_PTR completionKey) {
        CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), iocpHandle, completionKey, 0);
    }

    bool GetCompletionStatus(DWORD& bytesRead, ULONG_PTR& completionKey, LPOVERLAPPED& overlapped, DWORD timeout = INFINITE) {
        return GetQueuedCompletionStatus(iocpHandle, &bytesRead, &completionKey, &overlapped, timeout);
    }

    void PostCompletion(ULONG_PTR completionKey, DWORD numBytes = 0, LPOVERLAPPED overlapped = nullptr) {
        PostQueuedCompletionStatus(iocpHandle, numBytes, completionKey, overlapped);
    }

    HANDLE GetHandle() const {
        return iocpHandle;
    }
private:
    HANDLE iocpHandle;
};

