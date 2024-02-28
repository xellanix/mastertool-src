#include "pch.h"
#include "BannerControl.h"
#if __has_include("BannerControl.g.cpp")
#include "BannerControl.g.cpp"
#endif
#if __has_include("BannerChild.g.cpp")
#include "BannerChild.g.cpp"
#endif
#include "winrt/Windows.UI.Xaml.Media.Imaging.h"
#include "winrt/Windows.UI.Xaml.Media.Animation.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::xmtmui::implementation
{
    BannerControl::BannerControl() : m_bannerItems(single_threaded_observable_vector<xmtmui::BannerChild>())
    {
        InitializeComponent();
    }

    void BannerControl::ShowBanner(uint32_t index)
    {
        progIndexChanged = true;

        if (xmtmui::CardPager pager{ BannerPager() })
        {
            pager.CurrentIndex(index);
        }
        
        m_index = index;
        CloseBannerBoard().Begin();
        if (m_timer) m_timer.Stop();

        progIndexChanged = false;
    }

    void BannerControl::CloseBannerBoard_Completed(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::Foundation::IInspectable const& /*e*/)
    {
        if (auto bannerContent{ BannerContentPresenter() })
        {
            bannerContent.Content(nullptr);

            if (auto bs = m_bannerItems.Size(); bs == 0 || bs <= m_index)
            {
                progIndexChanged = false;
                return;
            }
            if (auto newbanner{ m_bannerItems.GetAt(m_index).try_as<xmtmui::BannerChild>() })
            {
                bannerContent.Content(newbanner.BannerContent());

                if (auto bannerImage{ ImagePresenter() })
                {
                    bannerImage.Source().try_as<Media::Imaging::BitmapImage>().UriSource(Windows::Foundation::Uri(newbanner.ImagePath()));
                }
            }

            OpenBannerBoard().Begin();
        }

        if (m_timer) m_timer.Start();
    }

    void BannerControl::BannerPager_CurrentIndexChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, uint32_t const& e)
    {
        if (progIndexChanged) return;

        ShowBanner(e);
    }

    void BannerControl::ControlLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!m_timer_revoker)
        {
            m_timer.Interval(std::chrono::seconds(5));
            m_timer_revoker = m_timer.Tick(auto_revoke, [=](auto&&, auto&&)
            {
                auto pagersize = m_index + 1;
                pagersize = pagersize >= m_bannerItems.Size() ? 0 : pagersize;

                ShowBanner(pagersize);
            });
        }

        if (!m_bannerItems_revoker)
        {
            m_bannerItems_revoker = m_bannerItems.VectorChanged(auto_revoke, [=](auto&&, auto&&)
            {
                if (xmtmui::CardPager pager{ BannerPager() })
                {
                    auto pagersize = m_bannerItems.Size();

                    pager.NumberOfCards(pagersize <= 0 ? 1 : pagersize);
                    ShowBanner(0);
                }
            });
        }
        
        auto pager{ sender.try_as<xmtmui::CardPager>() };
        if (!pager) pager = BannerPager();
        if (pager)
        {
            auto pagersize = m_bannerItems.Size();

            pager.NumberOfCards(pagersize <= 0 ? 1 : pagersize);
            ShowBanner(0);
        }
    }

    void BannerControl::ControlUnloaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (m_timer)
        {
            m_timer.Stop();
            m_timer = nullptr;
        }
        if (m_timer_revoker) m_timer_revoker.~event_revoker();

        if (m_bannerItems)
        {
            m_bannerItems.Clear();
            m_bannerItems = nullptr;
        }
        if (m_bannerItems_revoker) m_bannerItems_revoker.~event_revoker();
    }
}
