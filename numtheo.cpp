#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>
#include <cstdint>
#include <cstring>

namespace number_theory
{
    int p{998244353}, fac_[1000006]{1}, fact_, inv_[1000006]{0, 1}, invt_{1};

    void swap(auto &a, auto &b) noexcept
    {
        a ^= b;
        b ^= a;
        a ^= b;
    }

    inline int fac(const int n)
    {
        if (n <= fact_)
            return fac_[n];
        for (fact_++; fact_ < n; fact_++)
            fac_[fact_] = fac_[fact_ - 1] * fact_ % p;
        return fac_[n] = fac_[n - 1] * n % p;
    }

    inline int pow(int a, int b)
    {
        int r{1};
        for (; b; a = a * a % p, b >>= 1)
            if (b & 1)
                r = r * a % p;
        return r;
    }

    inline int gcd(int a, int b)
    {
        while (b)
            std::tie(a, b) = std::pair{b, a % b};
        return a;
    }

    // 优化后的 Stein（二进制）gcd
    inline int stein(int a, int b)
    {
        if (a == 0) return b;
        if (b == 0) return a;
        int shift = __builtin_ctz(a | b);
        a >>= __builtin_ctz(a);
        do
        {
            b >>= __builtin_ctz(b);
            if (a > b) std::swap(a, b);
            b = b - a;
        } while (b != 0);
        return a << shift;
    }

    // 基于 Stein（二进制）算法的扩展欧几里得（无除法/无辗转相除）
    // 返回 g = gcd(a,b)，并设置 x,y 使 ax + by = g
    // 使用 long long 中间量以减小溢出风险
    int exgcd(int a, int b, int &x, int &y)
    {
        if (a == 0)
        {
            x = 0; y = (b == 0 ? 0 : 1);
            return b;
        }
        if (b == 0)
        {
            x = 1; y = 0;
            return a;
        }

        long long a0 = a, b0 = b;
        int shift = __builtin_ctzll(a0 | b0);

        long long u = a0 >> __builtin_ctzll(a0);
        long long v = b0 >> __builtin_ctzll(b0);

        // coefficients: A,B for u; C,D for v
        long long A = 1, B = 0;
        long long C = 0, D = 1;

        while (u != v)
        {
            if ((u & 1) == 0)
            {
                u >>= 1;
                if ((A & 1) == 0 && (B & 1) == 0)
                {
                    A >>= 1; B >>= 1;
                }
                else
                {
                    A = (A + b0) >> 1;
                    B = (B - a0) >> 1;
                }
            }
            else if ((v & 1) == 0)
            {
                v >>= 1;
                if ((C & 1) == 0 && (D & 1) == 0)
                {
                    C >>= 1; D >>= 1;
                }
                else
                {
                    C = (C + b0) >> 1;
                    D = (D - a0) >> 1;
                }
            }
            else if (u >= v)
            {
                u = u - v;
                A = A - C;
                B = B - D;
            }
            else
            {
                v = v - u;
                C = C - A;
                D = D - B;
            }
        }

        long long g = u << shift; // gcd
        long long X = A, Y = B;
        // multiply coefficients by 2^shift
        X <<= shift;
        Y <<= shift;

        // Reduce X,Y to fit into int by taking modulo of b0/a0 if needed is NOT appropriate here.
        // We return them directly (casting). Caller should use types large enough if overflow is possible.
        x = (int)X;
        y = (int)Y;
        return (int)g;
    }

    // Lucas 非递归实现（基于模 p 的阶乘/逆元预处理）
    inline int C_mod(int m, int n)
    {
        if (n < 0 || n > m) return 0;
        return fac(m) * 1LL * inv_on(fac(n)) % p * inv_on(fac(m - n)) % p;
    }

    inline int lucas(long long m, long long n)
    {
        if (n < 0 || n > m) return 0;
        int res = 1;
        // 保证预处理至少到 p-1
        fac(p - 1);
        while (n > 0 || m > 0)
        {
            int mi = m % p;
            int ni = n % p;
            if (ni > mi) return 0;
            res = int(res * 1LL * C_mod(mi, ni) % p);
            m /= p;
            n /= p;
        }
        return res;
    }

    // 使用扩展（Stein）欧几里得求模逆（不使用辗转相除）
    inline int inv_on(const int n)
    {
        int x, y;
        int g = exgcd(n, p, x, y);
        if (g != 1) return -1; // inverse does not exist
        x %= p;
        if (x < 0) x += p;
        return x;
    }

    inline int inv_off(const int n)
    {
        if (n <= invt_)
            return inv_[n];
        for (invt_++; invt_ < n; invt_++)
            inv_[invt_] = (p - p / invt_) * inv_[p % invt_] % p;
        return inv_[n] = (p - p / n) * inv_[p % n] % p;
    }

