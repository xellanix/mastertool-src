#pragma once

#include "ResultData.g.h"
#include "MainWindow.g.h"
#include "..\..\xmtmui\Utilities.h"
#include "..\..\..\..\Libraries\xellanix.objects.h"
#include "winrt\Windows.UI.Xaml.Input.h"

namespace winrt::smbrui::implementation
{
    struct ResultData : ResultDataT<ResultData>
    {
    private:
        uint32_t m_id;
        hstring m_title;
        hstring m_description;
        bool m_canRun;
        bool m_canAdmin;
        bool m_canReveal;

    public:
        ResultData() = default;

        ResultData(uint32_t const& id, hstring const& title, hstring const& description, bool const& canRun, bool const& canAdmin, bool const& canReveal) : m_id(id), m_title(title), m_description(description), m_canRun(canRun), m_canAdmin(canAdmin), m_canReveal(canReveal)
        { }

        uint32_t ResultID() const { return m_id; }
        hstring Title() const { return m_title; }
        hstring Description() const { return m_description; }
        bool CanRun() const { return m_canRun; }
        bool CanAdmin() const { return m_canAdmin; }
        bool CanReveal() const { return m_canReveal; }

        void ResultID(uint32_t const& value)
        {
            m_id = value;
        }
        void Title(hstring const& value)
        {
            m_title = value;
        }
        void Description(hstring const& value)
        {
            m_description = value;
        }
        void CanRun(bool const& value)
        {
            m_canRun = value;
        }
        void CanAdmin(bool const& value)
        {
            m_canAdmin = value;
        }
        void CanReveal(bool const& value)
        {
            m_canReveal = value;
        }
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        using ObservableIns = Windows::Foundation::Collections::IObservableVector<IInspectable>;

        ObservableIns m_results{ nullptr };

        Xellanix::Objects::XSMF settings{ L"Smart Bar" };
        bool m_enableApps = true;
        bool m_enableFolders = true;
        uint32_t m_folderDrive = 0;
        bool m_enableFiles = true;
        uint32_t m_fileDrive = 0;
        bool explorerLoaded = false;
        std::vector<std::wstring> arrayOfDrives;
        std::vector<std::wstring> explorerItems;

        std::wstring searchText;
        bool hasEnter = false;

        std::wstring lowerstring(std::wstring str)
        {
            std::transform(str.begin(), str.end(), str.begin(), ::towlower);
            return str;
        }
        void SetAvailableDrives()
        {
            wchar_t* szDrives = new wchar_t[MAX_PATH]();
            if (GetLogicalDriveStringsW(MAX_PATH, szDrives))
            {
                for (size_t i = 0; i < MAX_PATH; i++)
                {
                    if (szDrives[i] != (wchar_t)0)
                    {
                        std::wstring driveLetter{ szDrives + i };
                        auto const dls = driveLetter.size();

                        driveLetter = driveLetter.substr(0, driveLetter.size() - 1);
                        driveLetter += L"\\";

                        arrayOfDrives.push_back(driveLetter);

                        i += dls;
                    }
                }

                delete[] szDrives;
            }
        }
        std::wstring get_filename(std::wstring path)
        {
            if (auto pos = path.find_last_of(L"\\/"); pos != std::wstring::npos)
                path = path.substr(pos + 1);

            return path;
        }
        std::wstring get_parent(std::wstring path)
        {
            if (auto pos = path.find_last_of(L"\\/"); pos != std::wstring::npos)
                path = path.substr(0, pos + 1);

            return path;
        }
        std::pair<std::wstring, std::wstring> split_path(std::wstring const& path)
        {
            if (auto pos = path.find_last_of(L"\\/"); pos != std::wstring::npos)
                return { path.substr(0, pos + 1), path.substr(pos + 1) };

            return { L"", path };
        }

        std::pair<std::wstring, std::wstring> split_keys(std::wstring str, std::initializer_list<std::wstring> keys)
        {
            str = lowerstring(str);
            for (auto const& key : keys)
            {
                if (str.substr(0, key.size()) == key)
                {
                    return { key, Xellanix::Utilities::trim_spaces(str.substr(key.size())) };
                }
            }

            return { L"", str };
        }

        Windows::Foundation::IAsyncAction SearchOperation{ nullptr };

        Windows::Foundation::IAsyncAction LoadExplorerItems();
        Windows::Foundation::IAsyncAction StartSearch();

    public:
        MainWindow() 
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

            m_results = single_threaded_observable_vector<IInspectable>(
                {
                    make<implementation::ResultData>(0, L"?apps:", L"Looking for apps only.", false, false, false),
                    make<implementation::ResultData>(1, L"?files:", L"Looking for files only.", false, false, false),
                    make<implementation::ResultData>(2, L"?folders:", L"Looking for folders only.", false, false, false)
                });
        }

        ObservableIns Results() const { return m_results; }

        void SearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs const& e);
        Windows::Foundation::IAsyncAction SearchBox_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void OpenFileClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void RunAdminClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void OpenLocationClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction StartBtnClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void SearchBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction SearchBox_LostFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
};
}

namespace winrt::smbrui::factory_implementation
{
    struct ResultData : ResultDataT<ResultData, implementation::ResultData>
    {
    };

    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
