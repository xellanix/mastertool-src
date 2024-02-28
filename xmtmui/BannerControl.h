#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "BannerControl.g.h"
#include "BannerChild.g.h"

namespace winrt::xmtmui::implementation
{
    struct BannerChild : BannerChildT<BannerChild>
    {
    private:
        hstring m_imagePath;
        Windows::UI::Xaml::UIElement m_bannerContent{ nullptr };

    public:
        BannerChild() = default;

        BannerChild(hstring const& imagePath, Windows::UI::Xaml::UIElement const& bannerContent) : m_imagePath(imagePath), m_bannerContent(bannerContent)
        {}

        hstring ImagePath() const { return m_imagePath; }
        Windows::UI::Xaml::UIElement BannerContent() const { return m_bannerContent; }

        void ImagePath(hstring const& value)
        {
            m_imagePath = value;
        }
        void BannerContent(Windows::UI::Xaml::UIElement const& value)
        {
            m_bannerContent = value;
        }
    };

    struct BannerControl : BannerControlT<BannerControl>
    {
    private:
        using BannerItemsT = Windows::Foundation::Collections::IObservableVector<xmtmui::BannerChild>;

        BannerItemsT m_bannerItems{ nullptr };
        BannerItemsT::VectorChanged_revoker m_bannerItems_revoker{};

        Windows::UI::Xaml::DispatcherTimer m_timer{};
        Windows::UI::Xaml::DispatcherTimer::Tick_revoker m_timer_revoker{};

        bool progIndexChanged = false;
        uint32_t m_index = 0;

        void ShowBanner(uint32_t index);

    public:
        BannerControl();

        BannerItemsT BannerItems() const { return m_bannerItems; }
        uint32_t SelectedBannerIndex() const { return m_index; }

        void BannerItems(BannerItemsT const& value)
        {
            if (value != m_bannerItems)
            {
                m_bannerItems = value;
            }
        }
        void SelectedBannerIndex(uint32_t const& value)
        {
            if (value != m_index)
            {
                ShowBanner(value);
            }
        }

        void ControlLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void ControlUnloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void CloseBannerBoard_Completed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void BannerPager_CurrentIndexChanged(winrt::Windows::Foundation::IInspectable const& sender, uint32_t const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct BannerChild : BannerChildT<BannerChild, implementation::BannerChild>
    {

    };

    struct BannerControl : BannerControlT<BannerControl, implementation::BannerControl>
    {
    };
}
