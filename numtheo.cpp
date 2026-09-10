#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>
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

    // 迭代版扩展欧几里得（无递归）
    // 返回 g = gcd(a,b)，并设置 x,y 使 ax+by = g
    int exgcd(int a, int b, int &x, int &y)
    {
        x = 1; y = 0;
        int x1 = 0, y1 = 1;
        while (b)
        {
            int q = a / b;
            int ta = a - q * b; a = b; b = ta;
            int tx = x - q * x1; x = x1; x1 = tx;
            int ty = y - q * y1; y = y1; y1 = ty;
        }
        return a;
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

    inline int powint(int a, int b)
    {
        return pow(a, b);
    }

    // ---------- 线性筛：8 个常用积性函数，共用一个连续数组 ----------
    // 8 个函数顺序（索引）：
    // 0: unit  (1 for all n)
    // 1: id    (n)
    // 2: mu    (Möbius)
    // 3: phi   (Euler totient)
    // 4: sigma (sum of divisors)
    // 5: tau   (number of divisors)
    // 6: rad   (radical: product of distinct prime factors)
    // 7: lambda (Liouville function: (-1)^Omega(n))
    
    enum MF_INDEX { MF_UNIT = 0, MF_ID = 1, MF_MU = 2, MF_PHI = 3, MF_SIGMA = 4, MF_TAU = 5, MF_RAD = 6, MF_LAMBDA = 7, MF_CNT = 8 };

    std::vector<long long> mf; // length = (n+1) * MF_CNT, mf[i*MF_CNT + k]
    std::vector<int> primes;
    std::vector<char> is_comp;
    std::vector<int> spf; // 最小质因子，供内部使用
    std::vector<int> cnt; // p 的幂次数
    std::vector<long long> ppow; // p^cnt
    std::vector<long long> spSum; // 1 + p + ... + p^cnt
    int sieve_n = 0;

    void linear_sieve(int n)
    {
        if (n <= sieve_n)
            return;
        primes.clear();
        is_comp.assign(n + 1, 0);
        spf.assign(n + 1, 0);
        cnt.assign(n + 1, 0);
        ppow.assign(n + 1, 0);
        spSum.assign(n + 1, 0);
        mf.assign((n + 1) * MF_CNT, 0);

        // 初始化 n=1
        mf[1 * MF_CNT + MF_UNIT] = 1;
        mf[1 * MF_CNT + MF_ID] = 1;
        mf[1 * MF_CNT + MF_MU] = 1;
        mf[1 * MF_CNT + MF_PHI] = 1;
        mf[1 * MF_CNT + MF_SIGMA] = 1;
        mf[1 * MF_CNT + MF_TAU] = 1;
        mf[1 * MF_CNT + MF_RAD] = 1;
        mf[1 * MF_CNT + MF_LAMBDA] = 1;

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
                mf[i * MF_CNT + MF_UNIT] = 1;
                mf[i * MF_CNT + MF_ID] = i;
                mf[i * MF_CNT + MF_MU] = -1;
                mf[i * MF_CNT + MF_PHI] = i - 1;
                mf[i * MF_CNT + MF_SIGMA] = spSum[i];
                mf[i * MF_CNT + MF_TAU] = 2;
                mf[i * MF_CNT + MF_RAD] = i;
                mf[i * MF_CNT + MF_LAMBDA] = -1;
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
                    // pr 与 spf[i] 相同：i = m * pr^k
                    cnt[t] = cnt[i] + 1;
                    ppow[t] = ppow[i] * pr;
                    spSum[t] = spSum[i] + ppow[t];

                    // unit
                    mf[t * MF_CNT + MF_UNIT] = 1;
                    // id: multiply by pr
                    mf[t * MF_CNT + MF_ID] = mf[i * MF_CNT + MF_ID] * pr;
                    // mu: zero because square factor
                    mf[t * MF_CNT + MF_MU] = 0;
                    // phi: phi(i*p) = phi(i) * p
                    mf[t * MF_CNT + MF_PHI] = mf[i * MF_CNT + MF_PHI] * pr;
                    // sigma: replace spSum[i] by spSum[t]
                    // sigma[i] = sigma[m] * spSum[i], sigma[t] = sigma[m] * spSum[t]
                    // so sigma[t] = sigma[i] / spSum[i] * spSum[t]
                    if (spSum[i] != 0)
                        mf[t * MF_CNT + MF_SIGMA] = mf[i * MF_CNT + MF_SIGMA] / spSum[i] * spSum[t];
                    else
                        mf[t * MF_CNT + MF_SIGMA] = mf[i * MF_CNT + MF_SIGMA];
                    // tau: multiplicative; adjust exponent
                    mf[t * MF_CNT + MF_TAU] = mf[i * MF_CNT + MF_TAU] / (cnt[i] + 1) * (cnt[t] + 1);
                    // rad: same as rad[i]
                    mf[t * MF_CNT + MF_RAD] = mf[i * MF_CNT + MF_RAD];
                    // lambda: changes sign if added one prime factor
                    // but since multiplicity increased, lambda = (-1)^{Omega(n)} where Omega counts multiplicity
                    // Omega(t) = Omega(i) + 1
                    mf[t * MF_CNT + MF_LAMBDA] = -mf[i * MF_CNT + MF_LAMBDA];

                    break;
                }
                else
                {
                    // pr is a new distinct prime factor
                    cnt[t] = 1;
                    ppow[t] = pr;
                    spSum[t] = 1 + pr;

                    mf[t * MF_CNT + MF_UNIT] = 1;
                    mf[t * MF_CNT + MF_ID] = mf[i * MF_CNT + MF_ID] * pr;
                    mf[t * MF_CNT + MF_MU] = -mf[i * MF_CNT + MF_MU];
                    mf[t * MF_CNT + MF_PHI] = mf[i * MF_CNT + MF_PHI] * (pr - 1);
                    mf[t * MF_CNT + MF_SIGMA] = mf[i * MF_CNT + MF_SIGMA] * spSum[t];
                    mf[t * MF_CNT + MF_TAU] = mf[i * MF_CNT + MF_TAU] * 2;
                    mf[t * MF_CNT + MF_RAD] = mf[i * MF_CNT + MF_RAD] * pr;
                    mf[t * MF_CNT + MF_LAMBDA] = -mf[i * MF_CNT + MF_LAMBDA];
                }
            }
        }
        sieve_n = n;
    }

} // namespace number_theory
