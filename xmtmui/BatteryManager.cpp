#include "pch.h"
#include "BatteryManager.h"
#if __has_include("BatteryManager.g.cpp")
#include "BatteryManager.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::xmtmui::implementation
{
    BatteryManager::BatteryManager()
    {
        InitializeComponent();
    }

    void BatteryManager::PageLoaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        using namespace std::chrono_literals;

        {
            SYSTEM_POWER_STATUS state{};
            if (GetSystemPowerStatus(&state) != 0)
            {
                if (state.BatteryFlag == (BYTE)128)
                {
                    NoBatInfo().IsOpen(true);
                }
            }
            else
            {
                NoBatInfo().IsOpen(true);
            }
        }

        LoadSettings();

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf1, current_buf2;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\BatteryManager.xsmf").wstring().c_str(), &current_buf1);
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

    void BatteryManager::Pageunloaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        fileChecker.Stop();
        fileChecker_revoker.~event_revoker();
        fileChecker = nullptr;

        settings.~XSMF();
        backbehav.~XSMF();
    }

    void BatteryManager::NavToBackBehav(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        auto nav{ find_ancestor<muxc::NavigationView>(*this) };
        if (!nav) return;

        for (auto&& item : nav.FooterMenuItems())
        {
            if (auto menu{ item.try_as<muxc::NavigationViewItem>() })
            {
                auto menutag{ unbox_value_or<hstring>(menu.Tag(), L"") };
                if (menutag != L"Background Behaviors") continue;

                nav.SelectedItem(menu);
                break;
            }
        }
    }

    void BatteryManager::EnableBatteryManagerChanged(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_enableBatteryManager = EnableBatteryManager().IsOn();
        SaveSettings(L"bdec9ff0-3b01-536c-a4d3-e50bd337640d", m_enableBatteryManager);
    }

    void BatteryManager::LowBatPercChanged(muxc::NumberBox const& sender, muxc::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = static_cast<unsigned short>(sender.Text() == L"" ? args.OldValue() : sender.Value());
        sender.Value(val);

        m_lowBatPerc = val;

        SaveSettings(L"cecf45fd-d5db-5c6c-9bd1-a7581746066c", m_lowBatPerc);
    }

    void BatteryManager::StopChargePercChanged(muxc::NumberBox const& sender, muxc::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = static_cast<unsigned short>(sender.Text() == L"" ? args.OldValue() : sender.Value());
        sender.Value(val);

        m_stopChargePerc = val;

        SaveSettings(L"5d8bf58a-6ae6-52cd-aa2b-e0d7e643904c", m_stopChargePerc);
    }

    void BatteryManager::UseModernBoxChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_useModernBox = UseModernBox().IsOn();
        SaveSettings(L"95a53186-3637-5b46-8640-3e1dd4915c1a", m_useModernBox);
    }

    void BatteryManager::LoadSettings()
    {
        isloaded = false;

        if (backbehav.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf"))
        {
            try
            {
                m_enableBatteryManager = (backbehav >> L"bdec9ff0-3b01-536c-a4d3-e50bd337640d").try_as<bool>(true);
            }
            catch (...)
            {
            }

            EnableBatteryManager().IsOn(m_enableBatteryManager);
        }
        else
        {
            backbehav[L"bdec9ff0-3b01-536c-a4d3-e50bd337640d"] = m_enableBatteryManager;
        }

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\BatteryManager.xsmf"))
        {
            try
            {
                m_lowBatPerc = (settings >> L"cecf45fd-d5db-5c6c-9bd1-a7581746066c").try_as<unsigned short>(25ui16);
                m_stopChargePerc = (settings >> L"5d8bf58a-6ae6-52cd-aa2b-e0d7e643904c").try_as<unsigned short>(90ui16);
                m_useModernBox = (settings >> L"95a53186-3637-5b46-8640-3e1dd4915c1a").try_as<bool>(true);
            }
            catch (...)
            {
            }

            LowBatPerc().Value(m_lowBatPerc);
            StopChargePerc().Value(m_stopChargePerc);
            UseModernBox().IsOn(m_useModernBox);
        }
        else
        {
            settings[L"cecf45fd-d5db-5c6c-9bd1-a7581746066c"] = m_lowBatPerc;
            settings[L"5d8bf58a-6ae6-52cd-aa2b-e0d7e643904c"] = m_stopChargePerc;
            settings[L"95a53186-3637-5b46-8640-3e1dd4915c1a"] = m_useModernBox;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        BatteryManager::SaveSettings(std::wstring const& name, T const& value)
    {
        if (name == L"bdec9ff0-3b01-536c-a4d3-e50bd337640d")
        {
            backbehav[name] = value;
            backbehav.Write(Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf");
            get_ftime(L"Settings\\BackBehav.xsmf", last_ctime2, last_mtime2, last_size2);
        }
        else
        {
            settings[name] = value;
            settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\BatteryManager.xsmf");
            get_ftime(L"Settings\\BatteryManager.xsmf", last_ctime1, last_mtime1, last_size1);
        }
    }
}
