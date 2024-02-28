#pragma once
#pragma comment(lib, "Urlmon.lib")

#include <string>
#include <functional>
#include <Windows.h>

enum DownloadFileStatus
{
    DOWNLOAD_STARTED = 0,
    DOWNLOAD_DOWNLOADING,
    DOWNLOAD_FINISHING,
    DOWNLOAD_SUCCESS,
    DOWNLOAD_ERROR = -1
};

struct DownloadFileArgs
{
    uint64_t CurrentDownloaded;
    uint64_t TotalToDownload;
    double Percent;
    std::wstring ErrorMessage;
};

struct DownloadFileHelper
{
private:
	std::function<void(DownloadFileStatus, DownloadFileArgs const)> m_progressFunc;
    bool m_isPaused = false;

    std::wstring tempFolder = L"";

    class ProgressCallbackHandler : public IBindStatusCallback
    {
    private:
        double m_percentLast;
        bool isCancel = false;
        std::function<void(DownloadFileStatus, DownloadFileArgs const)> onprogFunc;

    public:
        ProgressCallbackHandler() : m_percentLast(0), isCancel(false)
        {
        }

        // IUnknown

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
        {

            if (IsEqualIID(IID_IBindStatusCallback, riid)
                || IsEqualIID(IID_IUnknown, riid))
            {
                *ppvObject = reinterpret_cast<void*>(this);
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef()
        {
            return 2UL;
        }

        ULONG STDMETHODCALLTYPE Release()
        {
            return 1UL;
        }

        // IBindStatusCallback

        HRESULT STDMETHODCALLTYPE
            OnStartBinding(DWORD /*dwReserved*/, IBinding* /*pib*/)
        {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE
            GetPriority(LONG* /*pnPriority*/)
        {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE
            OnLowResource(DWORD /*reserved*/)
        {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE
            OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR /*szStatusText*/)
        {
            switch (ulStatusCode)
            {
                case BINDSTATUS_FINDINGRESOURCE:
                    // "Finding resource..."
                    break;
                case BINDSTATUS_CONNECTING:
                    // "Connecting..."
                    break;
                case BINDSTATUS_SENDINGREQUEST:
                    // "Sending request..."
                    break;
                case BINDSTATUS_MIMETYPEAVAILABLE:
                    // "Mime type available"
                    break;
                case BINDSTATUS_CACHEFILENAMEAVAILABLE:
                    // "Cache filename available"
                    break;
                case BINDSTATUS_BEGINDOWNLOADDATA:
                    // "Begin download"
                    break;
                case BINDSTATUS_DOWNLOADINGDATA:
                case BINDSTATUS_ENDDOWNLOADDATA:
                {
                    double percent = (100.0 * static_cast<double>(ulProgress) / static_cast<double>(ulProgressMax));
                    if (m_percentLast < percent)
                    {
                        onprogFunc(DownloadFileStatus::DOWNLOAD_DOWNLOADING, DownloadFileArgs{ ulProgress, ulProgressMax, percent, std::wstring() });
                        m_percentLast = percent;
                    }
                    if (ulStatusCode == BINDSTATUS_ENDDOWNLOADDATA)
                    {
                        onprogFunc(DownloadFileStatus::DOWNLOAD_FINISHING, DownloadFileArgs{ 0, 0, 0, std::wstring() });
                    }
                }
                break;

                default:
                {
                    // "Status code : " + ulStatusCode
                }
            }
            // The download can be cancelled by returning E_ABORT here
            // of from any other of the methods.

            if (isCancel)
            {
                return E_ABORT;
            }

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE
            OnStopBinding(HRESULT /*hresult*/, LPCWSTR /*szError*/)
        {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE
            GetBindInfo(DWORD* /*grfBINDF*/, BINDINFO* /*pbindinfo*/)
        {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE
            OnDataAvailable(DWORD /*grfBSCF*/, DWORD /*dwSize*/, FORMATETC* /*pformatetc*/, STGMEDIUM* /*pstgmed*/)
        {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE
            OnObjectAvailable(REFIID /*riid*/, IUnknown* /*punk*/)
        {
            return E_NOTIMPL;
        }

        void SetProgressCallback(std::function<void(DownloadFileStatus, DownloadFileArgs const)> func)
        {
            onprogFunc = func;
        }

        void CancelDownload(bool val)
        {
            isCancel = val;
        }
    };

    ProgressCallbackHandler callbackHandler;

public:
    void StartDownload(std::wstring const& fileurl, std::wstring const& savePath)
    {
        m_progressFunc(DownloadFileStatus::DOWNLOAD_STARTED, DownloadFileArgs{ 0, 0, 0, std::wstring() });
        {
            struct _stati64 buf;
            if (_wstat64((std::filesystem::path(tempFolder) / L"is_paused.temp").wstring().c_str(), &buf) != 0)
            {
                // invalidate cache, so file is always downloaded from web site
                // (if not called, the file will be retieved from the cache if
                // it's already been downloaded.)
                DeleteUrlCacheEntry(fileurl.c_str());
            }
        }

        callbackHandler.CancelDownload(false);
        callbackHandler.SetProgressCallback(m_progressFunc);
        IBindStatusCallback* pBindStatusCallback = NULL;
        callbackHandler.QueryInterface(IID_IBindStatusCallback,
                                       reinterpret_cast<void**>(&pBindStatusCallback));

        m_isPaused = false;

        HRESULT hr = URLDownloadToFile(
            NULL,   // A pointer to the controlling IUnknown interface
            fileurl.c_str(),
            savePath.c_str(),
            0,      // Reserved. Must be set to 0.
            pBindStatusCallback);
        if (SUCCEEDED(hr))
        {
            m_progressFunc(DownloadFileStatus::DOWNLOAD_SUCCESS, DownloadFileArgs{ 0, 0, 0, std::wstring() });

            try
            {
                std::filesystem::remove(std::filesystem::path(tempFolder) / L"is_paused.temp");
                DeleteUrlCacheEntry(fileurl.c_str());
            }
            catch (...)
            {
            }
        }
        else
        {
            std::wostringstream wos;
            wos << L"An error occured : error code = 0x" << std::hex << hr;
            m_progressFunc(DownloadFileStatus::DOWNLOAD_ERROR, DownloadFileArgs{ 0, 0, 0, wos.str() });

            if (!m_isPaused)
            {
                try
                {
                    std::filesystem::remove(std::filesystem::path(tempFolder) / L"is_paused.temp");
                    DeleteUrlCacheEntry(fileurl.c_str());
                }
                catch (...)
                {
                }
            }
        }

        // should usually call Release on a COM object, but this one (callbackHandler)
        // was created on the stack so is going out of scope now and will die anyway.
        pBindStatusCallback->Release();
        pBindStatusCallback = NULL;
	}

	void Progress(std::function<void(DownloadFileStatus, DownloadFileArgs const)> progressFunc)
	{
		m_progressFunc = progressFunc;
	}

    void Pause()
    {
        std::filesystem::create_directories(std::filesystem::path(tempFolder));

        std::wofstream wof;
        wof.open(std::filesystem::path(tempFolder) / L"is_paused.temp");
        wof << L"";
        wof.close();

        m_isPaused = true;
        callbackHandler.CancelDownload(true);
    }

    void SetTempFolder(std::wstring const& path)
    {
        tempFolder = path;
    }

	void Close()
	{
		if (m_progressFunc)
		{
			m_progressFunc = nullptr;
		}
	}
};