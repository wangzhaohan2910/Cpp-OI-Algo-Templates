#include <bitset>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

template <size_t len>
class BigInt : public std::bitset<len>
{
    using Base = std::bitset<len>;

    // ---------- 私有辅助 ----------
    bool is_zero() const
    {
        for (size_t i = 0; i < len; ++i)
            if (this->test(i))
                return false;
        return true;
    }

    bool is_negative() const
    {
        return this->test(len - 1);
    }

    BigInt abs_value() const
    {
        if (!is_negative())
            return *this;
        BigInt res = *this;
        res.flip();
        bool carry = true;
        for (size_t i = 0; i < len; ++i) {
            bool b = res.test(i);
            bool sum = b ^ carry;
            carry = b & carry;
            res.set(i, sum);
        }
        return res;
    }

public:
    // ---------- 默认构造 ----------
    BigInt() : Base() {}

    // ---------- 从标准整数 ----------
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    BigInt(T value) : Base()
    {
        using UT = std::make_unsigned_t<T>;
        UT u = static_cast<UT>(value);
        for (size_t i = 0; i < len && i < sizeof(T) * 8; ++i) {
            if (u & (UT(1) << i))
                this->set(i);
        }
        if constexpr (std::is_signed_v<T>) {
            if (len > sizeof(T) * 8 && value < 0) {
                for (size_t i = sizeof(T) * 8; i < len; ++i)
                    this->set(i);
            }
        }
    }

    // ---------- 从十进制字符串 ----------
    explicit BigInt(const std::string& s) : Base()
    {
        bool neg = false;
        size_t pos = 0;
        if (s[0] == '-') { neg = true; ++pos; }
        BigInt tmp(0);
        for (size_t i = pos; i < s.size(); ++i) {
            if (s[i] < '0' || s[i] > '9') continue;
            tmp = tmp * 10;
            tmp += BigInt(s[i] - '0');
        }
        *this = neg ? -tmp : tmp;
    }

    // ---------- 长度转换构造函数（条件隐式/显式） ----------
    // 目标 >= 源：隐式（等长复制，更长符号扩展）
    // 目标 < 源：显式（高位截断）
    template <size_t other_len>
    explicit(len < other_len)
    BigInt(const BigInt<other_len>& other) : Base()
    {
        constexpr size_t copy_bits = (len < other_len) ? len : other_len;
        for (size_t i = 0; i < copy_bits; ++i) {
            if (other.test(i))
                this->set(i);
        }
        if constexpr (len > other_len) {
            // 符号扩展
            if (other.test(other_len - 1)) {
                for (size_t i = other_len; i < len; ++i)
                    this->set(i);
            }
        }
    }

    // ---------- 算术运算 ----------
    BigInt operator+() const { return *this; }
    BigInt operator-() const
    {
        BigInt res = *this;
        res.flip();
        bool carry = true;
        for (size_t i = 0; i < len; ++i) {
            bool b = res.test(i);
            bool sum = b ^ carry;
            carry = b & carry;
            res.set(i, sum);
        }
        return res;
    }

    BigInt& operator+=(const BigInt& other)
    {
        bool carry = false;
        for (size_t i = 0; i < len; ++i) {
            bool a = this->test(i);
            bool b = other.test(i);
            bool sum = a ^ b ^ carry;
            carry = (a & b) | (a & carry) | (b & carry);
            this->set(i, sum);
        }
        return *this;
    }

    BigInt operator+(const BigInt& other) const
    {
        BigInt res = *this;
        res += other;
        return res;
    }

    BigInt& operator-=(const BigInt& other)
    {
        *this = *this - other;
        return *this;
    }

    BigInt operator-(const BigInt& other) const
    {
        return *this + (-other);
    }

    BigInt operator*(const BigInt& other) const
    {
        if (this->is_zero() || other.is_zero())
            return BigInt(0);
        BigInt a = this->abs_value();
        BigInt b = other.abs_value();
        BigInt res;
        for (size_t i = 0; i < len; ++i) {
            if (b.test(i)) {
                BigInt shifted = a << i;
                res += shifted;
            }
        }
        bool neg = this->is_negative() ^ other.is_negative();
        return neg ? -res : res;
    }

