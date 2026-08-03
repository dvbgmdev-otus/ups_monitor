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
#include "temp_ini_file.h"

#include <gtest/gtest.h>

#include <string>

class IniSectionReaderTest : public ::testing::Test {
protected:
    test::TempIniFileStorage m_tempIniFiles;
};

#if (1)  // Успешное чтение

// Тест 1.1: Успешное чтение списка секций
TEST_F(IniSectionReaderTest, Parse_ValidFile_ReturnsSections) {
    const std::string ini = m_tempIniFiles.write(R"(
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
    const std::string ini = m_tempIniFiles.write(R"(
[]
)");
    utils::IniSectionReader reader(ini);
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.lastError().empty());
    EXPECT_TRUE(reader.sections().empty());
}

// Тест 3.2: Дубликат имени секции
TEST_F(IniSectionReaderTest, Parse_DuplicateSection_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
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
    const std::string ini = m_tempIniFiles.write(R"(
# only comments
# and empty lines
)");
    utils::IniSectionReader reader(ini);
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.lastError().empty());
    EXPECT_TRUE(reader.sections().empty());
}
#endif
