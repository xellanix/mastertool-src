#pragma once

#include "QuickRename.g.h"
#include "ExplorerItem.g.h"
#include "Utilities.h"
#include "../../../Libraries/xellanix.objects.h"
#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Windows.ApplicationModel.DataTransfer.h"
#include "winrt/Windows.UI.Text.h"
#include "winrt/Windows.UI.Xaml.Media.h"
#include "winrt/Windows.UI.Xaml.Documents.h"
#include <unordered_set>
#include <functional>
#include <random>
#include <regex>
#include "winrt/Windows.UI.Xaml.Input.h"
#include "winrt/Windows.UI.Core.h"

inline const std::wstring ExplorerItem_FolderIcon{ LR"icon(F1 M 20 6.25 L 20 16.25 C 20 16.595053 19.934895 16.918945 19.804688 17.22168 C 19.674479 17.524414 19.495441 17.789713 19.267578 18.017578 C 19.039713 18.245443 18.774414 18.424479 18.47168 18.554688 C 18.168945 18.684896 17.845051 18.75 17.5 18.75 L 2.5 18.75 C 2.154948 18.75 1.831055 18.684896 1.52832 18.554688 C 1.225586 18.424479 0.960286 18.245443 0.732422 18.017578 C 0.504557 17.789713 0.325521 17.524414 0.195312 17.22168 C 0.065104 16.918945 0 16.595053 0 16.25 L 0 3.75 C 0 3.404949 0.065104 3.081055 0.195312 2.77832 C 0.325521 2.475586 0.504557 2.210287 0.732422 1.982422 C 0.960286 1.754559 1.225586 1.575521 1.52832 1.445312 C 1.831055 1.315105 2.154948 1.25 2.5 1.25 L 7.1875 1.25 C 7.591146 1.25 7.963867 1.337891 8.305664 1.513672 C 8.647461 1.689453 8.942057 1.933594 9.189453 2.246094 C 9.384766 2.49349 9.573567 2.744141 9.755859 2.998047 C 9.93815 3.251953 10.123697 3.502605 10.3125 3.75 L 17.5 3.75 C 17.845051 3.75 18.168945 3.815105 18.47168 3.945312 C 18.774414 4.075521 19.039713 4.254558 19.267578 4.482422 C 19.495441 4.710287 19.674479 4.975587 19.804688 5.27832 C 19.934895 5.581056 20 5.904949 20 6.25 Z M 2.5 2.5 C 2.324219 2.5 2.161458 2.532553 2.011719 2.597656 C 1.861979 2.662762 1.730143 2.752279 1.616211 2.866211 C 1.502279 2.980145 1.41276 3.11198 1.347656 3.261719 C 1.282552 3.411459 1.25 3.574219 1.25 3.75 L 1.25 5 L 7.558594 5 C 7.708333 5.000001 7.851562 4.973959 7.988281 4.921875 C 8.125 4.869792 8.256836 4.804688 8.383789 4.726562 C 8.510742 4.648438 8.632812 4.563803 8.75 4.472656 C 8.867188 4.381511 8.984375 4.296875 9.101562 4.21875 L 8.183594 2.998047 C 8.066406 2.841797 7.919922 2.719727 7.744141 2.631836 C 7.568359 2.543945 7.382812 2.5 7.1875 2.5 Z M 18.75 6.25 C 18.75 6.08073 18.717447 5.919598 18.652344 5.766602 C 18.587238 5.613607 18.497721 5.480144 18.383789 5.366211 C 18.269855 5.252279 18.136393 5.162761 17.983398 5.097656 C 17.830402 5.032553 17.66927 5.000001 17.5 5 L 10.195312 5 L 8.945312 5.830078 C 8.528646 6.110026 8.066406 6.25 7.558594 6.25 L 1.25 6.25 L 1.25 16.25 C 1.25 16.425781 1.282552 16.59017 1.347656 16.743164 C 1.41276 16.896158 1.500651 17.027994 1.611328 17.138672 C 1.722005 17.24935 1.853841 17.33724 2.006836 17.402344 C 2.159831 17.467447 2.324219 17.5 2.5 17.5 L 17.5 17.5 C 17.675781 17.5 17.838541 17.467447 17.988281 17.402344 C 18.13802 17.33724 18.269855 17.247721 18.383789 17.133789 C 18.497721 17.019857 18.587238 16.888021 18.652344 16.738281 C 18.717447 16.588543 18.75 16.425781 18.75 16.25 Z )icon" };
inline const std::wstring ExplorerItem_FileIcon{ LR"icon(F1 M 17.5 7.65625 L 17.5 17.5 C 17.5 17.845053 17.434895 18.168945 17.304688 18.47168 C 17.174479 18.774414 16.995441 19.039713 16.767578 19.267578 C 16.539713 19.495443 16.274414 19.674479 15.97168 19.804688 C 15.668945 19.934896 15.345052 20 15 20 L 5 20 C 4.654948 20 4.331055 19.934896 4.02832 19.804688 C 3.725586 19.674479 3.460286 19.495443 3.232422 19.267578 C 3.004557 19.039713 2.825521 18.774414 2.695312 18.47168 C 2.565104 18.168945 2.5 17.845053 2.5 17.5 L 2.5 2.5 C 2.5 2.154949 2.565104 1.831055 2.695312 1.52832 C 2.825521 1.225586 3.004557 0.960287 3.232422 0.732422 C 3.460286 0.504559 3.725586 0.325521 4.02832 0.195312 C 4.331055 0.065105 4.654948 0 5 0 L 9.84375 0 C 10.175781 0 10.494791 0.063477 10.800781 0.19043 C 11.106771 0.317383 11.376953 0.498047 11.611328 0.732422 L 16.767578 5.888672 C 17.001953 6.123047 17.182617 6.393229 17.30957 6.699219 C 17.436523 7.005209 17.5 7.324219 17.5 7.65625 Z M 16.25 7.65625 C 16.25 7.630209 16.25 7.604167 16.25 7.578125 C 16.25 7.552084 16.246744 7.526042 16.240234 7.5 L 12.5 7.5 C 12.154947 7.5 11.831055 7.434896 11.52832 7.304688 C 11.225586 7.17448 10.960286 6.995443 10.732422 6.767578 C 10.504557 6.539714 10.325521 6.274414 10.195312 5.97168 C 10.065104 5.668945 10 5.345053 10 5 L 10 1.259766 C 9.973958 1.253256 9.947916 1.25 9.921875 1.25 C 9.895833 1.25 9.869791 1.25 9.84375 1.25 L 5 1.25 C 4.824219 1.25 4.661458 1.282553 4.511719 1.347656 C 4.361979 1.412762 4.230143 1.502279 4.116211 1.616211 C 4.002278 1.730145 3.91276 1.86198 3.847656 2.011719 C 3.782552 2.161459 3.75 2.324219 3.75 2.5 L 3.75 17.5 C 3.75 17.675781 3.782552 17.84017 3.847656 17.993164 C 3.91276 18.146158 4.000651 18.277994 4.111328 18.388672 C 4.222005 18.49935 4.353841 18.58724 4.506836 18.652344 C 4.659831 18.717447 4.824219 18.75 5 18.75 L 15 18.75 C 15.17578 18.75 15.338541 18.717447 15.488281 18.652344 C 15.638021 18.58724 15.769856 18.497721 15.883789 18.383789 C 15.997721 18.269857 16.087238 18.138021 16.152344 17.988281 C 16.217447 17.838541 16.25 17.675781 16.25 17.5 Z M 15.371094 6.25 L 11.25 2.128906 L 11.25 5 C 11.25 5.175782 11.282552 5.34017 11.347656 5.493164 C 11.41276 5.646159 11.50065 5.777995 11.611328 5.888672 C 11.722005 5.99935 11.853841 6.08724 12.006836 6.152344 C 12.15983 6.217448 12.324219 6.25 12.5 6.25 Z )icon" };

