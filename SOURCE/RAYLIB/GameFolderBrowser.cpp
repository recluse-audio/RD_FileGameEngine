/**
 * Windows-only folder picker — kept in its own translation unit so that
 * windows.h / shobjidl.h never pollute the raylib translation units.
 * (windows.h redefines CloseWindow, ShowCursor, DrawText, Rectangle, etc.)
 */
#include "GameFolderBrowser.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

#include <filesystem>
#include <string>

std::string browseForGameFolder()
{
    std::string result;

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    IFileOpenDialog* pfd = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        DWORD opts = 0;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);
        pfd->SetTitle(L"Select a FILE_GAME folder");

        IShellItem* startDir = nullptr;
        SHCreateItemFromParsingName(L"C:\\FILE_GAMES\\GAMES", nullptr,
                                    IID_PPV_ARGS(&startDir));
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

                    namespace fs = std::filesystem;
                    std::string chosen = buf;
                    for (auto& candidate : { chosen + "\\DATA", chosen })
                    {
                        if (fs::is_directory(candidate + "\\LEVELS"))
                        {
                            result = candidate;
                            break;
                        }
                    }
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

std::string browseForGameFolder() { return ""; }

#endif
