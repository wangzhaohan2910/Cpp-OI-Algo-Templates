#include <vector>
#include <algorithm>
#include <utility>
#include <cstdint>

namespace number_theory
{
    int p{998244353}, fac_[1000006]{1}, fact_, inv_[1000006]{0, 1}, invt_{1};

    void swap(auto &a, auto &b) noexcept
    {
        b ^= a;
        a = b ^ a;
        b ^= a;
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
            tie(a, b) = pair{b, a % b};
        return a;
    }

    inline int stein(int a, int b)
    {
        int k{};
        while (a != b)
            if (!(a & 1) && !(b & 1))
                a >>= 1, b >>= 1, k++;
            else if (!(a & 1) && b & 1)
                a >>= 1;
            else if (a & 1 && !(b & 1))
                b >>= 1;
            else
                tie(a, b) = pair{max(a, b) - min(a, b), min(a, b)};
        return a << k;
    }

    // 经典扩展欧几里得
    int exgcd(const int a, const int b, int &x, int &y)
    {
        return b ? pair{exgcd(b, a % b, y, x), y -= a / b * x}.first : (x = 1, y = 0, a);
    }

    // 基于 Stein（二进制）算法的扩展欧几里得（支持 64 位）
    // 返回 gcd(a,b)，并给出 x,y 使得 ax+by = gcd(a,b)
    long long exgcd_stein(long long a, long long b, long long &x, long long &y)
    {
        if (a == 0)
        {
            x = 0;
            y = 1;
            return b;
        }
        if (b == 0)
        {
            x = 1;
            y = 0;
            return a;
        }
        // store originals
        const long long a0 = a, b0 = b;
        int shift = __builtin_ctzll(a | b); // common factors of 2
        // remove factors of 2 from a and b
        int za = __builtin_ctzll(a);
        a >>= za;
        int zb = __builtin_ctzll(b);
        b >>= zb;

        // initialize coefficient pairs:
        // u = a = A*a0 + B*b0, v = b = C*a0 + D*b0
        long long A = 1, B = 0; // corresponds to a
        long long C = 0, D = 1; // corresponds to b

        while (true)
        {
            if (a == b)
            {
                break;
            }

            if (a > b)
            {
                a -= b;
                A -= C;
                B -= D;
                // remove factors of 2 from a
                int tz = __builtin_ctzll(a);
                a >>= tz;
                while (tz-- > 0)
                {
                    // divide A and B by 2, but keep them integer coefficients by adjusting with a0/b0
                    if ((A & 1) == 0 && (B & 1) == 0)
                    {
                        A >>= 1;
                        B >>= 1;
                    }
                    else
                    {
                        A = (A + b0) >> 1;
                        B = (B - a0) >> 1;
                    }
                }
            }
            else
            {
                b -= a;
                C -= A;
                D -= B;
                int tz = __builtin_ctzll(b);
                b >>= tz;
                while (tz-- > 0)
                {
                    if ((C & 1) == 0 && (D & 1) == 0)
                    {
                        C >>= 1;
                        D >>= 1;
                    }
                    else
                    {
                        C = (C + b0) >> 1;
                        D = (D - a0) >> 1;
                    }
                }
            }
        }

        long long g = a << shift; // gcd
        x = A;
        y = B;
        // currently A*a0 + B*b0 = a, but gcd = a << shift, so multiply coefficients by 1<<shift
        // We have a = g >> shift, so to get coefficients for g we multiply by that factor
        for (int i = 0; i < shift; ++i)
        {
            // multiply coefficients by 2: x *= 2; y *= 2;
            x <<= 1;
            y <<= 1;
        }
        // Now x*a0 + y*b0 = g
        return g;
    }

    inline int eulerphi(int n)
    {
        int ans{n};
        for (int i{2}; i * i <= n; i++)
            if (!(n % i))
            {
                ans = ans / i * (i - 1);
                while (!(n % i))
                    n /= i;
            }
        if (n > 1)
            ans = ans / n * (n - 1);
        return ans;
    }

    inline int inv_on(const int n)
    {
        return pow(n, p - 2);
    }

