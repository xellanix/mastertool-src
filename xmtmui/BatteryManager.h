#pragma once

#include "BatteryManager.g.h"
#include "Utilities.h"
#include "../../../Libraries/xellanix.objects.h"
#include "winrt/Windows.UI.Xaml.Media.h"

namespace winrt::xmtmui::implementation
{
    struct BatteryManager : BatteryManagerT<BatteryManager>
    {
    private:
        Windows::UI::Xaml::DispatcherTimer::Tick_revoker fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer fileChecker{};

        bool isloaded = false;
        time_t last_ctime1 = 0, last_ctime2 = 0, last_mtime1 = 0, last_mtime2 = 0;
        long long last_size1 = 0, last_size2 = 0;
        Xellanix::Objects::XSMF settings{ L"Battery Manager" }, backbehav{ L"BackBehav" };
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);

        bool m_enableBatteryManager = true;
        bool m_useModernBox = true;
        unsigned short m_lowBatPerc = 25ui16;
        unsigned short m_stopChargePerc = 90ui16;

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

    public:
        BatteryManager();

        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void Pageunloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void NavToBackBehav(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void EnableBatteryManagerChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void LowBatPercChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void StopChargePercChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void UseModernBoxChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct BatteryManager : BatteryManagerT<BatteryManager, implementation::BatteryManager>
    {
    };
}
