#pragma once

#include "SmartBar.g.h"
#include "Utilities.h"
#include "../../../Libraries/xellanix.objects.h"
#include "winrt/Windows.UI.Xaml.Media.h"

namespace winrt::xmtmui::implementation
{
    struct SmartBar : SmartBarT<SmartBar>
    {
    private:
        using ObservableIns = Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable>;

        template <typename ancestor_type>
        auto find_ancestor(::winrt::Windows::UI::Xaml::DependencyObject root) noexcept
        {
            auto ancestor{ root.try_as<ancestor_type>() };
            while (!ancestor && root)
            {
                root = ::winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(root);
                ancestor = root.try_as<ancestor_type>();
            }
            return ancestor;
        }

        Windows::UI::Xaml::DispatcherTimer::Tick_revoker fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer fileChecker{};

        bool m_enableSmartBar = true;
        bool m_enableKeyShort = true;
        bool m_enableApps = true;
        bool m_enableFolders = true;
        uint32_t m_folderDrive = 0;
        bool m_enableFiles = true;
        uint32_t m_fileDrive = 0;

        bool isloaded = false;
        time_t last_ctime1 = 0, last_ctime2 = 0, last_mtime1 = 0, last_mtime2 = 0;
        long long last_size1 = 0, last_size2 = 0;
        Xellanix::Objects::XSMF settings{ L"Smart Bar" }, backbehav{ L"BackBehav" };
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);

        ObservableIns m_drives;
        void SetAvailableDrives();

    public:
        SmartBar();

        ObservableIns AvailableDrives() { return m_drives; }
        void AvailableDrives(ObservableIns const& value)
        {
            if (m_drives != value)
            {
                m_drives = value;
            }
        }

        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void Pageunloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void NavToBackBehav(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableSmartBarChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableKeyShortChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void EnableAppsChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableFoldersChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void FoldersDrivePathChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void EnableFilesChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void FilesDrivePathChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void EnableKeyLookupChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct SmartBar : SmartBarT<SmartBar, implementation::SmartBar>
    {
    };
}
