#pragma once

#include "charon/util/error.h"

#include <filesystem>
#include <fstream>
#include <string>

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

    static void moveFile(const std::filesystem::path &src, const std::filesystem::path &dst) {
        const auto fullSrcPath = getTestFilePath(src);
        const auto fullDstPath = getTestFilePath(dst);
        fs::rename(src, dst);
    }

    static void removeFile(const std::filesystem::path &path) {
        const auto fullPath = getTestFilePath(path);
        fs::remove(fullPath);
    }

    static std::ofstream openFileForWriting(const std::filesystem::path &path) {
        const auto fullPath = getTestFilePath(path);
        return std::ofstream{fullPath, std::ios::out};
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
        auto iterator = fs::directory_iterator(fullPath);
        return std::distance(fs::begin(iterator), fs::end(iterator));
    }

    static size_t countLinesInFile(const fs::path &path) {
        const auto fullPath = getTestFilePath(path);
        std::ifstream file{path};
        std::string line{};
        size_t result = 0;
        while (std::getline(file, line)) {
            result++;
        }
        return result;
    }
};
