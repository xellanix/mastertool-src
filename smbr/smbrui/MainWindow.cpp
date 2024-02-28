#include "pch.h"
#include "MainWindow.h"
#if __has_include("ResultData.g.cpp")
#include "ResultData.g.cpp"
#endif
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::smbrui::implementation
{
    Windows::Foundation::IAsyncAction MainWindow::SearchBox_Loaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        SearchBox().Focus(FocusState::Programmatic);

        co_await LoadExplorerItems();
    }

    void MainWindow::OpenFileClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto btn{ sender.try_as<Button>() };
        if (!btn) return;

        auto item{ btn.Parent().try_as<Grid>().Parent().try_as<ListViewItem>() };
        if (!item) return;

        uint32_t index = unbox_value_or<uint32_t>(btn.Tag(), 0);
        if (auto const&& raw{ m_results.GetAt(index).try_as<smbrui::ResultData>() })
        {
            std::wstring fullPath{ raw.Description() + raw.Title() };

            ShellExecute(NULL, NULL, fullPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }

    void MainWindow::RunAdminClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto btn{ sender.try_as<Button>() };
        if (!btn) return;

        auto item{ btn.Parent().try_as<Grid>().Parent().try_as<ListViewItem>() };
        if (!item) return;

        uint32_t index = unbox_value_or<uint32_t>(btn.Tag(), 0);
        std::wstring title{ m_results.GetAt(index).try_as<smbrui::ResultData>().Title() };
        MessageBox(NULL, title.c_str(), L"", MB_OK);
    }

    void MainWindow::OpenLocationClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto btn{ sender.try_as<Button>() };
        if (!btn) return;

        auto item{ btn.Parent().try_as<Grid>().Parent().try_as<ListViewItem>() };
        if (!item) return;

        uint32_t index = unbox_value_or<uint32_t>(btn.Tag(), 0);
        if (auto const&& raw{ m_results.GetAt(index).try_as<smbrui::ResultData>() })
        {
            std::wstring parentPath{ raw.Description() };

            ShellExecute(NULL, NULL, parentPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::LoadExplorerItems()
    {
        auto strong{ get_strong() };

        co_await resume_background();
        
        if (strong->settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\SmartBar.xsmf"))
        {
            try
            {
                strong->m_enableApps = (settings >> L"099a6edf-dc24-5bfd-95e9-f70ef1bc2936").try_as<bool>(true);
                strong->m_enableFolders = (settings >> L"4b18963f-3180-5943-8416-ad07ef9cffb8").try_as<bool>(true);
                strong->m_folderDrive = (settings >> L"16c4ac8e-fbd4-547f-9002-9892cfad574e").try_as<uint32_t>(0);
                strong->m_enableFiles = (settings >> L"3ee29586-94b7-58d8-84a6-ee4da971680d").try_as<bool>(true);
                strong->m_fileDrive = (settings >> L"69be0cf6-a495-5159-8798-adfc689ef468").try_as<uint32_t>(0);
            }
            catch (...)
            {
            }
        }
        strong->settings.~XSMF();
        
        strong->SetAvailableDrives();
        if (strong->m_fileDrive >= strong->arrayOfDrives.size() + 1) strong->m_fileDrive = 0;
        if (strong->m_folderDrive >= strong->arrayOfDrives.size() + 1) strong->m_folderDrive = 0;
        
        if (strong->m_fileDrive == strong->m_folderDrive)
        {
            for (auto const& drive : (strong->m_folderDrive == 0 ? arrayOfDrives : std::initializer_list<std::wstring>({ arrayOfDrives[strong->m_folderDrive - 1] })))
            {
                auto count = std::distance(fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied), fs::recursive_directory_iterator());
                strong->explorerItems.reserve(count);

                std::error_code ec;
                for (auto const& entry : fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied, ec))
                {
                    auto const path = entry.path().wstring();
                    DWORD attributes = GetFileAttributes(path.c_str());
                    if (!(attributes & FILE_ATTRIBUTE_HIDDEN) || attributes == INVALID_FILE_ATTRIBUTES)
                    {
                        if (!ec)
                        {
                            strong->explorerItems.push_back(path);
                        }
                    }
                }
            }
        }
        else
        {
            for (auto const& drive : (strong->m_fileDrive == 0 ? strong->arrayOfDrives : std::initializer_list<std::wstring>({ strong->arrayOfDrives[strong->m_fileDrive - 1] })))
            {
                auto count = std::distance(fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied), fs::recursive_directory_iterator());
                strong->explorerItems.reserve(count);

                std::error_code ec;
                for (auto const& entry : fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied, ec))
                {
                    auto const path = entry.path().wstring();
                    DWORD attributes = GetFileAttributes(path.c_str());
                    if (!(attributes & FILE_ATTRIBUTE_HIDDEN) || attributes == INVALID_FILE_ATTRIBUTES)
                    {
                        if (!fs::is_directory(entry.path()))
                        {
                            if (!ec)
                            {
                                strong->explorerItems.push_back(path);
                            }
                        }
                    }
                }
            }

            for (auto const& drive : (strong->m_folderDrive == 0 ? strong->arrayOfDrives : std::initializer_list<std::wstring>({ strong->arrayOfDrives[strong->m_folderDrive - 1] })))
            {
                auto count = std::distance(fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied), fs::recursive_directory_iterator());
                strong->explorerItems.reserve(count);

                std::error_code ec;
                for (auto const& entry : fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied, ec))
                {
                    auto const path = entry.path().wstring();
                    DWORD attributes = GetFileAttributes(path.c_str());
                    if (!(attributes & FILE_ATTRIBUTE_HIDDEN) || attributes == INVALID_FILE_ATTRIBUTES)
                    {
                        if (fs::is_directory(entry.path()))
                        {
                            if (!ec)
                            {
                                strong->explorerItems.push_back(path);
                            }
                        }
                    }
                }
            }
        }

        strong->explorerItems.shrink_to_fit();
        
        strong->explorerLoaded = true;
        if (strong->SearchOperation)
        {
            if (strong->SearchOperation.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                strong->SearchOperation.Cancel();
            }
        }
        if (!strong->searchText.empty())
        {
            strong->SearchOperation = StartSearch();
            co_await strong->SearchOperation;
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::StartSearch()
    {
        auto strong{ get_strong() };

        co_await resume_background();

        auto const [key, criteria] = split_keys(strong->searchText, { L"?apps:", L"?files:", L"?folders:" });

        if (key == L"?files:")
        {
            for (auto const& i : strong->explorerItems)
            {
                auto const [parent_path, name] = strong->split_path(i);

                if (strong->lowerstring(name).find(criteria) != std::wstring::npos)
                {
                    if (!fs::is_directory(fs::path(i)))
                    {
                        auto result = make<implementation::ResultData>(strong->m_results.Size(), hstring(name), hstring(parent_path), true, false, true);
                        strong->m_results.Append(result);
                    }
                }
            }
        }
        else if (key == L"?folders:")
        {
            for (auto const& i : strong->explorerItems)
            {
                auto const [parent_path, name] = strong->split_path(i);

                if (strong->lowerstring(name).find(criteria) != std::wstring::npos)
                {
                    if (fs::is_directory(fs::path(i)))
                    {
                        auto result = make<implementation::ResultData>(strong->m_results.Size(), hstring(name), hstring(parent_path), true, false, true);
                        strong->m_results.Append(result);
                    }
                }
            }
        }
        else if (key == L"?apps:")
        {

        }
        else if (key.size() == 0)
        {
            // For now it's only looking up on files and folders
            // Once it support apps lookup, please change any of this section
            for (auto const& i : strong->explorerItems)
            {
                auto const [parent_path, name] = strong->split_path(i);

                if (strong->lowerstring(name).find(criteria) != std::wstring::npos)
                {
                    auto result = make<implementation::ResultData>(strong->m_results.Size(), hstring(name), hstring(parent_path), true, false, true);
                    strong->m_results.Append(result);
                }
            }
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::StartBtnClicked(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strong{ get_strong() };

        if (!strong->explorerLoaded)
        {
            co_return;
        }

        if (strong->SearchOperation)
        {
            if (strong->SearchOperation.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                strong->SearchOperation.Cancel();
            }
        }

        strong->m_results.Clear();

        strong->SearchOperation = StartSearch();
        co_await strong->SearchOperation;
    }

    void MainWindow::SearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs const& /*e*/)
    {
        auto box{ sender.try_as<TextBox>() };
        if (!box) return;

        std::wstring text{ box.Text() };
        text = Xellanix::Utilities::trim_spaces(text);
        if (text == L"?" || text == L"")
        {
            m_results.ReplaceAll(
                {
                    make<implementation::ResultData>(0, L"?apps:", L"Looking for apps only.", false, false, false),
                    make<implementation::ResultData>(1, L"?files:", L"Looking for files only.", false, false, false),
                    make<implementation::ResultData>(2, L"?folders:", L"Looking for folders only.", false, false, false)
                });
            return;
        }

        searchText = text;
    }

    void MainWindow::SearchBox_KeyDown(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            using namespace Windows::UI::Xaml::Input;

            hasEnter = true;

            FindNextElementOptions option;
            option.SearchRoot(XamlRoot().Content());
            FocusManager::TryMoveFocus(FocusNavigationDirection::Next, option);
            FocusManager::TryMoveFocus(FocusNavigationDirection::Next, option);

            // Make sure to set the Handled to true, otherwise the RoutedEvent might fire twice
            e.Handled(true);
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::SearchBox_LostFocus(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strong{ get_strong() };

        if (strong->hasEnter) strong->hasEnter = false;
        else co_return;

        if (!strong->explorerLoaded) co_return;

        if (strong->SearchOperation)
        {
            if (strong->SearchOperation.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                strong->SearchOperation.Cancel();
            }
        }

        strong->m_results.Clear();

        strong->SearchOperation = StartSearch();
        co_await strong->SearchOperation;
    }
}
