#include "pch.h"
#include "AllFeatures.h"
#if __has_include("AllFeatures.g.cpp")
#include "AllFeatures.g.cpp"
#endif
#include "../../../Libraries/json.hpp"
#include "../../../Libraries/xellanix.objects.h"
#include "AppVersionInfo.h"

#pragma comment(lib, "Wininet.lib")
#include <wininet.h>
#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.UI.Xaml.Media.Animation.h"
#include "BannerControl.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::xmtmui::implementation
{
    AllFeatures::AllFeatures()
    {
        InitializeComponent();
    }

    void AllFeatures::NavigateFeature(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto btn{ sender.try_as<Controls::Button>() };
        if (!btn) return;

        auto tag{ unbox_value_or<hstring>(btn.Tag(), L"") };
        if (tag == L"") return;

        auto nav{ find_ancestor<Microsoft::UI::Xaml::Controls::NavigationView>(*this) };
        if (!nav) return;

        for (auto&& item : nav.MenuItems())
        {
            if (auto menu{ item.try_as<Microsoft::UI::Xaml::Controls::NavigationViewItem>() })
            {
                auto menutag{ unbox_value_or<hstring>(menu.Tag(), L"") };
                if (menutag == L"") continue;

                if (menutag == tag)
                {
                    nav.SelectedItem(menu);
                    break;
                }
            }
        }
    }

    class HInternet
    {
    public:
        HInternet(const HINTERNET handle)
            : m_handle(handle)
        {
        }

        ~HInternet()
        {
            if (m_handle != nullptr)
                InternetCloseHandle(m_handle);
        }

        operator HINTERNET() const
        {
            return m_handle;
        }

    private:
        const HINTERNET m_handle;
    };
    DWORD query_release_information(std::string& response)
    {
        static const WCHAR* Host = L"api.github.com";
        static const WCHAR* Path = L"/repos/xellanix/mastertools/releases";

        response.clear();

        const HInternet session = InternetOpen(L"mastertools", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
        if (!session)
            return GetLastError();

        const HInternet connection = InternetConnect(session, Host, INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
        if (!connection)
            return GetLastError();

        static const WCHAR* AcceptTypes[] = { L"application/json", nullptr };
        const HInternet request = HttpOpenRequest(connection, L"GET", Path, nullptr, nullptr, AcceptTypes, INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0);
        if (!request)
            return GetLastError();

        if (!HttpSendRequest(request, nullptr, 0, nullptr, 0))
            return GetLastError();

        std::vector<char> buffer(4096);

        while (true)
        {
            DWORD bytes_read;
            if (!InternetReadFile(request, &buffer[0], static_cast<DWORD>(buffer.size()), &bytes_read))
            {
                const DWORD last_error = GetLastError();
                if (last_error == ERROR_INSUFFICIENT_BUFFER)
                    buffer.resize(buffer.size() * 2);
                else return last_error;
            }

            if (bytes_read > 0)
                response.append(&buffer[0], bytes_read);
            else return ERROR_SUCCESS;
        }
    }
    winrt::Windows::Foundation::IAsyncOperation<bool> UpdateAvailable()
    {
        auto IsNeedUpdatedFile = [](fs::path const& path)
        {
            auto file_time = Xellanix::Utilities::FileDateModified(path);
            struct tm file_tm = Xellanix::Utilities::localtime_x(file_time);
            ++file_tm.tm_hour;

            if (mktime(&file_tm) <= time(0))
            {
                // After one hour
                return true;
            }

            return false;
        };

        try
        {
            std::wstring latestVersion;
            {
                const auto update_path_f = fs::path(Xellanix::Utilities::LocalAppData) / L"update_data";
                if (const auto update_path = update_path_f / L"update_data.json";
                    Xellanix::Utilities::CheckExist(update_path) && !IsNeedUpdatedFile(update_path))
                {
                    std::ifstream ifs;
                    ifs.open(update_path, std::ios::binary);
                    nlohmann::json release = nlohmann::json::parse(ifs);

                    // tag name
                    // latestVersion = L"1.0.1.050622";
                    latestVersion = Xellanix::Utilities::s_to_ws(release[0]["tag_name"].get<std::string>()).substr(1);
                }
                else
                {
                    std::string json;
                    if (query_release_information(json) != ERROR_SUCCESS)
                        return false;

                    nlohmann::json release = nlohmann::json::parse(json);

                    // tag name
                    // latestVersion = L"1.0.1.050622";
                    latestVersion = Xellanix::Utilities::s_to_ws(release[0]["tag_name"].get<std::string>()).substr(1);

                    fs::create_directories(update_path_f);
                    std::ofstream wof;
                    wof.open(update_path, std::ios::binary);
                    wof << release;
                    wof.close();
                }
            }

            if (latestVersion == AppVersion()) co_return false;
            else co_return true;
        }
        catch (...)
        {
            co_return false;
        }
    }
    Windows::Foundation::IAsyncAction AllFeatures::TryCheckUpdate()
    {
        co_await resume_background();

        if (Xellanix::Objects::XSMF settings{}; settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\About.xsmf"))
        {
            if (!(settings / L"08b46f12-df6f-597f-b020-b2a44b842a2b").try_as<bool>(true))
            {
                co_return;
            }
        }

        auto updateAvailable = co_await UpdateAvailable();
        if (updateAvailable)
        {
            bool rememberSection = false;

            co_await resume_foreground(Dispatcher());
            if (auto bannerView{ BannerView() })
            {
                bannerView.BannerItems().Append(unbox_value<xmtmui::BannerChild>(Resources().Lookup(box_value(L"UpdateBanner"))));
                bannerView.SelectedBannerIndex(bannerView.BannerItems().Size() - 1);
            }
            co_await resume_background();

            if (rememberSection)
            {
                co_return;
            }
        }
    }

    Windows::Foundation::IAsyncAction AllFeatures::AvailableUpdateBar_Loaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (checkingOp)
        {
            if (checkingOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                checkingOp.Cancel();
            }
        }
        checkingOp = TryCheckUpdate();
        co_await checkingOp;
    }

    void AllFeatures::AvailableUpdateBar_Unloaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (checkingOp)
        {
            if (checkingOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                checkingOp.Cancel();
            }
            checkingOp = nullptr;
        }
    }

    void AllFeatures::MoveToAboutPage(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (auto nav{ find_ancestor<Microsoft::UI::Xaml::Controls::NavigationView>(*this) })
        {
            nav.SelectedItem(nav.FooterMenuItems().GetAt(nav.FooterMenuItems().Size() - 1));
        }
    }
}
