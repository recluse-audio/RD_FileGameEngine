#pragma once
#include "FILE_OPERATOR/FileOperator.h"
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/**
 * In-test FileOperator. Serves named files from an in-memory map and writes
 * real files to disk. listDirectory("/GAME_STATE") resolves from the in-memory
 * map; all other paths scan the real filesystem.
 *
 * diskRoot (optional): when set, virtual paths (leading '/') that are not in
 * the in-memory map fall back to diskRoot + path on disk. Real paths (no
 * leading '/') always fall back to disk directly.
 *
 * writeRoot (optional): when set, virtual paths written via writeToFile or
 * appendToFile are resolved as writeRoot + path instead of literal path.
 * Use this to redirect game-state writes to a test output directory without
 * touching source data in diskRoot.
 */
class TestFileOperator : public FileOperator
{
public:
    std::map<std::string, std::string> files;
    std::string diskRoot;  // e.g. "../KSC/KSC_DATA"
    std::string writeRoot; // e.g. "TESTS/OUTPUT/GAME_RUNNER/KSC_DATA"

    std::string load(const std::string& path) override
    {
        auto it = files.find(path);
        if (it != files.end()) return it->second;

        std::string diskPath = (!diskRoot.empty() && !path.empty() && path[0] == '/')
                             ? diskRoot + path
                             : path;
        std::ifstream f(diskPath);
        if (!f.is_open()) return "";
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    void writeToFile(const std::string& path, const std::string& content) override
    {
        std::string diskPath = resolvePath(path, writeRoot);
        fs::path p(diskPath);
        fs::create_directories(p.parent_path());
        std::ofstream f(p);
        f << content;
    }

    void appendToFile(const std::string& path, const std::string& content) override
    {
        files[path] += content;

        if (writeRoot.empty()) return;

        // Write the full updated content so the disk file reflects the current state.
        std::string diskPath = resolvePath(path, writeRoot);
        fs::path p(diskPath);
        fs::create_directories(p.parent_path());
        std::ofstream f(p);
        f << files[path];
    }

    std::vector<std::string> listDirectory(const std::string& dirPath) override
    {
        // Resolve in-memory paths by prefix — return only direct children,
        // deduplicating directory names (matches real filesystem behaviour).
        if (!dirPath.empty() && dirPath[0] == '/')
        {
            std::vector<std::string> names;
            std::string prefix = dirPath + "/";
            for (auto& [path, _] : files)
            {
                if (path.rfind(prefix, 0) != 0) continue;
                std::string rel   = path.substr(prefix.size());
                std::string child = rel.substr(0, rel.find('/'));
                if (std::find(names.begin(), names.end(), child) == names.end())
                    names.push_back(child);
            }
            return names;
        }

        // Real filesystem directory
        std::vector<std::string> entries;
        if (!fs::exists(dirPath)) return entries;
        for (auto& entry : fs::directory_iterator(dirPath))
            entries.push_back(entry.path().filename().string());
        return entries;
    }

private:
    static std::string resolvePath(const std::string& path, const std::string& root)
    {
        if (!root.empty() && !path.empty() && path[0] == '/')
            return root + path;
        return path;
    }
};
