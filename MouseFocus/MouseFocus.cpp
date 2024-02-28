// MouseFocus.cpp : Defines the entry point for the application.
//

#pragma comment (lib,"Gdiplus.lib")

#include "framework.h"
#include "MouseFocus.h"
#include <string>
#include <array>
#include <tuple>
#include <ObjIdl.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <thread>
#include "../../../Libraries/xellanix.objects.h"
#include "../xmtmui/Utilities.h"

using namespace Gdiplus;
using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HANDLE m_singleInstanceMutex;
HWND mainWindow;
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
constexpr auto const szWindowMutex = L"com_xellanix_mastertools_mousefocus_8a6c5352-ceb1-4886-9fad-1c08eabe7516";

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void SetTransparency(HWND hwnd, BYTE alpha);
void DisplayText(Gdiplus::Color backColor, Gdiplus::Color frontColor, std::array<tuple<wstring, wstring, REAL, REAL>, 2> const& messages, HWND hwnd, HDC hdc);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    hInst = GetModuleHandle(NULL);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOUSEFOCUS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    m_singleInstanceMutex = CreateMutex(NULL, TRUE, szWindowMutex);
    if (m_singleInstanceMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND existingApp = FindWindow(szWindowClass, szTitle);
        if (existingApp)
        {
            SetForegroundWindow(existingApp);
        }
        ReleaseMutex(m_singleInstanceMutex);
        return FALSE; // Exit the app.
    }

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        ReleaseMutex(m_singleInstanceMutex);
        return FALSE;
    }

    thread([]()
    {
        while (true)
        {
            SetForegroundWindow(mainWindow);
        }
    }).detach();

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
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOUSEFOCUS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    mainWindow = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, szWindowClass, szTitle, WS_POPUP,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

    if (!mainWindow)
    {
        return FALSE;
    }

    // SetTransparency(hWnd, 128);
    ShowWindow(mainWindow, SW_SHOWMAXIMIZED);
    UpdateWindow(mainWindow);

    Gdiplus::Color backColor, frontColor;
    {
        unsigned short opacity = 75ui16;
        std::wstring overlayColor = L"#000000", helperColor = L"#FFFFFF";

        Xellanix::Objects::XSMF settings{};

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf"))
        {
            try
            {
                opacity = (std::min)((settings >> L"9f29036f-319d-5799-9b89-10cff10624a3").try_as<unsigned short>(75ui16), 100ui16);
                overlayColor = (settings >> L"63a2ea58-7586-5dd4-8de1-46ff08c9c8a3").try_as<std::wstring>(L"#000000");
                helperColor = (settings >> L"68dace38-7e85-5bbf-b3dd-667a0419d1ea").try_as<std::wstring>(L"#FFFFFF");
            }
            catch (...)
            {
            }
        }

        auto from_hex = [](BYTE alpha, std::wstring const& hexStr)
        {
            const std::wstring nohash = hexStr[0] == L'#' ? hexStr.substr(1) : hexStr;
            unsigned long hexV = 0;
            {
                std::wstringstream wss;
                wss << nohash;
                wss >> std::hex >> hexV;
            }

            const BYTE r = static_cast<BYTE>((hexV & 0x00ff0000) >> 16);
            const BYTE g = static_cast<BYTE>((hexV & 0x0000ff00) >> 8);
            const BYTE b = static_cast<BYTE>(hexV & 0x000000ff);

            return Gdiplus::Color(alpha, r, g, b);
        };

        backColor = from_hex((BYTE)(255.0 * (double)opacity / 100.0), overlayColor);
        frontColor = from_hex(255, helperColor);
    }

    RECT bounds;
    GetClientRect(mainWindow, &bounds);
    DisplayText(backColor, frontColor,
                {
                    make_tuple(L"Your Mouse Cursor Is There!", L"Segoe UI Semibold", 36.0f, 100.0f * (bounds.bottom - bounds.top) / 1080),
                    make_tuple(L"Press Esc or Alt+F4 to close this overlay", L"Segoe UI Semilight", 24.0f, 10.0f)
                },
                mainWindow, GetDC(mainWindow));

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
            
            //std::wstring mesText{ L"Your Mouse Cursor Is There!" };
            //DrawText(hdc, mesText.c_str(), mesText.size(), &bounds, DT_CENTER | DT_VCENTER);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_KEYUP:
        {
            auto vkCode = LOWORD(wParam);
            switch (vkCode)
            {
                case VK_ESCAPE:
                {
                    DestroyWindow(hWnd);
                    break;
                }
            }

            break;
        }
        case WM_DESTROY:
        {
            ReleaseMutex(m_singleInstanceMutex);

            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void SetTransparency(HWND hwnd, BYTE alpha)
{
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

// BYTE alpha_byte = (BYTE)(255 * 0.75);
// backColor =  Gdiplus::Color(alpha_byte, 0, 0, 0)
// frontColor = Gdiplus::Color(255, 255, 255, 255)
void DisplayText(Gdiplus::Color backColor, Gdiplus::Color frontColor, std::array<tuple<wstring, wstring, REAL, REAL>, 2> const& messages, HWND hwnd, HDC hdc)
{
    RECT client;
    GetClientRect(hwnd, &client);
    auto const MAX_WIDTH = client.right - client.left, MAX_HEIGHT = client.bottom - client.top;

    Bitmap softwareBitmap(MAX_WIDTH, MAX_HEIGHT, PixelFormat32bppARGB);
    Graphics g(&softwareBitmap);

    g.Clear(backColor);  

    RectF out(0.0f, 0.0f, 0.0f, 0.0f);
    for (auto const& [message, ff, fs, pos] : messages)
    {
        // Segoe UI Semibold
        FontFamily fontfamily(ff.c_str());
        Font font(&fontfamily, fs, FontStyleRegular, UnitPixel);
        RectF rectF((REAL)client.left, out.GetBottom() + pos, (REAL)client.right, (REAL)client.bottom);
        StringFormat stringFormat;
        SolidBrush solidBrush(frontColor);

        stringFormat.SetAlignment(StringAlignmentCenter);

        g.DrawString(message.c_str(), -1, &font, rectF, &stringFormat, &solidBrush);
        g.MeasureString(message.c_str(), -1, &font, rectF, &out);
    }
    
    HBITMAP bmp;
    softwareBitmap.GetHBITMAP(Color(0, 0, 0, 0), &bmp);

    HDC memdc = CreateCompatibleDC(hdc);
    HGDIOBJ original = SelectObject(memdc, bmp);

    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    POINT ptLocation = { 0, 0 };
    SIZE szWnd = { MAX_WIDTH, MAX_HEIGHT };
    POINT ptSrc = { 0, 0 };
    BOOL l = UpdateLayeredWindow(hwnd, hdc, &ptLocation, &szWnd, memdc, &ptSrc, 0, &blend, ULW_ALPHA);
    int err = GetLastError();
    SelectObject(hdc, original);

    DeleteObject(bmp);
    DeleteObject(memdc);
}
