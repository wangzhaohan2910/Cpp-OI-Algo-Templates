struct matrix
{
    const static int p{1000000007};
    int h, w;
    int **r;
    matrix(int h, int w)
        : h{h}, w{w}, r{new int*[h]}
    {
        if (h && w)
            for (int i{}; i < h; i++)
                r[i] = new int[w],
                memset(r[i], 0, w * sizeof(int));
        else
            delete[] r, r = (int **)(h = w = 0);
    }
    matrix(const initializer_list<initializer_list<int>> &l)
        : h{l.size()}, w{begin(l)->size()}, r{new int*[h]}
    {
        if (h && w)
        {
            auto it{begin(l)};
            for (int i{}; i < h; it++, i++)
                r[i] = new int[w],
                copy(begin(*it), end(*it), r[i]);
        }
        else
            delete[] r, r = (int **)(h = w = 0);
    }
    ~matrix()
    {
        for (int i{}; i < h; i++)
            delete[] r[i];
        delete[] r;
    }
    matrix(const matrix &rhs)
    : h{rhs.h}, w{rhs.w}, r{new int*[h]}
    {
        for (int i{}; i < h; i++)
            r[i] = new int[w],
            memcpy(r[i], rhs.r[i], w * sizeof(int));
    }
    matrix(matrix &&rhs)
    : h{rhs.h}, w{rhs.w}, r{rhs.r}
    {
        rhs.r = (int **)(rhs.h = rhs.w = 0);
    }
    matrix& operator=(const matrix &rhs)
    {
        for (int i{}; i < h; i++)
            delete[] r[i];
        delete[] r;
        h = rhs.h;
        w = rhs.w;
        r = new int*[h];
        for (int i{}; i < h; i++)
             r[i] = new int[w],
             memcpy(r[i], rhs.r[i], w * sizeof(int));
        return *this;
    }
    matrix& operator=(matrix &&rhs)
    {
        for (int i{}; i < h; i++)
            delete[] r[i];
        delete[] r;
        h = rhs.h;
        w = rhs.w;
        r = rhs.r;
        rhs.r = (int **)(rhs.h = rhs.w = 0);
        return *this;
    }
    matrix operator*(const matrix &rhs)
    {
        if (w != rhs.h)
            return matrix(0, 0);
        else
        {
            matrix m{h, rhs.w};
            for (int i{}; i < h; i++)
                for (int j{}; j < w; j++)
                    for (int k{}; k < rhs.w; k++)
                        m.r[i][k] += r[i][j] * rhs.r[j][k],
                        m.r[i][k] %= p;
            return m;
        }
    }
    matrix pow(int k, matrix res)
    {
        if (h != w)
            return matrix(0, 0);
        else
        {
            matrix m(*this);
            for (; k; m = m * m, k >>= 1)
                if (k & 1)
                    res = res * m;
            return res;
        }
    }
};