    inline int inv_off(const int n)
    {
        if (n <= invt_)
            return inv_[n];
        for (invt_++; invt_ < n; invt_++)
            inv_[invt_] = (p - p / invt_) * inv_[p % invt_] % p;
        return inv_[n] = (p - p / n) * inv_[p % n] % p;
    }

    inline bool isprime(const int n)
    {
        for (int i{2}; i * i <= n; i++)
            if (!(n % i))
                return false;
        return true;
    }

    inline int C(const int m, const int n)
    {
        return fac(m) * inv_on(fac(n)) % p * inv_on(fac(m - n)) % p;
    }

    inline int Austin(const int m, const int n)
    {
        if (m < p && n < p)
            return C(m, n);
        if (n % p > m % p)
            return 0;
        return C(m / p, n / p) * C(m % p, n % p) % p;
    }

    // ---------- 线性筛（计算常见积性函数） ----------
    // 使用后会填充下列容器（索引从 0..n）：
    // primes: 素数列表
    // is_comp: 标记合数
    // mu: Möbius 函数（int）
    // phi: 欧拉函数（int）
    // spf: 最小质因子
    // cnt: spf 的幂指数（p^cnt | i）
    // ppow: spf 的幂值 p^cnt（long long）
    // spSum: 1 + p + p^2 + ... + p^cnt（用于计算 sigma）
    // sigma: 约数和（long long）
    // d: 约数个数（int）

    std::vector<int> primes;
    std::vector<char> is_comp;
    std::vector<int> mu;
    std::vector<int> phi;
    std::vector<int> spf;
    std::vector<int> cnt;
    std::vector<long long> ppow;
    std::vector<long long> spSum;
    std::vector<long long> sigma;
    std::vector<int> d;
    int sieve_n = 0;

    void linear_sieve(int n)
    {
        if (n <= sieve_n)
            return;
        primes.clear();
        is_comp.assign(n + 1, 0);
        mu.assign(n + 1, 0);
        phi.assign(n + 1, 0);
        spf.assign(n + 1, 0);
        cnt.assign(n + 1, 0);
        ppow.assign(n + 1, 0);
        spSum.assign(n + 1, 0);
        sigma.assign(n + 1, 0);
        d.assign(n + 1, 0);

        mu[1] = 1;
        phi[1] = 1;
        spf[1] = 1;
        cnt[1] = 0;
        ppow[1] = 1;
        spSum[1] = 1;
        sigma[1] = 1;
        d[1] = 1;

        for (int i = 2; i <= n; ++i)
        {
            if (!is_comp[i])
            {
                primes.push_back(i);
                mu[i] = -1;
                phi[i] = i - 1;
                spf[i] = i;
                cnt[i] = 1;
                ppow[i] = i;
                spSum[i] = 1 + (long long)i;
                sigma[i] = spSum[i];
                d[i] = 2;
            }
            for (int pj = 0; pj < (int)primes.size(); ++pj)
            {
                int p = primes[pj];
                long long t = 1LL * i * p;
                if (t > n)
                    break;
                is_comp[t] = 1;
                spf[t] = p;
                if (i % p == 0)
                {
                    // p is the same as spf[i]
                    cnt[t] = cnt[i] + 1;
                    ppow[t] = ppow[i] * p;
                    spSum[t] = spSum[i] + ppow[t];
                    // sigma[t] = sigma[m] * spSum[t], and sigma[i] = sigma[m] * spSum[i]
                    // so sigma[t] = sigma[i] / spSum[i] * spSum[t]
                    sigma[t] = sigma[i] / spSum[i] * spSum[t];
                    d[t] = d[i] / (cnt[i] + 1) * (cnt[t] + 1);
                    phi[t] = phi[i] * p;
                    mu[t] = 0;
                    break;
                }
                else
                {
                    // new prime factor
                    cnt[t] = 1;
                    ppow[t] = p;
                    spSum[t] = 1 + p;
                    sigma[t] = sigma[i] * spSum[t];
                    d[t] = d[i] * 2;
                    phi[t] = phi[i] * (p - 1);
                    mu[t] = -mu[i];
                }
            }
        }
        sieve_n = n;
    }

} // namespace number_theory
