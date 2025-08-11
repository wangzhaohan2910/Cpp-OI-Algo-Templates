struct fenwick
{
    int n, r[500005]{};

    static int lowbit(const int x)
    {
        return x & -x;
    }

    explicit fenwick(const int n)
        : n{n} {}

    void add(int x, const int k)
    {
        for (; x <= n; x += lowbit(x))
            r[x] += k;
    }

    [[nodiscard]] int query(int x) const
    {
        int res{};
        for (; x; x -= lowbit(x))
            res += r[x];
        return res;
    }

    [[nodiscard]] int query(const int x, const int y) const
    {
        return query(y) - query(x - 1);
    }

    fenwick(const int *r, const int n)
        : n{n}
    {
        for (int i{1}; i <= n; i++)
            add(i, r[i]);
    }

};