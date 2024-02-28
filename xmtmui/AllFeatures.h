#pragma once

#include "AllFeatures.g.h"
#include "winrt/Windows.UI.Xaml.Media.h"
#include "Utilities.h"

namespace winrt::xmtmui::implementation
{
    struct AllFeatures : AllFeaturesT<AllFeatures>
    {
    private:
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

        Windows::Foundation::IAsyncAction checkingOp{ nullptr };
        Windows::Foundation::IAsyncAction TryCheckUpdate();

    public:
        AllFeatures();

        void NavigateFeature(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void MoveToAboutPage(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction AvailableUpdateBar_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void AvailableUpdateBar_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::xmtmui::factory_implementation
{
    struct AllFeatures : AllFeaturesT<AllFeatures, implementation::AllFeatures>
    {
    };
}