    std::pair<BigInt, BigInt> divmod(const BigInt& divisor) const
    {
        if (divisor.is_zero())
            throw std::domain_error("Division by zero");

        BigInt a = this->abs_value();
        BigInt b = divisor.abs_value();
        if (a < b)
            return { BigInt(0), *this };

        BigInt quotient;
        BigInt remainder;
        for (size_t i = len; i-- > 0; ) {
            remainder = remainder << 1;
            if (a.test(i))
                remainder.set(0);
            if (remainder >= b) {
                remainder = remainder - b;
                quotient.set(i);
            }
        }

        bool neg_q = this->is_negative() ^ divisor.is_negative();
        bool neg_r = this->is_negative();
        return { neg_q ? -quotient : quotient,
                 neg_r ? -remainder : remainder };
    }

    BigInt operator/(const BigInt& other) const
    {
        return divmod(other).first;
    }

    BigInt operator%(const BigInt& other) const
    {
        return divmod(other).second;
    }

    // ---------- 移位 ----------
    BigInt operator<<(size_t shift) const
    {
        BigInt res;
        if (shift >= len) return res;
        for (size_t i = 0; i < len - shift; ++i) {
            if (this->test(i))
                res.set(i + shift);
        }
        return res;
    }

    BigInt operator>>(size_t shift) const
    {
        BigInt res;
        if (shift >= len) {
            if (this->is_negative()) res.set(); // 算术右移
            return res;
        }
        bool sign = this->is_negative();
        for (size_t i = 0; i < len - shift; ++i) {
            if (this->test(i + shift))
                res.set(i);
        }
        if (sign) {
            for (size_t i = len - shift; i < len; ++i)
                res.set(i);
        }
        return res;
    }

    // ---------- 比较 ----------
    bool operator==(const BigInt& other) const
    {
        return static_cast<const Base&>(*this) == static_cast<const Base&>(other);
    }
    bool operator!=(const BigInt& other) const { return !(*this == other); }

    bool operator<(const BigInt& other) const
    {
        bool neg1 = this->is_negative();
        bool neg2 = other.is_negative();
        if (neg1 != neg2) return neg1;
        for (size_t i = len; i-- > 0; ) {
            bool a = this->test(i);
            bool b = other.test(i);
            if (a != b) return !a;
        }
        return false;
    }
    bool operator>(const BigInt& other) const { return other < *this; }
    bool operator<=(const BigInt& other) const { return !(*this > other); }
    bool operator>=(const BigInt& other) const { return !(*this < other); }

    // ---------- 位运算 ----------
    BigInt operator&(const BigInt& other) const
    {
        BigInt res;
        for (size_t i = 0; i < len; ++i)
            res.set(i, this->test(i) & other.test(i));
        return res;
    }
    BigInt operator|(const BigInt& other) const
    {
        BigInt res;
        for (size_t i = 0; i < len; ++i)
            res.set(i, this->test(i) | other.test(i));
        return res;
    }
    BigInt operator^(const BigInt& other) const
    {
        BigInt res;
        for (size_t i = 0; i < len; ++i)
            res.set(i, this->test(i) ^ other.test(i));
        return res;
    }
    BigInt operator~() const
    {
        BigInt res = *this;
        res.flip();
        return res;
    }

    // ---------- 快速幂取模 ----------
    template <size_t mod_len>
    BigInt<mod_len> pow_mod(unsigned long long exp, const BigInt<mod_len>& mod) const
    {
        BigInt<mod_len> base = static_cast<BigInt<mod_len>>(this->abs_value()) % mod;
        BigInt<mod_len> result(1);
        while (exp) {
            if (exp & 1)
                result = (result * base) % mod;
            exp >>= 1;
            base = (base * base) % mod;
        }
        if (this->is_negative() && (exp & 1)) {
            result = mod - result;
        }
        return result;
    }

    // ---------- 辅助工具 ----------
    uint64_t to_ullong() const
    {
        uint64_t res = 0;
        for (size_t i = 0; i < len && i < 64; ++i) {
            if (this->test(i))
                res |= (1ULL << i);
        }
        return res;
    }

    // ---------- 十进制 I/O ----------
    friend std::ostream& operator<<(std::ostream& os, const BigInt& num)
    {
        if (num.is_zero()) {
            os << '0';
            return os;
        }
        if (num.is_negative())
            os << '-';
        BigInt pos = num.abs_value();
        std::string digits;
        BigInt ten(10);
        while (!pos.is_zero()) {
            auto div = pos.divmod(ten);
            uint64_t rem = div.second.to_ullong();  // rem < 10
            digits.push_back('0' + rem);
            pos = div.first;
        }
        std::reverse(digits.begin(), digits.end());
        os << digits;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, BigInt& num)
    {
        std::string s;
        is >> s;
        num = BigInt(s);
        return is;
    }
};
