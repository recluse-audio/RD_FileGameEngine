/**
 * Made by Ryan Devens on 2026-02-25
 */

#pragma once
#include "../SHARED/FILE_OPERATOR/FileOperator.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

/**
 * Desktop (Raylib) implementation of FileOperator.
 * Reads files from the local filesystem using std::ifstream.
 *
 * Paths are data-root-relative (e.g. "/LOCATIONS/DESK/MAIN/Desk_Full.json").
 * The "KSC_DATA" directory in the working directory is treated as the data root,
 * so a leading '/' is replaced with "KSC_DATA/".
 */
class RaylibFileOperator : public FileOperator
{
public:
    std::string load(const std::string& path) override
    {
        std::string fullPath = sdPath(path);
        std::ifstream file(fullPath);
        if (!file.is_open())
            return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void writeToFile(const std::string& path, const std::string& content) override
    {
        namespace fs = std::filesystem;
        fs::path full = sdPath(path);
        fs::create_directories(full.parent_path());
        std::ofstream file(full);
        if (file.is_open())
            file << content;
    }

    void appendToFile(const std::string& path, const std::string& content) override
    {
        std::ofstream file(sdPath(path), std::ios::app);
        if (file.is_open())
            file << content;
    }

    std::vector<std::string> listDirectory(const std::string& path) override
    {
        namespace fs = std::filesystem;
        std::vector<std::string> entries;
        fs::path dir = sdPath(path);
        if (!fs::exists(dir)) return entries;
        for (auto& entry : fs::directory_iterator(dir))
            entries.push_back(entry.path().filename().string());
        return entries;
    }

private:
    static std::string sdPath(const std::string& path)
    {
        if (!path.empty() && path[0] == '/')
            return "KSC_DATA" + path;
        return path;
    }
};
