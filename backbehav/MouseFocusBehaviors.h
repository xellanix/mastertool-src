#pragma once
#include "framework.h"
#include <random>

inline bool isInstanceActive = false;

inline void ActivateMouseFocus()
{
    using namespace std::chrono_literals;

    auto exepath = fs::path(Xellanix::Utilities::AppDir) / L"MouseFocus\\MouseFocus.exe";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    //Prepare CreateProcess args
    const std::wstring app_w = exepath.wstring();
    const std::wstring arg_w = (L"\"" + app_w + L"\"");

    if (CreateProcess(app_w.c_str(), const_cast<wchar_t*>(arg_w.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        // Success
        isInstanceActive = true;

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    isInstanceActive = false;
}

inline time_t mfs_l_c = 0LL, mfs_l_m = 0LL;
inline time_t mfs_l_c2 = 0LL, mfs_l_m2 = 0LL;
inline long long mfs_l_s = 0LL;
inline long long mfs_l_s2 = 0LL;

inline bool m_isMouseFocusEnabled = false;
inline bool m_isActivateWithKey = true;
inline bool m_isActivateWithShake = true;
inline unsigned short m_mouseTimeIndex = 4ui16;
inline double m_shakeSensitivity = 7.0;
inline unsigned short m_shakeDuration = 2000ui16;

inline bool IsMouseFocusEnabled()
{
    Xellanix::Objects::XSMF settings{};
    {
        struct _stati64 current_buf1;
        const fs::path spath = (Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf");

        if (_wstat64(spath.wstring().c_str(), &current_buf1) == 0)
        {
            if (current_buf1.st_ctime != mfs_l_c ||
                current_buf1.st_mtime != mfs_l_m ||
                current_buf1.st_size != mfs_l_s)
            {
                mfs_l_c = current_buf1.st_ctime;
                mfs_l_m = current_buf1.st_mtime;
                mfs_l_s = current_buf1.st_size;

                if (settings.Read(spath))
                {
                    m_isMouseFocusEnabled = (settings >> L"ca283bd1-87a3-5082-93b8-8dbeb22d35a9").try_as<bool>(false);
                }
                else
                {
                    m_isMouseFocusEnabled = false;
                }
            }
        }
    }

    return m_isMouseFocusEnabled;
}

inline void GetMouseFocusIndex()
{
    Xellanix::Objects::XSMF mf;
    {
        struct _stati64 current_buf1;
        const fs::path spath = (Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf");

        if (_wstat64(spath.wstring().c_str(), &current_buf1) == 0)
        {
            if (current_buf1.st_ctime != mfs_l_c2 ||
                current_buf1.st_mtime != mfs_l_m2 ||
                current_buf1.st_size != mfs_l_s2)
            {
                mfs_l_c2 = current_buf1.st_ctime;
                mfs_l_m2 = current_buf1.st_mtime;
                mfs_l_s2 = current_buf1.st_size;

                if (mf.Read(spath))
                {
                    m_mouseTimeIndex = (mf >> L"4651485d-138e-5791-a0a9-940756ba18aa").try_as<unsigned short>(4);
                }
                else
                {
                    m_mouseTimeIndex = 4;
                }
            }
        }
    }
}
inline void HighlightCursor()
{
    using namespace std::chrono_literals;

    constexpr std::array<unsigned short, 16> secs{ 0, 60, 120, 180, 300, 600, 900, 1200, 1500, 1800, 2700, 3600, 7200, 10800, 14400, 18000 };

    while (!isInstanceActive)
    {
        if (!IsMouseFocusEnabled())
        {
            std::this_thread::sleep_for(1s);
            continue;
        }

        bool enabled = true;
        {
            GetMouseFocusIndex();
        }

        if (enabled && m_mouseTimeIndex > 0)
        {
            unsigned short sec = secs[m_mouseTimeIndex];
            for (unsigned short i = 0; i < sec; i++)
            {
                std::this_thread::sleep_for(1s);
                
                enabled = IsMouseFocusEnabled();
                if (enabled)
                {
                    GetMouseFocusIndex();
                }

                if (!enabled || m_mouseTimeIndex == 0) break;
            }
            if (!enabled || m_mouseTimeIndex == 0)
            {
                std::this_thread::sleep_for(1s);
                continue;
            }
            sec = secs[m_mouseTimeIndex];

            ActivateMouseFocus();
        }
    }
}

inline bool IsActivateWithKey()
{
    Xellanix::Objects::XSMF settings{};
    {
        struct _stati64 current_buf1;
        const fs::path spath = (Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf");

        if (_wstat64(spath.wstring().c_str(), &current_buf1) == 0)
        {
            if (current_buf1.st_ctime != mfs_l_c2 ||
                current_buf1.st_mtime != mfs_l_m2 ||
                current_buf1.st_size != mfs_l_s2)
            {
                mfs_l_c2 = current_buf1.st_ctime;
                mfs_l_m2 = current_buf1.st_mtime;
                mfs_l_s2 = current_buf1.st_size;

                if (settings.Read(spath))
                {
                    m_isActivateWithKey = (settings >> L"80dc8702-ff5e-5df6-bf21-b4c7673de371").try_as<bool>(true);
                }
                else
                {
                    m_isActivateWithKey = true;
                }
            }
        }
    }

    return m_isActivateWithKey;
}
inline void ActivateMouseFocusKey(bool& WinAltM)
{
    using namespace std::chrono_literals;
    
    while (true)
    {
        if (isInstanceActive)
        {
            std::this_thread::sleep_for(1s);
            continue;
        }
        if (!IsMouseFocusEnabled())
        {
            std::this_thread::sleep_for(1s);
            continue;
        }
        if (!IsActivateWithKey())
        {
            std::this_thread::sleep_for(1s);
            continue;
        }

        auto IsKeyDown = [](int key) -> bool
        {
            return (GetAsyncKeyState(key) & 0x8000) != 0;
        };

        if ((IsKeyDown(VK_LWIN) || IsKeyDown(VK_RWIN)) &&
            IsKeyDown(VK_MENU) &&
            IsKeyDown(0x4D))
        {
            if (WinAltM) return;

            {
                ActivateMouseFocus();
            }

            WinAltM = true;
        }
        else
        {
            WinAltM = false;
        }
    }
}

template <typename TResult, typename TParam>
TResult rnd(TParam val)
{
    return static_cast<TResult>((val + 0.5));
}
inline void GetMouseFocusShakeData()
{
    Xellanix::Objects::XSMF mf;
    {
        struct _stati64 current_buf1;
        const fs::path spath = (Xellanix::Utilities::LocalAppData / L"Settings\\MouseFocus.xsmf");

        if (_wstat64(spath.wstring().c_str(), &current_buf1) == 0)
        {
            if (current_buf1.st_ctime != mfs_l_c2 ||
                current_buf1.st_mtime != mfs_l_m2 ||
                current_buf1.st_size != mfs_l_s2)
            {
                mfs_l_c2 = current_buf1.st_ctime;
                mfs_l_m2 = current_buf1.st_mtime;
                mfs_l_s2 = current_buf1.st_size;

                if (mf.Read(spath))
                {
                    m_isActivateWithShake = (mf >> L"5986d0a7-8675-5f8b-adf5-c593e81d387e").try_as<bool>(true);
                    m_shakeSensitivity = (mf >> L"d549aab1-da90-5d59-8054-3698c1b7e120").try_as<double>(7.0);
                    m_shakeDuration = (mf >> L"c51ad80d-77f1-509b-8007-5b1a4d980d95").try_as<unsigned short>(2000ui16);
                }
                else
                {
                    m_isActivateWithShake = true;
                    m_shakeSensitivity = 7.0;
                    m_shakeDuration = 2000ui16;
                }
            }
        }
    }
}
inline void ActivateMouseFocusWithShake()
{
    using namespace std::chrono_literals;

    // Shake Sensitivity (2.0 - 10.0)
    const double sensitivity = (std::min)((std::max)(m_shakeSensitivity, 2.0), 10.0);
    // Shake Duration (500 - 3000) (ms)
    const unsigned short duration = (std::min)((std::max)(m_shakeDuration, 500ui16), 3000ui16);

    // This MINIMUM_DISTANCE_PER_SECOND value is for: 
    // sensitivity = 7.0f
    // duration = 1000
    constexpr double MINIMUM_DISTANCE_PER_SECOND = 879.4788522419750;

    // Minimal Distance (px/s)
    // More sensitive, less value
    const double min_dis = MINIMUM_DISTANCE_PER_SECOND * 7.0 / (double)sensitivity * (double)duration / 1000.0;

    // 0.125 * POWER(sensitivity,2) - 0.25 * sensitivity + 5
    const unsigned short allowed_space = rnd<unsigned short>(0.125 * (sensitivity * sensitivity) - 0.25 * sensitivity + 5.0);
    // y = 0.375x + 0.25
    const double sens = 0.375f * sensitivity + 0.25f;
    // 2.5 * POW(x,3) - 17 * POW(x,2) + 17.5x + 102
    const unsigned short target = rnd<unsigned short>(2.5 * sens * sens * sens - 17.0 * sens * sens + 17.5 * sens + 102.0);
    const unsigned short min_target = target - allowed_space;
    const unsigned short max_target = target + allowed_space;

    double total_dis = 0.0;
    POINT last_point = {};
    unsigned int steps = 0U;
    auto next_shake = std::chrono::system_clock::now();

    // Reset points after several seconds
    std::thread([](double& total_dis, unsigned short duration, std::chrono::system_clock::time_point& next_shake, POINT& last_point, unsigned int& steps)
    {
        while (true)
        {
            if (!IsMouseFocusEnabled())
            {
                std::this_thread::sleep_for(1s);
                continue;
            }
            if (!m_isActivateWithShake)
            {
                std::this_thread::sleep_for(1s);
                continue;
            }

            {
                using std::chrono::system_clock;
                next_shake = system_clock::now();
                next_shake += std::chrono::milliseconds(duration);
            }
            std::this_thread::sleep_until(next_shake);

            total_dis = 0.0;
            last_point = POINT{};
            steps = 0U;
        }
    }, std::ref(total_dis), duration, std::ref(next_shake), std::ref(last_point), std::ref(steps)).detach();

    auto IsEqual = [](POINT const l, POINT const r)
    {
        return l.x == r.x && l.y == r.y;
    };

    while (true)
    {
        if (!IsMouseFocusEnabled())
        {
            std::this_thread::sleep_for(1s);
            continue;
        }
        GetMouseFocusShakeData();
        if (!m_isActivateWithShake)
        {
            std::this_thread::sleep_for(1s);
            continue;
        }

        {
            POINT p;
            if (GetCursorPos(&p))
            {
                if (steps > 0)
                {
                    if (!IsEqual(p, last_point))
                    {
                        const auto diff_x = p.x - last_point.x;
                        const auto diff_y = p.y - last_point.y;
                        const auto distance = sqrt(diff_x * diff_x + diff_y * diff_y);
                        total_dis += distance;

                        last_point = p;
                        ++steps;
                    }
                }
                else
                {
                    last_point = p;
                    ++steps;
                }
            }
        }

        if (total_dis >= min_dis)
        {
            if (steps >= min_target && steps <= max_target)
            {
                // Detect current shake is really shake or not

                bool is_shaked = false;
                {
                    std::array<bool, 100> probabilities{};
                    {
                        // Fill and shuffle probabilities

                        unsigned int steps_to_target = steps < target ? target - steps : steps - target;
                        // if 0 -> steps == target
                        const unsigned short nFalse = rnd<unsigned short>((double)steps_to_target / (double)allowed_space * 100.0);

                        for (unsigned short i = 0ui16; i < 100ui16; i++)
                        {
                            bool prob = true;
                            if (i < nFalse) prob = false;

                            probabilities[i] = prob;
                        }

                        std::random_device rd;
                        std::default_random_engine rng(rd());
                        std::shuffle(probabilities.begin(), probabilities.end(), rng);
                    }

                    std::random_device rd;
                    std::mt19937_64 gen(rd());
                    std::uniform_int_distribution<unsigned short> dis(0ui16, 99ui16);

                    auto idx = dis(gen);
                    is_shaked = probabilities[idx];
                }

                if (is_shaked)
                {
                    OutputDebugString(L"Shaked!\n");
                    ActivateMouseFocus();

                    total_dis = 0.0;
                    last_point = POINT{};
                    steps = 0u;
                    std::this_thread::sleep_until(next_shake);
                }
            }
        }
    }
}