#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <string>
#include <mutex>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#define DWMWCP_ROUND 2

#define WM_UPDATE_TEXT (WM_USER + 1)
#define TIMER_DOCK_ID 1

HWND g_hwnd = nullptr;
HFONT g_fontBold = nullptr;
HFONT g_fontRegular = nullptr;
HBRUSH g_bgBrush = nullptr;

std::mutex g_mutex;
std::wstring g_lastLine = L"Oczekiwanie na output...";
int g_activePid = -1;

// Szerokość widgetu na pasku zadań (w pikselach)
const int WIDGET_WIDTH = 280;

std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], size);
    return result;
}

// Dokowanie okna wewnątrz paska zadań Windows 11 (po lewej stronie zasobnika systemowego)
void DockToTaskbar() {
    // Ukryj widget, gdy działa gra lub wideo na pełnym ekranie
    QUERY_USER_NOTIFICATION_STATE state;
    if (SUCCEEDED(SHQueryUserNotificationState(&state))) {
        if (state == QUNS_RUNNING_D3D_FULL_SCREEN || state == QUNS_PRESENTATION_MODE) {
            ShowWindow(g_hwnd, SW_HIDE);
            return;
        }
    }

    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!hTaskbar || !IsWindowVisible(hTaskbar)) {
        ShowWindow(g_hwnd, SW_HIDE);
        return;
    }

    RECT rcTaskbar;
    GetWindowRect(hTaskbar, &rcTaskbar);
    int taskbarHeight = rcTaskbar.bottom - rcTaskbar.top;

    // Jeśli pasek zadań jest schowany (auto-ukrywanie), schowaj również widget
    if (rcTaskbar.top >= GetSystemMetrics(SM_CYSCREEN) - 2) {
        ShowWindow(g_hwnd, SW_HIDE);
        return;
    }

    // Wysokość pigułki dopasowana do paska zadań (6 px marginesu od góry i dołu)
    int widgetHeight = taskbarHeight - 12;
    if (widgetHeight < 24) widgetHeight = 24;

    // Znajdź zasobnik systemowy (TrayNotifyWnd), aby umieścić widget tuż przed nim
    HWND hTray = FindWindowExW(hTaskbar, nullptr, L"TrayNotifyWnd", nullptr);
    RECT rcTray;
    int x = rcTaskbar.right - WIDGET_WIDTH - 160;
    if (hTray && GetWindowRect(hTray, &rcTray)) {
        x = rcTray.left - WIDGET_WIDTH - 8; // 8 px odstępu od strzałki zasobnika
    }
    int y = rcTaskbar.top + (taskbarHeight - widgetHeight) / 2;

    SetWindowPos(g_hwnd, HWND_TOPMOST, x, y, WIDGET_WIDTH, widgetHeight,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

DWORD WINAPI PipeServerThread(LPVOID) {
    char buffer[2048];
    while (true) {
        HANDLE hPipe = CreateNamedPipeW(
            L"\\\\.\\pipe\\PsLastLinePipe",
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0, sizeof(buffer), 0, nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(500);
            continue;
        }

        if (ConnectNamedPipe(hPipe, nullptr) || GetLastError() == ERROR_PIPE_CONNECTED) {
            DWORD bytesRead = 0;
            std::string payload;
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                payload.append(buffer, bytesRead);
            }

            while (!payload.empty() && (payload.back() == '\r' || payload.back() == '\n')) {
                payload.pop_back();
            }

            size_t p1 = payload.find('|');
            size_t p2 = (p1 != std::string::npos) ? payload.find('|', p1 + 1) : std::string::npos;

            if (p1 != std::string::npos && p2 != std::string::npos) {
                int pid = std::atoi(payload.substr(0, p1).c_str());
                std::string type = payload.substr(p1 + 1, p2 - p1 - 1);
                std::string text = payload.substr(p2 + 1);

                if (type == "FOCUS") {
                    g_activePid = pid;
                } else if (type == "OUT" && (pid == g_activePid || g_activePid == -1)) {
                    g_activePid = pid;
                    std::wstring wideText = Utf8ToWide(text);
                    {
                        std::lock_guard<std::mutex> lock(g_mutex);
                        g_lastLine = wideText;
                    }
                    PostMessageW(g_hwnd, WM_UPDATE_TEXT, 0, 0);
                }
            }
        }
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER:
            if (wParam == TIMER_DOCK_ID) {
                DockToTaskbar();
            }
            return 0;

        case WM_UPDATE_TEXT:
            InvalidateRect(hwnd, nullptr, TRUE);
            SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
            return 0;

        // Nie zabieraj fokusu z terminala po kliknięciu w widget
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        // Zamknięcie widgetu prawym przyciskiem myszy
        case WM_RBUTTONUP:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);

            // Kolor tła zbliżony do ciemnych pigułek paska zadań Windows 11 (#202024)
            FillRect(hdc, &rc, g_bgBrush);
            SetBkMode(hdc, TRANSPARENT);

            RECT rcPrefix = rc;
            rcPrefix.left += 12;
            SelectObject(hdc, g_fontBold);
            SetTextColor(hdc, RGB(56, 189, 248));
            DrawTextW(hdc, L"PS>", -1, &rcPrefix, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            RECT rcText = rc;
            rcText.left += 44;
            rcText.right -= 12;
            SelectObject(hdc, g_fontRegular);
            SetTextColor(hdc, RGB(244, 244, 245));

            std::wstring currentText;
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                currentText = g_lastLine;
            }
            DrawTextW(hdc, currentText.c_str(), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, TIMER_DOCK_ID);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    g_bgBrush = CreateSolidBrush(RGB(32, 32, 36));
    g_fontBold = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             FIXED_PITCH | FF_MODERN, L"Cascadia Mono");
    g_fontRegular = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                FIXED_PITCH | FF_MODERN, L"Cascadia Mono");

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"PsTaskbarWidgetClass";
    RegisterClassExW(&wc);

    // WS_EX_NOACTIVATE sprawia, że okno zachowuje się jak część paska zadań
    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        wc.lpszClassName, L"PS Taskbar Widget",
        WS_POPUP,
        0, 0, WIDGET_WIDTH, 36,
        nullptr, nullptr, hInstance, nullptr
    );

    // Zaokrąglone rogi DWM i delikatne obramowanie pasujące do stylu Windows 11
    DWORD cornerPref = DWMWCP_ROUND;
    COLORREF borderColor = RGB(60, 60, 67);
    DwmSetWindowAttribute(g_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPref, sizeof(cornerPref));
    DwmSetWindowAttribute(g_hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));

    DockToTaskbar();
    // Odświeżanie pozycji i warstwy Z-Order co 250 ms (utrzymuje widget nad paskiem zadań po kliknięciu)
    SetTimer(g_hwnd, TIMER_DOCK_ID, 250, nullptr);

    CreateThread(nullptr, 0, PipeServerThread, nullptr, 0, nullptr);
    SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(g_fontBold);
    DeleteObject(g_fontRegular);
    DeleteObject(g_bgBrush);
    return 0;
}