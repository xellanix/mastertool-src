#include "pch.h"
#include "MouseFocus.h"
#if __has_include("MouseFocus.g.cpp")
#include "MouseFocus.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::xmtmui::implementation
{
    MouseFocus::MouseFocus()
    {
        InitializeComponent();
    }

    void MouseFocus::PageLoaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        using namespace std::chrono_literals;

        LoadSettings();

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf1, current_buf2;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf").wstring().c_str(), &current_buf1);
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

    void MouseFocus::Pageunloaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        fileChecker.Stop();
        fileChecker_revoker.~event_revoker();
        fileChecker = nullptr;

        m_overlayColor.~basic_string();
        m_helperColor.~basic_string();

        settings.~XSMF();
        backbehav.~XSMF();
    }

    void MouseFocus::NavToBackBehav(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
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

    void MouseFocus::EnableMouseFocusChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableMouseFocus = EnableMouseFocus().IsOn();
        SaveSettings(L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9", m_enableMouseFocus);
    }

    void MouseFocus::AutoHighlightChanged(Windows::Foundation::IInspectable const& /*sender*/, SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) return;
        
        auto index = AutoHighlight().SelectedIndex();

        m_autoHighlight = (unsigned short)index;
        SaveSettings(L"4651485d-138e-5791-a0a9-940756ba18aa", m_autoHighlight);
    }

    void MouseFocus::EnableKeyShortChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableKeyShort = EnableKeyShort().IsOn();
        SaveSettings(L"80dc8702-ff5e-5df6-bf21-b4c7673de371", m_enableKeyShort);
    }

    void MouseFocus::EnableMouseShakeChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableMouseShake = EnableMouseShake().IsOn();
        SaveSettings(L"5986d0a7-8675-5f8b-adf5-c593e81d387e", m_enableMouseShake);
    }

    void MouseFocus::ShakeSensitivityChanged(muxc::NumberBox const& sender, muxc::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = (sender.Text() == L"" ? args.OldValue() : sender.Value());
        val = (std::min)((std::max)(val, 2.0), 10.0);
        sender.Value(val);

        m_shakeSensitivity = val;

        SaveSettings(L"d549aab1-da90-5d59-8054-3698c1b7e120", m_shakeSensitivity);
    }

    void MouseFocus::ShakeDurationChanged(muxc::NumberBox const& sender, muxc::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = static_cast<unsigned short>(sender.Text() == L"" ? args.OldValue() : sender.Value());
        val = (std::min)((std::max)(val, 500ui16), 3000ui16);
        sender.Value(val);

        m_shakeDuration = val;

        SaveSettings(L"c51ad80d-77f1-509b-8007-5b1a4d980d95", m_shakeDuration);
    }

    void MouseFocus::OverlayOpacityChanged(muxc::NumberBox const& sender, muxc::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = (std::min)(static_cast<unsigned short>(sender.Text() == L"" ? args.OldValue() : sender.Value()), 100ui16);
        sender.Value(val);

        m_overlayOpacity = val;

        SaveSettings(L"9f29036f-319d-5799-9b89-10cff10624a3", m_overlayOpacity);
    }

    void MouseFocus::OverlayColorChanged(muxc::ColorPicker const& sender, muxc::ColorChangedEventArgs const& /*args*/)
    {
        auto color = sender.Color();
        m_overlayColor = to_hex(color);

        OverlayColorPreview().Background(Media::SolidColorBrush(color));
        OverlayColorPreviewText().Text(m_overlayColor);

        if (!isloaded) return;
        SaveSettings(L"63a2ea58-7586-5dd4-8de1-46ff08c9c8a3", m_overlayColor);
    }

    void MouseFocus::HelperColorChanged(muxc::ColorPicker const& sender, muxc::ColorChangedEventArgs const& /*args*/)
    {
        auto color = sender.Color();
        m_helperColor = to_hex(color);

        HelperColorPreview().Background(Media::SolidColorBrush(color));
        HelperColorPreviewText().Text(m_helperColor);

        if (!isloaded) return;
        SaveSettings(L"68dace38-7e85-5bbf-b3dd-667a0419d1ea", m_helperColor);
    }

    void MouseFocus::LoadSettings()
    {
        isloaded = false;

        if (backbehav.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf"))
        {
            try
            {
                m_enableMouseFocus = (backbehav >> L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9").try_as<bool>(false);
            }
            catch (...)
            {
            }

            EnableMouseFocus().IsOn(m_enableMouseFocus);
        }
        else
        {
            backbehav[L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9"] = m_enableMouseFocus;
        }

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf"))
        {
            try
            {
                m_autoHighlight = (std::min)((settings >> L"4651485d-138e-5791-a0a9-940756ba18aa").try_as<unsigned short>(4ui16), 15ui16);
                m_enableKeyShort = (settings >> L"80dc8702-ff5e-5df6-bf21-b4c7673de371").try_as<bool>(true);
                m_enableMouseShake = (settings >> L"5986d0a7-8675-5f8b-adf5-c593e81d387e").try_as<bool>(true);
                m_shakeSensitivity = (std::min)((std::max)((settings >> L"d549aab1-da90-5d59-8054-3698c1b7e120").try_as<double>(7.0), 2.0), 10.0);
                m_shakeDuration = (std::min)((std::max)((settings >> L"c51ad80d-77f1-509b-8007-5b1a4d980d95").try_as<unsigned short>(2000ui16), 500ui16), 3000ui16);
                m_overlayOpacity = (std::min)((settings >> L"9f29036f-319d-5799-9b89-10cff10624a3").try_as<unsigned short>(75ui16), 100ui16);
                m_overlayColor = (settings >> L"63a2ea58-7586-5dd4-8de1-46ff08c9c8a3").try_as<std::wstring>(L"#000000");
                m_helperColor = (settings >> L"68dace38-7e85-5bbf-b3dd-667a0419d1ea").try_as<std::wstring>(L"#FFFFFF");
            }
            catch (...)
            {
            }

            AutoHighlight().SelectedIndex((int32_t)m_autoHighlight);
            EnableKeyShort().IsOn(m_enableKeyShort);
            EnableMouseShake().IsOn(m_enableMouseShake);
            ShakeSensitivity().Value(m_shakeSensitivity);
            ShakeDuration().Value(m_shakeDuration);
            OverlayOpacity().Value(m_overlayOpacity);
            OverlayColorPicker().Color(from_hex(m_overlayColor));
            HelperColorPicker().Color(from_hex(m_helperColor));
        }
        else
        {
            settings[L"4651485d-138e-5791-a0a9-940756ba18aa"] = m_autoHighlight;
            settings[L"80dc8702-ff5e-5df6-bf21-b4c7673de371"] = m_enableKeyShort;
            settings[L"5986d0a7-8675-5f8b-adf5-c593e81d387e"] = m_enableMouseShake;
            settings[L"d549aab1-da90-5d59-8054-3698c1b7e120"] = m_shakeSensitivity;
            settings[L"c51ad80d-77f1-509b-8007-5b1a4d980d95"] = m_shakeDuration;
            settings[L"9f29036f-319d-5799-9b89-10cff10624a3"] = m_overlayOpacity;
            settings[L"63a2ea58-7586-5dd4-8de1-46ff08c9c8a3"] = m_overlayColor;
            settings[L"68dace38-7e85-5bbf-b3dd-667a0419d1ea"] = m_helperColor;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        MouseFocus::SaveSettings(std::wstring const& name, T const& value)
    {
        if (name == L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9")
        {
            backbehav[name] = value;
            backbehav.Write(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf");
            get_ftime(L"Settings\\BackBehav.xsmf", last_ctime2, last_mtime2, last_size2);
        }
        else
        {
            settings[name] = value;
            settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf");
            get_ftime(L"Settings\\MouseFocus.xsmf", last_ctime1, last_mtime1, last_size1);
        }
    }
}
