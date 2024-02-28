#include "pch.h"
#include "QuickRename.h"
#if __has_include("QuickRename.g.cpp")
#include "QuickRename.g.cpp"
#endif
#if __has_include("ExplorerItem.g.cpp")
#include "ExplorerItem.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace muxc
{
    using namespace Microsoft::UI::Xaml::Controls;
}

namespace winrt::xmtmui::implementation
{
    QuickRename::QuickRename()
    {
        InitializeComponent();

        m_explorerItems = single_threaded_observable_vector<Windows::Foundation::IInspectable>();
    }

    void QuickRename::SortItems(std::vector<hstring>& itemList)
    {
        if (m_autoSort)
        {
            std::sort(itemList.begin(), itemList.end(), [](hstring const& lo, hstring const& ro)
            {
                const std::wstring lhs{ lo };
                const std::wstring rhs{ ro };

                return (fs::is_directory(lhs) > fs::is_directory(rhs)) ||
                    ((fs::is_directory(lhs) == fs::is_directory(rhs)) && (natural_less(lhs, rhs)));
            });
        }
    }

    Windows::Foundation::IAsyncAction QuickRename::AddExplorerItem(winrt::param::hstring const& filepath)
    {
        auto strong_this{ get_strong() };

        fs::path fpath{ filepath.operator const winrt::hstring&().c_str() };

        if (!fpath.has_parent_path() || fpath.parent_path() == fpath)
        {
            co_return;
        }

        if (strong_this->m_explorerItemsSet.count(fpath.wstring()) > 0)
        {
            co_return;
        }

        if (Xellanix::Utilities::CheckExist(fpath))
        {
            //auto fname = fpath.filename().wstring();
            auto fname = get_filename(fpath.wstring());
            bool isDir = fs::is_directory(fpath);

            hstring ficon{ isDir ? ExplorerItem_FolderIcon : ExplorerItem_FileIcon };

            auto newItem = winrt::make<implementation::ExplorerItem>(hstring(fname), hstring(fpath.wstring()), ficon, L"", false);
            strong_this->m_explorerItems.Append(newItem);
            strong_this->m_explorerItemsSet.insert(fpath.wstring());

            if (isDir)
            {
                // Add recursively or not?
                if (strong_this->m_recursiveBehav == 0ui16) co_return;

                bool isRecursive = false;
                if (strong_this->m_recursiveBehav == 1ui16)
                {
                    ContentDialog dialog{};
                    dialog.XamlRoot(XamlRoot());
                    dialog.PrimaryButtonText(L"Add");
                    dialog.CloseButtonText(L"No Need");
                    dialog.DefaultButton(ContentDialogButton::Primary);
                    dialog.Title(box_value(L"Add All Files in Folder?"));
                    {
                        TextBlock txtb{};
                        Documents::Run run1{}, run2{}, run3{};
                        Documents::Run bold1{}, bold2{};
                        run1.Text(L"Do you want to add all the files in ");
                        bold1.Text(L"\"" + fpath.wstring() + L"\"");
                        bold1.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
                        run2.Text(L" as well to rename? Select ");
                        bold2.Text(L"No Need");
                        bold2.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
                        run3.Text(L" to add only that folder.");

                        txtb.Inlines().Append(run1);
                        txtb.Inlines().Append(bold1);
                        txtb.Inlines().Append(run2);
                        txtb.Inlines().Append(bold2);
                        txtb.Inlines().Append(run3);

                        txtb.TextWrapping(TextWrapping::WrapWholeWords);
                        dialog.Content(txtb);
                    }

                    auto op{ co_await dialog.ShowAsync() };
                    if (op == ContentDialogResult::Primary)
                    {
                        isRecursive = true;
                    }
                    else
                    {
                        isRecursive = false;
                    }
                }
                else if (strong_this->m_recursiveBehav == 2ui16)
                {
                    if (!strong_this->m_recursiveSetted)
                    {
                        ContentDialog dialog{};
                        dialog.XamlRoot(XamlRoot());
                        dialog.PrimaryButtonText(L"Add");
                        dialog.CloseButtonText(L"No Need");
                        dialog.DefaultButton(ContentDialogButton::Primary);
                        dialog.Title(box_value(L"Add All Files in Folder?"));
                        {
                            TextBlock txtb{};
                            Documents::Run run1{}, run2{};
                            Documents::Run bold1{};
                            run1.Text(L"Do you want to add all the files that are in those folders as well to rename? Select ");
                            bold1.Text(L"No Need");
                            bold1.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
                            run2.Text(L" to add only those folders.");

                            txtb.Inlines().Append(run1);
                            txtb.Inlines().Append(bold1);
                            txtb.Inlines().Append(run2);

                            txtb.TextWrapping(TextWrapping::WrapWholeWords);
                            dialog.Content(txtb);
                        }

                        auto op{ co_await dialog.ShowAsync() };
                        if (op == ContentDialogResult::Primary)
                        {
                            strong_this->m_isRecursive = true;
                        }
                        else
                        {
                            strong_this->m_isRecursive = false;
                        }
                    }
                    
                    isRecursive = strong_this->m_isRecursive;
                }
                else if (strong_this->m_recursiveBehav == 3ui16)
                {
                    isRecursive = true;
                }

                if (!isRecursive) co_return;
                
                std::vector<fs::directory_entry> recurPaths;
                std::copy(fs::recursive_directory_iterator(fpath), fs::recursive_directory_iterator(), std::back_inserter(recurPaths));
                if (strong_this->m_autoSort)
                {
                    std::sort(recurPaths.begin(), recurPaths.end(), [](fs::directory_entry const& lhs, fs::directory_entry const& rhs)
                    {
                        return natural_less(lhs.path().wstring(), rhs.path().wstring());
                    });
                }
                for (auto& entry : recurPaths)
                {
                    AddFromRecursive(entry.path().wstring());
                }
            }
        }
    }
    void QuickRename::AddFromRecursive(winrt::param::hstring const& filepath)
    {
        fs::path fpath{ filepath.operator const winrt::hstring & ().c_str() };

        if (!fpath.has_parent_path() || fpath.parent_path() == fpath)
        {
            return;
        }

        if (m_explorerItemsSet.count(fpath.wstring()) > 0)
        {
            return;
        }

        if (Xellanix::Utilities::CheckExist(fpath))
        {
            //auto fname = fpath.filename().wstring();
            auto fname = get_filename(fpath.wstring());
            hstring ficon{ fs::is_directory(fpath) ? ExplorerItem_FolderIcon : ExplorerItem_FileIcon };

            auto newItem = winrt::make<implementation::ExplorerItem>(hstring(fname), hstring(fpath.wstring()), ficon, L"", false);
            m_explorerItems.Append(newItem);
            m_explorerItemsSet.insert(fpath.wstring());
        }
    }

