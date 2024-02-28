#include "pch.h"
#include "BackBehav.h"
#if __has_include("BackBehav.g.cpp")
#include "BackBehav.g.cpp"
#endif
#include <tlhelp32.h>

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::xmtmui::implementation
{
    BackBehav::BackBehav()
    {
        InitializeComponent();
    }

    void BackBehav::PageLoaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        using namespace std::chrono_literals;

        LoadSettings();

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf1;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf").wstring().c_str(), &current_buf1);

                if (res1 == 0)
                {
                    if (current_buf1.st_ctime != last_ctime1 ||
                        current_buf1.st_mtime != last_mtime1 ||
                        current_buf1.st_size != last_size1)
                    {
                        LoadSettings();

                        last_ctime1 = current_buf1.st_ctime;
                        last_mtime1 = current_buf1.st_mtime;
                        last_size1 = current_buf1.st_size;
                    }
                }
            });
        }
        fileChecker.Start();
    }

    void BackBehav::Pageunloaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        fileChecker.Stop();
        fileChecker_revoker.~event_revoker();
        fileChecker = nullptr;

        settings.~XSMF();
    }

    void BackBehav::EnableBackBehavChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        bool enabled = EnableBackBehav().IsOn();
        if (!enabled)
        {
            []() -> bool
            {
                HANDLE hProcessSnap;
                PROCESSENTRY32 pe32{};

                // Take a snapshot of all processes in the system.
                hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (hProcessSnap == INVALID_HANDLE_VALUE)
                {
                    return false;
                }

                // Set the size of the structure before using it.
                pe32.dwSize = sizeof(PROCESSENTRY32);

                // Retrieve information about the first process,
                // and exit if unsuccessful
                if (!Process32First(hProcessSnap, &pe32))
                {
                    CloseHandle(hProcessSnap);  // clean the snapshot object
                    return false;
                }

                // Now walk the snapshot of processes 
                do
                {
                    std::wstring str(pe32.szExeFile);

                    if (str == L"backbehav.exe") // put the name of your process you want to kill
                    {
                        auto TerminateMyProcess = [](DWORD dwProcessId, UINT uExitCode) -> bool
                        {
                            DWORD dwDesiredAccess = PROCESS_TERMINATE;
                            BOOL  bInheritHandle = FALSE;
                            HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
                            if (hProcess == NULL)
                                return false;

                            bool result = TerminateProcess(hProcess, uExitCode);

                            CloseHandle(hProcess);

                            return result;
                        };

                        TerminateMyProcess(pe32.th32ProcessID, 1);
                    }
                } while (Process32Next(hProcessSnap, &pe32));

                CloseHandle(hProcessSnap);
                return true;
            }();
        }

        if (!isloaded) return;

        m_enableBackBehav = enabled;
        SaveSettings(L"c638ee81-4390-58eb-b7d5-16772e2281b6", m_enableBackBehav);
    }

    void BackBehav::RunBackBehavClicked(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        ShellExecute(NULL, NULL, (fs::path(Xellanix::Utilities::AppDir) / L"backbehav.exe").wstring().c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    void BackBehav::EnableStartupChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        auto enabled = EnableStartup().IsOn();
        if (enabled == m_enableStartup) return;

        if (enabled)
        {
            const std::wstring progPath = (fs::path(Xellanix::Utilities::AppDir) / L"backbehav.exe").wstring();
            HKEY hkey = NULL;
            auto status = RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &hkey, NULL);
            if (status == ERROR_SUCCESS)
            {
                DWORD cbData = static_cast<DWORD>((progPath.size() + 1) * sizeof(wchar_t));
                status = RegSetValueExW(hkey, L"Xellanix MasterTools", 0, REG_SZ, (BYTE*)progPath.c_str(), cbData);
                if (status != ERROR_SUCCESS)
                {
                    enabled = m_enableStartup;
                    EnableStartup().IsOn(m_enableStartup);
                }
            }
            else
            {
                enabled = m_enableStartup;
                EnableStartup().IsOn(m_enableStartup);
            }
        }
        else
        {
            HKEY hkey = NULL;
            auto status = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0L, KEY_SET_VALUE, &hkey);
            if (status == ERROR_SUCCESS)
            {
                status = RegDeleteValueW(hkey, L"Xellanix MasterTools");
                if (status != ERROR_SUCCESS)
                {
                    enabled = m_enableStartup;
                    EnableStartup().IsOn(m_enableStartup);
                }
            }
            else
            {
                enabled = m_enableStartup;
                EnableStartup().IsOn(m_enableStartup);
            }
        }

        if (!isloaded) return;

        m_enableStartup = enabled;
        SaveSettings(L"171f8db8-2fd6-5fc0-b4d0-05a70b8178a1", m_enableStartup);
    }

    void BackBehav::LimitMemChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_limitMem = LimitMem().IsChecked().Value();
        SaveSettings(L"2345f93f-5245-555f-b1f0-dd752a5c991f", m_limitMem);
    }

    void BackBehav::MemLimitChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = (std::max)(uint64_t(std::round(sender.Text() == L"" ? args.OldValue() : sender.Value())), 30ui64);
        sender.Value((double)val);

        m_memLimit = val;

        SaveSettings(L"24c503d1-3656-5d59-be48-e4e394e6cec0", m_memLimit);
    }

    void BackBehav::EnableBatteryManagerChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableMouseFocus = EnableMouseFocus().IsOn();
        SaveSettings(L"bdec9ff0-3b01-536c-a4d3-e50bd337640d", m_enableMouseFocus);
    }

    void BackBehav::EnableMouseFocusChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableMouseFocus = EnableMouseFocus().IsOn();
        SaveSettings(L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9", m_enableMouseFocus);
    }

    void BackBehav::EnableSmartBarChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableSmartBar = EnableSmartBar().IsOn();
        SaveSettings(L"9be6b091-b18a-5022-9144-217e2d1183d6", m_enableSmartBar);
    }

    void BackBehav::LoadSettings()
    {
        isloaded = false;

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf"))
        {
            try
            {
                m_enableBackBehav = (settings >> L"c638ee81-4390-58eb-b7d5-16772e2281b6").try_as<bool>(false);
                m_enableStartup = (settings >> L"171f8db8-2fd6-5fc0-b4d0-05a70b8178a1").try_as<bool>(false);
                m_limitMem = (settings >> L"2345f93f-5245-555f-b1f0-dd752a5c991f").try_as<bool>(true);
                m_memLimit = (std::max)((settings >> L"24c503d1-3656-5d59-be48-e4e394e6cec0").try_as<uint64_t>(80), 30ui64);

                m_enableBatteryManager = (settings >> L"bdec9ff0-3b01-536c-a4d3-e50bd337640d").try_as<bool>(true);
                m_enableMouseFocus = (settings >> L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9").try_as<bool>(false);
                m_enableSmartBar = (settings >> L"9be6b091-b18a-5022-9144-217e2d1183d6").try_as<bool>(true);
            }
            catch (...)
            { }

            EnableBackBehav().IsOn(m_enableBackBehav);
            EnableStartup().IsOn(m_enableStartup);
            LimitMem().IsChecked(m_limitMem);
            MemLimit().Value((double)m_memLimit);
            EnableBatteryManager().IsOn(m_enableBatteryManager);
            EnableMouseFocus().IsOn(m_enableMouseFocus);
            EnableSmartBar().IsOn(m_enableSmartBar);
        }
        else
        {
            settings[L"c638ee81-4390-58eb-b7d5-16772e2281b6"] = m_enableBackBehav;
            settings[L"171f8db8-2fd6-5fc0-b4d0-05a70b8178a1"] = m_enableStartup;
            settings[L"2345f93f-5245-555f-b1f0-dd752a5c991f"] = m_limitMem;
            settings[L"24c503d1-3656-5d59-be48-e4e394e6cec0"] = m_memLimit;

            settings[L"bdec9ff0-3b01-536c-a4d3-e50bd337640d"] = m_enableBatteryManager;
            settings[L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9"] = m_enableMouseFocus;
            settings[L"9be6b091-b18a-5022-9144-217e2d1183d6"] = m_enableSmartBar;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        BackBehav::SaveSettings(std::wstring const& name, T const& value)
    {
        settings[name] = value;

        settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf");
        get_ftime(L"Settings\\BackBehav.xsmf", last_ctime1, last_mtime1, last_size1);
    }
}
