#include "diary/diary_data.h"
#include "diary/encrypt.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <string>

namespace fs = std::filesystem;

namespace diary {

void ensureLogDirectoryExists() {
    std::error_code ec;
    if (!fs::exists(kLogDir, ec)) {
        if (!fs::create_directory(kLogDir, ec)) {
            throw std::runtime_error("无法创建 log 目录: " + ec.message());
        }
    }
}

bool isFirstLaunch() {
    ensureLogDirectoryExists();
    std::error_code ec;
    return !fs::exists(kPasswordCheckFile, ec);
}

void setPassword(const std::string& password) {
    ensureLogDirectoryExists();

    std::vector<uint8_t> blob = encryptToBlob(kAuthCheckString, password);

    std::ofstream file(kPasswordCheckFile, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法写入密码校验文件");
    }
    file.write(reinterpret_cast<const char*>(blob.data()), blob.size());
    file.close();
}

bool verifyPassword(const std::string& password) {
    std::error_code ec;
    if (!fs::exists(kPasswordCheckFile, ec)) {
        return false;
    }

    std::ifstream file(kPasswordCheckFile, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("无法读取密码校验文件");
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> blob(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(blob.data()), fileSize)) {
        throw std::runtime_error("读取密码校验文件失败");
    }
    file.close();

    try {
        std::string decrypted = decryptFromBlob(blob, password);
        return decrypted == kAuthCheckString;
    } catch (const std::exception&) {
        return false;
    }
}

static std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y%m%d_%H%M%S");
    return oss.str();
}

static std::string getDiaryFilePath(const std::string& folderPath) {
    return folderPath + "/diary.hyw";
}

std::string createDiary(const std::string& content, const std::string& password) {
    ensureLogDirectoryExists();

    std::string folderName = getCurrentTimestamp();
    std::string folderPath = std::string(kLogDir) + "/" + folderName;

    std::error_code ec;
    if (!fs::create_directory(folderPath, ec)) {
        throw std::runtime_error("创建日记文件夹失败: " + ec.message());
    }

    std::vector<uint8_t> blob = encryptToBlob(content, password);

    std::string filePath = getDiaryFilePath(folderPath);
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法写入日记文件");
    }
    file.write(reinterpret_cast<const char*>(blob.data()), blob.size());
    file.close();

    return folderPath;
}

void editDiary(const std::string& folderPath, const std::string& newContent, const std::string& password) {
    std::string filePath = getDiaryFilePath(folderPath);

    std::vector<uint8_t> blob = encryptToBlob(newContent, password);

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法写入日记文件");
    }
    file.write(reinterpret_cast<const char*>(blob.data()), blob.size());
    file.close();
}

void deleteDiary(const std::string& folderPath) {
    std::error_code ec;

    std::string filePath = getDiaryFilePath(folderPath);
    if (fs::exists(filePath, ec)) {
        fs::remove(filePath, ec);
    }

    if (fs::exists(folderPath, ec)) {
        fs::remove(folderPath, ec);
    }
}

std::string readDiary(const std::string& folderPath, const std::string& password) {
    std::string filePath = getDiaryFilePath(folderPath);

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("无法读取日记文件: " + filePath);
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> blob(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(blob.data()), fileSize)) {
        throw std::runtime_error("读取日记文件失败");
    }
    file.close();

    return decryptFromBlob(blob, password);
}

std::vector<DiaryEntry> listDiaryEntries(const std::string& password, int maxPreviewLen) {
    std::vector<DiaryEntry> entries;
    std::error_code ec;

    ensureLogDirectoryExists();

    for (const auto& entry : fs::directory_iterator(kLogDir, ec)) {
        if (!entry.is_directory()) {
            continue;
        }

        std::string folderName = entry.path().filename().string();
        std::string filePath = getDiaryFilePath(entry.path().string());

        if (!fs::exists(filePath, ec)) {
            continue;
        }

        DiaryEntry de;
        de.folderName = folderName;
        de.filePath = filePath;

        try {
            std::string content = readDiary(entry.path().string(), password);
            if (static_cast<int>(content.size()) > maxPreviewLen) {
                de.preview = content.substr(0, static_cast<size_t>(maxPreviewLen)) + "...";
            } else {
                de.preview = content;
            }
            std::replace(de.preview.begin(), de.preview.end(), '\n', ' ');
        } catch (const std::exception&) {
            de.preview = "(无法解密此日记)";
        }

        entries.push_back(de);
    }

    std::sort(entries.begin(), entries.end(),
              [](const DiaryEntry& a, const DiaryEntry& b) {
                  return a.folderName > b.folderName;
              });

    return entries;
}

Settings loadSettings() {
    Settings settings;

    if (!fs::exists(kSettingsFile)) {
        return settings;
    }

    try {
        std::ifstream file(kSettingsFile, std::ios::binary);
        if (!file.is_open()) {
            return settings;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        size_t themePos = content.find("\"theme\":");
        if (themePos != std::string::npos) {
            themePos += 8;
            int themeValue = std::stoi(content.substr(themePos));
            if (themeValue >= 0 && themeValue <= 1) {
                settings.theme = themeValue;
            }
        }

        size_t effectPos = content.find("\"effect\":");
        if (effectPos != std::string::npos) {
            effectPos += 9;
            int effectValue = std::stoi(content.substr(effectPos));
            if (effectValue >= 0 && effectValue <= 1) {
                settings.effect = effectValue;
            }
        }

        size_t namePos = content.find("\"name\":");
        if (namePos != std::string::npos) {
            namePos += 7;
            while (namePos < content.size() && (content[namePos] == ' ' || content[namePos] == ':' || content[namePos] == '\t')) {
                namePos++;
            }
            if (namePos < content.size() && content[namePos] == '"') {
                namePos++;
            }
            size_t nameEnd = namePos;
            while (nameEnd < content.size() && content[nameEnd] != '"') {
                nameEnd++;
            }
            if (nameEnd > namePos) {
                settings.name = content.substr(namePos, nameEnd - namePos);
            }
        }
    } catch (...) {
    }

    return settings;
}

void saveSettings(const Settings& settings) {
    ensureLogDirectoryExists();

    std::ofstream file(kSettingsFile, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开设置文件进行写入");
    }

    std::string content = "{\n    \"theme\": " + std::to_string(settings.theme) + ",\n    \"effect\": " + std::to_string(settings.effect) + ",\n    \"name\": \"" + settings.name + "\"\n}\n";
    file.write(content.c_str(), content.size());
}

} // namespace diary
