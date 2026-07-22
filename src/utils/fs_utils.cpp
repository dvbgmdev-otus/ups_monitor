/**
 * @file fs_utils.cpp
 * @ingroup utils
 * @brief Реализация утилит для работы с файловой системой.
 */
#include "fs_utils.h"

#include <limits.h>
#include <unistd.h>

#include <string>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <stdlib.h>
#endif

namespace {

/**
 * @brief Возвращает абсолютный путь к каталогу текущего исполняемого файла.
 *
 * На Linux используется /proc/self/exe.
 * На macOS используется _NSGetExecutablePath().
 *
 * @return Абсолютный путь к каталогу бинарника.
 * @note В случае ошибки возвращает ".".
 */
std::string getBinaryDir() {
#if defined(__linux__)
    // ----- Linux -----
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len == -1)
        // LCOV_EXCL_START
        return ".";
    // LCOV_EXCL_STOP

    buf[len] = '\0';
    std::string path(buf);
    return path.substr(0, path.find_last_of('/'));

#elif defined(__APPLE__)
    // ----- macOS -----
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);

    // Пытаемся писать сразу в локальный массив
    if (_NSGetExecutablePath(buf, &size) != 0) {
        // Буфер мал — выделяем вручную
        char* m = new char[size];

        if (_NSGetExecutablePath(m, &size) != 0) {
            delete[] m;
            return ".";
        }

        std::string path(m);
        delete[] m;

        // Разрешаем symlink
        char resolved[PATH_MAX];
        if (realpath(path.c_str(), resolved) != nullptr) path = resolved;

        return path.substr(0, path.find_last_of('/'));
    }

    // Если buf поместился
    std::string path(buf);

    // Разрешаем symlink
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved) != nullptr) path = resolved;

    return path.substr(0, path.find_last_of('/'));
#else
    return ".";
#endif
}

}  // anonymous namespace

namespace utils {

std::string resolvePath(const std::string& path) {
    if (!path.empty() && path[0] == '/') return path;
    return getBinaryDir() + "/" + path;
}

}  // namespace utils
