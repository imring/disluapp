#pragma once
#include <vector>
#include <functional>
#include <string>
#include <type_traits>
#include <stdexcept>

#include "const.hpp"

namespace dislua {
    /*!
     * \brief   Данный класс является основным для анализа/записи байт-кода Lua.
     * \details Важно понимать, что получение из буфера данных напрямую через элемент контейнера нельзя, для этого необходимо использовать метод dislua::buffer::read.
     */
    class buffer {
    public:
        buffer() = default;

        /*!
         * Конструктор буферного класса с помощью двух итераторов.
         * 
         * \param[in] first, last Диапазон элементов, которые будут внесены в буфер.
         * 
         * \tparam It Тип итератора.
         */
        template<typename It>
        buffer(It first, It last) {
            write(first, last);
            reset_index();
        }

        /*!
         * Конструктор буферного класса с помощью cписка инициализации.
         * 
         * \param[in] arr Список инициализации с которого данные будут скопированны в буфер.
         */
        buffer(std::initializer_list<byte> arr): buffer(arr.begin(), arr.end()) {}

        /*!
         * Читает с буфера значение указанного типа.
         * 
         * \code
         * dislua::buffer buf{ 0xA, 0xBC };
         * std::cout << std::hex << std::uppercase
         *           << int(buf.read(false)) << ' '
         *           << int(buf.read<word>()) << '\n';
         * 
         * // output: A BC0A
         * \endcode
         * 
         * \param[in] next Переход к следующему индексу для чтения.
         * \return Значение указанного типа.
         * 
         * \tparam T Тип значения. Если не будет указан шаблон вручную, то будет присвоен шаблон типа dislua::byte.
         * 
         * \warning Тип T не должен быть указателем.
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        template<typename T = byte>
        T read(bool next = true) {
            static_assert(!std::is_pointer_v<T>, "This method doesn't support pointer");
            T val;
            read(&val, sizeof(T), next);
            return val;
        }

        /*!
         * Читает с буфера значения и записывает в массив указанной длины.
         * 
         * \code
         * dislua::buffer buf{ 0x10, 0x80, 0xD, 0x2 };
         * dislua::byte result[3] = { 0 };
         * buf.read(result, 1, false);
         * buf.read(&result[1], 2);
         * 
         * std::cout << std::hex << std::uppercase;
         * for (dislua::byte val: result)
         *     std::cout << int(val) << ' ';
         * 
         * // output: 10 10 80
         * \endcode
         * 
         * \param[out] obj  Массив, в которой запишутся данные с буфера.
         * \param[in]  len  Длина массива.
         * \param[in]  next Переход к следующему индексу для чтения.
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        void read(void *obj, size_t len, bool next = true) {
            index_better_size_throw(len);
            byte *data = container.data();
            memcpy(obj, data + index, len);
            if (next) ignore(len);
        }

        /*!
         * Читает с буфера значения и записывает в контейнер с помощью двух итераторов.
         * 
         * \code
         * dislua::buffer buf{ 0x10, 0x80, 0xD, 0x2 };
         * dislua::byte result[3] = { 0 };
         * buf.read(std::begin(result), std::begin(result) + 1, false);
         * buf.read(std::begin(result) + 1, std::end(result));
         * 
         * std::cout << std::hex << std::uppercase;
         * for (dislua::byte val: result)
         *     std::cout << int(val) << ' ';
         * 
         * // output: 10 10 80
         * \endcode
         * 
         * \param[out] first, last Диапазон элементов, в которые будут записаны значения.
         * \param[in]  next        Переход к следующему индексу для чтения.
         * 
         * \tparam It Тип итератора.
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        template<typename It>
        void read(It first, It last, bool next = true) {
            size_t len = last - first;
            index_better_size_throw(len);
            decltype(container.begin()) start = container.begin() + index;
            std::copy(start, start + len, first);
            if (next) ignore(len);
        }

        /*!
         * Читает с буфера значения и записывает в контейнер до значения 0.
         * 
         * \code
         * dislua::buffer buf{ 0x5, 0xF, 0x0, 0x38 };
         * std::vector<dislua::buffer> result;
         * buf.read_to_zero(result);
         * 
         * std::cout << std::hex << std::uppercase;
         * for (dislua::byte val: result)
         *     std::cout << int(val) << ' ';
         * 
         * // output: 5 F
         * \endcode
         * 
         * \param[out] container Контейнер, где будет записаны данные.
         * \param[in]  next      Переход к следующему индексу для чтения.
         * 
         * \tparam T Тип контейнера (например std::string, std::vector<byte>).
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        template<typename T>
        void read_to_zero(T &container, bool next = true) {
            size_t first = get_index();
            byte val = 0;
            while (true) {
                val = read();
                if (!val) break;
                container.push_back(val);
            }
            if(!next) index = first;
        }

        /*!
         * Читает с буфера значение, кодированного форматом [ULEB128](https://en.wikipedia.org/wiki/LEB128) (Unsigned Little Endian Base 128).
         * 
         * \code
         * dislua::buffer buf{ 0x90, 0x3 };
         * std::cout << buf.read_uleb128() << '\n';
         * 
         * // output: 400
         * \endcode
         * 
         * \param[in] next Переход к следующему индексу для чтения.
         * 
         * \return Декодированное значение формы ULEB128.
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        uleb128 read_uleb128(bool next = true) {
            size_t first = get_index();
            byte vb = read();
            uleb128 val = uleb128(vb);

            if (val >= 0x80) {
                int sh = 0;
                val &= 0x7f;
                do {
                    vb = read();
                    val |= ((vb & 0x7f) << (sh += 7));
                } while (vb >= 0x80);
            }

            if(!next) index = first;
            return val;
        }

        /*!
         * Читает с буфера значения формата ULEB128 и записывает в массив указанной длины.
         * 
         * \code
         * dislua::buffer buf{ 0x90, 0x3, 0xA5, 0x95, 0x3 };
         * dislua::uleb128 result[2];
         * buf.read_uleb128(result, 2);
         *
         * for (dislua::uleb128 val: result)
         *     std::cout << val << ' ';
         * 
         * // output: 400 51877
         * \endcode
         * 
         * \param[out] obj  Массив, в которой запишутся данные с буфера.
         * \param[in]  len  Длина массива.
         * \param[in]  next Переход к следующему индексу для чтения.
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        void read_uleb128(uleb128 *obj, size_t len, bool next = true) {
            size_t first = get_index();
            while(len--) *obj++ = read_uleb128();
            if(!next) index = first;
        }

        /*!
         * Читает с буфера верхние 32 бита 33-битного значения формата ULEB128.
         * 
         * \code
         * dislua::buffer buf{ 0xA7, 0x54 };
         * std::cout << buf.read_uleb128_33() << '\n';
         * 
         * // output: 5395
         * \endcode
         * 
         * \param[in] next Переход к следующему индексу для чтения.
         * 
         * \return Декодированное значение формы ULEB128.
         * 
         * \exception std::out_of_range Индекс буфера больше, чем размер контейнера.
         */
        uleb128 read_uleb128_33(bool next = true) {
            size_t first = get_index();
            byte vb = read();
            uleb128 val = uleb128(vb >> 1);

            if (val >= 0x40) {
                int sh = -1;
                val &= 0x3f;
                do {
                    vb = read();
                    val |= ((vb & 0x7f) << (sh += 7));
                } while (vb >= 0x80);
            }

            if(!next) index = first;
            return val;
        }

