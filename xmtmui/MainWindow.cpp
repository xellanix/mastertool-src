#include "pch.h"
#include "MainWindow.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <Psapi.h>
#include <tlhelp32.h>

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace muxc
{
    using namespace Microsoft::UI::Xaml::Controls;
}

#define itows(x, y) winrt::unbox_value_or<winrt::hstring>(x, y).c_str()

namespace winrt::xmtmui::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        InitWindowHandle();

        InitPages();
    }

    void MainWindow::InitPages()
    {
        m_pages[L"All Features"] = xaml_typename<xmtmui::AllFeatures>();
        m_pages[L"Battery Manager"] = xaml_typename<xmtmui::BatteryManager>();
        m_pages[L"Image Merger"] = xaml_typename<xmtmui::ImageMerger>();
        m_pages[L"Mouse Focus"] = xaml_typename<xmtmui::MouseFocus>();
        m_pages[L"Quick Rename"] = xaml_typename<xmtmui::QuickRename>();
        m_pages[L"Smart Bar"] = xaml_typename<xmtmui::SmartBar>();
        m_pages[L"Background Behaviors"] = xaml_typename<xmtmui::BackBehav>();
        m_pages[L"About"] = xaml_typename<xmtmui::About>();
    }

    void MainWindow::InitWindowHandle()
    {
        winrt::com_ptr<Xellanix::Desktop::ICoreWindowInterop> interop{};
        try
        {
            winrt::check_hresult(winrt::get_unknown(Windows::UI::Core::CoreWindow::GetForCurrentThread())->QueryInterface(interop.put()));
            winrt::check_hresult(interop->get_WindowHandle(&Xellanix::Desktop::WindowHandle));
        }
        catch (...)
        {
            Xellanix::Desktop::WindowHandle = ::FindWindow(L"com_xellanix_mastertools", L"Xellanix MasterTools");
        }
    }

    void MainWindow::StartNavigate(std::wstring navItemTag, Windows::UI::Xaml::Media::Animation::NavigationTransitionInfo const& transitionInfo, Windows::Foundation::IInspectable const& parameters)
    {
        if (navItemTag == L"") return;

        Interop::TypeName pageTypeName = m_pages[navItemTag];

        const std::wstring pageName{ pageTypeName.Name };

        if (auto navView{ MainNavView() })
        {
            if (pageName == L"xmtmui.AllFeatures")
            {
                navView.AlwaysShowHeader(false);
            }
            else
            {
                if (!navView.AlwaysShowHeader())
                {
                    navView.AlwaysShowHeader(true);
                }
            }
        }

        auto prev = ContentFrame().CurrentSourcePageType();
        if (pageName != L"" && prev.Name != pageName)
        {
            if (auto contentFrame{ ContentFrame() })
            {
                auto cacheTemp = contentFrame.CacheSize();
                contentFrame.CacheSize(0);
                contentFrame.CacheSize(cacheTemp);

                contentFrame.Navigate(pageTypeName, parameters, transitionInfo);
            }
        }
    }

    void MainWindow::NavigatePage(muxc::NavigationView const& /*sender*/, muxc::NavigationViewSelectionChangedEventArgs const& args)
    {
        if (auto selected{ args.SelectedItemContainer() })
        {
            if (!is_pass_params)
            {
                if (passed_files)
                {
                    passed_files.Clear();
                    passed_files = nullptr;
                }
            }

            StartNavigate(itows(selected.Tag(), L""), Media::Animation::EntranceNavigationTransitionInfo(),
                          is_pass_params ? box_value(passed_files) : nullptr);
            
            is_pass_params = false;
        }
    }

    void MainWindow::NavViewLoaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        MainNavView().SelectedItem(MainNavView().MenuItems().GetAt(feature_id));

        [=]() -> fire_and_forget { co_await UpdateMemUsage(); }();
    }

    bool GetWorkingSetPriavte(size_t& retval)
    {
        size_t ws_private = 0;
        size_t ws_shareable = 0;
        size_t ws_shared = 0;

        DWORD number_of_entries = 4096;  // Just a guess.
        PSAPI_WORKING_SET_INFORMATION* buffer = NULL;
        int retries = 5;
        for (;;)
        {
            DWORD buffer_size = sizeof(PSAPI_WORKING_SET_INFORMATION) +
                (number_of_entries * sizeof(PSAPI_WORKING_SET_BLOCK));

            // if we can't expand the buffer, don't leak the previous
            // contents or pass a NULL pointer to QueryWorkingSet
            PSAPI_WORKING_SET_INFORMATION* new_buffer =
                reinterpret_cast<PSAPI_WORKING_SET_INFORMATION*>(
                realloc(buffer, buffer_size));
            if (!new_buffer)
            {
                free(buffer);
                return false;
            }
            buffer = new_buffer;

            // Call the function once to get number of items
            if (QueryWorkingSet(GetCurrentProcess(), buffer, buffer_size))
                break;  // Success

            if (GetLastError() != ERROR_BAD_LENGTH)
            {
                free(buffer);
                return false;
            }

            number_of_entries = static_cast<DWORD>(buffer->NumberOfEntries);

            // Maybe some entries are being added right now. Increase the buffer to
            // take that into account.
            number_of_entries = static_cast<DWORD>(number_of_entries * 1.25);

            if (--retries == 0)
            {
                free(buffer);  // If we're looping, eventually fail.
                return false;
            }
        }

        // On windows 2000 the function returns 1 even when the buffer is too small.
        // The number of entries that we are going to parse is the minimum between the
        // size we allocated and the real number of entries.
        number_of_entries =
            (std::min)(number_of_entries, static_cast<DWORD>(buffer->NumberOfEntries));
        for (unsigned int i = 0; i < number_of_entries; i++)
        {
            if (buffer->WorkingSetInfo[i].Shared)
            {
                ws_shareable++;
                if (buffer->WorkingSetInfo[i].ShareCount > 1)
                    ws_shared++;
            }
            else
            {
                ws_private++;
            }
        }

        // 256 = 1024 (MB) / 4 (PAGESIZE_KB)
        retval = ws_private / 256;
        // ws_usage->shareable = ws_shareable * PAGESIZE_KB;
        // ws_usage->shared = ws_shared * PAGESIZE_KB;
        free(buffer);
        return true;
    }
    DWORD GetProcessID()
    {
        PROCESSENTRY32 pt{};
        HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        pt.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hsnap, &pt))
        { // must call this first
            do
            {
                if (std::wstring(pt.szExeFile) == L"MasterTools.exe")
                {
                    CloseHandle(hsnap);
                    return pt.th32ProcessID;
                }
            } while (Process32Next(hsnap, &pt));
        }
        CloseHandle(hsnap); // close handle on failure
        return 0;
    }

    Windows::Foundation::IAsyncAction MainWindow::UpdateMemUsage()
    {
        using namespace std::chrono_literals;

        co_await resume_background();

        bool m_limitMem = true;
        while (true)
        {
            if (m_limitMem)
            {
                if (size_t memThis = 0; GetWorkingSetPriavte(memThis))
                {
                    uint64_t m_memLimit = 80;
                    if (Xellanix::Objects::XSMF settings{ L"BackBehav" }; settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf"))
                    {
                        try
                        {
                            m_limitMem = (settings >> L"2345f93f-5245-555f-b1f0-dd752a5c991f").try_as<bool>(true);
                            m_memLimit = (std::max)((settings >> L"24c503d1-3656-5d59-be48-e4e394e6cec0").try_as<uint64_t>(80), 30ui64);
                        }
                        catch (...)
                        {
                        }
                    }

                    if (m_limitMem)
                    {
                        if (memThis >= m_memLimit)
                        {
                            auto exepath = fs::path(Xellanix::Utilities::AppDir) / L"rstrm\\rstrm.exe";
                            const auto rstrPath = Xellanix::Utilities::LocalTemp / L"rstrm";
                            STARTUPINFO si;
                            PROCESS_INFORMATION pi;
                            ZeroMemory(&si, sizeof(si));
                            si.cb = sizeof(si);
                            ZeroMemory(&pi, sizeof(pi));
                            //Prepare CreateProcess args
                            const std::wstring app_w = exepath.wstring();
                            const std::wstring arg_w = (L"\"" + app_w + L"\" \"" + std::to_wstring(GetProcessID()) + L"\" \"" + (fs::path(Xellanix::Utilities::AppDir) / L"MasterTools.exe").wstring() + L"\" \"" + rstrPath.wstring() + L"\"");

                            if (CreateProcess(app_w.c_str(), const_cast<wchar_t*>(arg_w.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                            {
                                // Success
                                MessageBeep(MB_ICONERROR);

                                WaitForSingleObject(pi.hProcess, INFINITE);

                                CloseHandle(pi.hProcess);
                                CloseHandle(pi.hThread);

                                if (auto rstrF = (rstrPath / L"15f1b877-4e1a-5532-9bb7-d2d9c9005f72.temp"); Xellanix::Utilities::CheckExist(rstrF))
                                {
                                    fs::remove(rstrF);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            co_await resume_after(5s);
        }
    }
}
