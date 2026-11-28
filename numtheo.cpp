struct number_theory
{
    static void swap(auto &a, auto &b) noexcept
    {
        a ^= b;
        b ^= a;
        a ^= b;
    }

    static int exgcd(int a, int b, int &x, int &y)
    {
        if (a == 0 && b == 0)
        {
            x = y = 0;
            return 0;
        }
        if (a == 0)
        {
            x = 0;
            y = (b > 0) ? 1 : -1;
            return (b > 0) ? b : -b;   // 要求 b != INT_MIN
        }
        if (b == 0)
        {
            x = (a > 0) ? 1 : -1;
            y = 0;
            return (a > 0) ? a : -a;   // 要求 a != INT_MIN
        }

        const int sa = (a < 0) ? -1 : 1;
        const int sb = (b < 0) ? -1 : 1;
        const int A  = (a < 0) ? -a : a;   // 要求 a != INT_MIN
        const int B  = (b < 0) ? -b : b;   // 要求 b != INT_MIN

        const int shift = __builtin_ctzll((long long)(A | B));
        const int x0 = A >> shift;
        const int y0 = B >> shift;

        int u = x0, v = y0;
        int P = 1, Q = 0; // P*x0 + Q*y0 = u
        int R = 0, S = 1; // R*x0 + S*y0 = v

        auto norm = [&](int &P, int &Q)
        {
            if (P >= y0)  P -= y0, Q += x0;
            if (P <= -y0) P += y0, Q -= x0;
        };

        while (u != v)
        {
            while (!(u & 1))
            {
                u >>= 1;
                if (!(P & 1) && !(Q & 1))
                    P >>= 1, Q >>= 1;
                else
                    P = (P + y0) >> 1, Q = -((x0 - Q) >> 1);
            }
            while (!(v & 1))
            {
                v >>= 1;
                if (!(R & 1) && !(S & 1))
                    R >>= 1, S >>= 1;
                else
                    R = (R + y0) >> 1, S = -((x0 - S) >> 1);
            }
            if (u > v)
                u -= v, P -= R, Q -= S, norm(P, Q);
            else if (v > u)
                v -= u, R -= P, S -= Q, norm(R, S);
        }

        const int g = u << shift;
        x = sa * P;
        y = sb * Q;
        return g;
    }

    int p;
    vector<int> fc{1}, iv{0, 1};

    number_theory(int p = 998244353) : p{p} {}

    [[nodiscard]] int pow(int a, int b)
    {
        int r{1};
        for (; b; a = a * a % p, b >>= 1)
            if (b & 1)
                r = r * a % p;
        return r;
    }

    // Lucas 非递归实现（基于模 p 的阶乘/逆元预处理）
    [[nodiscard]] int C_mod(int m, int n)
    {
        if (n < 0 || n > m) return 0;
        return fac(m) * 1LL * inv_on(fac(n)) % p * inv_on(fac(m - n)) % p;
    }

    [[nodiscard]] int lucas(long long m, long long n)
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
    [[nodiscard]] int inv_on(const int n)
    {
        int x, y;
        int g = exgcd(n, p, x, y);
        if (g != 1) return -1; // inverse does not exist
        x %= p;
        if (x < 0) x += p;
        return x;
    }

    int fac(int n)
    {
        int i{fc.size()};
        if (i <= n)
        {
            fc.resize(n + 1);
            for (; i <= n; ++i)
                fc[i] = fc[i - 1] * i % p;
        }
        return fc[n];
    }

    int inv_off(int n)
    {
        int i{iv.size()};
        if (i <= n)
        {
            iv.resize(n + 1);
            for (; i <= n; ++i)
                iv[i] = (p - p / i) * iv[p % i] % p;
        }
        return iv[n];
    }
};
