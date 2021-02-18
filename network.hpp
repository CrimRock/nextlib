#ifndef NEXTLIB_NETWORK_HPP
#define NEXTLIB_NETWORK_HPP

#ifdef __unix__
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define IsWindows
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <nextlib/error.hpp>
#include <string>
#include <string.h>

using std::string;

class Socket {
public:
    Socket() {
        // Initialize socket
        #ifdef IsWindows
        WSAData wsData;
        WORD ver = MAKEWORD(2, 2);
        int wsOk = WSAStartup(ver, &wsData);
        if (wsOk != 0) {
            throw (Exception("Failed to intialize winsock!"));
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            throw (Exception("Failed to create socket!"));
            WSACleanup();
        }
        #else
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw (Exception("Failed to initialize socket!"));
        }
        #endif
    }

    void Bind(string ip, int port) {
        if (ip == "") {
            ip = "0.0.0.0";
        }
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);
        
        #ifdef IsWindows
        // InetPtonW(AF_INET, ip.c_str(), &hint.sin_addr);
        hint.sin_addr.s_addr = inet_addr(ip.c_str());
        if (bind(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
            throw (Exception("Failed to bind socket!"));
        }
        #else
        inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);
        if (bind(sock, (sockaddr*)&hint, sizeof(hint)) == -1) {
            throw (Exception("Failed to bind socket!"));
        }
        #endif
    }

    void Listen() {
        #ifdef IsWindows
        if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
            throw (Exception("Failed to listen socket connections!"));
        }
        #else
        if (listen(sock, SOMAXCONN) == -1) {
            throw (Exception("Failed to listen socket connections!"));
        }
        #endif
    }

    int Accept() {
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        int clientSocket = accept(sock, (sockaddr*)&client, &clientSize);
        if (clientSocket == -1) {
            throw (Exception("Failed to accept client connection!"));
        }

        return clientSocket;
    }

    void Connect(string ip, int port) {
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);

        int conn = connect(sock, (sockaddr*)&hint, sizeof(hint));
        #ifdef IsWindows
        hint.sin_addr.s_addr = inet_addr(ip.c_str());
        if (conn == SOCKET_ERROR) {
            throw (Exception("Failed to connect!"));
        }
        #else
        inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);
        if (conn == -1) {
            throw (Exception("Failed to connect!"));
        }
        #endif
    }

    void Send(string data) {
        int sendRes = send(sock, data.c_str(), data.size() + 1, 0);
        #ifdef IsWindows
        if (sendRes == SOCKET_ERROR) {
            throw (Exception("Failed to send data to server!"));
        }
        #else
        if (sendRes == -1) {
            throw (Exception("Failed to send data to server!"));
        }
        #endif
    }

    void Send(int socket, string data) {
        int sendRes = send(socket, data.c_str(), data.size() + 1, 0);
        #ifdef IsWindows
        if (sendRes == SOCKET_ERROR) {
            throw (Exception("Failed to send data to server!"));
        }
        #else
        if (sendRes == -1) {
            throw (Exception("Failed to send data to client socket!"));
        }
        #endif
    }

    string Receive() {
        memset(buffer, 0, 8192);
        int bytesReceived = recv(sock, buffer, 8192, 0);
        return string(buffer, bytesReceived);
    }

    string Receive(int socket) {
        memset(buffer, 0, 8192);
        int bytesReceived = recv(socket, buffer, 8192, 0);
        #ifdef IsWindows
        if (bytesReceived == SOCKET_ERROR) {
            throw (Exception("Connection issue occured, while trying receive data"));
        }
        #else
        if (bytesReceived == -1) {
            throw (Exception("Connection issue occured, while trying receive data"));
        }
        #endif
        else if (bytesReceived == 0) {
            throw (Exception("Client has been disconnected"));
        }
        return string(buffer, 0, bytesReceived);
    }

    #ifdef IsWindows
    void Close(bool clean = false) {
        closesocket(sock);
        if (clean == true) {
            WSACleanup();
        }
    }
    #else
    void Close() {
        close(sock);
    }
    #endif
private:
    #ifdef IsWindows
    SOCKET sock;
    #else
    int sock;
    #endif
    sockaddr_in hint;
    char buffer[8192];
    char host[NI_MAXHOST], svc[NI_MAXSERV];
};

#endif // NEXTLIB_NETWORK_HPP