#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// ============================================================================
// 底层类 MmapIO：仅提供构造、析构、get、put
// ============================================================================
class MmapIO
{
private:
    int fd;
    size_t file_size;
    size_t window_pos;
    size_t window_size;
    const char* __restrict__ window_data;
    const char* __restrict__ window_ptr;
    const char* __restrict__ window_end;

    static constexpr size_t WINDOW_BYTES = 1ULL << 30;   // 1GB
    static constexpr size_t OUT_BYTES   = 1ULL << 24;   // 16MB

    char* __restrict__ out_buf;
    char* __restrict__ out_ptr;

    inline void flush()
    {
        if (__builtin_expect(out_ptr > out_buf, 0))
        {
            ::write(STDOUT_FILENO, out_buf, out_ptr - out_buf);
            out_ptr = out_buf;
        }
    }

    inline void slide_window()
    {
        if (__builtin_expect(window_data != nullptr, 1))
        {
            munmap(const_cast<char*>(window_data), window_size);
            window_data = nullptr;
        }

        if (__builtin_expect(window_pos >= file_size, 0))
        {
            window_ptr = window_end = nullptr;
            window_size = 0;
            return;
        }

        window_size = std::min(WINDOW_BYTES, file_size - window_pos);
        void* p = mmap(nullptr, window_size, PROT_READ,
                       MAP_PRIVATE | MAP_POPULATE, fd, window_pos);
        if (__builtin_expect(p == MAP_FAILED, 0))
        {
            window_data = window_ptr = window_end = nullptr;
            window_size = 0;
            file_size = window_pos;
            return;
        }

        window_data = static_cast<const char*>(p);
        window_ptr = window_data;
        window_end = window_data + window_size;
        window_pos += window_size;
    }

public:
    MmapIO()
        : fd(STDIN_FILENO), file_size(0), window_pos(0), window_size(0),
          window_data(nullptr), window_ptr(nullptr), window_end(nullptr),
          out_buf(new char[OUT_BYTES]), out_ptr(out_buf)
    {
        struct stat st;
        if (fstat(fd, &st) == 0 && st.st_size > 0)
        {
            file_size = st.st_size;
            slide_window();
        }
    }

    ~MmapIO()
    {
        flush();
        delete[] out_buf;
        if (window_data != nullptr)
            munmap(const_cast<char*>(window_data), window_size);
    }

    MmapIO(const MmapIO&) = delete;
    MmapIO& operator=(const MmapIO&) = delete;

    // ----- 唯一四个公开 API -----
    inline char get()
    {
        if (__builtin_expect(window_ptr < window_end, 1))
            return *window_ptr++;

        slide_window();

        if (__builtin_expect(window_ptr < window_end, 0))
            return *window_ptr++;

        return '\0';  // EOF
    }

    inline void put(char c)
    {
        if (__builtin_expect(out_ptr >= out_buf + OUT_BYTES, 0))
            flush();
        *out_ptr++ = c;
    }
};

// ============================================================================
// 抽象类 FastIO：继承 MmapIO，提供 >> 和 <<，跳过空白
// ============================================================================
class FastIO : public MmapIO
{
private:
    char cache_;
    bool has_cache_;

    inline char get_char()
    {
        if (__builtin_expect(has_cache_, 0))
        {
            has_cache_ = false;
            return cache_;
        }
        return MmapIO::get();
    }

    inline void unget_char(char c)
    {
        cache_ = c;
        has_cache_ = true;
    }

    inline void skip_whitespace_and_cache()
    {
        char c = get_char();
        while (__builtin_expect(c != '\0', 1) && c <= ' ')
            c = get_char();
        unget_char(c);
    }

    // ---------- 整数解析 ----------
    template <typename T>
    T parse_integer()
    {
        static_assert(std::is_integral<T>::value, "parse_integer requires integral type");

        skip_whitespace_and_cache();

        char c = get_char();
        int sign = 1;

        if constexpr (std::is_signed<T>::value)
        {
            if (c == '-')
            {
                sign = -1;
                c = get_char();
            }
            else if (c == '+')
            {
                c = get_char();
            }
        }
        else
        {
            if (c == '+')
                c = get_char();
        }

        using UT = std::make_unsigned<T>::type;
        __uint128_t value = 0;
        while (__builtin_expect(c >= '0' && c <= '9', 1) && c != '\0')
        {
            value = value * 10 + (c - '0');
            c = get_char();
        }
        if (__builtin_expect(c != '\0', 0))
            unget_char(c);

        if constexpr (std::is_signed<T>::value)
        {
            if (sign == -1)
                return static_cast<T>(-static_cast<__int128_t>(value));
            else
                return static_cast<T>(static_cast<__int128_t>(value));
        }
        else
        {
            return static_cast<T>(value);
        }
    }