#define CALL_WITH_DEF(FUNC) []() { return FUNC(); }

namespace winrt::xmtmui::implementation
{
    struct ExplorerItem : ExplorerItemT<ExplorerItem>
    {
    private:
        hstring m_itemName{ L"" };
        hstring m_itemPath{ L"" };
        hstring m_itemIcon{ L"" };
        hstring m_itemRenamed{ L"" };
        bool m_isRemember{ false };

        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    public:
        ExplorerItem() = default;

        ExplorerItem(hstring const& itemName, hstring const& itemPath, hstring const& itemIcon, hstring const& itemRenamed, bool const& isRemember) : m_itemName(itemName), m_itemPath(itemPath), m_itemIcon(itemIcon), m_itemRenamed(itemRenamed), m_isRemember(isRemember)
        { }

        hstring ItemName() { return m_itemName; }
        hstring ItemPath() { return m_itemPath; }
        hstring ItemIcon() { return m_itemIcon; }
        hstring ItemRenamed() { return m_itemRenamed; }
        bool IsRemember() { return m_isRemember; }

        void ItemName(hstring const& value)
        {
            m_itemName = value;
        }
        void ItemPath(hstring const& value)
        {
            if (m_itemPath != value)
            {
                m_itemPath = value;
                m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"ItemPath" });
            }
        }
        void ItemIcon(hstring const& value)
        {
            m_itemIcon = value;
        }
        void ItemRenamed(hstring const& value)
        {
            if (m_itemRenamed != value)
            {
                m_itemRenamed = value;
                m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"ItemRenamed" });
            }
        }
        void IsRemember(bool const& value)
        {
            if (m_isRemember != value)
            {
                m_isRemember = value;
                m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"IsRemember" });
            }
        }

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_propertyChanged.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }
    };

    struct QuickRename : QuickRenameT<QuickRename>
    {
    private:
        using IVectorString = Windows::Foundation::Collections::IVector<hstring>;
        using IExplorerItems = Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable>;

        IExplorerItems::VectorChanged_revoker vectorToken{};
        Windows::UI::Xaml::DispatcherTimer::Tick_revoker fileChecker_revoker{};
        Windows::UI::Xaml::DispatcherTimer fileChecker{};

        Windows::Foundation::Collections::IVector<hstring> passed_files{ single_threaded_vector<hstring>() };

        bool isloaded = false;
        time_t last_ctime1 = 0, last_mtime1 = 0;
        long long last_size1 = 0;
        Xellanix::Objects::XSMF settings{ L"Quick Rename" };
        void LoadSettings();
        template<typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double, std::wstring>>::value>::type
            SaveSettings(std::wstring const& name, T const& value);

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

        template<typename T>
        fire_and_forget no_await(T t)
        {
            if constexpr (std::is_invocable_v<T>)
            {
                co_await t();
            }
            else
            {
                co_await t;
            }
        }

        IExplorerItems m_explorerItems;
        std::unordered_set<std::wstring> m_explorerItemsSet;

        bool m_isRecursive = false;
        bool m_recursiveSetted = false;

        unsigned short m_recursiveBehav = 2;
        bool m_autoSort = true;
        bool m_rememberRenamed = true;
        bool m_renameRepText = true;
        bool m_matchWhole = false;
        bool m_matchCase = false;
        unsigned short m_applyTo = 0;

        std::unordered_set<uint32_t> m_selectedIndex;
        std::vector<std::wstring> m_rememberItems;
        std::wstring m_textToFind, m_newNameFormat;

        Windows::Foundation::IAsyncAction setNewNameOp{ nullptr };

        void SortItems(std::vector<hstring>& itemList);
        Windows::Foundation::IAsyncAction AddExplorerItem(winrt::param::hstring const& filepath);
        void AddFromRecursive(winrt::param::hstring const& filepath);
        Windows::Foundation::IAsyncAction ShowFilesPicker();
        Windows::Foundation::IAsyncOperation<hstring> GenerateNewName(std::wstring const& itemName, std::wstring const& itemPath, bool usespecialsymbols, uint32_t id, bool useregex, std::unordered_multiset<std::wstring>& renamed_paths);
        Windows::Foundation::IAsyncAction SetNewName();

        std::wstring ReplaceAll(std::wstring const& original, bool matchWhole, bool caseSensitive, std::wstring const& before, std::wstring const& after)
        {
            using string_t = std::wstring;

            auto searchOp = [](wchar_t ch1, wchar_t ch2)
            {
                return std::towupper(ch1) == std::towupper(ch2);
            };

            string_t retval;
            string_t::const_iterator end = original.end();
            string_t::const_iterator current = original.begin();
            string_t::const_iterator next = caseSensitive ? std::search(current, end, before.begin(), before.end()) : std::search(current, end, before.begin(), before.end(), searchOp);

            while (next != end)
            {
                retval.append(current, next);

                if (matchWhole)
                {
                    auto const behind = next == original.begin() ? std::wstring(1, L'\0') : std::wstring(1, *(next - 1));
                    auto const front = (next + before.size()) != end ? std::wstring(1, *(next + before.size())) : std::wstring(1, L'\0');
                    if (!std::iswalnum(behind[0]) && !std::iswalnum(front[0]))
                    {
                        retval.append(after);
                    }
                    else
                    {
                        retval.append(before);
                    }
                }
                else
                {
                    retval.append(after);
                }
                
                current = next + before.size();

                if (caseSensitive)
                    next = std::search(current, end, before.begin(), before.end());
                else
                    next = std::search(current, end, before.begin(), before.end(), searchOp);
            }
            retval.append(current, next);
            return retval;
        }
        std::wstring ReplaceSymbol(std::wstring const& original, std::wstring const& before, std::function<std::wstring()> const& after, unsigned long long& contains)
        {
            using string_t = std::wstring;
            string_t retval;
            string_t::const_iterator end = original.end();
            string_t::const_iterator current = original.begin();
            string_t::const_iterator next =
                std::search(current, end, before.begin(), before.end());

            while (next != end)
            {
                retval.append(current, next);
                if (next == original.begin())
                {
                    retval.append(after());
                    ++contains;
                }
                else
                {
                    if (auto behide = std::wstring(1, *(next - 1)); behide == L"\\")
                    {
                        retval.pop_back();
                        retval.append(before);
                    }
                    else
                    {
                        retval.append(after());
                        ++contains;
                    }
                }
                current = next + before.size();
                next = std::search(current, end, before.begin(), before.end());
            }
            retval.append(current, next);
            return retval;
        }
        template <typename T>
        typename std::enable_if<std::bool_constant<std::_Is_any_of_v<std::remove_cv_t<T>, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double>>::value, std::wstring>::type
            ReplaceIndexSymbol(std::wstring const& original, T& after)
        {
            using string_t = std::wstring;

            string_t before = L"#";

            string_t retval;
            string_t::const_iterator end = original.end();
            string_t::const_iterator current = original.begin();
            string_t::const_iterator next =
                std::search(current, end, before.begin(), before.end());

            while (next != end)
            {
                retval.append(current, next);
                if (next == original.begin())
                {
                    for (auto front_it = next + 1; front_it != end; front_it++)
                    {
                        auto front = std::wstring(1, *front_it);
                        if (front == L"#")
                        {
                            before += front;
                        }
                        else
                        {
                            break;
                        }
                    }

                    auto numstr = std::to_wstring(after);
                    auto diff = before.size() > numstr.size() ? before.size() - numstr.size() : 0ULL;
                    numstr = std::wstring(diff, L'0') + numstr;

                    retval.append(numstr);
                }
                else
                {
                    if (auto behide = std::wstring(1, *(next - 1)); behide == L"\\")
                    {
                        retval.pop_back();
                        retval.append(before);
                    }
                    else
                    {
                        for (auto front_it = next + 1; front_it != end; front_it++)
                        {
                            auto front = std::wstring(1, *front_it);
                            if (front == L"#")
                            {
                                before += front;
                            }
                            else
                            {
                                break;
                            }
                        }

                        auto numstr = std::to_wstring(after);
                        auto diff = before.size() > numstr.size() ? before.size() - numstr.size() : 0ULL;
                        numstr = std::wstring(diff, L'0') + numstr;

                        retval.append(numstr);
                    }
                }
                current = next + before.size();
                next = std::search(current, end, before.begin(), before.end());
                before = L"#";
            }
            retval.append(current, next);
            return retval;
        }
        // h = hours, hh (leading 0)
        // m = minutes, mm (leading 0)
        // s = seconds, ss (leading 0)
        // d = days of month, dd (leading 0)
        // ddd = name of the day of week, dddd (Abbreviated)
        // M = month, MM (leading 0)
        // MMM = name of the month, MMMM (Abbreviated)
        // yy = year (2 digits), yyyy (4 digits)
        std::wstring ReplaceTimeSymbol(std::wstring const& original, std::wstring const& type, time_t createdTime)
        {
            using string_t = std::wstring;

            string_t before = L"\\" + type;
            const auto _typelength = static_cast<unsigned short>(type.size());

            string_t retval;
            string_t::const_iterator end = original.end();
            string_t::const_iterator current = original.begin();
            string_t::const_iterator next =
                std::search(current, end, before.begin(), before.end());

            while (next != end)
            {
                retval.append(current, next);
                if (next == original.begin())
                {
                    auto front_it = next + before.size();
                    if (front_it != end)
                    {
                        auto front = std::wstring(std::wstring(front_it, end).size() < _typelength ? std::wstring(front_it, end).size() : _typelength, *front_it);
                        if (front == type)
                        {
                            before += front;

                            if (type == L"d" || type == L"M")
                            {
                                front_it++;
                                for (auto repeater = 0ui16; front_it != end; front_it++, repeater++)
                                {
                                    if (repeater >= 2)
                                    {
                                        break;
                                    }

                                    front = std::wstring(1, *front_it);
                                    if (front == type)
                                    {
                                        before += front;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    struct tm _tm = Xellanix::Utilities::localtime_x(createdTime);

                    std::wstring time_str;

                    auto typelength = static_cast<unsigned short>(before.size() - 1);
                    if (type == L"d")
                    {
                        if (typelength == 1ui16)
                        {
                            time_str = std::to_wstring(_tm.tm_mday);
                        }
                        else if (typelength == 2ui16)
                        {
                            time_str = std::to_wstring(_tm.tm_mday);
                            if (typelength > time_str.size())
                                time_str = std::wstring(1, L'0') + time_str;
                        }
                        else if (typelength == 3ui16)
                        {
                            auto wd = _tm.tm_wday;
                            const auto days = make_array<std::wstring>(L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat");
                            time_str = days[wd];
                        }
                        else if (typelength == 4ui16)
                        {
                            auto wd = _tm.tm_wday;
                            const auto days = make_array<std::wstring>(L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday");
                            time_str = days[wd];
                        }
                    }
                    else if (type == L"M")
                    {
                        if (typelength == 1ui16)
                        {
                            time_str = std::to_wstring(_tm.tm_mon + 1);
                        }
                        else if (typelength == 2ui16)
                        {
                            time_str = std::to_wstring(_tm.tm_mon + 1);
                            if (typelength > time_str.size())
                                time_str = std::wstring(1, L'0') + time_str;
                        }
                        else if (typelength == 3ui16)
                        {
                            auto mon = _tm.tm_mon;
                            const auto monn = make_array<std::wstring>(L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec");
                            time_str = monn[mon];
                        }
                        else if (typelength == 4ui16)
                        {
                            auto mon = _tm.tm_mon;
                            const auto monn = make_array<std::wstring>(L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December");
                            time_str = monn[mon];
                        }
                    }
                    if (type == L"yy")
                    {
                        time_str = std::to_wstring(_tm.tm_year + 1900);
                        if (typelength < time_str.size())
                        {
                            time_str = time_str.substr(time_str.size() - typelength);
                        }
                    }
                    else if (type == L"h")
                    {
                        time_str = std::to_wstring(_tm.tm_hour);
                        if (typelength > time_str.size())
                            time_str = std::wstring(1, L'0') + time_str;
                    }
                    else if (type == L"m")
                    {
                        time_str = std::to_wstring(_tm.tm_min);
                        if (typelength > time_str.size())
                            time_str = std::wstring(1, L'0') + time_str;
                    }
                    else if (type == L"s")
                    {
                        time_str = std::to_wstring(_tm.tm_sec);
                        if (typelength > time_str.size())
                            time_str = std::wstring(1, L'0') + time_str;
                    }

                    retval.append(time_str);
                }
                else
                {
                    if (auto behide = std::wstring(1, *(next - 1)); behide == L"\\")
                    {
                        retval.pop_back();
                        retval.append(before);
                    }
                    else
                    {
                        auto front_it = next + before.size();
                        if (front_it != end)
                        {
                            auto front = std::wstring(std::wstring(front_it, end).size() < _typelength ? std::wstring(front_it, end).size() : _typelength, *front_it);
                            if (front == type)
                            {
                                before += front;

                                if (type == L"d" || type == L"M")
                                {
                                    front_it++;
                                    for (auto repeater = 0ui16; front_it != end; front_it++, repeater++)
                                    {
                                        if (repeater >= 2)
                                        {
                                            break;
                                        }

                                        front = std::wstring(1, *front_it);
                                        if (front == type)
                                        {
                                            before += front;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        struct tm _tm = Xellanix::Utilities::localtime_x(createdTime);

                        std::wstring time_str;

                        auto typelength = static_cast<unsigned short>(before.size() - 1);
                        if (type == L"d")
                        {
                            if (typelength == 1ui16)
                            {
                                time_str = std::to_wstring(_tm.tm_mday);
                            }
                            else if (typelength == 2ui16)
                            {
                                time_str = std::to_wstring(_tm.tm_mday);
                                if (typelength > time_str.size())
                                    time_str = std::wstring(1, L'0') + time_str;
                            }
                            else if (typelength == 3ui16)
                            {
                                auto wd = _tm.tm_wday;
                                const auto days = make_array<std::wstring>(L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat");
                                time_str = days[wd];
                            }
                            else if (typelength == 4ui16)
                            {
                                auto wd = _tm.tm_wday;
                                const auto days = make_array<std::wstring>(L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday");
                                time_str = days[wd];
                            }
                        }
                        else if (type == L"M")
                        {
                            if (typelength == 1ui16)
                            {
                                time_str = std::to_wstring(_tm.tm_mon + 1);
                            }
                            else if (typelength == 2ui16)
                            {
                                time_str = std::to_wstring(_tm.tm_mon + 1);
                                if (typelength > time_str.size())
                                    time_str = std::wstring(1, L'0') + time_str;
                            }
                            else if (typelength == 3ui16)
                            {
                                auto mon = _tm.tm_mon;
                                const auto monn = make_array<std::wstring>(L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec");
                                time_str = monn[mon];
                            }
                            else if (typelength == 4ui16)
                            {
                                auto mon = _tm.tm_mon;
                                const auto monn = make_array<std::wstring>(L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December");
                                time_str = monn[mon];
                            }
                        }
                        if (type == L"yy")
                        {
                            time_str = std::to_wstring(_tm.tm_year + 1900);
                            if (typelength < time_str.size())
                            {
                                time_str = time_str.substr(time_str.size() - typelength);
                            }
                        }
                        else if (type == L"h")
                        {
                            time_str = std::to_wstring(_tm.tm_hour);
                            if (typelength > time_str.size())
                                time_str = std::wstring(1, L'0') + time_str;
                        }
                        else if (type == L"m")
                        {
                            time_str = std::to_wstring(_tm.tm_min);
                            if (typelength > time_str.size())
                                time_str = std::wstring(1, L'0') + time_str;
                        }
                        else if (type == L"s")
                        {
                            time_str = std::to_wstring(_tm.tm_sec);
                            if (typelength > time_str.size())
                                time_str = std::wstring(1, L'0') + time_str;
                        }

                        retval.append(time_str);
                    }
                }
                current = next + before.size();
                before = L"\\" + type;
                next = std::search(current, end, before.begin(), before.end());
            }
            retval.append(current, next);
            return retval;
        }
        
        std::wstring get_filename(std::wstring path)
        {
            if (auto pos = path.find_last_of(L"\\/"); pos != std::wstring::npos)
                path = path.substr(pos + 1);

            return path;
        }
        std::pair<std::wstring, std::wstring> split_filename(std::wstring path)
        {
            if (auto pos = path.find_last_of(L"\\/"); pos != std::wstring::npos)
                path = path.substr(pos + 1);

            if (size_t pos = path.find_last_of(L"."); pos != std::wstring::npos)
                return { path.substr(0, pos), path.substr(pos) };

            return { path, L"" };
        }
        std::wstring add_before_extension(std::wstring path, std::wstring const& toappend)
        {
            std::wstring retval;

            auto spos = path.find_last_of(L"\\/") + 1;
            if (spos != std::wstring::npos)
            {
                retval = path.substr(0, spos);
                path = path.substr(spos);
            }

            if (size_t pos = path.find_last_of(L"."); pos != std::wstring::npos)
            {
                path = path.substr(0, pos) + toappend + path.substr(pos);
                retval += Xellanix::Utilities::trim_spaces(path);

                return retval;
            }

            return retval + Xellanix::Utilities::trim_spaces(path + toappend);
        }
        bool IsRestartName(std::wstring const& finalpath);

    public:
        QuickRename();

        IExplorerItems ExplorerItems() { return m_explorerItems; }
        void ExplorerItems(IExplorerItems const& value)
        {
            if (m_explorerItems != value)
            {
                m_explorerItems = value;
            }
        }

        void PageLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void Pageunloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);
         
        Windows::Foundation::IAsyncAction FileListChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        
        Windows::Foundation::IAsyncAction AddClick(winrt::Microsoft::UI::Xaml::Controls::SplitButton const& sender, winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& e);
        Windows::Foundation::IAsyncAction AddFilesFolderClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::ItemClickEventArgs const& e);;
        void DeleteItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void ClearItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void SelectAllClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void InvertSelectClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void DeselectClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction ApplyClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
         
        void FilesDragOver(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::DragEventArgs const& e);
        Windows::Foundation::IAsyncAction FilesDrop(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::DragEventArgs const& e);
        
        void RecursiveBehavChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void AutoSortChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void RememberRenamedChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction ResetRememberClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction RenameRepTextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction MatchWholeTextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction MatchCaseChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction ApplyToChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        
        Windows::Foundation::IAsyncAction SpecialFormattingCheckChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void TextToFind_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction TextToFind_LostFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void NewNameFormat_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction NewNameFormat_LostFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
};
}

namespace winrt::xmtmui::factory_implementation
{
    struct ExplorerItem : ExplorerItemT<ExplorerItem, implementation::ExplorerItem>
    {
    };

    struct QuickRename : QuickRenameT<QuickRename, implementation::QuickRename>
    {
    };
}
