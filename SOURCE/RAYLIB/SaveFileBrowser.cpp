/**
 * Windows-only save file picker — kept in its own translation unit so that
 * windows.h / shobjidl.h never pollute the raylib translation units.
 */
#include "SaveFileBrowser.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

#include <filesystem>
#include <string>

std::string browseForSaveFile(const std::string& gameId, const std::string& saveDir)
{
    std::string result;

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    IFileOpenDialog* pfd = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        // Filter to JSON files for this game (e.g. "BBB_*.json")
        std::wstring filterName = std::wstring(gameId.begin(), gameId.end()) + L" save files";
        std::wstring filterSpec = std::wstring(gameId.begin(), gameId.end()) + L"_*.json";
        COMDLG_FILTERSPEC filter[] = { { filterName.c_str(), filterSpec.c_str() } };
        pfd->SetFileTypes(1, filter);
        pfd->SetTitle(L"Load save file");

        // Start in the save directory if it exists
        IShellItem* startDir = nullptr;
        std::wstring wSaveDir(saveDir.begin(), saveDir.end());
        SHCreateItemFromParsingName(wSaveDir.c_str(), nullptr, IID_PPV_ARGS(&startDir));
        if (startDir) { pfd->SetFolder(startDir); startDir->Release(); }

        if (SUCCEEDED(pfd->Show(nullptr)))
        {
            IShellItem* item = nullptr;
            if (SUCCEEDED(pfd->GetResult(&item)))
            {
                PWSTR wpath = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &wpath)))
                {
                    char buf[MAX_PATH] = {};
                    WideCharToMultiByte(CP_UTF8, 0, wpath, -1, buf, MAX_PATH, nullptr, nullptr);
                    CoTaskMemFree(wpath);
                    result = buf;
                }
                item->Release();
            }
        }
        pfd->Release();
    }

    CoUninitialize();
    return result;
}

#else

std::string browseForSaveFile(const std::string&, const std::string&) { return ""; }

#endif
