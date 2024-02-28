#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "ItemsGrid.g.h"

namespace winrt::xmtmui::implementation
{
    struct ItemsGrid : ItemsGridT<ItemsGrid>
    {
    private:
        bool _loadedItems = false;
        Windows::Foundation::Collections::IObservableVector<Windows::UI::Xaml::FrameworkElement>::VectorChanged_revoker itemControlsChanged_token;

        Windows::Foundation::Collections::IObservableVector<Windows::UI::Xaml::FrameworkElement> m_itemControls{ nullptr };
        int32_t m_maxItemInRow = 1;
        double m_itemMinWidth = 0, m_rowMinSpace = 0, m_columnMinSpace = 0;
        double m_itemActualWidth = 0, m_spaceActualWidth = 0, m_spaceActualHeight = 0;
        xmtmui::FreeSpaceJustification m_freeSpaceJustification = xmtmui::FreeSpaceJustification::Item;

        Windows::UI::Xaml::Controls::ItemsWrapGrid wrapgrid{ nullptr };

        void AdjustWrapGrid();
        void AdjustFreeSpace(const double finalWidth, const double rw, const double approxNumber);
        void AdjustSizeSpace();

    public:
        ItemsGrid();

        Windows::Foundation::Collections::IObservableVector<Windows::UI::Xaml::FrameworkElement> ItemControls() const { return m_itemControls; }
        int32_t MaxItemInRow() const { return m_maxItemInRow; }
        double ItemActualWidth() const { return m_itemActualWidth; }
        double ItemMinWidth() const { return m_itemMinWidth; }
        double RowMinSpace() const { return m_rowMinSpace; }
        double ColumnMinSpace() const { return m_columnMinSpace; }
        xmtmui::FreeSpaceJustification FreeSpaceJustification() const { return m_freeSpaceJustification; }

        void ItemControls(Windows::Foundation::Collections::IObservableVector<Windows::UI::Xaml::FrameworkElement> const& value)
        {
            m_itemControls = value;
        }
        void ItemMinWidth(double const& value)
        {
            m_itemMinWidth = value;
        }
        void RowMinSpace(double const& value)
        {
            m_rowMinSpace = value;
        }
        void ColumnMinSpace(double const& value)
        {
            m_columnMinSpace = value;
        }
        void FreeSpaceJustification(xmtmui::FreeSpaceJustification const& value)
        {
            m_freeSpaceJustification = value;
        }

        void ControlUnloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void ControlLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void ControlSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& e);
        void InitWrapGrid(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct ItemsGrid : ItemsGridT<ItemsGrid, implementation::ItemsGrid>
    {
    };
}
