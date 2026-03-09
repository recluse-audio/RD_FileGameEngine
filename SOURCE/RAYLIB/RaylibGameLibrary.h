/**
 * Made by Ryan Devens on 2026-03-09
 */

#pragma once
#include <filesystem>
#include <string>
#include <vector>

struct GameEntry
{
    std::string name;      // display name shown in the picker
    std::string dataPath;  // absolute path to the game's DATA directory
};

/**
 * Discovers FILE_GAME data directories on the local filesystem.
 *
 * Scans every subdirectory of FILE_GAMES_ROOT for a folder named "DATA"
 * that contains a "LEVELS" subdirectory (searched up to maxDepth levels deep).
 * The default root is C:\FILE_GAMES\GAMES on Windows.
 */
class RaylibGameLibrary
{
public:
    static constexpr const char* k_DefaultRoot =
#ifdef _WIN32
        R"(C:\FILE_GAMES\GAMES)";
#else
        "";
#endif

    static std::vector<GameEntry> discover(
        const std::string& gamesRoot = k_DefaultRoot,
        int maxDepth = 3)
    {
        namespace fs = std::filesystem;
        std::vector<GameEntry> entries;

        fs::path root(gamesRoot);
        if (!fs::exists(root) || !fs::is_directory(root))
            return entries;

        for (auto& gameDir : fs::directory_iterator(root))
        {
            if (!gameDir.is_directory()) continue;

            // Accept the game folder itself as the data root if it contains LEVELS/
            std::string found;
            if (fs::exists(gameDir.path() / "LEVELS"))
                found = gameDir.path().string();
            else
                found = findDataDir(gameDir.path(), maxDepth);

            if (!found.empty())
            {
                GameEntry e;
                e.name     = prettify(gameDir.path().filename().string());
                e.dataPath = found;
                entries.push_back(std::move(e));
            }
        }

        return entries;
    }

private:
    static std::string prettify(const std::string& folderName)
    {
        std::string s = folderName;
        for (const char* suffix : { "_FILE_GAME", "_GAME" })
        {
            std::string sfx(suffix);
            if (s.size() > sfx.size() &&
                s.substr(s.size() - sfx.size()) == sfx)
            {
                s = s.substr(0, s.size() - sfx.size());
                break;
            }
        }
        for (char& c : s)
            if (c == '_') c = ' ';
        if (!s.empty())
            s[0] = (char)toupper((unsigned char)s[0]);
        return s;
    }

    static std::string findDataDir(const std::filesystem::path& dir, int depth)
    {
        namespace fs = std::filesystem;
        if (depth <= 0) return "";

        for (auto& entry : fs::directory_iterator(dir))
        {
            if (!entry.is_directory()) continue;
            if (entry.path().filename() == "DATA" &&
                fs::exists(entry.path() / "LEVELS"))
            {
                return entry.path().string();
            }
            std::string found = findDataDir(entry.path(), depth - 1);
            if (!found.empty()) return found;
        }
        return "";
    }
};