    void QuickRename::PageLoaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        using namespace std::chrono_literals;

        LoadSettings();

        constexpr bool morethan0 = false;
        DeleteItemBtn().IsEnabled(morethan0);
        ApplyBtn().IsEnabled(morethan0);
        ClearItemBtn().IsEnabled(morethan0);
        SelectAllBtn().IsEnabled(morethan0);
        InvertBtn().IsEnabled(morethan0);
        if (!NoFileAnim().IsPlaying())
        {
            no_await(NoFileAnim().PlayAsync(0, 1, true));
            NoFilePanel().Visibility(Visibility::Visible);
        }

        if (!vectorToken)
        {
            vectorToken = m_explorerItems.VectorChanged(auto_revoke, [=](auto&&, auto&&)
            {
                bool _morethan0 = m_explorerItems.Size() > 0;
                ClearItemBtn().IsEnabled(_morethan0);
                SelectAllBtn().IsEnabled(_morethan0);
                InvertBtn().IsEnabled(_morethan0);

                if (_morethan0)
                {
                    if (NoFileAnim().IsPlaying())
                    {
                        NoFilePanel().Visibility(Visibility::Collapsed);
                        NoFileAnim().Stop();
                    }
                }
                else
                {
                    if (!NoFileAnim().IsPlaying())
                    {
                        no_await(NoFileAnim().PlayAsync(0, 1, true));
                        NoFilePanel().Visibility(Visibility::Visible);
                    }

                    swapreset(m_explorerItemsSet);
                    swapreset(m_selectedIndex);
                    swapreset(m_rememberItems);
                }
            });
        }

        no_await([=]() -> Windows::Foundation::IAsyncAction
        {
            auto strong_this{ get_strong() };

            strong_this->m_explorerItems.Clear();
            swapreset(strong_this->m_explorerItemsSet);
            strong_this->m_recursiveSetted = false;
            
            if (auto pfiles{ strong_this->passed_files })
            {
                std::vector<hstring> temp{ pfiles.Size() };
                pfiles.GetMany(0, temp);
                pfiles.Clear();
                pfiles = nullptr;

                strong_this->SortItems(temp);
                for (auto const& i : temp)
                {
                    co_await strong_this->AddExplorerItem(i);
                }
            }
        });

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf1;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\QuickRename.xsmf").wstring().c_str(), &current_buf1);
                
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

