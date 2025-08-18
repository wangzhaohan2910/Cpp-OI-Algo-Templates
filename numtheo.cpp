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

    int exgcd(const int a, const int b, int &x, int &y)
    {
        return b ? pair{exgcd(b, a % b, y, x), y -= a / b * x}.first : (x = 1, y = 0, a);
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
}
