#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

map<string, int> moodCount;

string analyzeSentiment(const string& message) {
    if (message.find("happy") != string::npos || message.find("good") != string::npos) {
        return "Positive";
    } else if (message.find("sad") != string::npos || message.find("bad") != string::npos) {
        return "Negative";
    } else {
        return "Neutral";
    }
}

void updateMoodCount(const string& mood) {
    moodCount[mood]++;
}

void printAnalytics() {
    cout << "\n--- Chat Analytics ---\n";
    for (const auto& pair : moodCount) {
        cout << pair.first << ": " << pair.second << " messages\n";
    }
    cout << "----------------------\n";
}

void handleClient(SOCKET clientSocket) {
    char buffer[1024] = {0};
    while (true) {
        int valread = recv(clientSocket, buffer, 1024, 0);
        if (valread <= 0) {
            break;
        }
        string message(buffer);
        string mood = analyzeSentiment(message);
        updateMoodCount(mood);
        string response = "Message: " + message + " (Mood: " + mood + ")\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        printAnalytics();
    }
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int opt = 1;
    int addrlen = sizeof(clientAddr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 3);

    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrlen);
        thread(handleClient, clientSocket).detach();
    }

    WSACleanup();
    return 0;
}
