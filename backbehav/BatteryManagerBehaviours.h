#pragma once
#include "framework.h"
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

inline time_t bms_l_c = 0LL, bms_l_m = 0LL;
inline time_t bms_l_c2 = 0LL, bms_l_m2 = 0LL;
inline long long bms_l_s = 0LL;
inline long long bms_l_s2 = 0LL;

inline bool m_isBatteryManagerEnabled = true;
inline bool m_useModernBox = true;
inline unsigned short m_lowBatPerc = 25ui16;
inline unsigned short m_stopChargePerc = 90ui16;

inline bool IsBatteryManagerEnabled()
{
    Xellanix::Objects::XSMF settings{};
    {
        struct _stati64 current_buf1;
        const fs::path spath = (Xellanix::Utilities::LocalAppData / L"Settings\\BackBehav.xsmf");

        if (_wstat64(spath.wstring().c_str(), &current_buf1) == 0)
        {
            if (current_buf1.st_ctime != bms_l_c ||
                current_buf1.st_mtime != bms_l_m ||
                current_buf1.st_size != bms_l_s)
            {
                bms_l_c = current_buf1.st_ctime;
                bms_l_m = current_buf1.st_mtime;
                bms_l_s = current_buf1.st_size;

                if (settings.Read(spath))
                {
                    m_isBatteryManagerEnabled = (settings >> L"bdec9ff0-3b01-536c-a4d3-e50bd337640d").try_as<bool>(true);
                }
                else
                {
                    m_isBatteryManagerEnabled = true;
                }
            }
        }
    }

    return m_isBatteryManagerEnabled;
}
inline void GetBatteryManagerData()
{
    Xellanix::Objects::XSMF mf;
    {
        struct _stati64 current_buf1;
        const fs::path spath = (Xellanix::Utilities::LocalAppData / L"Settings\\BatteryManager.xsmf");

        if (_wstat64(spath.wstring().c_str(), &current_buf1) == 0)
        {
            if (current_buf1.st_ctime != bms_l_c2 ||
                current_buf1.st_mtime != bms_l_m2 ||
                current_buf1.st_size != bms_l_s2)
            {
                bms_l_c2 = current_buf1.st_ctime;
                bms_l_m2 = current_buf1.st_mtime;
                bms_l_s2 = current_buf1.st_size;

                if (mf.Read(spath))
                {
                    m_lowBatPerc = (mf >> L"cecf45fd-d5db-5c6c-9bd1-a7581746066c").try_as<unsigned short>(25ui16);
                    m_stopChargePerc = (mf >> L"5d8bf58a-6ae6-52cd-aa2b-e0d7e643904c").try_as<unsigned short>(90ui16);
                    m_useModernBox = (mf >> L"95a53186-3637-5b46-8640-3e1dd4915c1a").try_as<bool>(true);
                }
                else
                {
                    m_lowBatPerc = 25ui16;
                    m_stopChargePerc = 90ui16;
                    m_useModernBox = true;
                }
            }
        }
    }
}

