#pragma once

#include "BackBehav.g.h"
#include "Utilities.h"
#include "../../../Libraries/xellanix.objects.h"

namespace winrt::xmtmui::implementation
{
    struct BackBehav : BackBehavT<BackBehav>
    {
    private:
        bool m_enableBackBehav = false;
        bool m_enableStartup = false;
        bool m_limitMem = true;
        uint64_t m_memLimit = 80;
        bool m_enableBatteryManager = true;
        bool m_enableMouseFocus = false;
        bool m_enableSmartBar = true;

        Windows::UI::Xaml::DispatcherTimer::Tick_revoker fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer fileChecker{};

        bool isloaded = false;
        time_t last_ctime1 = 0, last_mtime1 = 0;
        long long last_size1 = 0;
        Xellanix::Objects::XSMF settings{ L"BackBehav" };
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);

    public:
        BackBehav();

        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void Pageunloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableBackBehavChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void RunBackBehavClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableStartupChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void LimitMemChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void MemLimitChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);

        void EnableBatteryManagerChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableMouseFocusChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableSmartBarChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct BackBehav : BackBehavT<BackBehav, implementation::BackBehav>
    {
    };
}
