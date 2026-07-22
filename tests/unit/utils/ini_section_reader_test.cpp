/**
 * @file ini_section_reader_test.cpp
 * @brief Unit-тесты utils::IniSectionReader.
 *
 * Проверяется:
 *  - успешное чтение секций (happy path)
 *  - отсутствие файла
 *  - пустое имя секции
 *  - дубликаты секций
 *  - файл без секций
 *  - корректность lastError()
 *
 * Используются временные ini-файлы, создаваемые прямо в тестах.
 */

#include "ini_section_reader.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class IniSectionReaderTest : public ::testing::Test {
protected:
    std::vector<std::string> m_tempFiles;

    std::string writeTempIni(const std::string& content) {
        const char* tempDir = std::getenv("TMPDIR");
        std::string pathTemplate = tempDir != nullptr && tempDir[0] != '\0' ? tempDir : "/tmp";
        if (pathTemplate.back() != '/') pathTemplate += '/';
        pathTemplate += "ups_monitor_ini_section_reader_XXXXXX";

        std::vector<char> pathBuffer(pathTemplate.begin(), pathTemplate.end());
        pathBuffer.push_back('\0');

        const int fileDescriptor = mkstemp(pathBuffer.data());
        if (fileDescriptor == -1) {
            throw std::runtime_error("Failed to create temporary ini file");
        }

        const std::string path(pathBuffer.data());
        close(fileDescriptor);

        std::ofstream out(path);
        if (!out.good()) {
            std::remove(path.c_str());
            throw std::runtime_error("Failed to create temp ini file: " + path);
        }

        out << content;
        out.close();

        m_tempFiles.push_back(path);
        return path;
    }

    void TearDown() override {
        for (const std::string& file : m_tempFiles) {
            std::remove(file.c_str());
        }
    }
};

#if (1)  // Успешное чтение

// Тест 1.1: Успешное чтение списка секций
TEST_F(IniSectionReaderTest, Parse_ValidFile_ReturnsSections) {
    const std::string ini = writeTempIni(R"(
# comment
[SECTION_A]

[SECTION_B]
)");
    utils::IniSectionReader reader(ini);
    EXPECT_TRUE(reader.ok());
    EXPECT_TRUE(reader.lastError().empty());
    ASSERT_EQ(reader.sections().size(), 2u);
    EXPECT_EQ(reader.sections()[0], "SECTION_A");
    EXPECT_EQ(reader.sections()[1], "SECTION_B");
}
#endif

#if (1)  // Ошибки файла

// Тест 2.1: Файл не существует
TEST_F(IniSectionReaderTest, Parse_MissingFile_ReturnsError) {
    utils::IniSectionReader reader("definitely_no_such_file.ini");
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.lastError().empty());
    EXPECT_TRUE(reader.sections().empty());
}
#endif

#if (1)  // Синтаксические ошибки

// Тест 3.1: Пустое имя секции []
TEST_F(IniSectionReaderTest, Parse_EmptySectionName_ReturnsError) {
    const std::string ini = writeTempIni(R"(
[]
)");
    utils::IniSectionReader reader(ini);
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.lastError().empty());
    EXPECT_TRUE(reader.sections().empty());
}

// Тест 3.2: Дубликат имени секции
TEST_F(IniSectionReaderTest, Parse_DuplicateSection_ReturnsError) {
    const std::string ini = writeTempIni(R"(
[A]
[A]
)");
    utils::IniSectionReader reader(ini);
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.lastError().empty());
}
#endif

#if (1)  // Структурные ошибки

// Тест 4.1: В файле нет ни одной секции
TEST_F(IniSectionReaderTest, Parse_FileWithoutSections_ReturnsError) {
    const std::string ini = writeTempIni(R"(
# only comments
# and empty lines
)");
    utils::IniSectionReader reader(ini);
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.lastError().empty());
    EXPECT_TRUE(reader.sections().empty());
}
#endif
