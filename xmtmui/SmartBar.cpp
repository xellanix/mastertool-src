#include "pch.h"
#include "SmartBar.h"
#if __has_include("SmartBar.g.cpp")
#include "SmartBar.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::xmtmui::implementation
{
    SmartBar::SmartBar()
    {
        InitializeComponent();

        SetAvailableDrives();
    }

    void SmartBar::SetAvailableDrives()
    {
        std::vector<Windows::Foundation::IInspectable> arrayOfDrives;
        arrayOfDrives.push_back(box_value(L"All Drives"));

        wchar_t* szDrives = new wchar_t[MAX_PATH]();
        if (GetLogicalDriveStringsW(MAX_PATH, szDrives))
        {
            for (size_t i = 0; i < MAX_PATH; i++)
            {
                if (szDrives[i] != (wchar_t)0)
                {
                    std::wstring driveLetter{ szDrives + i };

                    wchar_t szVolumeName[MAX_PATH];
                    if (GetVolumeInformationW(driveLetter.c_str(), szVolumeName, MAX_PATH, NULL, NULL, NULL, NULL, 0))
                    {
                        std::wstring volname{ szVolumeName };
                        if (volname.empty())
                        {
                            volname = L"Local Disk";
                        }

                        volname += L" (" + driveLetter.substr(0, driveLetter.size() - 1) + L")";

                        arrayOfDrives.push_back(box_value(volname));
                    }
                    else
                    {
                        arrayOfDrives.push_back(box_value(driveLetter.substr(0, driveLetter.size() - 1)));
                    }

                    i += driveLetter.size();
                }
            }

            delete[] szDrives;
        }

        m_drives = single_threaded_observable_vector<Windows::Foundation::IInspectable>(std::move(arrayOfDrives));
    }

    void SmartBar::PageLoaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        using namespace std::chrono_literals;

        LoadSettings();

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf1, current_buf2;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\SmartBar.xsmf").wstring().c_str(), &current_buf1);
                auto res2 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf").wstring().c_str(), &current_buf2);

                if (res1 == 0 || res2 == 0)
                {
                    if ((current_buf1.st_ctime != last_ctime1 ||
                        current_buf1.st_mtime != last_mtime1 ||
                        current_buf1.st_size != last_size1) ||
                        (current_buf2.st_ctime != last_ctime2 ||
                        current_buf2.st_mtime != last_mtime2 ||
                        current_buf2.st_size != last_size2))
                    {
                        LoadSettings();

                        last_ctime1 = current_buf1.st_ctime;
                        last_mtime1 = current_buf1.st_mtime;
                        last_size1 = current_buf1.st_size;

                        last_ctime2 = current_buf2.st_ctime;
                        last_mtime2 = current_buf2.st_mtime;
                        last_size2 = current_buf2.st_size;
                    }
                }
            });
        }
        fileChecker.Start();
    }

    void SmartBar::Pageunloaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        fileChecker.Stop();
        fileChecker_revoker.~event_revoker();
        fileChecker = nullptr;

        if (m_drives)
        {
            m_drives.Clear();
            m_drives = nullptr;
        }

        settings.~XSMF();
        backbehav.~XSMF();
    }

    void SmartBar::NavToBackBehav(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        auto nav{ find_ancestor<Microsoft::UI::Xaml::Controls::NavigationView>(*this) };
        if (!nav) return;

        for (auto&& item : nav.FooterMenuItems())
        {
            if (auto menu{ item.try_as<Microsoft::UI::Xaml::Controls::NavigationViewItem>() })
            {
                auto menutag{ unbox_value_or<hstring>(menu.Tag(), L"") };
                if (menutag != L"Background Behaviors") continue;

                nav.SelectedItem(menu);
                break;
            }
        }
    }

    void SmartBar::EnableSmartBarChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableSmartBar = EnableSmartBar().IsOn();
        SaveSettings(L"9be6b091-b18a-5022-9144-217e2d1183d6", m_enableSmartBar);
    }

    void SmartBar::EnableAppsChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableApps = EnableApps().IsOn();
        SaveSettings(L"099a6edf-dc24-5bfd-95e9-f70ef1bc2936", m_enableApps);
    }

    void SmartBar::EnableKeyShortChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        // Win + Alt + S
        if (!isloaded) return;

        m_enableKeyShort = EnableKeyShort().IsOn();
        SaveSettings(L"2906b355-8fd0-5f72-b494-71d6fb8e3be1", m_enableKeyShort);
    }

    void SmartBar::EnableFoldersChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableFolders = EnableFolders().IsOn();
        SaveSettings(L"4b18963f-3180-5943-8416-ad07ef9cffb8", m_enableFolders);
    }

    void SmartBar::FoldersDrivePathChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) return;
        if (!m_drives) return;
        if (m_drives.Size() == 0) return;

        m_folderDrive = (uint32_t)FoldersDrivePath().SelectedIndex();
        SaveSettings(L"16c4ac8e-fbd4-547f-9002-9892cfad574e", m_folderDrive);
    }

    void SmartBar::EnableFilesChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableFiles = EnableFiles().IsOn();
        SaveSettings(L"3ee29586-94b7-58d8-84a6-ee4da971680d", m_enableFiles);
    }

    void SmartBar::FilesDrivePathChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) return;
        if (!m_drives) return;
        if (m_drives.Size() == 0) return;

        m_fileDrive = (uint32_t)FilesDrivePath().SelectedIndex();
        SaveSettings(L"69be0cf6-a495-5159-8798-adfc689ef468", m_fileDrive);
    }

    void SmartBar::EnableKeyLookupChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
    }

    void SmartBar::LoadSettings()
    {
        isloaded = false;

        if (backbehav.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf"))
        {
            try
            {
                m_enableSmartBar = (backbehav >> L"9be6b091-b18a-5022-9144-217e2d1183d6").try_as<bool>(false);
            }
            catch (...)
            {
            }

            EnableSmartBar().IsOn(m_enableSmartBar);
        }
        else
        {
            backbehav[L"9be6b091-b18a-5022-9144-217e2d1183d6"] = m_enableSmartBar;
        }

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\SmartBar.xsmf"))
        {
            try
            {
                m_enableKeyShort = (settings >> L"2906b355-8fd0-5f72-b494-71d6fb8e3be1").try_as<bool>(true);
                m_enableApps = (settings >> L"099a6edf-dc24-5bfd-95e9-f70ef1bc2936").try_as<bool>(true);
                m_enableFolders = (settings >> L"4b18963f-3180-5943-8416-ad07ef9cffb8").try_as<bool>(true);
                m_folderDrive = (settings >> L"16c4ac8e-fbd4-547f-9002-9892cfad574e").try_as<uint32_t>(0);
                m_enableFiles = (settings >> L"3ee29586-94b7-58d8-84a6-ee4da971680d").try_as<bool>(true);
                m_fileDrive = (settings >> L"69be0cf6-a495-5159-8798-adfc689ef468").try_as<uint32_t>(0);
            }
            catch (...)
            {
            }

            EnableKeyShort().IsOn(m_enableKeyShort);
            EnableApps().IsOn(m_enableApps);
            EnableFolders().IsOn(m_enableFolders);
            EnableFiles().IsOn(m_enableFiles);
            
            if (m_drives)
            {
                const auto ds = m_drives.Size();
                auto i_folder = (int32_t)m_folderDrive, i_file = (int32_t)m_fileDrive;

                FoldersDrivePath().SelectedIndex(ds > m_folderDrive ? i_folder : 0);
                FilesDrivePath().SelectedIndex(ds > m_fileDrive ? i_file : 0);
            }
        }
        else
        {
            settings[L"2906b355-8fd0-5f72-b494-71d6fb8e3be1"] = m_enableKeyShort;
            settings[L"099a6edf-dc24-5bfd-95e9-f70ef1bc2936"] = m_enableApps;
            settings[L"4b18963f-3180-5943-8416-ad07ef9cffb8"] = m_enableFolders;
            settings[L"16c4ac8e-fbd4-547f-9002-9892cfad574e"] = m_folderDrive;
            settings[L"3ee29586-94b7-58d8-84a6-ee4da971680d"] = m_enableFiles;
            settings[L"69be0cf6-a495-5159-8798-adfc689ef468"] = m_fileDrive;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        SmartBar::SaveSettings(std::wstring const& name, T const& value)
    {
        if (name == L"9be6b091-b18a-5022-9144-217e2d1183d6")
        {
            backbehav[name] = value;
            backbehav.Write(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf");
            get_ftime(L"Settings\\BackBehav.xsmf", last_ctime2, last_mtime2, last_size2);
        }
        else
        {
            settings[name] = value;

            settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\SmartBar.xsmf");
            get_ftime(L"Settings\\SmartBar.xsmf", last_ctime1, last_mtime1, last_size1);
        }
    }
}
