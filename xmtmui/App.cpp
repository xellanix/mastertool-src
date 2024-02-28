#include "pch.h"
#include "App.h"
#include "App.g.cpp"
#include <locale>
#include "../../../Libraries/utf8cvt.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::xmtmui::implementation
{
    App::App()
    {
        std::locale::global(std::locale(std::locale("C"), new cvt::utf8cvt<wchar_t>()));

        Initialize();
        AddRef();
        m_inner.as<::IUnknown>()->Release();
    }
    App::~App()
    {
        Close();
    }

    void App::DeleteUpdateFile()
    {
        if (auto upd = Xellanix::Utilities::LocalAppData / L"update_temp"; fs::exists(upd))
        {
            fs::remove_all(upd);
        }
    }
}