struct matrix
{
    size_t h, w;
    vector<int> data;
    int *operator[](const size_t x)
    {
        return data.data() + x * w;
    }
    const int *operator[](const size_t x) const
    {
        return data.data() + x * w;
    }
    matrix(const size_t h, const size_t w) : h(h), w(w), data(h * w)
    {
    }
    matrix(const initializer_list<initializer_list<int>> l) : h{l.size()}, w{begin(l)->size()}
    {
        data.reserve(h * w);
        for (const initializer_list<int> &x : l)
            move(begin(x), end(x), back_inserter(data));
    }
    static matrix id(const size_t a)
    {
        matrix res(a, a);
        for (size_t i{}; i < a; i++)
            res[i][i] = 1;
        return res;
    }
    static matrix row(initializer_list<int> l)
    {
        return {l};
    }
    static matrix col(const initializer_list<int> l)
    {
        matrix res{l};
        res.h = res.w;
        res.w = 1;
        return res;
    }
};
inline matrix operator*(const matrix &a, const matrix &b)
{
    if (a.w != b.h)
        return {};
    matrix res(a.h, b.w);
    for (size_t i{}; i < a.h; i++)
        for (size_t j{}; j < a.w; j++)
            for (size_t k{}; k < b.w; k++)
                res[i][k] += a[i][j] * b[j][k];
    return res;
}
inline matrix &operator*=(matrix &a, const matrix &b)
{
    if (a.w != b.h)
        return a;
    return a = a * b;
}
inline matrix pow(matrix init, matrix base, size_t exp)
{
    if (init.w != base.h || base.h != base.w)
        return {};
    for (; exp; base *= base, exp >>= 1)
        if (exp & 1)
            init *= base;
    return init;
}
inline matrix pow(const matrix &base, const size_t exp)
{
    return pow(matrix::id(base.h), base, exp);
}