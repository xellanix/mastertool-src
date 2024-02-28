#include "pch.h"
#include "ItemsGrid.h"
#if __has_include("ItemsGrid.g.cpp")
#include "ItemsGrid.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::xmtmui::implementation
{
    ItemsGrid::ItemsGrid() : m_itemControls(single_threaded_observable_vector<FrameworkElement>())
    {
        InitializeComponent();
    }

    void ItemsGrid::InitWrapGrid(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        wrapgrid = sender.try_as<ItemsWrapGrid>();

        AdjustWrapGrid();
    }

    void ItemsGrid::AdjustWrapGrid()
    {
        if (!wrapgrid) return;
        wrapgrid.MaximumRowsOrColumns(m_maxItemInRow);
        wrapgrid.ItemWidth(m_itemActualWidth);
    }

    void ItemsGrid::AdjustFreeSpace(const double finalWidth, const double rw, const double approxNumber)
    {
        if (finalWidth < rw)
        {
            // Free space
            double freespace = rw - finalWidth;

            if (m_freeSpaceJustification == xmtmui::FreeSpaceJustification::Item)
            {
                m_itemActualWidth = m_itemMinWidth + freespace / approxNumber;

                m_itemActualWidth += m_spaceActualWidth;
            }
            else if (m_freeSpaceJustification == xmtmui::FreeSpaceJustification::Space)
            {
                // Space actual width
                m_spaceActualWidth = m_columnMinSpace + freespace / (approxNumber - 1);
            }
        }
    }

    void ItemsGrid::AdjustSizeSpace()
    {
        if (!_loadedItems) return;

        auto root{ RootGrid() };
        if (!root) return;

        const auto rw{ root.ActualWidth() };
        m_spaceActualWidth = m_columnMinSpace;
        m_spaceActualHeight = m_rowMinSpace;
        m_itemActualWidth = (m_itemMinWidth + m_spaceActualWidth);

        unsigned int approxNumber = static_cast<unsigned int>(rw / m_itemActualWidth);
        approxNumber = min(approxNumber, static_cast<unsigned int>(m_itemControls.Size()));
        double finalWidth = m_itemActualWidth * approxNumber;

        if (finalWidth < rw)
        {
            AdjustFreeSpace(finalWidth, rw, approxNumber);
        }
        else if (finalWidth > rw)
        {
            while (finalWidth > rw)
            {
                --approxNumber;
                finalWidth = m_itemActualWidth * approxNumber;
            }

            if (finalWidth < rw)
            {
                AdjustFreeSpace(finalWidth, rw, approxNumber);
            }
        }

        m_maxItemInRow = max(approxNumber, 1);
        m_itemActualWidth = std::isfinite(m_itemActualWidth) ? max(m_itemActualWidth, m_itemMinWidth) : m_itemMinWidth;
        AdjustWrapGrid();
        
        auto scroll{ ScrollRoot() };
        if (!scroll) return;
        scroll.Padding(ThicknessHelper::FromLengths(-m_spaceActualWidth, -m_spaceActualHeight, m_spaceActualWidth, m_spaceActualHeight));
        for (auto&& control : root.Items())
        {
            if (auto fe{ control.try_as<FrameworkElement>() })
            {
                fe.Margin(ThicknessHelper::FromLengths(m_spaceActualWidth, m_spaceActualHeight, 0, 0));
            }
        }
    }

    void ItemsGrid::ControlLoaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        _loadedItems = false;

        if (!itemControlsChanged_token)
        {
            itemControlsChanged_token = m_itemControls.VectorChanged(auto_revoke, [=, weak_this{ get_weak() }](auto&&, Windows::Foundation::Collections::IVectorChangedEventArgs const& args)
            {
                using namespace Windows::Foundation::Collections;

                if (auto strong_this{ weak_this.get() })
                {
                    auto root{ strong_this->RootGrid() };
                    if (!root) return;
                    auto items{ root.Items() };
                    auto index{ args.Index() };

                    switch (args.CollectionChange())
                    {
                        case CollectionChange::ItemChanged:
                            items.RemoveAt(index);
                            items.InsertAt(index, strong_this->m_itemControls.GetAt(index));
                            break;
                        case CollectionChange::ItemInserted:
                            items.InsertAt(index, strong_this->m_itemControls.GetAt(index));
                            break;
                        case CollectionChange::ItemRemoved:
                            items.RemoveAt(index);
                            break;
                        case CollectionChange::Reset:
                            items.Clear();
                            break;
                    }

                    if (static_cast<uint32_t>(strong_this->m_maxItemInRow) > strong_this->m_itemControls.Size() && strong_this->_loadedItems)
                    {
                        strong_this->AdjustSizeSpace();
                    }
                }
            });
        }
        {
            auto root{ RootGrid() };
            if (!root) return;
            auto items{ root.Items() };
            items.Clear();

            for (auto&& item : m_itemControls)
            {
                items.Append(item);
            }
        }

        _loadedItems = true;

        AdjustSizeSpace();
    }

    void ItemsGrid::ControlSizeChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& /*e*/)
    {
        AdjustSizeSpace();
    }

    void ItemsGrid::ControlUnloaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (m_itemControls)
        {
            m_itemControls.Clear();
            m_itemControls = nullptr;
        }
        m_maxItemInRow = 1;
        m_itemMinWidth = m_rowMinSpace = m_columnMinSpace = 0;
        m_itemActualWidth = m_spaceActualWidth = m_spaceActualHeight = 0;
        m_freeSpaceJustification = xmtmui::FreeSpaceJustification::Item;

        wrapgrid = nullptr;

        if (itemControlsChanged_token) itemControlsChanged_token.~event_revoker();
    }
}
