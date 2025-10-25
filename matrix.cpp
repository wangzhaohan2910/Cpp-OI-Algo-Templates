struct matrix
{
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
    [[nodiscard]] int *operator[](const int &x)
    {
        return v.data() + x * w;
    }
    [[nodiscard]] const int *operator[](const int &x) const
    {
        return v.data() + x * w;
    }
    [[nodiscard]] friend matrix operator*(const matrix &lhs, const matrix &rhs)
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
    [[nodiscard]] matrix pow(matrix rhs, int k)
    {
        if (w != rhs.h || rhs.h != rhs.w)
            return {};
        matrix res(*this);
        for (; k; rhs *= rhs, k >>= 1)
            if (k & 1)
                res *= rhs;
        return res;
    }
    [[nodiscard]] matrix pow(int k)
    {
        if (h != w)
            return {};
        matrix res(h, w);
        for (int i{}; i < h; i++)
            res[i][i] = 1;
        return res.pow(*this, k);
    }
};
