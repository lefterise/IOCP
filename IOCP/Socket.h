#include "CompletionPort.h"

class Socket {
public:
    struct SocketOverlapped : public WSAOVERLAPPED {
        enum { ReceiveOperation, SendOperation}  operationType;
        WSABUF dataBuffer;
    };

    Socket(CompletionPort& completionPort) : socketHandle(INVALID_SOCKET) {
        socketHandle = socket(AF_INET, SOCK_STREAM, 0);
        if (socketHandle == INVALID_SOCKET) {
            throw makeWinsockException("Unable to create socket");
        }

        completionPort.AssociateSocket(socketHandle, (ULONG_PTR)this);
    }

    ~Socket() {
        closesocket(socketHandle);
    }

    bool Connect(const char* ipAddress, int port) {
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr);

        return (connect(socketHandle, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0);
    }

    bool AsyncRecv(char* buffer, int bufferSize) {        
        DWORD flags = 0;
        memset(&receiveOverlapped, 0, sizeof(receiveOverlapped));
        receiveOverlapped.dataBuffer.buf = buffer;
        receiveOverlapped.dataBuffer.len = bufferSize;        
        receiveOverlapped.operationType = SocketOverlapped::ReceiveOperation;

        return WSARecv(socketHandle, &receiveOverlapped.dataBuffer, 1, NULL, &flags, &receiveOverlapped, NULL) == 0;
    }

    bool AsyncSend(char* buffer, int bufferSize) {
        memset(&sendOverlapped, 0, sizeof(sendOverlapped));
        sendOverlapped.dataBuffer.buf = buffer;
        sendOverlapped.dataBuffer.len = bufferSize;        
        sendOverlapped.operationType = SocketOverlapped::SendOperation;

        return WSASend(socketHandle, &sendOverlapped.dataBuffer, 1, NULL, 0, &sendOverlapped, NULL) == 0;
    }

    SOCKET GetSocketHandle() const {
        return socketHandle;
    }

private:
    SOCKET socketHandle;
    SocketOverlapped receiveOverlapped;
    SocketOverlapped sendOverlapped;
};