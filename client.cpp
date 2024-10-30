#include <iostream>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    std::string message = "I am happy!";
    send(sock, message.c_str(), message.length(), 0);
    recv(sock, buffer, 1024, 0);

    std::cout << "Server response: " << buffer << std::endl;

    closesocket(sock);
    WSACleanup();
    return 0;
}