        /*!
         * Записывает в буфер значение любого типа.
         * 
         * \code
         * dislua::buffer buf;
         * buf.write<dislua::dword>(1);
         * \endcode
         * 
         * \param[in] val Значение, которое будет записано в буфер.
         * 
         * \tparam T Тип значения.
         * 
         * \warning Тип T не должен быть указателем.
         */
        template<typename T>
        void write(T val) {
            static_assert(!std::is_pointer_v<T>, "This method doesn't support pointer");
            write(&val, sizeof(T));
        }

        /*!
         * Записывает в буфер значения с массива указанной длины.
         * 
         * \code
         * dislua::buffer buf;
         * dislua::byte arr[3] = { 0x5, 0xA0, 0xFF };
         * buf.write(arr, 3);
         * \endcode
         * 
         * \param[in] arr Массив, с которого будут записаны значения в буфер.
         * \param[in] len Длина массива.
         */
        void write(void *arr, size_t len) {
            if (index_better_size(len))
                container.resize(container.size() + len);
            byte *data = container.data();
            memcpy(data + index, arr, len);
            ignore(len);
        }

        /*!
         * Записывает в буфер значения с контейнера с помощью двух итераторов.
         * 
         * \code
         * dislua::buffer buf;
         * std::vector<dislua::byte> cont = { 0x5, 0xA0, 0xFF };
         * buf.write(cont.begin(), cont.end());
         * \endcode
         * 
         * \param[in] first, last Диапазон элементов, с которого будут записаны значения в буфер.
         * 
         * \tparam T Тип итератора.
         */
        template<typename It>
        void write(It first, It last) {
            using T = typename std::iterator_traits<It>::value_type;
            static_assert(sizeof(T) == sizeof(byte), "Size of type not equals size of byte type.");
            size_t len = last - first;
            if (index_better_size(len))
                container.resize(container.size() + len);
            std::copy(first, last, container.begin() + index);
            ignore(len);
        }