inline void CheckBatteryPerc()
{
    using namespace std::chrono_literals;

    auto ShowMsg = [](std::wstring const& mes, std::wstring const& title = L"Xellanix Battery Manager")
    {
        MessageBox(NULL, mes.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
    };

    auto ProcessWMI = [&ShowMsg](std::wstring const& _class, std::wstring const& _property, UINT& retVal, std::wstring const& _namespace = L"ROOT\\WMI")
    {
        auto ShowErr = [&ShowMsg](std::wstring const& mes, HRESULT err)
        {
            std::wostringstream wss;
            wss << mes << std::hex << err;
            ShowMsg(wss.str());
        };

        HRESULT hres;

        // Step 1: --------------------------------------------------
        // Initialize COM. ------------------------------------------
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres))
        {
            ShowErr(L"Failed to initialize COM library. Error code = 0x", hres);
            return false;
        }

        // Step 2: --------------------------------------------------
        // Set general COM security levels --------------------------
        hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
        if (FAILED(hres))
        {
            ShowErr(L"Failed to initialize security. Error code = 0x", hres);
            CoUninitialize();
            return false;
        }

        // Step 3: ---------------------------------------------------
        // Obtain the initial locator to WMI -------------------------
        IWbemLocator* pLoc = NULL;
        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hres))
        {
            ShowErr(L"Failed to create IWbemLocator object. Err code = 0x", hres);
            CoUninitialize();
            return false;
        }

        // Step 4: -----------------------------------------------------
        // Connect to WMI through the IWbemLocator::ConnectServer method
        IWbemServices* pSvc = NULL;
        // Connect to the namespace with
        // the current user and obtain pointer pSvc
        // to make IWbemServices calls.
        hres = pLoc->ConnectServer(_bstr_t(_namespace.c_str()), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        if (FAILED(hres))
        {
            ShowErr(L"Could not connect. Error code = 0x", hres);
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        // Step 5: --------------------------------------------------
        // Set security levels on the proxy -------------------------
        hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if (FAILED(hres))
        {
            ShowErr(L"Could not set proxy blanket. Error code = 0x", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        // Step 6: --------------------------------------------------
        // Use the IWbemServices pointer to make requests of WMI ----
        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t((L"SELECT * FROM " + _class).c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
        if (FAILED(hres))
        {
            ShowErr(L"Query for operating system name failed. Error code = 0x", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        // Step 7: -------------------------------------------------
        // Get the data from the query in step 6 -------------------
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (0 == uReturn)
            {
                break;
            }

            VARIANT vtProp;

            VariantInit(&vtProp);
            // Get the value of the Name property
            hr = pclsObj->Get(_property.c_str(), 0, &vtProp, 0, 0);
            retVal = vtProp.uintVal;
            VariantClear(&vtProp);

            pclsObj->Release();
        }

        // Cleanup
        // ========
        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();

        return true;
    };

    unsigned short lastperc = 0ui16;

    UINT fullCap;
    while (!ProcessWMI(L"BatteryFullChargedCapacity", L"FullChargedCapacity", fullCap)) {}

    auto BatteryBox = [](std::wstring const& title, std::wstring const& content)
    {
        if (m_useModernBox)
        {
            // Use Custom WinUI Style Message Box

            auto exepath = fs::path(Xellanix::Utilities::AppDir) / L"msgbx\\msgbx.exe";

            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));
            //Prepare CreateProcess args
            const std::wstring app_w = exepath.wstring();
            const std::wstring arg_w = (L"\"" + app_w + L"\" \"Xellanix Battery Manager\" \"" + title + L"\" \"" + content + L"\"");

            if (CreateProcess(app_w.c_str(), const_cast<wchar_t*>(arg_w.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                // Success
                MessageBeep(MB_ICONERROR);
                
                WaitForSingleObject(pi.hProcess, INFINITE);
                
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                return true;
            }

            return false;
        }
        else
        {
            // Use Windows Default Message Box
            const auto newcontent = title + L"\n\n" + content;
            MessageBox(NULL, newcontent.c_str(), L"Xellanix Battery Manager", MB_OK | MB_ICONERROR);

            return true;
        }
    };

    bool b_lowHasShow = false, b_fullHasShow = false;

    while (true)
    {
        if (!IsBatteryManagerEnabled())
        {
            std::this_thread::sleep_for(1s);
            continue;
        }

        UINT remainCap;
        if (ProcessWMI(L"BatteryStatus", L"RemainingCapacity", remainCap))
        {
            const auto perc = (unsigned short)std::ceil((double)remainCap * 100.0 / (double)fullCap);

            if (perc != lastperc)
            {
                GetBatteryManagerData();

                if (perc < lastperc)
                {
                    // Not Charging
                    b_fullHasShow = false;

                    if (!b_lowHasShow)
                    {
                        if (perc <= m_lowBatPerc)
                        {
                            const std::wstring title{ std::to_wstring(m_lowBatPerc) + L"% Remaining Battery" };
                            if (BatteryBox(title, L"In order to maintain battery life, please charge your device now, before it completely runs out of power!"))
                            {
                                b_lowHasShow = true;
                            }
                        }
                    }
                }
                else if (perc > lastperc)
                {
                    if (lastperc != 0ui16) // Pretend from showing the box while on first startup
                    {
                        // On Charging
                        b_lowHasShow = false;

                        if (!b_fullHasShow)
                        {
                            if (perc >= m_stopChargePerc)
                            {
                                const std::wstring title{ L"Battery Has Reached " + std::to_wstring(m_stopChargePerc) + L"%" };
                                if (BatteryBox(title, L"In order to maintain battery life, please stop charging the device, according to the maximum reasonable level that has been determined!"))
                                {
                                    b_fullHasShow = true;
                                }
                            }
                        }
                    }
                }

                lastperc = perc;
            }
        }

        std::this_thread::sleep_for(2500ms);
    }
}