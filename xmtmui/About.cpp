#include "pch.h"
#include "About.h"
#if __has_include("About.g.cpp")
#include "About.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::xmtmui::implementation
{
    About::About()
    {
        InitializeComponent();
    }

    bool About::GetLatestVersionInfo()
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
                // update logs
                update_logs = Xellanix::Utilities::s_to_ws(release[0]["body"].get<std::string>());
                // asset download url
                url = Xellanix::Utilities::s_to_ws(release[0]["assets"][0]["browser_download_url"].get<std::string>());
                // asset size
                asset_size = release[0]["assets"][0]["size"].get<unsigned long long>();

                downloaded_updated_size = asset_size;
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
                // update logs
                update_logs = Xellanix::Utilities::s_to_ws(release[0]["body"].get<std::string>());
                // asset download url
                url = Xellanix::Utilities::s_to_ws(release[0]["assets"][0]["browser_download_url"].get<std::string>());
                // asset size
                asset_size = release[0]["assets"][0]["size"].get<unsigned long long>();

                downloaded_updated_size = asset_size;

                fs::create_directories(update_path_f);
                std::ofstream wof;
                wof.open(update_path, std::ios::binary);
                wof << release;
                wof.close();
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    Windows::Foundation::IAsyncAction About::StartDownloadUpdate(std::wstring url_download)
    {
        auto strong{ get_strong() };

        // new sections
        co_await resume_background();

        // downloaded_update_path = fs::path(Xellanix::Utilities::AppDir) / L"update_temp" / std::wstring(L"v" + strong->latestVersion + L".zip");
        fs::create_directories(downloaded_update_path.parent_path());

        strong->helper.SetTempFolder(Xellanix::Utilities::LocalTemp.wstring());
        strong->helper.Progress([=](DownloadFileStatus status, DownloadFileArgs const args) -> fire_and_forget
        {
            co_await strong->ui_thread;
            
            if (auto bar{ strong->UpdateBar() })
            {
                switch (status)
                {
                    case DOWNLOAD_STARTED:
                    {
                        bar.Title(L"Downloading version " + strong->latestVersion + L"...");
                        bar.Message(L"");
                        bar.Severity(muxc::InfoBarSeverity::Informational);
                        bar.ActionButton().Visibility(Visibility::Collapsed);
                        bar.ActionButton().IsEnabled(false);
                        bar.Content(nullptr);
                        bar.IsOpen(true);

                        break;
                    }
                    case DOWNLOAD_DOWNLOADING:
                    {
                        // Message: 0.00 MB of 0.00 MB (100%)
                        const auto receivedText = strong->make_size_s(args.CurrentDownloaded);
                        const auto totalText = strong->make_size_s(args.TotalToDownload);
                        bar.Message(receivedText + L" of " + totalText + L" (" + Xellanix::Utilities::NumFormatter(args.Percent) + L"%)");
                        bar.IsOpen(true);

                        break;
                    }
                    case DOWNLOAD_FINISHING:
                    {
                        bar.Title(L"Finishing...");
                        bar.Message(L"");
                        bar.Severity(muxc::InfoBarSeverity::Informational);
                        bar.ActionButton().Visibility(Visibility::Collapsed);
                        bar.ActionButton().IsEnabled(false);
                        bar.Content(nullptr);
                        bar.IsOpen(true);

                        break;
                    }
                    case DOWNLOAD_SUCCESS:
                    {
                        bar.IsOpen(false);

                        bar.Title(L"Ready to install!");
                        bar.Message(L"Your updates will be automatically installed when you exit the app.");
                        bar.Severity(muxc::InfoBarSeverity::Success);
                        bar.ActionButton().Visibility(Visibility::Collapsed);
                        bar.ActionButton().IsEnabled(false);
                        bar.Content(nullptr);
                        bar.IsOpen(true);

                        break;
                    }
                    case DOWNLOAD_ERROR:
                    {
                        bar.Title(L"Error: ");
                        bar.Message(args.ErrorMessage);
                        bar.Severity(muxc::InfoBarSeverity::Error);
                        bar.ActionButton().Visibility(Visibility::Collapsed);
                        bar.ActionButton().IsEnabled(false);
                        bar.Content(nullptr);
                        bar.IsOpen(true);

                        break;
                    }
                }
            }
        });
        strong->helper.StartDownload(url_download, downloaded_update_path.wstring());

        co_await strong->ui_thread;

        if (auto ring{ strong->UpdateLoading() })
        {
            ring.IsActive(false);
        }
        if (auto btn{ strong->CheckUpdateBtn() })
        {
            btn.IsEnabled(true);
        }
    }

    RichTextBlock About::GetUpdateLog()
    {
        RichTextBlock main{};
        main.LineHeight(20);
        main.LineStackingStrategy(LineStackingStrategy::MaxHeight);
        main.TextWrapping(TextWrapping::WrapWholeWords);

        std::vector<std::wstring> log_lines;
        {
            std::wstringstream wss;
            wss << update_logs;
            std::wstring line = L"";
            while (std::getline(wss, line))
            {
                line.erase(std::remove(line.begin(), line.end(), L'\r'), line.end());
                log_lines.emplace_back(line);
            }
        }

        {
            Documents::Paragraph prag{};
            Documents::Run textp{};
            textp.Text(L"Download size: " + make_size_s(downloaded_updated_size) + L"\n");
            prag.Inlines().Append(textp);

            main.Blocks().Append(prag);
        }

        for (auto const& log_l : log_lines)
        {
            if (log_l.size() >= 2)
            {
                if (auto fc = log_l.substr(0, 2); fc == L"**")
                {
                    if (log_l.substr(log_l.size() - 2, 2) == L"**")
                    {
                        Documents::Paragraph prag{};
                        prag.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
                        prag.FontSize(16);

                        Documents::Run textp{};
                        textp.Text(log_l.substr(2, log_l.size() - 4));
                        prag.Inlines().Append(textp);

                        main.Blocks().Append(prag);
                    }
                }
                else if (fc == L"> ")
                {
                    Documents::Paragraph prag{};

                    Documents::InlineUIContainer uic{};
                    Border border{};
                    border.Style(unbox_value<Windows::UI::Xaml::Style>(Resources().Lookup(box_value(L"ParagraphBorder"))));
                    
                    TextBlock textb{};
                    textb.Text(log_l.substr(2));

                    border.Child(textb);
                    uic.Child(border);
                    prag.Inlines().Append(uic);

                    main.Blocks().Append(prag);
                }
                else if (fc == L"- ")
                {
                    Documents::Paragraph prag{};
                    prag.TextIndent(10);

                    Documents::Run textp{};
                    std::wstring rep = log_l;
                    rep = rep.replace(0, 2, L"●    ");

                    textp.Text(rep);
                    prag.Inlines().Append(textp);

                    main.Blocks().Append(prag);
                }
                else
                {
                    Documents::Paragraph prag{};
                    prag.TextIndent(10);

                    Documents::Run textp{};
                    textp.Text(log_l);
                    prag.Inlines().Append(textp);

                    main.Blocks().Append(prag);
                }
            }
            else if (log_l.size() == 1)
            {
                Documents::Paragraph prag{};
                prag.TextIndent(10);

                Documents::Run textp{};
                textp.Text(log_l);
                prag.Inlines().Append(textp);

                main.Blocks().Append(prag);
            }
            else
            {
                main.Blocks().Append(Windows::UI::Xaml::Documents::Paragraph{});
            }
        }

        main.Blocks().Append(Windows::UI::Xaml::Documents::Paragraph{});

        return main;
    }

    Windows::Foundation::IAsyncAction About::TryToUpdate()
    {
        auto strong{ get_strong() };

        auto btn{ strong->CheckUpdateBtn() };
        if (!btn) co_return;
        btn.IsEnabled(false);

        if (auto bar{ strong->UpdateBar() })
        {
            bar.IsOpen(false);

            bar.Title(L"Checking...");
            bar.Message(L"");
            bar.Severity(muxc::InfoBarSeverity::Informational);
            bar.ActionButton().Visibility(Visibility::Collapsed);
            bar.ActionButton().IsEnabled(false);
            bar.Content(nullptr);
            bar.IsOpen(true);
            strong->UpdateLoading().IsActive(true);

            co_await resume_background();
            const bool ok = strong->GetLatestVersionInfo();
            co_await strong->ui_thread;

            strong->UpdateLoading().IsActive(false);

            bar.IsOpen(false);

            if (!ok)
            {
                bar.Title(L"Failed to get latest version info!");
                bar.Message(L"Try again to check for update.");
                bar.Severity(muxc::InfoBarSeverity::Error);
                bar.ActionButton().Visibility(Visibility::Collapsed);
                bar.ActionButton().IsEnabled(false);
                bar.Content(nullptr);
                bar.IsOpen(true);

                btn.IsEnabled(true);

                co_return;
            }

            if (m_currentVersion == latestVersion)
            {
                // latest version
                bar.Title(L"You are using the latest version!");
                bar.Message(L"Version " + latestVersion);
                bar.Severity(muxc::InfoBarSeverity::Success);
                bar.ActionButton().Visibility(Visibility::Collapsed);
                bar.ActionButton().IsEnabled(false);
                bar.Content(nullptr);
                bar.IsOpen(true);

                btn.IsEnabled(true);
            }
            else
            {
                downloaded_update_path = Xellanix::Utilities::LocalAppData / L"update_temp" / std::wstring(L"MasterTools_v" + strong->latestVersion + L".exe");

                struct _stati64 buf;
                if (_wstat64(downloaded_update_path.wstring().c_str(), &buf) == 0 && static_cast<unsigned long long>(buf.st_size) == strong->asset_size)
                {
                    bar.Title(L"Ready to install!");
                    bar.Message(L"Your updates will be automatically installed when you exit the app.");
                    bar.Severity(muxc::InfoBarSeverity::Success);
                    bar.ActionButton().Visibility(Visibility::Collapsed);
                    bar.ActionButton().IsEnabled(false);
                    bar.Content(nullptr);
                    bar.IsOpen(true);

                    btn.IsEnabled(true);

                    co_return;
                }

                if (!m_autoDownloadUpdate)
                {
                    bar.Title(L"Update available!");
                    bar.Message(L"Latest version " + strong->latestVersion);
                    bar.Severity(muxc::InfoBarSeverity::Informational);
                    bar.ActionButton().Visibility(Visibility::Visible);
                    bar.ActionButton().IsEnabled(true);

                    bar.Content(GetUpdateLog());

                    bar.IsOpen(true);

                    btn.IsEnabled(true);
                }
                else
                {
                    bar.Title(L"Downloading version " + strong->latestVersion + L"...");
                    bar.Message(L"");
                    bar.Severity(muxc::InfoBarSeverity::Informational);
                    bar.ActionButton().Visibility(Visibility::Collapsed);
                    bar.ActionButton().IsEnabled(false);
                    bar.Content(nullptr);
                    bar.IsOpen(true);

                    strong->UpdateLoading().IsActive(true);

                    strong->no_await(strong->StartDownloadUpdate(url));
                }
            }
        }
    }

    void About::CheckUpdate(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        no_await(TryToUpdate());
    }

    void About::AutoUpdateChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_autoUpdate = AutoUpdate().IsOn();
        SaveSettings(L"08b46f12-df6f-597f-b020-b2a44b842a2b", m_autoUpdate);
    }

    void About::AutoDownloadUpdateChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;
        
        m_autoDownloadUpdate = AutoDownloadUpdate().IsOn();
        SaveSettings(L"582b5c91-2083-5a96-a64b-3fef4a53ef3f", m_autoDownloadUpdate);
    }

    void About::DownloadUpdate(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto btn{ CheckUpdateBtn() };
        if (!btn) return;
        btn.IsEnabled(false);

        if (auto bar{ UpdateBar() })
        {
            bar.IsOpen(false);

            bar.Title(L"Downloading version " + latestVersion + L"...");
            // Message: 0.00MB of 0.00MB (100%)
            bar.Message(L"");
            bar.Severity(muxc::InfoBarSeverity::Informational);
            bar.ActionButton().Visibility(Visibility::Collapsed);
            bar.ActionButton().IsEnabled(false);
            bar.Content(nullptr);
            bar.IsOpen(true);

            UpdateLoading().IsActive(true);

            no_await(StartDownloadUpdate(url));
        }
    }

    void About::PageLoaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        ui_thread = apartment_context{};

        VersionText().Text(L"Version " + m_currentVersion + L" (64-bit)");

        LoadSettings();

        if (m_autoUpdate)
        {
            no_await(TryToUpdate());
        }

        using namespace std::chrono_literals;

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf1;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\SmartBar.xsmf").wstring().c_str(), &current_buf1);

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

    Windows::Foundation::IAsyncAction About::OnNavigatingFrom(Windows::UI::Xaml::Navigation::NavigatingCancelEventArgs const& /*args*/)
    {
        auto weak{ get_weak() };

        if (auto strong{ weak.get() })
        {
            co_await strong->ui_thread;

            strong->helper.Pause();

            strong->fileChecker.Stop();
            strong->fileChecker_revoker.~event_revoker();
            strong->fileChecker = nullptr;
        }
    }

    void About::LoadSettings()
    {
        isloaded = false;

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\About.xsmf"))
        {
            m_autoUpdate = (settings / L"08b46f12-df6f-597f-b020-b2a44b842a2b").try_as<bool>(true);
            m_autoDownloadUpdate = (settings / L"582b5c91-2083-5a96-a64b-3fef4a53ef3f").try_as<bool>(false);
            
            AutoUpdate().IsOn(m_autoUpdate);
            AutoDownloadUpdate().IsOn(m_autoDownloadUpdate);
        }
        else
        {
            settings[L"08b46f12-df6f-597f-b020-b2a44b842a2b"] = m_autoUpdate;
            settings[L"582b5c91-2083-5a96-a64b-3fef4a53ef3f"] = m_autoDownloadUpdate;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        About::SaveSettings(std::wstring const& name, T const& value)
    {
        settings[name] = value;

        settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\About.xsmf");
        get_ftime(L"Settings\\About.xsmf", last_ctime1, last_mtime1, last_size1);
    }

    std::wstring About::make_size_s(unsigned long long const& val)
    {
        const auto&& [bytes, stype] = Xellanix::Utilities::NormalizeBytes(val);
        return Xellanix::Utilities::NumFormatter(bytes) + L" " + stype;
    }
}