        /*!
         * Записывает в буфер значения с другого буфера.
         * 
         * \code
         * dislua::buffer buf = {1,2,3};
         * dislua::buffer buf1;
         * buf1.write(buf);
         * \endcode
         * 
         * \param[in] buf Буфер, с которого будут записаны значения в основной буфер.
         */
        void write(buffer &buf) {
            write(buf.container.begin(), buf.container.end());
        }

        /*!
         * Записывает в буфер значение форматом ULEB128.
         * 
         * \code
         * dislua::buffer buf;
         * buf.write_uleb128(400);
         * \endcode
         * 
         * \param[in] val Значение, которое будет записано форматом ULEB128 в буфер.
         */
        void write_uleb128(uleb128 val) {
            for (; val >= 0x80; val >>= 7)
                write(byte((val & 0x7f) | 0x80));
            //std::cout << val << '\n';
            write(byte(val));
        }

        /*!
         * Записывает в буфер значения с массива форматом ULEB128.
         * 
         * \code
         * dislua::buffer buf;
         * dislua::buffer vals[2] = { 400, 51877 };
         * buf.write_uleb128(400);
         * \endcode
         * 
         * \param[in] obj Массив, с которого будут записаны значения в буфер форматом ULEB128.
         * \param[in] len Длина массива.
         */
        void write_uleb128(uleb128 *obj, size_t len) {
            while(len--)
                write_uleb128(*obj++);
        }

        /*!
         * Записывает в буфер значение форматом ULEB128 в 33 битах.
         * 
         * \code
         * dislua::buffer buf;
         * buf.write_uleb128_33(400);
         * \endcode
         * 
         * \param[in] val   Значение, которое будет записано форматом ULEB128 в буфер.
         * \param[in] isnum Является ли значение, формата ULEB128, целым числом?
         */
        void write_uleb128_33(uleb128 val, bool isnum = false) {
            size_t index = get_index();
            write_uleb128(1 + 2 * val);
            byte &v = container[index];
            if (isnum) v |= 1;
            else v &= 0xfe;

            if (val >= 0x80000000)
                container[get_index() - 1] |= 0x10;
        }

        /// Очистить буфер.
        void clear() {
            container.clear();
            reset_index();
        }
        
        /// Обнулить индекс буфера.
        void reset_index() { index = 0; }

        /*!
         * Пропускать элементы буфера.
         * 
         * \param[in] offset Кол-во пропущенных байтов.
         */
        void ignore(size_t offset) { index += offset; }

        /// Получить индекс буфера.
        const size_t get_index() const { return index; }

        /*!
         * Является ли индекс с размером (если он имеется) больше, чем размер контейнера?
         * 
         * \param[in] size Дополнительный размер (по-умолчанию является 0).
         */
        bool index_better_size(size_t size = 0) const { return index + size - 1 >= container.size(); }

        /// Получает копию контейнера буфера.
        std::vector<byte> get_copy_container() const { return container; }
    private:
        inline void index_better_size_throw(size_t size = 0) const {
            if (index_better_size(size))
                throw std::out_of_range("Index (" + std::to_string(index + size) + ") better than container size (" + std::to_string(container.size()) + ").");
        }

        std::vector<byte> container;
        size_t index = 0;
    };
};