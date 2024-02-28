#include "pch.h"
#include "CardPager.h"
#if __has_include("CardPager.g.cpp")
#include "CardPager.g.cpp"
#endif
#if __has_include("BooleanToStyleConverter.g.cpp")
#include "BooleanToStyleConverter.g.cpp"
#endif

#include "winrt/Windows.Foundation.Metadata.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::xmtmui::implementation
{
    CardPager::CardPager()
    {
        InitializeComponent();
    }

    void CardPager::ApplyStyle(uint32_t index, bool isSelected)
    {
        m_cardItems.SetAt(m_currentIndex, box_value(false));
        m_cardItems.SetAt(index, box_value(isSelected));

        m_isSelected = isSelected;
        m_currentIndex = index;

        m_currentIndexChangedEvent(*this, m_currentIndex);

        ShowButton();
        ScrollToCenter();
    }

    void CardPager::ShowButton()
    {
        if (auto btn{ PreviousButton() })
        {
            if (m_currentIndex == 0)
            {
                btn.Visibility(Visibility::Collapsed);
                btn.IsEnabled(false);
            }
            else
            {
                btn.Visibility(Visibility::Visible);
                btn.IsEnabled(true);
            }
        }

        if (auto btn{ NextButton() })
        {
            if (m_currentIndex == m_numberOfCards - 1)
            {
                btn.Visibility(Visibility::Collapsed);
                btn.IsEnabled(false);
            }
            else
            {
                btn.Visibility(Visibility::Visible);
                btn.IsEnabled(true);
            }
        }
    }

    void CardPager::NormalizeMaxItems()
    {
        // Allocated Width:
        // Button: 20
        // Space: 4
        constexpr auto allocWidth = 20u * 2u + 4u * 3u;

        auto indicatorWidth = uint32_t(ActualWidth()) - allocWidth;
        auto allowedVisible = indicatorWidth / 16u;

        m_actualVisible = (std::min)(allowedVisible, m_maxVisibleIndicator);
        auto maxWidth = m_actualVisible * 16u + 4u;
        if (auto scrollviewer{ CardPagerScrollViewer() })
        {
            scrollviewer.MaxWidth(maxWidth);
        }

        ScrollToCenter();
    }

    void CardPager::ScrollToCenter()
    {
        if (const auto scrollViewer{ CardPagerScrollViewer() })
        {
            const auto offset = 16.0 * (std::max)(double(m_currentIndex) - std::floor(double(m_actualVisible) / 2.0), 0.0);
            scrollViewer.ChangeView(offset, nullptr, nullptr);
        }
    }

    void CardPager::PreviousClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto index = (std::max)(m_currentIndex - 1, 0u);
        ApplyStyle(index);
    }

    void CardPager::NextClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto index = (std::min)(m_currentIndex + 1, m_numberOfCards - 1);
        ApplyStyle(index);
    }

    void CardPager::PagerSelected(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (auto uielement{ ::winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(sender.try_as<DependencyObject>()).try_as<UIElement>() })
        {
            if (auto container{ find_ancestor<Controls::ItemsStackPanel>(uielement) })
            {
                uint32_t index = 0;
                if (container.Children().IndexOf(uielement.try_as<UIElement>(), index))
                {
                    ApplyStyle(index);
                }
            }
        }
    }

    void CardPager::UserControl_SizeChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& /*e*/)
    {
        NormalizeMaxItems();
    }

    void CardPager::UserControl_Unloaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (m_currentIndexChangedEvent)
        {
            m_currentIndexChangedEvent.clear();
        }

        if (m_cardItems)
        {
            m_cardItems.Clear();
            m_cardItems = nullptr;
        }
    }
}
