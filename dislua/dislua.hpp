#pragma once
#include "const.hpp"
#include "buffer.hpp"
#include "interface.hpp"

#include "luajit2/parser.hpp"

/*!
 * \brief   Основное пространство имён библиотеки DisLua.
 * \details DisLua — C++ библиотека, задачей которой является анализ байт-кода скомпилированных Lua скриптов в обход их прямого запуска, в результате
 * чего разработчик получает в свое распоряжение интерфейс с данными, извлеченными во время анализа. На данный момент библиотека поддерживает работу
 * со скриптами, скомпилированными через LuaJIT v2.
 */
namespace dislua {
    /*!
     * Функция библиотеки, задача которой заключается в анализе буфера определенным парсером, который обособлен в шаблонном типе.
     * 
     * \code
     * dislua::buffer buf = { ... };
     * dislua::interface *info = nullptr;
     * 
     * try {
     *     info = dislua::read_current<dislua::lj2::parser>(buf);
     *     std::cout << "COMPILER_LUAJIT2\n";
     * } catch(const std::exception &e) {
     *     std::cerr << e.what() << '\n';
     *     delete info;
     *     info = nullptr;
     * };
     * \endcode
     *
     * \param[in] buf Буфер, с которого будет считываться данные определённым парсером.
     * 
     * \return Указатель на интерфейс, где будут находится данные после анализа.
     *
     * \tparam T Класс парсера.
     *
     * \warning Класс типа T должен быть дочерним класса dislua::interface.
     * \note    Если надо проанализировать всеми парсерами, которые доступны в библиотеки DisLua, то используйте функцию dislua::read.
     *
     * \exception std::out_of_range  Индекс буфера больше, чем размер контейнера.
     * \exception std::runtime_error Ошибка во время анализа.
     */
    template<typename T>
    static interface *read_current(buffer &buf) {
        static_assert(std::is_base_of_v<interface, T> && !std::is_same_v<interface, T>, "This type is not a dislua parser.");
        interface *result = new T(buf);
        result->read();
        return result;
    }

    /*!
     * Функция библиотеки, задача которой заключается в анализе буфера всеми парсерами, которые доступны в библиотеки DisLua.
     * 
     * \code
     * dislua::buffer buf = { ... };
     * dislua::interface *info = dislua::read(buf);
     * if (info) {
     *     // ...
     *     delete info;
     * }
     * \endcode
     *
     * \param[in] buf Буфер, с которого будет считываться данные парсерами.
     * \return Указатель на интерфейс, где будут находится данные после анализа.
     *
     * \note Если надо проанализировать конкретным парсером, то используйте функцию dislua::read_current.
     */
    static interface *read(buffer &buf) {
        interface *in = nullptr;
        std::string errors;

#define CHECK_COMPILER(T, ENUM) \
    try { \
        in = read_current<T>(buf); \
        if (in->version_compiler == ENUM) return in; \
    } catch (...) { delete in; in = nullptr; }

        CHECK_COMPILER(lj2::parser, COMPILER_LUAJIT2);
        
#undef CHECK_COMPILER

        return in;
    }
};