    // ---------- 线性筛：分开 8 个函数，使用同一块共享内存（覆盖） ----------
    // 全局常量大小
    constexpr int MAXS = 1000006; // 1e6 + 6
    static long long mf_buffer[MAXS]; // 共享缓冲区，用户在两次函数调用之间 memcpy 出去

    std::vector<int> primes;
    std::vector<char> is_comp;
    std::vector<int> spf; // 最小质因子
    std::vector<int> cnt; // spf 的幂次数
    std::vector<long long> ppow; // p^cnt
    std::vector<long long> spSum; // 1 + p + ... + p^cnt
    int sieve_n = 0;

    // 预处理分解信息（primes, spf, cnt, ppow, spSum），只做一次到上界 n
    void sieve_prepare(int n)
    {
        if (n <= sieve_n) return;
        if (n >= MAXS) n = MAXS - 1;
        primes.clear();
        is_comp.assign(n + 1, 0);
        spf.assign(n + 1, 0);
        cnt.assign(n + 1, 0);
        ppow.assign(n + 1, 0);
        spSum.assign(n + 1, 0);

        spf[1] = 1;
        cnt[1] = 0;
        ppow[1] = 1;
        spSum[1] = 1;

        for (int i = 2; i <= n; ++i)
        {
            if (!is_comp[i])
            {
                primes.push_back(i);
                spf[i] = i;
                cnt[i] = 1;
                ppow[i] = i;
                spSum[i] = 1 + (long long)i;
            }
            for (int pj = 0; pj < (int)primes.size(); ++pj)
            {
                int pr = primes[pj];
                long long t = 1LL * i * pr;
                if (t > n) break;
                is_comp[t] = 1;
                spf[t] = pr;
                if (i % pr == 0)
                {
                    cnt[t] = cnt[i] + 1;
                    ppow[t] = ppow[i] * pr;
                    spSum[t] = spSum[i] + ppow[t];
                    break;
                }
                else
                {
                    cnt[t] = 1;
                    ppow[t] = pr;
                    spSum[t] = 1 + pr;
                }
            }
        }
        sieve_n = n;
    }

    // 每个函数覆盖同一块 mf_buffer[0..n]
    void sieve_unit(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        for (int i = 1; i <= n; ++i) mf_buffer[i] = 1;
    }

    void sieve_id(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        for (int i = 1; i <= n; ++i) mf_buffer[i] = i;
    }

    void sieve_mu(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        mf_buffer[1] = 1;
        for (int i = 2; i <= n; ++i)
        {
            int p = spf[i];
            int m = i / p;
            if (m % p == 0) mf_buffer[i] = 0;
            else mf_buffer[i] = -mf_buffer[m];
        }
    }

    void sieve_phi(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        mf_buffer[1] = 1;
        for (int i = 2; i <= n; ++i)
        {
            int p = spf[i];
            int m = i / p;
            if (m % p == 0) mf_buffer[i] = mf_buffer[m] * p;
            else mf_buffer[i] = mf_buffer[m] * (p - 1);
        }
    }

    void sieve_sigma(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        mf_buffer[1] = 1;
        for (int i = 2; i <= n; ++i)
        {
            int p = spf[i];
            int m = i / p;
            if (m % p == 0)
            {
                // combine with previous power
                long long spSum_i = spSum[i];
                long long spSum_m = spSum[m];
                if (spSum_m != 0)
                    mf_buffer[i] = mf_buffer[m] / spSum_m * spSum_i;
                else
                    mf_buffer[i] = mf_buffer[m];
            }
            else
            {
                mf_buffer[i] = mf_buffer[m] * spSum[i];
            }
        }
    }

    void sieve_tau(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        mf_buffer[1] = 1;
        for (int i = 2; i <= n; ++i)
        {
            int p = spf[i];
            int m = i / p;
            if (m % p == 0)
            {
                // exponent increases from cnt[m] to cnt[m]+1
                mf_buffer[i] = mf_buffer[m] / (cnt[m] + 1) * (cnt[m] + 2);
            }
            else
            {
                mf_buffer[i] = mf_buffer[m] * 2;
            }
        }
    }

    void sieve_rad(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        mf_buffer[1] = 1;
        for (int i = 2; i <= n; ++i)
        {
            int p = spf[i];
            int m = i / p;
            if (m % p == 0) mf_buffer[i] = mf_buffer[m];
            else mf_buffer[i] = mf_buffer[m] * p;
        }
    }

    void sieve_lambda(int n)
    {
        sieve_prepare(n);
        if (n >= MAXS) n = MAXS - 1;
        mf_buffer[0] = 0;
        mf_buffer[1] = 1;
        for (int i = 2; i <= n; ++i)
        {
            int p = spf[i];
            int m = i / p;
            mf_buffer[i] = -mf_buffer[m];
        }
    }

} // namespace number_theory