    void QuickRename::Pageunloaded(Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        // Do Cleanup!

        vectorToken.~event_revoker();

        fileChecker.Stop();
        fileChecker_revoker.~event_revoker();
        fileChecker = nullptr;

        if (passed_files)
        {
            passed_files.Clear();
            passed_files = nullptr;
        }

        settings.~XSMF();

        if (m_explorerItems)
        {
            m_explorerItems.Clear();
            m_explorerItems = nullptr;
        }
        swapreset(m_explorerItemsSet);
        swapreset(m_selectedIndex);
        m_rememberItems.~vector();
        m_textToFind.~basic_string();
        m_newNameFormat.~basic_string();

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = nullptr;
    }

    void QuickRename::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        passed_files = unbox_value_or<IVectorString>(e.Parameter(), single_threaded_vector<hstring>());
    }

    Windows::Foundation::IAsyncAction QuickRename::FileListChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        auto items{ FileList().SelectedRanges() };

        // Enable or Disable Delete and Merge Button basedon 'items' size
        bool morethan0 = items.Size() > 0 ? true : false;
        DeleteItemBtn().IsEnabled(morethan0);
        ApplyBtn().IsEnabled(morethan0);

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        swapreset(m_selectedIndex);
        for (auto const&& i : items)
        {
            uint32_t start{ static_cast<uint32_t>(i.FirstIndex()) };
            for (uint32_t index = start; index < start + i.Length(); index++)
            {
                m_selectedIndex.insert(index);
            }
        }

        auto canReorder = m_selectedIndex.size() < 2;
        FileList().CanReorderItems(canReorder);
        FileList().CanDragItems(canReorder);

        setNewNameOp = SetNewName();
        co_await setNewNameOp;
    }

    Windows::Foundation::IAsyncAction QuickRename::ShowFilesPicker()
    {
        auto strong_this{ get_strong() };

        Windows::Storage::Pickers::FileOpenPicker picker{};
        picker.as<Xellanix::Desktop::IInitializeWithWindow>()->Initialize(Xellanix::Desktop::WindowHandle);
        picker.FileTypeFilter().Append(L"*");
        auto&& files{ co_await picker.PickMultipleFilesAsync() };

        if (!files) co_return;

        strong_this->m_recursiveSetted = false;
        std::vector<hstring> temp;
        temp.reserve(files.Size());
        for (auto&& file : files)
        {
            temp.emplace_back(file.Path());
        }
        temp.shrink_to_fit();

        strong_this->SortItems(temp);
        for (auto const& file : temp)
        {
            co_await strong_this->AddExplorerItem(file);
        }
    }

    Windows::Foundation::IAsyncAction QuickRename::AddClick(winrt::Microsoft::UI::Xaml::Controls::SplitButton const& /*sender*/, winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& /*e*/)
    {
        co_await ShowFilesPicker();
    }

    Windows::Foundation::IAsyncAction QuickRename::AddFilesFolderClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::ItemClickEventArgs const& e)
    {
        auto type = e.ClickedItem().as<hstring>();
        if (type == L"Files")
        {
            co_await ShowFilesPicker();
        }
        else if (type == L"Folders")
        {
            auto strong_this{ get_strong() };

            Windows::Storage::Pickers::FolderPicker picker{};
            picker.as<Xellanix::Desktop::IInitializeWithWindow>()->Initialize(Xellanix::Desktop::WindowHandle);
            picker.FileTypeFilter().Append(L"*");
            std::vector<hstring> folders;

            while (true)
            {
                auto&& folder{ co_await picker.PickSingleFolderAsync() };
                if (!folder)
                {
                    break;
                }
                else
                {
                    folders.emplace_back(folder.Path());
                }
            }

            if (folders.empty()) co_return;

            strong_this->m_recursiveSetted = false;
            strong_this->SortItems(folders);
            for (auto& folder : folders)
            {
                co_await strong_this->AddExplorerItem(folder);
            }
        }
    }

    void QuickRename::DeleteItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ FileList() };
        if (!list) return;

        std::vector<uint32_t> removed;
        removed.reserve(list.Items().Size());

        for (auto&& i : list.SelectedRanges())
        {
            uint32_t start{ static_cast<uint32_t>(i.FirstIndex()) };
            for (uint32_t index = start; index < start + i.Length(); index++)
            {
                removed.push_back(index);
            }
        }

        removed.shrink_to_fit();
        std::reverse(removed.begin(), removed.end());
        list.DeselectRange(Data::ItemIndexRange(0, list.Items().Size()));

        for (auto& i : removed)
        {
            m_explorerItemsSet.erase(m_explorerItems.GetAt(i).try_as<xmtmui::ExplorerItem>().ItemPath().c_str());
            m_explorerItems.RemoveAt(i);
            // list.Items().RemoveAt(i);
        }
    }

    void QuickRename::ClearItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_explorerItems.Clear();
        swapreset(m_explorerItemsSet);
    }

    void QuickRename::SelectAllClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ FileList() };
        if (!list) return;

        list.SelectAll();
    }

    void QuickRename::InvertSelectClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ FileList() };
        if (!list) return;

        using range_t = Data::ItemIndexRange;
        std::vector<range_t> invertedRange;
        {
            const auto available = list.Items().Size();
            std::vector<range_t> selectedRange;
            selectedRange.reserve(list.SelectedRanges().Size());
            for (auto&& i : list.SelectedRanges())
            {
                selectedRange.push_back(i);
            }

            if (selectedRange.size() > 0)
            {
                bool is_selectedFirst = selectedRange.front().FirstIndex() != 0;
                bool is_selectedLast = selectedRange.back().FirstIndex() + selectedRange.back().Length() != available;
                short to_reserve = (short)is_selectedFirst + (short)is_selectedLast - 1;
                invertedRange.reserve(selectedRange.size() + to_reserve);

                typedef decltype(range_t{ 0, 1 }.FirstIndex()) first_t;
                typedef decltype(range_t{ 0, 1 }.Length()) length_t;

                for (size_t i = 0; i < selectedRange.size(); i++)
                {
                    auto&& val_t = selectedRange[i];
                    if (val_t.FirstIndex() == 0) continue;
                    auto&& before_t = i == 0 ? range_t{ 0, 0 } : selectedRange[i - 1];

                    auto before = static_cast<first_t>(before_t.FirstIndex() + before_t.Length());
                    auto length = static_cast<length_t>((std::max)(val_t.FirstIndex() - before, 1));
                    range_t range{ before, length };
                    invertedRange.push_back(std::move(range));
                }

                if (auto lastval = static_cast<first_t>(selectedRange.back().FirstIndex() + selectedRange.back().Length()); static_cast<length_t>(lastval) != available)
                {
                    range_t range{ lastval, (std::max)(available - lastval, (length_t)1) };
                    invertedRange.push_back(std::move(range));
                }
            }
            else
            {
                invertedRange.reserve(1);
                range_t range{ 0, available };
                invertedRange.push_back(std::move(range));
            }
        }

        list.DeselectRange(range_t(0, list.Items().Size()));
        for (auto&& range : invertedRange)
        {
            list.SelectRange(range);
        }
    }

    void QuickRename::DeselectClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ FileList() };
        if (!list) return;

        list.DeselectRange(Data::ItemIndexRange(0, list.Items().Size()));
    }

    Windows::Foundation::IAsyncAction QuickRename::ApplyClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strong_this{ get_strong() };

        strong_this->RenamingPane().Visibility(Visibility::Visible);
        strong_this->RenamingBar().IsIndeterminate(true);
        if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong_this) })
        {
            mainPage.IsEnabled(false);
        }
        else
        {
            strong_this->ControlList().IsEnabled(false);
        }

        if (auto items{ strong_this->m_explorerItems })
        {
            const auto selectedCount = strong_this->m_selectedIndex.size();

            std::vector<std::tuple<uint32_t, std::wstring>> errors;
            std::vector<uint32_t> toRemoves;
            errors.reserve(selectedCount);
            toRemoves.reserve(selectedCount);

            co_await[=, &errors, &toRemoves]() -> Windows::Foundation::IAsyncAction
            {
                using folder_t = std::tuple<std::wstring, std::wstring, uint32_t>;

                strong_this->RenamingBar().Maximum((double)selectedCount);
                strong_this->RenamingBar().Value(0.0);
                strong_this->RenamingBar().IsIndeterminate(false);
                
                std::vector<folder_t> folders;
                folders.reserve(selectedCount);

                co_await resume_background();

                auto RenameImpl = [=, &errors, &toRemoves](std::wstring const renamed, fs::path const oldPath, uint32_t index)
                {
                    const auto newpath = oldPath.parent_path() / renamed;

                    std::error_code errcode;
                    fs::rename(oldPath, newpath, errcode);
                    if (errcode)
                    {
                        const auto errmes = Xellanix::Utilities::s_to_ws(errcode.message());
                        errors.emplace_back(index, errmes);
                    }
                    else
                    {
                        toRemoves.emplace_back(index);
                    }
                };

                for (auto& index : strong_this->m_selectedIndex)
                {
                    const auto&& item = items.GetAt(index).try_as<xmtmui::ExplorerItem>();
                    auto const oldpath = fs::path(std::wstring(item.ItemPath()));
                    auto const renamed = std::wstring(item.ItemRenamed());

                    if (fs::is_directory(oldpath))
                    {
                        folders.emplace_back(oldpath.wstring(), renamed, index);
                    }
                    else
                    {
                        RenameImpl(renamed, oldpath, index);

                        if (auto bar{ strong_this->RenamingBar() })
                        {
                            co_await resume_foreground(bar.Dispatcher());

                            bar.Value(bar.Value() + 1);

                            co_await resume_background();
                        }
                    }
                }

                folders.shrink_to_fit();
                std::sort(folders.begin(), folders.end(), [](folder_t const& lhs, folder_t const& rhs)
                {
                    return natural_less(std::get<0>(lhs), std::get<0>(rhs));
                });
                for (auto rit = folders.rbegin(); rit != folders.rend(); ++rit)
                {
                    const auto& [oldpath, renamed, index] = (*rit);
                    
                    RenameImpl(renamed, fs::path(oldpath), index);

                    if (auto bar{ strong_this->RenamingBar() })
                    {
                        co_await resume_foreground(bar.Dispatcher());

                        bar.Value(bar.Value() + 1);

                        co_await resume_background();
                    }
                }
            }();

            errors.shrink_to_fit();
            toRemoves.shrink_to_fit();
            std::sort(toRemoves.begin(), toRemoves.end(), std::greater<uint32_t>());

            if (errors.size() > 0)
            {
                ContentDialog dialog{};
                dialog.XamlRoot(XamlRoot());
                dialog.CloseButtonText(L"Close");
                dialog.DefaultButton(ContentDialogButton::Close);
                dialog.Title(box_value(L"Oh No, There Are Errors That Occur During the Rename Process!"));

                StackPanel dialogContent{};
                {
                    TextBlock message{};
                    message.Text(L"There is some additional information about the errors that occurred, which may be helpful in resolving the errors.");
                    message.TextWrapping(TextWrapping::WrapWholeWords);

                    dialogContent.Children().Append(message);
                }

                ScrollViewer scrollView{};
                scrollView.HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
                scrollView.HorizontalScrollMode(ScrollMode::Disabled);
                scrollView.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
                scrollView.VerticalScrollMode(ScrollMode::Enabled);
                scrollView.Margin(ThicknessHelper::FromLengths(0.0, 20.0, 0.0, 0.0));
                TextBlock txtb{};
                txtb.TextWrapping(TextWrapping::WrapWholeWords);

                auto isRemember = strong_this->m_rememberRenamed;
                if (isRemember)
                    strong_this->m_rememberItems.reserve(errors.size());

                for (size_t i = 0; i < errors.size(); i++)
                {
                    auto const& [_index, _mes] = errors[i];
                    std::wstring _path;
                    {
                        auto _item = items.GetAt(_index).try_as<xmtmui::ExplorerItem>();
                        _path = _item.ItemPath();

                        _item.IsRemember(isRemember);
                        if (isRemember)
                            strong_this->m_rememberItems.emplace_back(fs::path(_path).parent_path() / std::wstring(_item.ItemRenamed()));
                    }
                    Documents::Run run1{}, run2{};
                    run1.Text(L"\"" + _path + L"\"");
                    run1.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
                    run2.Text(L" >> " + _mes);

                    txtb.Inlines().Append(run1);
                    txtb.Inlines().Append(run2);

                    if (i + 1 == errors.size()) continue;

                    txtb.Inlines().Append(Documents::LineBreak{});
                    txtb.Inlines().Append(Documents::LineBreak{});
                }

                scrollView.Content(txtb);
                dialogContent.Children().Append(scrollView);
                dialog.Content(dialogContent);

                co_await dialog.ShowAsync();
            }

            if (auto filelist{ strong_this->FileList() })
            {
                filelist.DeselectRange(Data::ItemIndexRange(0, filelist.Items().Size()));
            }
            for (auto& i : toRemoves)
            {
                strong_this->m_explorerItemsSet.erase(strong_this->m_explorerItems.GetAt(i).try_as<xmtmui::ExplorerItem>().ItemPath().c_str());
                strong_this->m_explorerItems.RemoveAt(i);
            }
        }

        if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong_this) })
        {
            mainPage.IsEnabled(true);
        }
        else
        {
            strong_this->ControlList().IsEnabled(true);
        }

        strong_this->RenamingBar().IsIndeterminate(false);
        strong_this->RenamingPane().Visibility(Visibility::Collapsed);
    }

    void QuickRename::FilesDragOver(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::DragEventArgs const& e)
    {
        e.AcceptedOperation(Windows::ApplicationModel::DataTransfer::DataPackageOperation::Copy);
    }

    Windows::Foundation::IAsyncAction QuickRename::FilesDrop(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::DragEventArgs const& e)
    {
        auto strong_this{ get_strong() };

        if (e.DataView().Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems()))
        {
            auto&& items{ co_await e.DataView().GetStorageItemsAsync() };
            if (items.Size() == 0)
                co_return;

            strong_this->m_recursiveSetted = false;

            std::vector<hstring> temp;
            temp.reserve(items.Size());
            for (auto&& item : items)
            {
                if (item.IsOfType(Windows::Storage::StorageItemTypes::File))
                {
                    if (auto file{ item.try_as<Windows::Storage::StorageFile>() })
                    {
                        temp.emplace_back(file.Path());
                    }
                }
                else if (item.IsOfType(Windows::Storage::StorageItemTypes::Folder))
                {
                    if (auto folder{ item.try_as<Windows::Storage::StorageFolder>() })
                    {
                        temp.emplace_back(folder.Path());
                    }
                }
            }
            temp.shrink_to_fit();

            strong_this->SortItems(temp);
            for (auto const& item : temp)
            {
                co_await strong_this->AddExplorerItem(item);
            }
        }
    }

    void QuickRename::RecursiveBehavChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_recursiveBehav = static_cast<unsigned short>(RecursiveBehav().SelectedIndex());
        SaveSettings(L"95c41af7-cd4a-5d5a-b08c-24ce60cfb55b", m_recursiveBehav);
    }

    void QuickRename::AutoSortChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_autoSort = AutoSort().IsOn();
        SaveSettings(L"1102f064-e390-5331-bd67-760f6fe1c487", m_autoSort);
    }

    void QuickRename::RememberRenamedChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        m_rememberRenamed = RememberRenamed().IsOn();
        SaveSettings(L"a698a7d1-be04-54f2-8c62-f3414f5294ee", m_rememberRenamed);
    }

    Windows::Foundation::IAsyncAction QuickRename::ResetRememberClicked(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (m_rememberItems.size() == 0) co_return;

        for (uint32_t i = 0; i < m_explorerItems.Size(); i++)
        {
            if (auto item{ m_explorerItems.GetAt(i).try_as<xmtmui::ExplorerItem>() })
            {
                item.IsRemember(false);
            }
        }

        swapreset(m_rememberItems);

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;
    }

    Windows::Foundation::IAsyncAction QuickRename::RenameRepTextChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        m_renameRepText = RenameRepText().IsOn();

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;

        SaveSettings(L"b65d17ad-229c-5b56-8d54-96f70b60665b", m_renameRepText);
    }

    Windows::Foundation::IAsyncAction QuickRename::MatchWholeTextChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        m_matchWhole = MatchWholeText().IsOn();
        
        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;

        SaveSettings(L"3209a0fa-f2f5-5917-9b40-7639c83a87f9", m_matchWhole);
    }

    Windows::Foundation::IAsyncAction QuickRename::MatchCaseChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        m_matchCase = MatchCase().IsOn();
        
        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;

        SaveSettings(L"e1e7dd78-1dd8-566e-84a8-3cc91e5514a7", m_matchCase);
    }

    Windows::Foundation::IAsyncAction QuickRename::ApplyToChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        m_applyTo = static_cast<unsigned short>(ApplyTo().SelectedIndex());
        
        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;

        SaveSettings(L"21edaabb-210d-5dba-92fe-e90c95ae80e9", m_applyTo);
    }

    std::wstring RandomCharacter(std::wstring const& availableChars = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis(0ULL, (availableChars.size() - 1));

        return availableChars.substr(dis(gen), 1);
    }
    std::wstring RandomLowerCharacter()
    {
        return RandomCharacter(L"abcdefghijklmnopqrstuvwxyz");
    }
    std::wstring RandomUpperCharacter()
    {
        return RandomCharacter(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }

    bool IsRegexValid(const std::wstring& regex, std::regex_constants::syntax_option_type const& option)
    {
        try
        {
            std::wregex re(regex, option);
        }
        catch (const std::regex_error&)
        {
            return false;
        }
        return true;
    }
    bool QuickRename::IsRestartName(std::wstring const& finalpath)
    {
        if (Xellanix::Utilities::CheckExist(finalpath))
        {
            return true;
        }

        for (auto const& i : m_rememberItems)
        {
            if (i == finalpath)
            {
                return true;
            }
        }
        return false;
    }
    std::wstring CombinePaths(fs::path const& parent, std::wstring const& current)
    {
        return (parent / current).wstring();
    }

    Windows::Foundation::IAsyncOperation<hstring> QuickRename::GenerateNewName(std::wstring const& itemName, std::wstring const& itemPath, bool usespecialsymbols, uint32_t id, bool useregex, std::unordered_multiset<std::wstring>& renamed_paths)
    {
        co_await resume_background();

        auto const&& [nameonly, extonly] = split_filename(itemName);
        std::wstring renamed{ m_applyTo == 2 ? itemName :
            m_applyTo == 1 ? extonly :
            nameonly };

        const auto parent_p = fs::path(itemPath).parent_path();
        double posibilities;

        do
        {
            posibilities = 0.0;

            std::wstring newnameformat{ m_newNameFormat };
            if (usespecialsymbols)
            {
                const auto _time = Xellanix::Utilities::FileDateCreated(itemPath);
                unsigned long long contain;

                newnameformat = ReplaceIndexSymbol(newnameformat, id);

                contain = 0;

                newnameformat = ReplaceSymbol(newnameformat, LR"ss(@)ss", CALL_WITH_DEF(RandomCharacter), contain);
                newnameformat = ReplaceSymbol(newnameformat, LR"ss(\a)ss", RandomLowerCharacter, contain);
                newnameformat = ReplaceSymbol(newnameformat, LR"ss(\A)ss", RandomUpperCharacter, contain);
                posibilities = std::pow(26ULL, contain);
                contain = 0;

                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(h)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(m)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(s)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(d)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(M)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(yy)ss", _time);

                // Remove any leaving '\' characters
                for (auto beginOf = newnameformat.find_first_of(L"\\"); beginOf != std::wstring::npos; beginOf = newnameformat.find_first_of(L"\\"))
                {
                    const auto endOf = newnameformat.find_first_not_of(L"\\", beginOf);

                    const auto range = endOf - beginOf;

                    newnameformat.erase(beginOf, range);
                }
            }

            if (m_renameRepText)
            {
                renamed = std::wstring(m_applyTo == 2 ? itemName :
                                       m_applyTo == 1 ? extonly :
                                       nameonly);

                if (useregex)
                {
                    auto regexSetting = std::regex_constants::ECMAScript;
                    if (m_matchCase) regexSetting |= std::regex_constants::icase;

                    if (IsRegexValid(m_textToFind, regexSetting))
                    {
                        std::wregex reg(m_textToFind, regexSetting);
                        renamed = std::regex_replace(renamed, reg, newnameformat);
                    }
                }
                else
                {
                    // Dont use regex!
                    // Use ReplaceAll instead
                    // Find 'tofind' in 'renamed' and replace it with 'newnameformat'
                    renamed = ReplaceAll(renamed, m_matchWhole, m_matchCase, m_textToFind, newnameformat);
                }
            }
            else
            {
                // Replace all content of 'renamed' without looking what, with 'newnameformat'
                renamed = newnameformat;
            }

            if (m_applyTo == 1)
            {
                renamed = nameonly + renamed;
            }
            else if (m_applyTo == 0)
            {
                renamed += extonly;
            }

            size_t same_count;
            {
                auto _finalPath = CombinePaths(parent_p, renamed);
                renamed_paths.insert(_finalPath);

                same_count = renamed_paths.count(_finalPath);
            }
            if (same_count > 1)
            {
                if (id >= posibilities)
                {
                    renamed = add_before_extension(renamed, L" " + std::to_wstring(same_count));
                }
            }

            if (renamed == itemName)
            {
                break;
            }
        } while (IsRestartName(CombinePaths(parent_p, renamed)));

        co_return hstring(renamed);
    }

    Windows::Foundation::IAsyncAction QuickRename::SetNewName()
    {
        auto cancellation = co_await get_cancellation_token();
        cancellation.enable_propagation();

        auto strong{ get_strong() };
        
        auto itemsSize = strong->m_explorerItems.Size();
        using list_size = decltype(itemsSize);

        list_size id = 1;
        std::unordered_multiset<std::wstring> renamed_paths;
        renamed_paths.reserve(itemsSize);

        for (list_size i = 0; i < itemsSize; i++)
        {
            if (cancellation()) co_return;

            auto&& item = strong->m_explorerItems.GetAt(i).try_as<xmtmui::ExplorerItem>();
            if (item.IsRemember()) continue;

            if (strong->m_selectedIndex.count(i) == 0)
            {
                // not selected!
                item.ItemRenamed(L"");
            }
            else
            {
                // selected!
                hstring renamed = item.ItemName();
                if (!strong->m_textToFind.empty() || !strong->m_renameRepText)
                {
                    renamed = co_await strong->GenerateNewName(std::wstring(item.ItemName()),
                                                               std::wstring(item.ItemPath()),
                                                               strong->UseSpecialSymbols().IsChecked().Value(),
                                                               id,
                                                               strong->UseRegex().IsChecked().Value(),
                                                               renamed_paths);
                }

                item.ItemRenamed(renamed);
                ++id;
            }
        }
    }

    Windows::Foundation::IAsyncAction QuickRename::SpecialFormattingCheckChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;
    }

    void QuickRename::TextToFind_KeyDown(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            using namespace Windows::UI::Xaml::Input;
            
            FindNextElementOptions option;
            option.SearchRoot(XamlRoot().Content());
            FocusManager::TryMoveFocus(FocusNavigationDirection::Next, option);
            
            // Make sure to set the Handled to true, otherwise the RoutedEvent might fire twice
            e.Handled(true);
        }
    }

    Windows::Foundation::IAsyncAction QuickRename::TextToFind_LostFocus(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        if (auto control{ TextToFind() })
        {
            auto tempText = control.Text();
            if (m_textToFind != tempText)
                m_textToFind = tempText;
            else co_return;
        }

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;
    }

    void QuickRename::NewNameFormat_KeyDown(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            using namespace Windows::UI::Xaml::Input;

            FindNextElementOptions option;
            option.SearchRoot(XamlRoot().Content());
            FocusManager::TryMoveFocus(FocusNavigationDirection::Next, option);

            // Make sure to set the Handled to true, otherwise the RoutedEvent might fire twice
            e.Handled(true);
        }
    }

    Windows::Foundation::IAsyncAction QuickRename::NewNameFormat_LostFocus(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) co_return;

        if (auto control{ NewNameFormat() })
        {
            auto tempText = control.Text();
            if (m_newNameFormat != tempText)
                m_newNameFormat = tempText;
            else co_return;
        }

        if (!m_newNameFormat.empty())
        {
            // For '\' character, remove it GenerateNewName function
            std::wstring notSupported{ L"/:*\"<>|" };

            for (auto beginOf = m_newNameFormat.find_first_of(notSupported); beginOf != std::wstring::npos; beginOf = m_newNameFormat.find_first_of(notSupported))
            {
                const auto endOf = m_newNameFormat.find_first_not_of(notSupported, beginOf);
                
                const auto range = endOf - beginOf;

                m_newNameFormat.erase(beginOf, range);
            }
        }

        if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }
        setNewNameOp = SetNewName();
        co_await setNewNameOp;
    }

    void QuickRename::LoadSettings()
    {
        isloaded = false;

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\QuickRename.xsmf"))
        {
            try
            {
                m_recursiveBehav = (settings >> L"95c41af7-cd4a-5d5a-b08c-24ce60cfb55b").try_as<unsigned short>(2);
                m_autoSort = (settings >> L"1102f064-e390-5331-bd67-760f6fe1c487").try_as<bool>(true);
                m_rememberRenamed = (settings >> L"a698a7d1-be04-54f2-8c62-f3414f5294ee").try_as<bool>(true);
                m_renameRepText = (settings >> L"b65d17ad-229c-5b56-8d54-96f70b60665b").try_as<bool>(true);
                m_matchWhole = (settings >> L"3209a0fa-f2f5-5917-9b40-7639c83a87f9").try_as<bool>(false);
                m_matchCase = (settings >> L"e1e7dd78-1dd8-566e-84a8-3cc91e5514a7").try_as<bool>(false);
                m_applyTo = (settings >> L"21edaabb-210d-5dba-92fe-e90c95ae80e9").try_as<unsigned short>(0);
            }
            catch (...)
            {
            }

            RecursiveBehav().SelectedIndex(m_recursiveBehav);
            AutoSort().IsOn(m_autoSort);
            RememberRenamed().IsOn(m_rememberRenamed);
            RenameRepText().IsOn(m_renameRepText);
            MatchWholeText().IsOn(m_matchWhole);
            MatchCase().IsOn(m_matchCase);
            ApplyTo().SelectedIndex(m_applyTo);
        }
        else
        {
            settings[L"95c41af7-cd4a-5d5a-b08c-24ce60cfb55b"] = m_recursiveBehav;
            settings[L"1102f064-e390-5331-bd67-760f6fe1c487"] = m_autoSort;
            settings[L"a698a7d1-be04-54f2-8c62-f3414f5294ee"] = m_rememberRenamed;
            settings[L"b65d17ad-229c-5b56-8d54-96f70b60665b"] = m_renameRepText;
            settings[L"3209a0fa-f2f5-5917-9b40-7639c83a87f9"] = m_matchWhole;
            settings[L"e1e7dd78-1dd8-566e-84a8-3cc91e5514a7"] = m_matchCase;
            settings[L"21edaabb-210d-5dba-92fe-e90c95ae80e9"] = m_applyTo;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        QuickRename::SaveSettings(std::wstring const& name, T const& value)
    {
        settings[name] = value;

        settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\QuickRename.xsmf");
        get_ftime(L"Settings\\QuickRename.xsmf", last_ctime1, last_mtime1, last_size1);
    }
}
