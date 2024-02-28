#pragma once

#include "ImageMerger.g.h"
#include "ImageItem.g.h"
#include "Utilities.h"
#include "winrt/Windows.UI.Xaml.Media.Imaging.h"
#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Windows.ApplicationModel.DataTransfer.h"
#include "winrt/Windows.Graphics.Imaging.h"
#include "winrt/Windows.UI.Xaml.Documents.h"
#include "winrt/Windows.UI.Text.h"
#include "../../../Libraries/xellanix.objects.h"
#include "Lottie/AddFile.h"

namespace winrt::xmtmui::implementation
{
    struct ImageItem : ImageItemT<ImageItem>
    {
    private:
        hstring m_itemName{ L"" };
        hstring m_itemPath{ L"" };

    public:
        ImageItem() = default;

        ImageItem(hstring const& itemName, hstring const& itemPath) : m_itemName(itemName), m_itemPath(itemPath)
        {
        }

        hstring ItemName() { return m_itemName; }
        hstring ItemPath() { return m_itemPath; }

        void ItemName(hstring const& value)
        {
            m_itemName = value;
        }
        void ItemPath(hstring const& value)
        {
            m_itemPath = value;
        }
    };

    struct ImageMerger : ImageMergerT<ImageMerger>
    {
    private:
        using InspectableVector = Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable>;

        InspectableVector::VectorChanged_revoker vectorToken{};
        Windows::UI::Xaml::DispatcherTimer::Tick_revoker tick_revoker{}, fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer checkProcess{}, fileChecker{};
        PROCESS_INFORMATION pi = {};

        Windows::Foundation::Collections::IVector<hstring> passed_files{ single_threaded_vector<hstring>() };

        std::wstring defaultType{ L".pdf" };
        std::time_t last_op_t = time(0);
        std::wstring last_op = L"";
        std::wstring last_file_path = L"";
        std::wstring _expansionMode = L"vertical";
        bool _useCompression = false;
        unsigned short _compressionIntensity = 25;

        bool isloaded = false;
        time_t last_ctime1 = 0, last_mtime1 = 0;
        long long last_size1 = 0;
        Xellanix::Objects::XSMF settings{ L"Image Merger" };

        InspectableVector m_imageItems;

        void FromFile(winrt::param::hstring const& filepath);
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);

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

    public:
        ImageMerger();

        InspectableVector ImageItems() { return m_imageItems; }
        void ImageItems(InspectableVector const& value)
        {
            if (m_imageItems != value)
            {
                m_imageItems = value;
            }
        }

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);

        void ImageSelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        Windows::Foundation::IAsyncAction ClipboardClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void DeleteItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction MergeItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction AddClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void ClearItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void Pageunloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void FilesDragOver(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::DragEventArgs const& e);
        Windows::Foundation::IAsyncAction FilesDrop(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::DragEventArgs const& e);
        void SelectAllClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void InvertSelectClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void DeselectClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void DefaultExtensionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void UseCompressionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void IntensityChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void ExpansionModeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void OpenResultClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct ImageItem : ImageItemT<ImageItem, implementation::ImageItem>
    {
    };

    struct ImageMerger : ImageMergerT<ImageMerger, implementation::ImageMerger>
    {
    };
}
