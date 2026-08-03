/**
 * @file ini_section_reader.h
 * @ingroup utils
 * @brief Утилита для чтения списка секций INI-файла.
 *
 * Содержит класс, предназначенный для извлечения имён секций
 * из INI-файла без разбора их содержимого.
 */
#ifndef INI_SECTION_READER_H
#define INI_SECTION_READER_H

#include <string>
#include <vector>

namespace utils {

/**
 * @class IniSectionReader
 * @brief Считывает список секций INI-файла.
 *
 * Класс выполняет однопроходное чтение INI-файла и извлекает
 * имена секций в порядке их появления.
 *
 * Используется для предварительного анализа конфигурации
 * и проверки структуры файла.
 */
class IniSectionReader {
public:
    /**
     * @brief Создаёт объект для чтения секций указанного INI-файла.
     *
     * @param path Путь к INI-файлу.
     */
    explicit IniSectionReader(const std::string& path);

    /**
     * @brief Указывает, успешно ли прошло чтение файла.
     *
     * @return true, если файл был успешно прочитан.
     */
    bool ok() const;

    /**
     * @brief Возвращает список найденных секций INI-файла.
     *
     * Секции возвращаются в порядке их появления в файле.
     *
     * @return Список имён секций.
     */
    const std::vector<std::string>& sections() const;

    /**
     * @brief Возвращает описание последней ошибки.
     *
     * @return Строка с описанием ошибки или пустая строка,
     *         если ошибок не возникло.
     */
    const std::string& lastError() const;

private:
    bool m_ok = false;                       ///< Флаг успешности операции.
    std::string m_path;                      ///< Путь к INI-файлу.
    std::vector<std::string> m_sections;     ///< Список секций.
    std::string m_lastError;                 ///< Текст ошибки.

    /**
     * @brief Выполняет разбор файла и заполнение списка секций.
     */
    void scan();
};

}  // namespace utils

#endif  // INI_SECTION_READER_H
