/**
 * Made by Ryan Devens on 2026-02-25
 */

#pragma once
#include <string>
#include <vector>

/**
 * Abstract base class for file I/O operations on storage. Subclass per platform
 * (ESP32, Raylib desktop, etc.) to provide the appropriate implementation.
 */
class FileOperator
{
public:
    virtual ~FileOperator() = default;

    /**
     * Read the file at the given path and return its full contents as a string.
     * Returns an empty string if the file cannot be opened.
     */
    virtual std::string load(const std::string& path) = 0;

    /**
     * Write content to the file at the given path, overwriting any existing data.
     */
    virtual void writeToFile(const std::string& path, const std::string& content) = 0;

    /**
     * Append content to the file at the given path without overwriting existing data.
     */
    virtual void appendToFile(const std::string& path, const std::string& content) = 0;

    /**
     * Return the names of all entries (files and subdirectories) inside dirPath.
     * Returns an empty vector if the directory does not exist or cannot be opened.
     */
    virtual std::vector<std::string> listDirectory(const std::string& dirPath) = 0;

};
