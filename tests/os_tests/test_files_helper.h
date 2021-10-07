#pragma once

#include <filesystem>
#include <fstream>

struct TestFilesHelper {
    static void cleanupTestDirectory() {
        std::filesystem::remove_all(TEST_DIRECTORY_PATH);
        std::filesystem::create_directories(TEST_DIRECTORY_PATH);
    }

    static std::filesystem::path getTestFilePath(const std::filesystem::path &path) {
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
};
