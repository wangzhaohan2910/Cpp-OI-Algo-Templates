struct union_find
{
    int fa[500005]{}, sz[500005]{}, n;
    [[nodiscard]] int find(const int x)
    {
        if (x == fa[x])
            return x;
        return fa[x] = find(fa[x]);
    }
    void merge(int x, int y)
    {
        x = find(x);
        y = find(y);
        if (x != y)
            if (sz[x] < sz[y])
                fa[x] = y, sz[y] += sz[x];
            else
                fa[y] = x, sz[x] += sz[y];
    }
    explicit union_find(const int n) : n{n}
    {
        for (int i{1}; i <= n; i++)
            fa[i] = i, sz[i] = 1;
    }
};
