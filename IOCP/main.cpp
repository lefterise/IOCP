#include "ListenSocket.h"
#include "CompletionPort.h"
#include "Socket.h"
#include <iostream>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	try{
		CompletionPort completionPort;
		ListenSocket listenSocket(completionPort);
		Socket client(completionPort);
		listenSocket.bind(80);
		listenSocket.listen();
		listenSocket.accept(client.GetSocketHandle());

		DWORD bytesRead;
		ULONG_PTR completionKey;
		LPOVERLAPPED pOverlapped;

		completionPort.GetCompletionStatus(bytesRead, completionKey, pOverlapped);//get rid of outparams
		if (completionKey == (ULONG_PTR)&listenSocket) {
			std::cout << "Accepted" << std::endl;
		}

		char buffer[128];
		client.AsyncRecv(buffer, 128);
		completionPort.GetCompletionStatus(bytesRead, completionKey, pOverlapped);
		if (completionKey == (ULONG_PTR)&client) {
			std::cout << "Received" << std::endl;
		}

		DWORD bytesSent;
		client.AsyncSend(buffer, bytesRead);
		completionPort.GetCompletionStatus(bytesSent, completionKey, pOverlapped);
		if (completionKey == (ULONG_PTR)&client) {
			std::cout << "Sent" << std::endl;
		}
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}

	WSACleanup();
	return 0;
}