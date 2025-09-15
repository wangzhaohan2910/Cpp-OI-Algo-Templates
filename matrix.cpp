class matrix
{
    const int h, w;
    int **r;
    matrix(int h, int w)
	: h{h}, w{w}, r{new int*[h]}
    {
	for (int i{}; i < h; i++)
	    r[i] = new int[w];
    }
    ~matrix()
    {
	for (int i{}; i < h; i++)
	    delete[] r[i];
	delete[] r;
    }
    matrix(const matrix& rhs)
	: h{rhs.h}, w{rhs.w}, r{new int*[h]}
    {
	for (int i{}; i < h; i++)
	    r[i] = new int[w],
	    memcpy(r[i], rhs.r[i], w);
    }
    matrix(matrix&& rhs)
	: h{rhs.h}, w{rhs.w}, r{rhs.r}
    {
	rhs.h = rhs.w = rhs.r = 0;
    }
    matrix operator*(const matrix& )
};
