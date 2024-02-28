#pragma once

#include "MouseFocus.g.h"
#include "Utilities.h"
#include "winrt/Windows.UI.Xaml.Media.h"
#include "winrt/Windows.UI.Xaml.Input.h"
#include "../../../Libraries/xellanix.objects.h"

namespace winrt::xmtmui::implementation
{
    struct MouseFocus : MouseFocusT<MouseFocus>
    {
    private:
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

        std::wstring to_hex(Windows::UI::Color const& color)
        {
            const unsigned long hexValue = (color.R << 16) + (color.G << 8) + color.B;

            // We'll size this string to accommodate "#XXXXXX" - i.e., a full RGB number with a # sign.
            std::wostringstream wss;
            wss << std::uppercase << L"#" << std::setfill(L'0') << std::setw(6) << std::hex << hexValue;

            return wss.str();
        }
        Windows::UI::Color from_hex(std::wstring const& hexStr)
        {
            const std::wstring nohash = hexStr[0] == L'#' ? hexStr.substr(1) : hexStr;
            unsigned long hexV = 0;
            {
                std::wstringstream wss;
                wss << nohash;
                wss >> std::hex >> hexV;
            }

            const byte r = static_cast<byte>((hexV & 0x00ff0000) >> 16);
            const byte g = static_cast<byte>((hexV & 0x0000ff00) >> 8);
            const byte b = static_cast<byte>(hexV & 0x000000ff);

            return Windows::UI::ColorHelper::FromArgb(255, r, g, b);
        }

        bool m_enableMouseFocus = false;
        unsigned short m_autoHighlight = 4;
        bool m_enableKeyShort = true;
        bool m_enableMouseShake = true;
        double m_shakeSensitivity = 7.0;
        unsigned short m_shakeDuration = 2000;
        unsigned short m_overlayOpacity = 75;
        std::wstring m_overlayColor = L"#000000";
        std::wstring m_helperColor = L"#FFFFFF";

        Windows::UI::Xaml::DispatcherTimer::Tick_revoker fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer fileChecker{};

        bool isloaded = false;
        time_t last_ctime1 = 0, last_ctime2 = 0, last_mtime1 = 0, last_mtime2 = 0;
        long long last_size1 = 0, last_size2 = 0;
        Xellanix::Objects::XSMF settings{ L"Image Merger" }, backbehav{ L"BackBehav" };
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);

    public:
        MouseFocus();

        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void Pageunloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void NavToBackBehav(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableMouseFocusChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void AutoHighlightChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void EnableKeyShortChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void EnableMouseShakeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void ShakeSensitivityChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void ShakeDurationChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void OverlayOpacityChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void OverlayColorChanged(winrt::Microsoft::UI::Xaml::Controls::ColorPicker const& sender, winrt::Microsoft::UI::Xaml::Controls::ColorChangedEventArgs const& args);
        void HelperColorChanged(winrt::Microsoft::UI::Xaml::Controls::ColorPicker const& sender, winrt::Microsoft::UI::Xaml::Controls::ColorChangedEventArgs const& args);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct MouseFocus : MouseFocusT<MouseFocus, implementation::MouseFocus>
    {
    };
}
