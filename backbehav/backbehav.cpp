// backbehav.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "backbehav.h"

#include "MouseFocusBehaviors.h"
#include "BatteryManagerBehaviours.h"

// Global Variables:
HANDLE m_singleInstanceMutex;
constexpr WCHAR const szWindowMutex[] = L"com_xellanix_mastertools_backbehav_c4fded98-90b0-47bb-b34f-e8bf6dac8348";

static constexpr auto WM_TRAY = WM_USER + 1;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_TRAY:
        {
            if (lParam == WM_RBUTTONUP)
            {
                POINT p;
                GetCursorPos(&p);

                HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_CONTEXTMENU));
                if (hMenu)
                {
                    HMENU hSubMenu = GetSubMenu(hMenu, 0);
                    if (hSubMenu)
                    {
                        // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
                        SetForegroundWindow(hwnd);

                        // respect menu drop alignment
                        UINT uFlags = TPM_RETURNCMD | TPM_NONOTIFY;
                        if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
                        {
                            uFlags |= TPM_RIGHTALIGN;
                        }
                        else
                        {
                            uFlags |= TPM_LEFTALIGN;
                        }

                        auto cmd = TrackPopupMenuEx(hSubMenu, uFlags, p.x, p.y, hwnd, NULL);
                        SendMessage(hwnd, WM_COMMAND, cmd, 0);
                    }
                    DestroyMenu(hMenu);
                }
                return 0;
            }
            break;
        }
        case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_LAUNCH:
                {
                    ShellExecute(NULL, NULL, (fs::path(Xellanix::Utilities::AppDir) / L"MasterTools.exe").wstring().c_str(), NULL, NULL, SW_SHOWNORMAL);
                    break;
                }
                case IDM_EXIT:
                {
                    PostQuitMessage(0);
                    break;
                }
                default:
                    return DefWindowProc(hwnd, msg, wParam, lParam);
            }
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

std::pair<NOTIFYICONDATA, HWND> SetTray(std::wstring const& identifier, HICON icon, std::wstring const& tip)
{
    WNDCLASSEX windowClass;
    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WndProc;
    windowClass.lpszClassName = identifier.c_str();
    windowClass.hInstance = GetModuleHandle(nullptr);

    if (RegisterClassEx(&windowClass) == 0)
    {
        throw std::runtime_error("Failed to register class");
    }

    // NOLINTNEXTLINE
    HWND hwnd = CreateWindow(identifier.c_str(), nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, windowClass.hInstance,
                             nullptr);
    if (hwnd == nullptr)
    {
        throw std::runtime_error("Failed to create window");
    }

    if (UpdateWindow(hwnd) == 0)
    {
        throw std::runtime_error("Failed to update window");
    }

    NOTIFYICONDATA notifyData;
    memset(&notifyData, 0, sizeof(NOTIFYICONDATA));
    notifyData.cbSize = sizeof(NOTIFYICONDATA);
    notifyData.hWnd = hwnd;
    notifyData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    notifyData.uCallbackMessage = WM_TRAY;
    notifyData.hIcon = icon;
    notifyData.uVersion = NOTIFYICON_VERSION_4;
    wcscpy_s(notifyData.szTip, sizeof(notifyData.szTip), tip.c_str());

    if (Shell_NotifyIcon(NIM_ADD, &notifyData) == FALSE)
    {
        throw std::runtime_error("Failed to register tray icon");
    }

    return std::make_pair(notifyData, hwnd);
}

void DeleteTray(std::wstring const& identifier, NOTIFYICONDATA notifyData, HWND hwnd)
{
    Shell_NotifyIcon(NIM_DELETE, &notifyData);
    DestroyIcon(notifyData.hIcon);

    UnregisterClass(identifier.c_str(), GetModuleHandle(nullptr));
    PostMessage(hwnd, WM_QUIT, 0, 0);

    DestroyIcon(notifyData.hIcon);
}

void MainWork();
void CustomKeyboardHook();

bool IsKeyDown(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool IsBackBehavEnabled()
{
    bool _ = false;

    Xellanix::Objects::XSMF setting;
    if (setting.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf"))
    {
        _ = (setting >> L"c638ee81-4390-58eb-b7d5-16772e2281b6").try_as<bool>(_);
    }

    return _;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    if (!IsBackBehavEnabled()) return FALSE;

    m_singleInstanceMutex = CreateMutex(NULL, TRUE, szWindowMutex);
    if (m_singleInstanceMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        ReleaseMutex(m_singleInstanceMutex);
        return FALSE; // Exit the app.
    }

    auto&& [data, hwnd] = SetTray(szWindowMutex, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BACKBEHAV)), L"Xellanix MasterTools Background Behaviors");

    MainWork();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseMutex(m_singleInstanceMutex);
    DeleteTray(szWindowMutex, data, hwnd);
    return (int)msg.wParam;
}

void MainWork()
{
    // Mouse Focus Section
    std::thread(HighlightCursor).detach();
    std::thread(CustomKeyboardHook).detach();
    std::thread(ActivateMouseFocusWithShake).detach();

    // Battery Manager Section
    std::thread(CheckBatteryPerc).detach();
}

void CustomKeyboardHook()
{
    using namespace std::chrono_literals;

    bool WinAltM = false;

    ActivateMouseFocusKey(WinAltM);
}
