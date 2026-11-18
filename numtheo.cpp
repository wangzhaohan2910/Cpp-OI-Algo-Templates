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

    // 基于 Stein（二进制）算法的扩展欧几里得
    // 返回 g = gcd(a,b)，并设置 x,y 使 ax + by = g
    // 要求 a,b >= 0；全程只用移位、加减和比较，不做除法
    int exgcd(int a, int b, int &x, int &y)
    {
        if (a == 0)
        {
            x = 0;
            y = (b == 0 ? 0 : 1);
            return b;
        }
        if (b == 0)
        {
            x = 1;
            y = 0;
            return a;
        }

        const int shift = __builtin_ctzll((unsigned long long)(a | b));
        const long long x0 = (long long)a >> shift;
        const long long y0 = (long long)b >> shift;

        long long u = x0, v = y0;
        long long A = 1, B = 0; // A*x0 + B*y0 = u
        long long C = 0, D = 1; // C*x0 + D*y0 = v

        // 减法后 |P| < 2*y0，一次调整即可回到 (-y0, y0)
        auto norm = [&](long long &P, long long &Q)
        {
            if (P >= y0)
            {
                P -= y0;
                Q += x0;
            }
            if (P <= -y0)
            {
                P += y0;
                Q -= x0;
            }
        };

        while (u != v)
        {
            while ((u & 1) == 0)
            {
                u >>= 1;
                if ((A & 1) == 0 && (B & 1) == 0)
                {
                    A >>= 1;
                    B >>= 1;
                }
                else
                {
                    A = (A + y0) >> 1;
                    B = -((x0 - B) >> 1);
                }
            }

            while ((v & 1) == 0)
            {
                v >>= 1;
                if ((C & 1) == 0 && (D & 1) == 0)
                {
                    C >>= 1;
                    D >>= 1;
                }
                else
                {
                    C = (C + y0) >> 1;
                    D = -((x0 - D) >> 1);
                }
            }

            if (u > v)
            {
                u -= v;
                A -= C;
                B -= D;
                norm(A, B);
            }
            else if (v > u)
            {
                v -= u;
                C -= A;
                D -= B;
                norm(C, D);
            }
        }

        x = (int)A;
        y = (int)B;
        return (int)(u << shift);
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
} // namespace number_theory