    // ---------- 浮点数解析 ----------
    template <typename T>
    T parse_float()
    {
        static_assert(std::is_floating_point<T>::value, "parse_float requires floating type");

        skip_whitespace_and_cache();

        char c = get_char();
        int sign = 1;
        if (c == '-')
        {
            sign = -1;
            c = get_char();
        }
        else if (c == '+')
        {
            c = get_char();
        }

        long double value = 0.0L;
        while (__builtin_expect(c >= '0' && c <= '9', 1) && c != '\0')
        {
            value = value * 10.0L + (c - '0');
            c = get_char();
        }

        if (c == '.')
        {
            c = get_char();
            long double frac = 0.0L, factor = 0.1L;
            while (__builtin_expect(c >= '0' && c <= '9', 1) && c != '\0')
            {
                frac += (c - '0') * factor;
                factor *= 0.1L;
                c = get_char();
            }
            value += frac;
        }

        if (c == 'e' || c == 'E')
        {
            c = get_char();
            int exp_sign = 1;
            if (c == '-')
            {
                exp_sign = -1;
                c = get_char();
            }
            else if (c == '+')
            {
                c = get_char();
            }
            int exp_val = 0;
            while (__builtin_expect(c >= '0' && c <= '9', 1) && c != '\0')
            {
                exp_val = exp_val * 10 + (c - '0');
                c = get_char();
            }
            long double exp_pow = 1.0L;
            for (int i = 0; i < exp_val; ++i)
                exp_pow *= 10.0L;
            if (exp_sign == -1)
                exp_pow = 1.0L / exp_pow;
            value *= exp_pow;
        }

        if (__builtin_expect(c != '\0', 0))
            unget_char(c);

        return static_cast<T>(value * sign);
    }

    // ---------- 输出无符号整数 ----------
    template <typename U>
    void write_unsigned_impl(U value)
    {
        char buf[25];
        int n = 0;
        do
        {
            buf[n++] = static_cast<char>('0' + (value % 10));
            value /= 10;
        } while (value);
        while (n--)
            MmapIO::put(buf[n]);
    }

public:
    FastIO() : cache_('\0'), has_cache_(false) {}

    // ---------- 重载 >> ----------
    template <typename T>
    FastIO& operator>>(T& value)
    {
        using DecayT = std::decay_t<T>;

        // 优先处理字符类型（避免被当作整数）
        if constexpr (std::is_same<DecayT, char>::value ||
                      std::is_same<DecayT, unsigned char>::value ||
                      std::is_same<DecayT, signed char>::value)
        {
            skip_whitespace_and_cache();
            char c = get_char();
            value = static_cast<DecayT>(c);
        }
        else if constexpr (std::is_integral<DecayT>::value)
        {
            value = parse_integer<DecayT>();
        }
        else if constexpr (std::is_floating_point<DecayT>::value)
        {
            value = parse_float<DecayT>();
        }
        else if constexpr (std::is_same<DecayT, std::string>::value)
        {
            skip_whitespace_and_cache();
            std::string s;
            char c = get_char();
            while (__builtin_expect(c != '\0', 1) && c > ' ')
            {
                s.push_back(c);
                c = get_char();
            }
            if (__builtin_expect(c != '\0', 0))
                unget_char(c);
            value = std::move(s);
        }
        else
        {
            static_assert(sizeof(DecayT) == 0, "Unsupported type for operator>>");
        }
        return *this;
    }

    // ---------- 重载 << ----------
    template <typename T>
    FastIO& operator<<(const T& value)
    {
        using DecayT = std::decay_t<T>;

        // 优先处理字符类型（避免被当作整数输出ASCII码）
        if constexpr (std::is_same<DecayT, char>::value ||
                      std::is_same<DecayT, unsigned char>::value ||
                      std::is_same<DecayT, signed char>::value)
        {
            MmapIO::put(static_cast<char>(value));
        }
        else if constexpr (std::is_integral<DecayT>::value)
        {
            if constexpr (std::is_signed<DecayT>::value)
            {
                if (value < 0)
                {
                    MmapIO::put('-');
                    using UT = std::make_unsigned<DecayT>::type;
                    write_unsigned_impl(static_cast<UT>(-static_cast<UT>(value)));
                }
                else
                {
                    write_unsigned_impl(static_cast<std::make_unsigned_t<DecayT>>(value));
                }
            }
            else
            {
                write_unsigned_impl(value);
            }
        }
        else if constexpr (std::is_floating_point<DecayT>::value)
        {
            char buf[64];
            if constexpr (std::is_same<DecayT, float>::value)
                snprintf(buf, sizeof(buf), "%.9g", static_cast<double>(value));
            else if constexpr (std::is_same<DecayT, double>::value)
                snprintf(buf, sizeof(buf), "%.17g", value);
            else if constexpr (std::is_same<DecayT, long double>::value)
                snprintf(buf, sizeof(buf), "%.21Lg", value);
            else
                static_assert(sizeof(DecayT) == 0, "Unsupported floating type");
            for (int i = 0; buf[i]; ++i)
                MmapIO::put(buf[i]);
        }
        else if constexpr (std::is_same<DecayT, const char*>::value ||
                           std::is_same<DecayT, char*>::value)
        {
            for (const char* p = value; *p; ++p)
                MmapIO::put(*p);
        }
        else if constexpr (std::is_same<DecayT, std::string>::value)
        {
            for (char c : value)
                MmapIO::put(c);
        }
        else
        {
            static_assert(sizeof(DecayT) == 0, "Unsupported type for operator<<");
        }
        return *this;
    }

    // 针对 C 字符串字面量的重载（避免模板歧义）
    FastIO& operator<<(const char* s) { return this->operator<<<const char*>(s); }
    FastIO& operator<<(char* s) { return this->operator<<<char*>(s); }
} cio;
