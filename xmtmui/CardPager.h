#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "winrt/Windows.UI.Xaml.Media.h"
#include "winrt/Windows.UI.Xaml.Automation.h"
#include "CardPager.g.h"
#include "BooleanToStyleConverter.g.h"

namespace winrt::xmtmui::implementation
{
    struct BooleanToStyleConverter : BooleanToStyleConverterT<BooleanToStyleConverter>
    {
    private:
        Windows::UI::Xaml::Style m_false{ nullptr }, m_true{ nullptr };

    public:
        BooleanToStyleConverter() = default;

        Windows::UI::Xaml::Style FalseStyle() const noexcept
        {
            return m_false;
        }
        void FalseStyle(Windows::UI::Xaml::Style const& value) noexcept
        {
            m_false = value;
        }

        Windows::UI::Xaml::Style TrueStyle() const noexcept
        {
            return m_true;
        }
        void TrueStyle(Windows::UI::Xaml::Style const& value) noexcept
        {
            m_true = value;
        }

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
        {
            return box_value(unbox_value_or<bool>(value, false) ? m_true : m_false);
        }

        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const&, hstring const& /*language*/)
        {
            throw hresult_not_implemented();
        }
    };

    struct CardPager : CardPagerT<CardPager>
    {
    private:
        using ObservableIns = Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable>;

        uint32_t m_numberOfCards = 1, m_currentIndex = 0;
        uint32_t m_maxVisibleIndicator = 5, m_actualVisible;
        bool m_isSelected = false, m_showNavigationButton = true;

        ObservableIns m_cardItems{ single_threaded_observable_vector(std::move(std::vector<Windows::Foundation::IInspectable>(1, box_value(false)))) };

        winrt::event<Windows::Foundation::EventHandler<uint32_t>> m_currentIndexChangedEvent;

        void ApplyStyle(uint32_t index, bool isSelected = true);
        void ShowButton();
        void NormalizeMaxItems();
        void ScrollToCenter();

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
        template <typename child_type>
        auto find_child(::winrt::Windows::UI::Xaml::DependencyObject const& root)
        {
            if (!root) return child_type{ nullptr };

            if (auto child{ root.try_as<child_type>() }) return child;

            using VisualTreeHelper = ::winrt::Windows::UI::Xaml::Media::VisualTreeHelper;

            auto childCount = VisualTreeHelper::GetChildrenCount(root);
            for (decltype(childCount) i = 0; i < childCount; i++)
            {
                auto temproot = VisualTreeHelper::GetChild(root, i);
                auto child = temproot.try_as<child_type>();
                if (!child)
                {
                    child = find_child<child_type>(temproot);
                }

                if (child) return child;
            }

            return child_type{ nullptr };
        }

    public:
        CardPager();

        uint32_t NumberOfCards() { return m_numberOfCards; };
        void NumberOfCards(uint32_t const& value)
        {
            if (value != m_numberOfCards)
            {
                std::vector<Windows::Foundation::IInspectable> temp{ m_cardItems.Size() };
                m_cardItems.GetMany(0, temp);

                if (value < m_numberOfCards)
                {
                    temp.erase(temp.begin() + value, temp.end());
                    if (auto size = uint32_t(temp.size()); size == m_currentIndex)
                    {
                        auto newindex = size - 1;

                        m_currentIndex = newindex;
                        temp[m_currentIndex] = box_value(m_isSelected);

                        m_currentIndexChangedEvent(*this, m_currentIndex);
                    }
                }
                else
                {
                    uint32_t i = (uint32_t)temp.size();
                    auto distance = value - i;

                    temp.reserve((std::max)(value, i));
                    temp.insert(temp.end(), distance, box_value(false));
                    temp.shrink_to_fit();
                }

                m_cardItems.ReplaceAll(std::move(temp));
                m_numberOfCards = value;

                ShowButton();

                if (auto const& repeater{ CardPagerItemsRepeater() })
                {
                    if (auto const& container{ find_child<Windows::UI::Xaml::Controls::ItemsStackPanel>(repeater) })
                    {
                        int32_t index = 1;
                        for (auto const& element : container.Children())
                        {
                            Windows::UI::Xaml::Automation::AutomationProperties::SetName(element, L"Card " + to_hstring(index));
                            Windows::UI::Xaml::Automation::AutomationProperties::SetPositionInSet(element, index);
                            Windows::UI::Xaml::Automation::AutomationProperties::SetSizeOfSet(element, (int32_t)m_numberOfCards);
                            index++;
                        }
                    }
                }
            }
        }

        uint32_t CurrentIndex() { return m_currentIndex; };
        void CurrentIndex(uint32_t const& value)
        {
            if (value != m_currentIndex)
            {
                ApplyStyle(value, m_isSelected);
            }
        }

        bool IsSelected() { return m_isSelected; }
        void IsSelected(bool const& value)
        {
            if (value != m_isSelected)
            {
                m_isSelected = value;

                ApplyStyle(m_currentIndex, m_isSelected);
            }
        }

        bool ShowNavigationButton() { return m_showNavigationButton; }
        void ShowNavigationButton(bool const& value)
        {
            if (value != m_showNavigationButton)
            {
                m_showNavigationButton = value;
            }
        }

        uint32_t MaxVisibleIndicator() { return m_maxVisibleIndicator; }
        void MaxVisibleIndicator(uint32_t const& value)
        {
            if (value != m_maxVisibleIndicator)
            {
                m_maxVisibleIndicator = value;

                NormalizeMaxItems();
            }
        }

        ObservableIns CardItems() { return m_cardItems; }

        winrt::event_token CurrentIndexChanged(Windows::Foundation::EventHandler<uint32_t> const& handler)
        {
            return m_currentIndexChangedEvent.add(handler);
        }
        void CurrentIndexChanged(winrt::event_token const& token) noexcept
        {
            m_currentIndexChangedEvent.remove(token);
        }

        void PreviousClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void NextClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void PagerSelected(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& e);
        void UserControl_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct BooleanToStyleConverter : BooleanToStyleConverterT<BooleanToStyleConverter, implementation::BooleanToStyleConverter>
    {
    };

    struct CardPager : CardPagerT<CardPager, implementation::CardPager>
    {
    };
}
