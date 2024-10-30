#include <winsock2.h>
#include <windows.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define ID_EDIT 101
#define ID_BUTTON 102
#define ID_CHAT 103

SOCKET clientSocket;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    static HWND hButton;
    static HWND hChat;

    switch (uMsg) {
        case WM_CREATE: {
            hChat = CreateWindowEx(0, "EDIT", "",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                   10, 10, 400, 200, hwnd, (HMENU)ID_CHAT, GetModuleHandle(NULL), NULL);
            hEdit = CreateWindowEx(0, "EDIT", "",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                   10, 220, 300, 25, hwnd, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL);
            hButton = CreateWindowEx(0, "BUTTON", "Send",
                                     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     320, 220, 90, 25, hwnd, (HMENU)ID_BUTTON, GetModuleHandle(NULL), NULL);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUTTON) {
                char buffer[1024];
                GetWindowText(hEdit, buffer, sizeof(buffer));
                if (strlen(buffer) > 0) {
                    send(clientSocket, buffer, strlen(buffer), 0);

                    // Clear the input box
                    SetWindowText(hEdit, "");

                    // Receive server response
                    char recvBuffer[1024] = {0};
                    recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

                    // Append the response to the chat window
                    int len = GetWindowTextLength(hChat);
                    SendMessage(hChat, EM_SETSEL, len, len);
                    SendMessage(hChat, EM_REPLACESEL, 0, (LPARAM)recvBuffer);
                    SendMessage(hChat, EM_REPLACESEL, 0, (LPARAM)"\r\n");
                }
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    const char CLASS_NAME[] = "ChatClient";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Chat Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

