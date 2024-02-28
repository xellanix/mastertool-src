#pragma once
#include "App.g.h"
#include "App.base.h"
#include "Utilities.h"

namespace winrt::xmtmui::implementation
{
    class App : public AppT2<App>
    {
    public:
        App();
        ~App();

        void DeleteUpdateFile();
    };
}

namespace winrt::xmtmui::factory_implementation
{
    class App : public AppT<App, implementation::App>
    {};
}
