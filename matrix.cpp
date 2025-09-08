int p{1000000007};
template <int a, int b>
struct matrix: array<array<int, b>, a>
{
	template <int c>
	matrix<a, c> operator*(const matrix<b, c>& rhs) const
	{
		matrix<a, c> res{};
		for (int i{}; i < a; i++)
			for (int j{}; j < b; j++)
				for (int k{}; k < c; k++)
                    if ((res[i][k] += (*this)[i][j] * rhs[j][k] % p) >= p)
                        res[i][k] -= p;
		return res;
	}
    matrix<a, b> pow(int n)
    {
        if constexpr (a == b)
        {
        	matrix<a, a> res{}, m{*this};
        	for (int i{}; i < a; i++)
        		res[i][i] = 1;
        	for (; n; m = m * m, n >>= 1)
        		if (n & 1)
        			res = res * m;
        	return res;
        }
        else
            return *this;
    }
};
