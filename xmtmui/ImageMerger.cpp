#include "pch.h"
#include "ImageMerger.h"
#if __has_include("ImageMerger.g.cpp")
#include "ImageMerger.g.cpp"
#endif
#if __has_include("ImageItem.g.cpp")
#include "ImageItem.g.cpp"
#endif
#include <fstream>

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::xmtmui::implementation
{
    ImageMerger::ImageMerger()
    {
        InitializeComponent();
        
        m_imageItems = single_threaded_observable_vector<Windows::Foundation::IInspectable>();
    }

    void ImageMerger::FromFile(param::hstring const& filepath)
    {
        fs::path fpath{ filepath.operator const winrt::hstring & ().c_str() };

        if (Xellanix::Utilities::CheckExist(fpath))
        {
            auto fname = fpath.filename().wstring();
            
            auto newItem = winrt::make<implementation::ImageItem>(hstring(fname), hstring(fpath.wstring()));
            m_imageItems.Append(newItem);
        }
    }

    void ImageMerger::ImageSelectionChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        auto items{ ImageList().SelectedRanges() };

        // Enable or Disable Delete and Merge Button basedon 'items' size
        bool morethan0 = items.Size() > 0 ? true : false;
        DeleteImageBtn().IsEnabled(morethan0);
        MergeImageBtn().IsEnabled(morethan0);
    }

    Windows::Foundation::IAsyncAction ImageMerger::AddClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strong_this{ get_strong() };

        Windows::Storage::Pickers::FileOpenPicker picker{};
        picker.as<Xellanix::Desktop::IInitializeWithWindow>()->Initialize(Xellanix::Desktop::WindowHandle);
        picker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);
        picker.FileTypeFilter().Append(L".png");
        picker.FileTypeFilter().Append(L".jpg");
        picker.FileTypeFilter().Append(L".jpeg");
        auto&& files{ co_await picker.PickMultipleFilesAsync() };

        if (!files) co_return;

        for (auto&& file : files)
        {
            strong_this->FromFile(file.Path());
        }
    }

    Windows::Foundation::IAsyncAction ImageMerger::ClipboardClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        using namespace Windows::ApplicationModel::DataTransfer;
        using namespace Windows::Graphics::Imaging;
        using namespace Windows::Storage;

        auto strong_this{ get_strong() };

        auto content{ Clipboard::GetContent() };
        if (content.Contains(StandardDataFormats::Bitmap()))
        {
            try
            {
                auto&& image{ co_await content.GetBitmapAsync() };

                if (image)
                {
                    auto&& imageStream = co_await image.OpenReadAsync();
                    {
                        auto&& decoder{ co_await BitmapDecoder::CreateAsync(imageStream.CloneStream()) };
                        
                        const auto tempFolder = Xellanix::Utilities::LocalTemp / L"Screenshots";
                        if (!fs::exists(tempFolder))
                            fs::create_directories(tempFolder);

                        auto&& folder = co_await StorageFolder::GetFolderFromPathAsync(tempFolder.wstring());
                        auto&& storageFile{ co_await folder.CreateFileAsync(L"Screenshot.png", CreationCollisionOption::GenerateUniqueName) };
                        if (storageFile)
                        {
                            auto&& encoder{ co_await BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), co_await storageFile.OpenAsync(FileAccessMode::ReadWrite)) };
                            {
                                auto&& pixels{ co_await decoder.GetPixelDataAsync() };
                                encoder.SetPixelData(decoder.BitmapPixelFormat(),
                                                     BitmapAlphaMode::Premultiplied,
                                                     decoder.OrientedPixelWidth(),
                                                     decoder.OrientedPixelHeight(),
                                                     decoder.DpiX(), decoder.DpiY(),
                                                     pixels.DetachPixelData());
                            }
                            
                            co_await encoder.FlushAsync();

                            strong_this->FromFile(storageFile.Path());
                        }
                    }

                    imageStream.Close();
                }
            }
            catch (...)
            {

            }
        }
    }

    void ImageMerger::DeleteItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ ImageList() };
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

        for (auto& i : removed)
        {
            m_imageItems.RemoveAt(i);
        }
    }

    void ImageMerger::ClearItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_imageItems.Clear();
    }

    Windows::Foundation::IAsyncAction ImageMerger::MergeItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strong_this{ get_strong() };
        strong_this->last_op_t = time(0);
        strong_this->last_op = Xellanix::Utilities::TimeString(Xellanix::Utilities::localtime_x(strong_this->last_op_t), L"%a, %B %d, %Y, %H:%M:%S");

        Windows::Storage::Pickers::FileSavePicker picker{};
        picker.as<Xellanix::Desktop::IInitializeWithWindow>()->Initialize(Xellanix::Desktop::WindowHandle);
        picker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);
        picker.SuggestedFileName(L"Merge Result");
        if (strong_this->defaultType == L".png")
        {
            picker.FileTypeChoices().Insert(L"PNG File", single_threaded_vector<hstring>({ L".png" }));
            picker.FileTypeChoices().Insert(L"JPEG File", single_threaded_vector<hstring>({ L".jpeg", L".jpg" }));
            picker.FileTypeChoices().Insert(L"PDF File", single_threaded_vector<hstring>({ L".pdf" }));
        }
        else if(strong_this->defaultType == L".jpeg")
        {
            picker.FileTypeChoices().Insert(L"JPEG File", single_threaded_vector<hstring>({ L".jpeg", L".jpg" }));
            picker.FileTypeChoices().Insert(L"PNG File", single_threaded_vector<hstring>({ L".png" }));
            picker.FileTypeChoices().Insert(L"PDF File", single_threaded_vector<hstring>({ L".pdf" }));
        }
        else if (strong_this->defaultType == L".pdf")
        {
            picker.FileTypeChoices().Insert(L"PDF File", single_threaded_vector<hstring>({ L".pdf" }));
            picker.FileTypeChoices().Insert(L"PNG File", single_threaded_vector<hstring>({ L".png" }));
            picker.FileTypeChoices().Insert(L"JPEG File", single_threaded_vector<hstring>({ L".jpeg", L".jpg" }));
        }
        auto&& file{ co_await picker.PickSaveFileAsync() };
        if (file)
        {
            strong_this->last_file_path = file.Path().c_str();

            strong_this->MergingPane().Visibility(Visibility::Visible);
            strong_this->MergingBar().IsIndeterminate(true);

            auto list{ strong_this->ImageList() };
            if (!list) co_return;

            if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong_this) })
            {
                mainPage.IsEnabled(false);
            }
            else
            {
                strong_this->ControlList().IsEnabled(false);
                list.IsEnabled(false);
            }

            if (auto info{ strong_this->MergingInfo() })
            {
                info.IsOpen(false);

                info.Severity(muxc::InfoBarSeverity::Warning);
                info.Title(L"Warning: ");
                info.Message(L"Don't move to another page during the merging process! Moving to another page may cause the merge process to be aborted.");
                info.ActionButton().Visibility(Visibility::Collapsed);
                info.ActionButton().IsEnabled(false);

                info.IsOpen(true);
            }

            // Contruct images path list and config file
            {
                std::locale::global(std::locale(std::locale("C"), new cvt::utf8cvt<wchar_t>()));

                const fs::path appft = Xellanix::Utilities::LocalAppData / L"pdftools";
                if (!fs::exists(appft))
                    fs::create_directories(appft);
                const std::wstring ip = (appft / L"images.txt").wstring();
                const std::wstring cp = (appft / L"config.txt").wstring();
                std::wofstream iwof(ip), cwof(cp);

                cwof << LR"(d6bb8c8c-d255-46af-9d2d-11ee63f8d85a : )" << std::wstring(file.FileType().c_str()).substr(1) << L"\n";
                cwof << LR"(b4897706-b448-4ca2-9322-4d8cdc474ef5 : )" << strong_this->last_file_path << L"\n";
                cwof << LR"(6814ba7a-45f3-47df-907e-bbcfda04e924 : )" << strong_this->_expansionMode << L"\n";
                cwof << LR"(f7d64306-d529-4972-aaa1-75d622626acc : )" << (strong_this->_useCompression ? L"true" : L"false") << L"\n";
                cwof << LR"(0019f40c-feba-42de-875a-10d7a8626e2e : )" << std::to_wstring(strong_this->_compressionIntensity);
                {
                    std::vector<std::wstring> simages;
                    simages.reserve(list.Items().Size());
                    for (auto&& range : list.SelectedRanges())
                    {
                        uint32_t start{ static_cast<uint32_t>(range.FirstIndex()) };
                        for (uint32_t index = start; index < start + range.Length(); index++)
                        {
                            if (auto imgitem{ list.Items().GetAt(index).try_as<xmtmui::ImageItem>() })
                            {
                                simages.push_back(imgitem.ItemPath().c_str());
                            }
                        }
                    }

                    simages.shrink_to_fit();

                    for (size_t i = 0; i < simages.size(); i++)
                    {
                        iwof << simages[i];
                        if (i + 1 < simages.size()) iwof << L"\n";
                    }
                }

                iwof.close();
                cwof.close();
            }

            STARTUPINFO si;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&strong_this->pi, sizeof(strong_this->pi));

            //Prepare CreateProcess args
            const std::wstring app_w = (fs::path(Xellanix::Utilities::AppDir) / L"pdftools\\pdftools.exe").wstring();
            const std::wstring arg_w = (L"\"" + app_w +
                                        L"\" \"" +
                                        (Xellanix::Utilities::LocalAppData / L"pdftools").wstring() +
                                        L"\" \"" +
                                        (Xellanix::Utilities::LocalTemp / L"pdftools").wstring() +
                                        L"\"");
            
            if (!CreateProcess(app_w.c_str(), const_cast<wchar_t*>(arg_w.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &strong_this->pi))
            {
                if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong_this) })
                {
                    mainPage.IsEnabled(true);
                }
                else
                {
                    list.IsEnabled(true);
                    strong_this->ControlList().IsEnabled(true);
                }
                strong_this->MergingBar().IsIndeterminate(false);
                strong_this->MergingPane().Visibility(Visibility::Collapsed);

                strong_this->checkProcess.Stop();

                if (auto info{ strong_this->MergingInfo() })
                {
                    info.IsOpen(false);

                    info.Severity(muxc::InfoBarSeverity::Error);
                    info.Title(L"Merging Failed!");
                    info.Message(L"last operation time: " + strong_this->last_op);
                    info.ActionButton().Visibility(Visibility::Collapsed);
                    info.ActionButton().IsEnabled(false);
                    info.IsOpen(true);
                }
            }
            else
            {
                strong_this->checkProcess.Start();
            }
        }
    }

    void ImageMerger::OpenResultClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        ShellExecute(NULL, NULL, last_file_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    void ImageMerger::PageLoaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        LoadSettings();

        constexpr bool morethan0 = false;
        DeleteImageBtn().IsEnabled(morethan0);
        MergeImageBtn().IsEnabled(morethan0);
        ClearImageBtn().IsEnabled(morethan0);
        SelectAllBtn().IsEnabled(morethan0);
        InvertBtn().IsEnabled(morethan0);
        if (!NoFileAnim().IsPlaying())
        {
            no_await(NoFileAnim().PlayAsync(0, 1, true));
            NoFilePanel().Visibility(Visibility::Visible);
        }

        if (!vectorToken)
        {
            vectorToken = m_imageItems.VectorChanged(auto_revoke, [=](auto&&, auto&&)
            {
                bool _morethan0 = m_imageItems.Size() > 0 ? true : false;
                ClearImageBtn().IsEnabled(_morethan0);
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
                }
            });
        }

        using namespace std::chrono_literals;
        using namespace Xellanix::Utilities;

        checkProcess.Interval(1s);
        if (!tick_revoker)
        {
            tick_revoker = checkProcess.Tick(auto_revoke, [=, weak_this{ get_weak() }](auto&&, auto&&)
            {
                if (auto strong{ weak_this.get() })
                {
                    if (!IsProcessRunning(strong->pi))
                    {
                        if (auto info{ strong->MergingInfo() })
                        {
                            if (CheckExist(strong->last_file_path))
                            {
                                // Check date created or modif was after last operation
                                // if not it was failed!

                                time_t dc = 0, dm = 0;
                                long long ds = 0;
                                {
                                    struct _stati64 buf;
                                    if (_wstat64(strong->last_file_path.c_str(), &buf) == 0)
                                    {
                                        dc = buf.st_ctime;
                                        dm = buf.st_mtime;
                                        ds = buf.st_size;
                                    }
                                }

                                if ((dc >= strong->last_op_t || dm >= strong->last_op_t) && ds > 0)
                                {
                                    // Success!
                                    info.IsOpen(false);

                                    info.Severity(muxc::InfoBarSeverity::Success);
                                    info.Title(L"Merging Success!");
                                    info.Message(L"last operation time: " + strong->last_op);
                                    info.ActionButton().Visibility(Visibility::Visible);
                                    info.ActionButton().IsEnabled(true);
                                    info.IsOpen(true);
                                }
                                else
                                {
                                    // Failed!
                                    info.IsOpen(false);

                                    info.Severity(muxc::InfoBarSeverity::Error);
                                    info.Title(L"Merging Failed!");
                                    info.Message(L"last operation time: " + strong->last_op);
                                    info.ActionButton().Visibility(Visibility::Collapsed);
                                    info.ActionButton().IsEnabled(false);
                                    info.IsOpen(true);
                                }
                            }
                            else
                            {
                                // Failed to create file
                                info.IsOpen(false);

                                info.Severity(muxc::InfoBarSeverity::Error);
                                info.Title(L"Merging Failed!");
                                info.Message(L"last operation time: " + strong->last_op);
                                info.ActionButton().Visibility(Visibility::Collapsed);
                                info.ActionButton().IsEnabled(false);
                                info.IsOpen(true);
                            }
                        }

                        if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong) })
                        {
                            mainPage.IsEnabled(true);
                        }
                        else
                        {
                            strong->ImageList().IsEnabled(true);
                            strong->ControlList().IsEnabled(true);
                        }
                        strong->MergingBar().IsIndeterminate(false);
                        strong->MergingPane().Visibility(Visibility::Collapsed);

                        strong->checkProcess.Stop();

                        if (strong->pi.hProcess != NULL || strong->pi.hThread != NULL)
                        {
                            if (CloseHandle(strong->pi.hProcess))
                            {
                                strong->pi.hProcess = NULL;
                            }
                            if (CloseHandle(strong->pi.hThread))
                            {
                                strong->pi.hProcess = NULL;
                            }
                        }
                    }
                }
            });
        }

        m_imageItems.Clear();
        for (auto const& i : passed_files)
        {
            FromFile(i);
        }
        passed_files.Clear();
        passed_files = nullptr;

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                struct _stati64 current_buf;
                auto res = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\ImageMerger.xsmf").wstring().c_str(), &current_buf);

                if (res == 0)
                {
                    if (current_buf.st_ctime != last_ctime1 ||
                        current_buf.st_mtime != last_mtime1 ||
                        current_buf.st_size != last_size1)
                    {
                        LoadSettings();

                        last_ctime1 = current_buf.st_ctime;
                        last_mtime1 = current_buf.st_mtime;
                        last_size1 = current_buf.st_size;
                    }
                }
            });
        }
        fileChecker.Start();
    }

    void ImageMerger::Pageunloaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        vectorToken.~event_revoker();

        checkProcess.Stop();
        tick_revoker.~event_revoker();
        checkProcess = nullptr;

        if (pi.hProcess != NULL || pi.hThread != NULL)
        {
            if (CloseHandle(pi.hProcess))
            {
                pi.hProcess = NULL;
            }
            if (CloseHandle(pi.hThread))
            {
                pi.hProcess = NULL;
            }
        }
        pi = PROCESS_INFORMATION{};

        fileChecker.Stop();
        fileChecker_revoker.~event_revoker();
        fileChecker = nullptr;

        if (passed_files)
        {
            passed_files.Clear();
            passed_files = nullptr;
        }

        defaultType.~basic_string();
        last_op.~basic_string();
        last_file_path.~basic_string();
        _expansionMode.~basic_string();

        settings.~XSMF();

        if (m_imageItems)
        {
            m_imageItems.Clear();
            m_imageItems = nullptr;
        }

        const auto tempFolder = Xellanix::Utilities::LocalTemp / L"Screenshots";
        if (Xellanix::Utilities::CheckExist(tempFolder))
        {
            fs::remove_all(tempFolder);
        }
    }

    void ImageMerger::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        using IVectorString = Windows::Foundation::Collections::IVector<hstring>;

        passed_files = unbox_value_or<IVectorString>(e.Parameter(), single_threaded_vector<hstring>());
    }

    void ImageMerger::FilesDragOver(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::DragEventArgs const& e)
    {
        e.AcceptedOperation(Windows::ApplicationModel::DataTransfer::DataPackageOperation::Copy);
    }

    Windows::Foundation::IAsyncAction ImageMerger::FilesDrop(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::DragEventArgs const& e)
    {
        auto strong_this{ get_strong() };

        if (e.DataView().Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems()))
        {
            auto&& items{ co_await e.DataView().GetStorageItemsAsync() };
            if (items.Size() == 0)
                co_return;

            for (auto&& item : items)
            {
                if (!item.IsOfType(Windows::Storage::StorageItemTypes::File))
                    continue;

                if (auto file{ item.try_as<Windows::Storage::StorageFile>() })
                {
                    auto contentType{ file.ContentType() };
                    if (contentType == L"image/jpg" || contentType == L"image/png" || contentType == L"image/jpeg")
                    {
                        strong_this->FromFile(file.Path());
                    }
                }
            }
        }
    }

    void ImageMerger::SelectAllClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ ImageList() };
        if (!list) return;

        list.SelectAll();
    }

    void ImageMerger::InvertSelectClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ ImageList() };
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

        std::vector<range_t> ranges;
        for (auto&& range : list.SelectedRanges())
        {
            ranges.push_back(range);
        }
        for (size_t i = 0; i < ranges.size(); i++)
        {
            list.DeselectRange(ranges[i]);
        }

        for (auto&& range : invertedRange)
        {
            list.SelectRange(range);
        }
    }

    void ImageMerger::DeselectClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ ImageList() };
        if (!list) return;

        std::vector<Windows::UI::Xaml::Data::ItemIndexRange> ranges;
        for (auto&& range : list.SelectedRanges())
        {
            ranges.push_back(range);
        }
        for (size_t i = 0; i < ranges.size(); i++)
        {
            list.DeselectRange(ranges[i]);
        }
    }

    void ImageMerger::DefaultExtensionChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        auto index = DefaultExtension().SelectedIndex();
        switch (index)
        {
            case 0: // PDF
                defaultType = L".pdf";
                break;
            case 1: // PNG
                defaultType = L".png";
                break;
            case 2: // JPEG
                defaultType = L".jpeg";
                break;
            default: // PDF
                defaultType = L".pdf";
                break;
        }

        SaveSettings(L"0b96051e-d349-5ed2-b747-55e4dff705b0", defaultType);
    }

    void ImageMerger::UseCompressionChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        _useCompression = UseCompression().IsOn();

        SaveSettings(L"32a7dff8-7bc2-51af-aec0-74b5bb63f4c3", _useCompression);
    }

    void ImageMerger::IntensityChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args)
    {
        if (!isloaded) return;

        auto val = (std::min)(static_cast<unsigned short>(sender.Text() == L"" ? args.OldValue() : sender.Value()), 100ui16);
        sender.Value(val);

        _compressionIntensity = val;

        SaveSettings(L"f7ecd462-6ab5-5904-b95f-1a2c58f5519c", _compressionIntensity);
    }

    void ImageMerger::ExpansionModeChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        _expansionMode = ExpansionMode().SelectedIndex() == 0 ? L"horizontal" : L"vertical";

        SaveSettings(L"12b83ac9-aa3e-5ce4-8eee-fc9bbc80edad", _expansionMode);
    }

    void ImageMerger::LoadSettings()
    {
        isloaded = false;

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\ImageMerger.xsmf"))
        {
            try
            {
                defaultType = (settings >> L"0b96051e-d349-5ed2-b747-55e4dff705b0").try_as<std::wstring>(L".pdf");
                _useCompression = (settings >> L"32a7dff8-7bc2-51af-aec0-74b5bb63f4c3").try_as<bool>(false);
                _compressionIntensity = (std::min)((settings >> L"f7ecd462-6ab5-5904-b95f-1a2c58f5519c").try_as<unsigned short>(25ui16), 100ui16);
                _expansionMode = (settings >> L"12b83ac9-aa3e-5ce4-8eee-fc9bbc80edad").try_as<std::wstring>(L"vertical");
            }
            catch (...)
            { }

            if (defaultType == L".pdf")
            {
                DefaultExtension().SelectedIndex(0);
            }
            else if (defaultType == L".png")
            {
                DefaultExtension().SelectedIndex(1);
            }
            else if (defaultType == L".jpeg")
            {
                DefaultExtension().SelectedIndex(2);
            }
            else
            {
                DefaultExtension().SelectedIndex(0);
            }
            UseCompression().IsOn(_useCompression);
            CompressionIntensity().Value(_compressionIntensity);
            ExpansionMode().SelectedIndex(_expansionMode == L"horizontal" ? 0 : 1);
        }
        else
        {
            settings[L"0b96051e-d349-5ed2-b747-55e4dff705b0"] = defaultType;
            settings[L"32a7dff8-7bc2-51af-aec0-74b5bb63f4c3"] = _useCompression;
            settings[L"f7ecd462-6ab5-5904-b95f-1a2c58f5519c"] = _compressionIntensity;
            settings[L"12b83ac9-aa3e-5ce4-8eee-fc9bbc80edad"] = _expansionMode;
        }

        isloaded = true;
    }

    template<typename T>
    typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
        ImageMerger::SaveSettings(std::wstring const& name, T const& value)
    {
        settings[name] = value;

        settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\ImageMerger.xsmf");
        get_ftime(L"Settings\\ImageMerger.xsmf", last_ctime1, last_mtime1, last_size1);
    }
}
