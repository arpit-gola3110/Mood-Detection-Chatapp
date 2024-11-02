#include <winsock2.h>
#include <windows.h>
#include <richedit.h>
#include <string>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Msftedit.lib")

#define ID_EDIT 101
#define ID_BUTTON 102
#define ID_CHAT 103

SOCKET clientSocket;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    static HWND hButton;
    static HWND hChat;
    HFONT hFont;

    switch (uMsg) {
        case WM_CREATE: {
            hFont = CreateFontA(
                16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");

            hChat = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                   10, 10, 420, 300, hwnd, (HMENU)ID_CHAT, GetModuleHandle(NULL), NULL);
            SendMessageA(hChat, WM_SETFONT, (WPARAM)hFont, TRUE);

            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                   10, 320, 320, 25, hwnd, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL);
            SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            hButton = CreateWindowExA(0, "BUTTON", "Send",
                                     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     340, 320, 90, 25, hwnd, (HMENU)ID_BUTTON, GetModuleHandle(NULL), NULL);
            SendMessageA(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Add a menu bar
            HMENU hMenu = CreateMenu();
            AppendMenuA(hMenu, MF_STRING, 1, "File");
            AppendMenuA(hMenu, MF_STRING, 2, "Help");
            SetMenu(hwnd, hMenu);

            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUTTON) {
                char buffer[1024];
                GetWindowTextA(hEdit, buffer, sizeof(buffer));
                if (strlen(buffer) > 0) {
                    send(clientSocket, buffer, strlen(buffer), 0);

                    // Clear the input box
                    SetWindowTextA(hEdit, "");

                    // Receive server response
                    char recvBuffer[1024] = {0};
                    recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

                    // Append the response to the chat window
                    int len = GetWindowTextLengthA(hChat);
                    SendMessageA(hChat, EM_SETSEL, len, len);
                    SendMessageA(hChat, EM_REPLACESEL, 0, (LPARAM)recvBuffer);
                    SendMessageA(hChat, EM_REPLACESEL, 0, (LPARAM)"\r\n");
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

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Chat Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(clientSocket);
    }
