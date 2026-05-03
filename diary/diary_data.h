#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace diary {

constexpr const char* kLogDir = "log";
constexpr const char* kPasswordCheckFile = "log/password_check.txt";
constexpr const char* kSettingsFile = "log/settings.json";
constexpr const char* kAuthCheckString = "DIARY_AUTH_OK";

struct DiaryEntry {
    std::string folderName;
    std::string filePath;
    std::string preview;
};

struct Settings {
    int theme = -1;
    int effect = -1;
    std::string name;
};

Settings loadSettings();

void saveSettings(const Settings& settings);

bool isFirstLaunch();

void setPassword(const std::string& password);

bool verifyPassword(const std::string& password);

void resetBruteForceProtection();

void ensureLogDirectoryExists();

std::string createDiary(const std::string& content, const std::string& password);

void editDiary(const std::string& folderPath, const std::string& newContent, const std::string& password);

void deleteDiary(const std::string& folderPath);

std::string readDiary(const std::string& folderPath, const std::string& password);

std::vector<DiaryEntry> listDiaryEntries(const std::string& password, int maxPreviewLen = 60);

} // namespace diary
