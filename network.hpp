#ifndef NEXTLIB_NETWORK_HPP
#define NEXTLIB_NETWORK_HPP

#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>

using std::string;
using std::runtime_error;

class Socket {
public:
    Socket() {
        // Initialize socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw (runtime_error("Failed to initialize socket!"));
        }
    }

    void Bind(string ip, int port) {
        if (ip == "") {
            this->ip = "0.0.0.0";
        } else {
            this->ip = ip;
        }
        this->port = port;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(this->port);
        inet_pton(AF_INET, this->ip.c_str(), &hint.sin_addr);

        if (bind(sock, /*AF_INET,*/ (sockaddr*)&hint, sizeof(hint)) == -1) {
            throw (runtime_error("Failed to bind socket!"));
        }
    }

    void Listen() {
        if (listen(sock, SOMAXCONN) == -1) {
            throw (runtime_error("Failed to listen socket connections!"));
        }
    }

    int Accept() {
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);
        // char host[NI_MAXHOST];
        // char svc[NI_MAXSERV];

        int clientSocket = accept(sock, (sockaddr*)&client, &clientSize);
        if (clientSocket == -1) {
            throw (runtime_error("Failed to accept client connection!"));
        }

        return clientSocket;
    }

    void Connect(string ip, int port) {
        this->ip = ip;
        this->port = port;

        hint.sin_family = AF_INET;
        hint.sin_port = htons(this->port);
        inet_pton(AF_INET, this->ip.c_str(), &hint.sin_addr);

        conn = connect(sock, (sockaddr*)&hint, sizeof(hint));
        if (conn == -1) {
            throw (runtime_error("Failed to connect!"));
        }
    }

    void Send(string data) {
        int sendRes = send(sock, data.c_str(), data.size() + 1, 0);
        if (sendRes == -1) {
            throw (runtime_error("Failed to send data to server!"));
        }
    }

    void Send(int socket, string data) {
        int sendRes = send(socket, data.c_str(), data.size() + 1, 0);
        if (sendRes == -1) {
            throw (runtime_error("Failed to send data to client socket!"));
        }
    }

    string Receive() {
        memset(buffer, 0, 8192);
        int bytesReceived = recv(sock, buffer, 8192, 0);
        return string(buffer, bytesReceived);
    }

    string Receive(int socket) {
        memset(buffer, 0, 8192);
        int bytesReceived = recv(socket, buffer, 8192, 0);
        if (bytesReceived == -1) {
            throw (runtime_error("Connection issue occured, while trying receive data"));
        } else if (bytesReceived == 0) {
            throw (runtime_error("Client has been disconnected"));
        }
        return string(buffer, 0, bytesReceived);
    }

    void Close() {
        close(sock);
    }
private:
    int sock, port, conn;
    string ip;
    sockaddr_in hint;
    char buffer[8192];
    char host[NI_MAXHOST], svc[NI_MAXSERV];
};

#endif // NEXTLIB_NETWORK_HPP