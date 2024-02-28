#pragma once

#include "MainWindow.g.h"
#include "Utilities.h"
#include "../../../Libraries/xellanix.objects.h"
#include "Lottie\UnderConstruction.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/Microsoft.UI.Xaml.Controls.AnimatedVisuals.h"
#include "winrt/Windows.UI.Xaml.Media.Animation.h"

namespace winrt::xmtmui::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        std::unordered_map<std::wstring, Windows::UI::Xaml::Interop::TypeName> m_pages;

        hstring m_startArguments;
        unsigned short feature_id = 0; // based-on navviewitem index
        bool is_pass_params = false;
        Windows::Foundation::Collections::IVector<hstring> passed_files{ single_threaded_vector<hstring>() };

        void InitPages();
        void InitWindowHandle();
        void StartNavigate(std::wstring navItemTag, Windows::UI::Xaml::Media::Animation::NavigationTransitionInfo const& transitionInfo, Windows::Foundation::IInspectable const& parameters);

        Windows::Foundation::IAsyncAction UpdateMemUsage();

    public:
        MainWindow();

        hstring DownloadedUpdatePath() const { return downloaded_update_path.wstring().c_str(); }
        uint64_t DownloadedUpdateSize() const { return downloaded_updated_size; }

        void StartArguments(hstring const& args)
        {
            m_startArguments = args;
            {
                int argc = 0;
                auto argv = CommandLineToArgvW(m_startArguments.c_str(), &argc);

                if (argc >= 3)
                {
                    std::wstring argType = argv[1];
                    if (argType == L"--imgr")
                    {
                        feature_id = 4;
                        is_pass_params = true;

                        for (size_t i = 2; i < argc; i++)
                        {
                            fs::path argp{ argv[i] };
                            if (auto ext = argp.extension().wstring(); ext == L".jpeg" || ext == L".jpg" || ext == L".png")
                                passed_files.Append(argv[i]);
                        }
                    }
                    else if (argType == L"--qrnm")
                    {
                        feature_id = 6;
                        is_pass_params = true;

                        for (size_t i = 2; i < argc; i++)
                        {
                            passed_files.Append(argv[i]);
                        }
                    }
                    else
                    {
                        is_pass_params = false;
                    }
                }

                LocalFree(argv);
            }
        }
        void TryInstallUpdate()
        {
            auto u_path = DownloadedUpdatePath();
            uint64_t u_size = DownloadedUpdateSize();

            struct _stati64 buf;
            if (_wstat64(u_path.c_str(), &buf) == 0 && static_cast<unsigned long long>(buf.st_size) == u_size)
            {
                ShellExecuteW(NULL, NULL, u_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
        }

        void NavigatePage(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
        void NavViewLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
