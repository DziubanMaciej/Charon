#pragma once

#include "charon/util/error.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

struct TestFilesHelper {
    static void cleanupTestDirectory() {
        std::filesystem::remove_all(TEST_DIRECTORY_PATH);
        std::filesystem::create_directories(TEST_DIRECTORY_PATH);
    }

    static std::filesystem::path getTestFilePath(const std::filesystem::path &path) {
        if (path.is_absolute()) {
            for (std::filesystem::path currentPath = path; !currentPath.empty(); currentPath = currentPath.parent_path()) {
                if (currentPath == TEST_DIRECTORY_PATH) {
                    return path;
                }
            }
            FATAL_ERROR("Path not in test directory path");
        }
        return std::filesystem::path{TEST_DIRECTORY_PATH} / path;
    }

    static std::filesystem::path createFile(const std::filesystem::path &path) {
        const auto fullPath = getTestFilePath(path);
        std::ofstream file{fullPath};
        file.close();
        return fullPath;
    }

    static std::filesystem::path createDirectory(const std::filesystem::path &path) {
        const auto fullPath = getTestFilePath(path);
        std::filesystem::create_directories(fullPath);
        return fullPath;
    }

    static bool fileExists(const std::filesystem::path &path) {
        const auto fullPath = getTestFilePath(path);
        return std::filesystem::exists(fullPath);
    }

    static size_t countFilesInDirectory(const std::filesystem::path &path) {
        const auto fullPath = getTestFilePath(path);
        size_t result = 0u;
        auto iterator = fs::directory_iterator(fullPath);
        return std::distance(fs::begin(iterator), fs::end(iterator));
    }
};
