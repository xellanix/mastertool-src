// MasterTools.cpp : Defines the entry point for the application.
//

#pragma comment(lib, "Wininet.lib")

#include "framework.h"
#include "MasterTools.h"
#include <locale>
#include <shellapi.h>
#include "../../../Libraries/utf8cvt.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
const auto constexpr szMutexWindow = L"com_xellanix_mastertools_5ef055c1-f34c-44ec-a94f-027a8d43fdb7";
const auto constexpr szWindowClass = L"com_xellanix_mastertools";
const auto constexpr szTitle = L"Xellanix MasterTools";
WinUI::App hostApp{ nullptr };
WinUI::DesktopWindowXamlSource _desktopWindowXamlSource{ nullptr };
WinUI::MainWindow _mainWindow{ nullptr };

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

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MASTERTOOLS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    if (FindWindow(szWindowClass, szTitle) == nullptr)
    {
        hostApp.DeleteUpdateFile();
    }

    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
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

        _mainWindow.StartArguments(GetCommandLineW());

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
            if (_mainWindow != nullptr)
            {
                _mainWindow.TryInstallUpdate();
            }

            PostQuitMessage(0);
            if (_desktopWindowXamlSource != nullptr)
            {
                _desktopWindowXamlSource.Close();
                _desktopWindowXamlSource = nullptr;
            }
            break;
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