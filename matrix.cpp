struct matrix
{
<<<<<<< HEAD
    const static int p{1000000007};
    int h, w;
    vector<int> v;
    matrix(const int &h, const int &w) : h(h), w(w), v(h * w) {}
    matrix(initializer_list<initializer_list<int>> l) : h(l.size()), w(begin(l)->size())
    {
        v.reserve(h * w);
        for (const initializer_list<int> &x : l)
            move(begin(x), end(x), back_inserter(v));
    }
    int *operator[](const int &x)
    {
        return v.data() + x * w;
    }
    const int *operator[](const int &x) const
    {
        return v.data() + x * w;
    }
    friend matrix operator*(const matrix &lhs, const matrix &rhs)
    {
        if (lhs.w != rhs.h)
            return {};
        matrix res(lhs.h, rhs.w);
        for (int i{}; i < lhs.h; i++)
            for (int j{}; j < lhs.w; j++)
                for (int k{}; k < rhs.w; k++)
                    res[i][k] += lhs[i][j] * rhs[j][k],
                    res[i][k] %= p;
        return res;
    }
    matrix &operator*=(const matrix &rhs) &
    {
        return (*this) = (*this) * rhs;
    }
    matrix pow(matrix rhs, int k)
    {
        if (w != rhs.h || rhs.h != rhs.w)
            return {};
        matrix res(*this);
        for (; k; rhs *= rhs, k >>= 1)
            if (k & 1)
                res *= rhs;
        return res;
    }
    matrix pow(int k)
    {
        if (h != w)
            return {};
        matrix res(h, w);
        for (int i{}; i < h; i++)
            res[i][i] = 1;
        return res.pow(*this, k);
=======
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
>>>>>>> 11c61ed86bb237a8dc202e9b2aa7b29c2a779bb0
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