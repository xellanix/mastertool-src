// contextmenumanager.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "contextmenumanager.h"
#include <vector>
#include <string>
#include <filesystem>
#include <shellapi.h>

// Global Variables:
HANDLE m_singleInstanceMutex;
HINSTANCE hInst;                                // current instance
constexpr auto const szTitle = L"contextmenumanager";                  // The title bar text
constexpr auto const szWindowClass = L"contextmenumanager";            // the main window class name

LPTSTR* Args;
int ArgsCount;
std::wstring arguments;
constexpr unsigned long long imagemergercode = 4'179'420;
constexpr unsigned long long quickrenamecode = 4'474'170;
constexpr unsigned long long IDT_TIMER1 = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
std::filesystem::path GetAppDir();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    MyRegisterClass(hInstance);

    Args = CommandLineToArgvW(GetCommandLine(), &ArgsCount);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    LocalFree(Args);
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = szWindowClass;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    m_singleInstanceMutex = CreateMutex(NULL, TRUE, L"com_xellanix_mastertools_contextmenumgr_5ef055c1-f34c-44ec-a94f-027a8d43fdb7");
    if (m_singleInstanceMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND existingApp = FindWindow(szWindowClass, szTitle);
        if (existingApp)
        {
            LPCTSTR lpszString = Args[2];
            COPYDATASTRUCT cds;
            {
                std::wstring argt = Args[1];
                if (argt == L"--imgr")
                {
                    cds.dwData = imagemergercode;
                }
                else if (argt == L"--qrnm")
                {
                    cds.dwData = quickrenamecode;
                }
                else
                {
                    cds.dwData = 0;
                }
            }
            cds.cbData = static_cast<DWORD>(sizeof(TCHAR) * (_tcslen(lpszString) + 1));
            cds.lpData = (void*)lpszString;
            SendMessage(existingApp, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cds);
            SetForegroundWindow(existingApp);
        }
        ReleaseMutex(m_singleInstanceMutex);
        LocalFree(Args);
        return FALSE; // Exit the app.
    }

    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, szWindowClass, szTitle, WS_POPUP,
                                0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    {
        arguments = Args[1];
        arguments += L" \"";
        arguments += Args[2];
        arguments += L"\"";
    }
    SetTimer(hWnd, IDT_TIMER1, 1000, 0);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COPYDATA:
        {
            auto pcds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
            if (pcds->dwData == imagemergercode)
            {
                KillTimer(hWnd, IDT_TIMER1);

                LPCTSTR lpszString = reinterpret_cast<LPCTSTR>(pcds->lpData);
                arguments += L" \"";
                arguments += lpszString;
                arguments += L"\"";

                SetTimer(hWnd, IDT_TIMER1, 1000, 0);
            }
            else if (pcds->dwData == quickrenamecode)
            {
                KillTimer(hWnd, IDT_TIMER1);

                LPCTSTR lpszString = reinterpret_cast<LPCTSTR>(pcds->lpData);
                arguments += L" \"";
                arguments += lpszString;
                arguments += L"\"";

                SetTimer(hWnd, IDT_TIMER1, 1000, 0);
            }
            else
            {

            }
            break;
        }
        case WM_TIMER:
        {
            KillTimer(hWnd, IDT_TIMER1);
            ShellExecuteW(NULL, L"open", (GetAppDir() / L"MasterTools.exe").wstring().c_str(), arguments.c_str(), 0, SW_SHOWNORMAL);
            PostQuitMessage(0);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

std::filesystem::path GetAppDir()
{
    DWORD path_buffer_size = MAX_PATH;
    std::unique_ptr<WCHAR[]> path_buf{ new WCHAR[path_buffer_size] };

    while (true)
    {
        const auto bytes_written = GetModuleFileName(NULL, path_buf.get(), path_buffer_size);
        const auto last_error = GetLastError();

        if (bytes_written == 0)
        {
            return std::filesystem::path{};
        }

        if (last_error == ERROR_INSUFFICIENT_BUFFER)
        {
            path_buffer_size *= 2;
            path_buf.reset(new WCHAR[path_buffer_size]);
            continue;
        }

        auto path = std::wstring{ path_buf.get() };
        if (auto found = path.find_last_of(L"/\\"); found != std::wstring::npos)
        {
            path = path.substr(0, found);
        }

        return std::filesystem::path(path);
    }
}