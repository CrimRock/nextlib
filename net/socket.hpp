#ifndef NEXTLIB_SOCKET_HPP
#define NEXTLIB_SOCKET_HPP

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
#else
#error "Platform not supported."
#endif

#include <nextlib/error.hpp>
#include <string>
#include <string.h>

namespace Next
{
    namespace Net
    {
        class Socket {
        public:
            Socket() {
                // Initialize socket
#ifdef __unix__
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock == -1) {
                    throw (Exception("Failed to initialize socket!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                WSAData wsData;
                WORD ver = MAKEWORD(2, 2);
                int wsOk = WSAStartup(ver, &wsData);
                if (wsOk != 0) {
                    throw (Exception("Failed to initialize winsock!"));
                }

                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock == INVALID_SOCKET) {
                    throw (Exception("Failed to initialize socket!"));
                    WSACleanup();
                }
#endif
            }

            void Bind(std::string ip, int port) {
                if (ip == "") {
                    ip = "0.0.0.0";
                }
                hint.sin_family = AF_INET;
                hint.sin_port = htons(port);
                
#ifdef __unix__
                inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);
                if (bind(sock, (sockaddr*)&hint, sizeof(hint)) == -1) {
                    throw (Exception("Failed to bind socket!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                hint.sin_addr.s_addr = inet_addr(ip.c_str());
                if (bind(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
                    throw (Exception("Failed to bind socket!"));
                }
#endif
            }

            void Listen() {
#ifdef __unix__
                if (listen(sock, SOMAXCONN) == -1) {
                    throw (Exception("Failed to listen socket connections!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
                    throw (Exception("Failed to listen socket connections!"));
                }
#endif
            }

            void Listen(int backlog) {
#ifdef __unix__
                if (listen(sock, backlog) == -1) {
                    throw (Exception("Failed to listen socket connections!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                if (listen(sock, backlog) == SOCKET_ERROR) {
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

            void Connect(std::string ip, int port) {
                hint.sin_family = AF_INET;
                hint.sin_port = htons(port);

                int conn = connect(sock, (sockaddr*)&hint, sizeof(hint));
#ifdef __unix__
                inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);
                if (conn == -1) {
                    throw (Exception("Failed to connect!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                hint.sin_addr.s_addr = inet_addr(ip.c_str());
                if (conn == SOCKET_ERROR) {
                    throw (Exception("Failed to connect!"));
                }
#endif
            }

            void Send(std::string data) {
                int sendRes = send(sock, data.c_str(), data.size() + 1, 0);
#ifdef __unix__
                if (sendRes == -1) {
                    throw (Exception("Failed to send data to server!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                if (sendRes == SOCKET_ERROR) {
                    throw (Exception("Failed to send data to server!"));
                }
#endif
            }

            void Send(int socket, std::string data) {
                int sendRes = send(socket, data.c_str(), data.size() + 1, 0);
#ifdef __unix__
                if (sendRes == -1) {
                    throw (Exception("Failed to send data to client socket!"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                if (sendRes == SOCKET_ERROR) {
                    throw (Exception("Failed to send data to server!"));
                }
#endif
            }

            std::string Receive() {
                memset(buffer, 0, 8192);
                int bytesReceived = recv(sock, buffer, 8192, 0);
                return std::string(buffer, bytesReceived);
            }

            std::string Receive(int socket) {
                memset(buffer, 0, 8192);
                int bytesReceived = recv(socket, buffer, 8192, 0);
#ifdef __unix__
                if (bytesReceived == -1) {
                    throw (Exception("Connection issue occured, while trying receive data"));
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
                if (bytesReceived == SOCKET_ERROR) {
                    throw (Exception("Connection issue occured, while trying receive data"));
                }
#endif
                else if (bytesReceived == 0) {
                    throw (Exception("Client has been disconnected"));
                }
                return std::string(buffer, 0, bytesReceived);
            }

#ifdef __unix__
            void Close() {
                close(sock);
            }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            void Close(bool clean = false) {
                closesocket(sock);
                if (clean == true) {
                    WSACleanup();
                }
            }
#endif
        private:
#ifdef __unix__
            int sock;
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            SOCKET sock;
#endif
            sockaddr_in hint;
            char buffer[8192];
            char host[NI_MAXHOST], svc[NI_MAXSERV];
        };
    }
}

#endif // NEXTLIB_SOCKET_HPP