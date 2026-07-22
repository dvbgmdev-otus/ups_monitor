/**
 * @file fs_utils_test.cpp
 * @brief Unit-тесты утилит разрешения путей.
 */
#include "fs_utils.h"

#include <gtest/gtest.h>
#include <sys/stat.h>

#include <string>

namespace {

bool dirExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false;
    return S_ISDIR(info.st_mode);
}

}  // anonymous namespace

class FsUtilsTest : public ::testing::Test {
protected:
    std::string m_binDir;

    void SetUp() override {
        const std::string currentDir = utils::resolvePath(".");
        ASSERT_GT(currentDir.size(), 2u);
        ASSERT_EQ(currentDir.substr(currentDir.size() - 2), "/.");
        m_binDir = currentDir.substr(0, currentDir.size() - 2);

        // Базовые проверки
        ASSERT_FALSE(m_binDir.empty());
        ASSERT_TRUE(dirExists(m_binDir));
    }
};

#if (1)  // Разрешение путей

// Тест 1.1: Абсолютный путь возвращается без изменений
TEST_F(FsUtilsTest, ResolvePath_Absolute_ReturnsSame) {
    const std::string input = "/etc/passwd";
    const std::string output = utils::resolvePath(input);
    EXPECT_EQ(output, input);
}

// Тест 1.2: Относительный путь дополняется директорией бинарника
TEST_F(FsUtilsTest, ResolvePath_Relative_UsesBinaryDir) {
    const std::string rel = "config/ups.ini";
    const std::string full = utils::resolvePath(rel);
    // full должен начинаться с m_binDir + "/"
    const std::string expectedPrefix = m_binDir + "/";
    ASSERT_GE(full.size(), expectedPrefix.size());
    EXPECT_EQ(full.substr(0, expectedPrefix.size()), expectedPrefix);
    // И дальше — относительный путь
    EXPECT_EQ(full.substr(expectedPrefix.size()), rel);
}

// Тест 1.3: Для "." возвращается путь относительно директории бинарника
TEST_F(FsUtilsTest, ResolvePath_CurrentDirFallback) {
    const std::string out = utils::resolvePath(".");
    const std::string expected = m_binDir + "/.";
    EXPECT_EQ(out, expected);
}

#endif
