#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <exception>
#include <string>

inline std::string winsockErrorToString(int errorNumber)
{
    auto format = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK;
    LPSTR buffer = NULL;

    FormatMessageA(
        format,
        0,
        errorNumber,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer,
        0,
        NULL);

    std::string message = buffer;

    LocalFree(buffer);
    return message;
}

inline std::exception makeWinsockException(const std::string& message) {
    return std::exception((message + " " + winsockErrorToString(WSAGetLastError())).c_str());
}

inline void throwIfNotIoPending(const std::string& message) {
    auto err = WSAGetLastError();
    if (err == ERROR_IO_PENDING)
        return;
    throw std::exception((message + " " + winsockErrorToString(err)).c_str());
}
