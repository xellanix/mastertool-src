#pragma once

#pragma comment(lib, "Wininet.lib")

#include "About.g.h"
#include "Utilities.h"
#include "../../../Libraries/xellanix.objects.h"
#include "../../../Libraries/json.hpp"
#include <wininet.h>
#include "DownloadFileHelper.h"
#include "winrt/Windows.UI.Text.h"
#include "winrt/Windows.UI.Xaml.Documents.h"
#include "AppVersionInfo.h"

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

inline DWORD query_release_information(std::string& response)
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

namespace winrt::xmtmui::implementation
{
    struct About : AboutT<About>
    {
    private:
        const std::wstring m_currentVersion{ AppVersion() };
        bool m_autoUpdate = true;
        bool m_autoDownloadUpdate = false;

        std::wstring latestVersion = m_currentVersion;
        std::wstring url = L"";
        std::wstring update_logs = L"";
        unsigned long long asset_size = 0;

        bool GetLatestVersionInfo();
        Windows::Foundation::IAsyncAction StartDownloadUpdate(std::wstring url_download);
        Windows::UI::Xaml::Controls::RichTextBlock GetUpdateLog();
        Windows::Foundation::IAsyncAction TryToUpdate();

        bool isloaded = false;
        time_t last_ctime1 = 0, last_mtime1 = 0;
        long long last_size1 = 0;
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);
        Xellanix::Objects::XSMF settings{ L"Image Merger" };

        Windows::UI::Xaml::DispatcherTimer::Tick_revoker fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer fileChecker{};

        std::wstring make_size_s(unsigned long long const& val);

        DownloadFileHelper helper;
        apartment_context ui_thread;

        template<typename T>
        fire_and_forget no_await(T t)
        {
            if constexpr (std::is_invocable_v<T>)
            {
                co_await t();
            }
            else
            {
                co_await t;
            }
        }

    public:
        About();
        void CheckUpdate(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void AutoUpdateChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void AutoDownloadUpdateChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void DownloadUpdate(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction OnNavigatingFrom(Windows::UI::Xaml::Navigation::NavigatingCancelEventArgs const& args);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct About : AboutT<About, implementation::About>
    {
    };
}
