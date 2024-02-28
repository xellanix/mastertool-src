// smbr.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "smbr.h"
#include <shellapi.h>
#include "..\..\..\..\Libraries\utf8cvt.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
const auto constexpr szMutexWindow = L"com_xellanix_mastertools_smartbar_5ef055c1-f34c-44ec-a94f-027a8d43fdb7";
const auto constexpr szWindowClass = L"com_xellanix_mastertools_smartbar";
const auto constexpr szTitle = L"Xellanix MasterTools";
WinUI::App hostApp{ nullptr };
WinUI::DesktopWindowXamlSource _desktopWindowXamlSource{ nullptr };
WinUI::MainWindow _mainWindow{ nullptr };
bool isLockWindow = false;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void AdjustLayout(HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    std::locale::global(std::locale(std::locale("C"), new cvt::utf8cvt<wchar_t>()));

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    WinUI::init_apartment(WinUI::apartment_type::single_threaded);
    hostApp = WinUI::App{};
    _desktopWindowXamlSource = WinUI::DesktopWindowXamlSource{};

    // Initialize global strings
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
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

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    MONITORINFO info{};
    {
        info.cbSize = sizeof(MONITORINFO);
        const POINT ptZero = { 0, 0 };
        auto monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);

        if (GetMonitorInfo(monitor, &info) == FALSE)
            return FALSE;
    }
    constexpr short windowWidthNormal = 800, windowHeightNormal = 400;
    HWND hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_POPUP,
                                (info.rcWork.right - windowWidthNormal) / 2, (info.rcWork.bottom - windowHeightNormal) / 2,
                                windowWidthNormal, windowHeightNormal,
                                nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }
    {
        auto iDpi = GetDpiForWindow(hWnd);
        auto scale = (double)iDpi / 96.0;

        const auto windowWidth = (int)std::ceil(windowWidthNormal * scale), windowHeight = (int)std::ceil(windowHeightNormal * scale);
        const auto windowx = (info.rcWork.right - windowWidth) / 2;
        const auto windowy = (info.rcWork.bottom - windowHeight) / 2;
        const auto roundSize = (int)std::ceil(16.0 * scale);
        const auto hrgn = CreateRoundRectRgn(0, 0, windowWidth + 1, windowHeight + 1, roundSize, roundSize);

        SetWindowPos(hWnd, NULL, windowx, windowy, windowWidth, windowHeight, SWP_SHOWWINDOW);
        SetWindowRgn(hWnd, hrgn, TRUE);

        DeleteObject(hrgn);
    }

    // Begin XAML Islands walkthrough code.
    if (_desktopWindowXamlSource != nullptr)
    {
        auto interop = _desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
        WinUI::check_hresult(interop->AttachToWindow(hWnd));
        HWND hWndXamlIsland = nullptr;
        interop->get_WindowHandle(&hWndXamlIsland);
        RECT windowRect;
        ::GetClientRect(hWnd, &windowRect);
        ::SetWindowPos(hWndXamlIsland, NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_SHOWWINDOW);
        _mainWindow = WinUI::MainWindow();
        
        if (auto lockWindow{ _mainWindow.FindName(L"LockWindow").as<WinUI::Primitives::ToggleButton>() })
        {
            lockWindow.Checked([=](auto&&, auto&&)
            {
                isLockWindow = true;
            });
            lockWindow.Unchecked([=](auto&&, auto&&)
            {
                isLockWindow = false;
            });
        }

        _desktopWindowXamlSource.Content(_mainWindow);
    }
    // End XAML Islands walkthrough code.

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            if (_desktopWindowXamlSource != nullptr)
            {
                _desktopWindowXamlSource.Close();
                _desktopWindowXamlSource = nullptr;
            }
            break;
        case WM_ACTIVATE:
        {
            if (isLockWindow) break;

            if (LOWORD(wParam) == WA_INACTIVE)
            {
                TerminateProcess(GetCurrentProcess(), 0);
            }
            break;
        }
        case WM_SIZE:
            AdjustLayout(hWnd);
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void AdjustLayout(HWND hWnd)
{
    if (_desktopWindowXamlSource != nullptr)
    {
        auto interop = _desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
        HWND xamlHostHwnd = NULL;
        WinUI::check_hresult(interop->get_WindowHandle(&xamlHostHwnd));
        RECT windowRect;
        ::GetClientRect(hWnd, &windowRect);
        ::SetWindowPos(xamlHostHwnd, NULL, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_SHOWWINDOW);
    }
}