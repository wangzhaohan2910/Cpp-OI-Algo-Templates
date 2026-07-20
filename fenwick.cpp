// ============================================================================
// 多维树状数组（Fenwick Tree）
// 模板参数：Rank = 维度数，Dims... = 各维度大小
// ============================================================================
template <size_t Rank, size_t... Dims>
class Fenwick
{
    static_assert(sizeof...(Dims) == Rank,
                  "Number of dimensions must match Rank");

    static constexpr size_t dims[Rank] = {Dims...};
    static constexpr size_t TOTAL = (Dims * ...); // C++17 折叠表达式

    int tree[TOTAL + 1]{}; // 静态数组，零初始化

    // ---------- 展平（1‑based） ----------
    size_t flatten(const size_t idx[]) const
    {
        size_t offset = 0;
        size_t stride = 1;
        for (size_t d = 0; d < Rank; ++d)
        {
            offset += (idx[d] - 1) * stride;
            stride *= dims[d];
        }
        return offset + 1;
    }

    // ---------- 递归更新 ----------
    template <size_t D = 0>
    void update_rec(size_t pos[], int delta, const size_t target[])
    {
        if constexpr (D == Rank)
        {
            tree[flatten(pos)] += delta;
            return;
        }
        else
        {
            for (size_t i = target[D]; i <= dims[D]; i += i & -i)
            {
                pos[D] = i;
                update_rec<D + 1>(pos, delta, target);
            }
        }
    }

    // ---------- 递归前缀查询 ----------
    template <size_t D = 0>
    int query_rec(size_t pos[], const size_t target[]) const
    {
        if constexpr (D == Rank)
        {
            return tree[flatten(pos)];
        }
        else
        {
            int sum = 0;
            for (size_t i = target[D]; i > 0; i -= i & -i)
            {
                pos[D] = i;
                sum += query_rec<D + 1>(pos, target);
            }
            return sum;
        }
    }

    // ---------- 编译期容斥递归 ----------
    template <size_t D = 0>
    int inclusion_exclusion_rec(const size_t l[], const size_t r[],
                                int sign, size_t cur[]) const
    {
        if constexpr (D == Rank)
        {
            size_t pos[Rank] = {}; // 独立工作区
            return sign * query_rec<0>(pos, cur);
        }
        else
        {
            int sum = 0;
            // 分支1：选右边界
            cur[D] = r[D];
            sum += inclusion_exclusion_rec<D + 1>(l, r, sign, cur);
            // 分支2：选左边界 - 1
            cur[D] = l[D] - 1;
            sum += inclusion_exclusion_rec<D + 1>(l, r, -sign, cur);
            return sum;
        }
    }

public:
    Fenwick() = default;

    // ---------- 单点增加：void add(int delta, size_t... args) ----------
    template <typename... Args>
    void add(int delta, Args... args)
    {
        static_assert(sizeof...(Args) == Rank,
                      "Invalid number of indices");
        static_assert((std::is_convertible_v<Args, size_t> && ...),
                      "All index arguments must be convertible to size_t");

        size_t target[Rank] = {static_cast<size_t>(args)...};
        for (size_t d = 0; d < Rank; ++d)
        {
            target[d] += 1; // 转为 1‑based
            if (target[d] == 0 || target[d] > dims[d])
                return;
        }
        size_t pos[Rank] = {};
        update_rec<0>(pos, delta, target);
    }

    // ---------- 前缀查询（0‑based） ----------
    template <typename... Args>
    int prefix(Args... args) const
    {
        static_assert(sizeof...(Args) == Rank,
                      "Invalid number of indices");
        static_assert((std::is_convertible_v<Args, size_t> && ...),
                      "All index arguments must be convertible to size_t");

        size_t target[Rank] = {static_cast<size_t>(args)...};
        for (size_t d = 0; d < Rank; ++d)
        {
            target[d] += 1;
            if (target[d] > dims[d])
                target[d] = dims[d];
        }
        size_t pos[Rank] = {};
        return query_rec<0>(pos, target);
    }

    // ---------- 矩形查询（0‑based） ----------
    // 参数顺序：l0, r0, l1, r1, ...
    template <typename... Args>
    int range(Args... args) const
    {
        static_assert(sizeof...(Args) == 2 * Rank,
                      "range requires 2*Rank arguments (l0,r0,l1,r1,...)");
        static_assert((std::is_convertible_v<Args, size_t> && ...),
                      "All arguments must be convertible to size_t");

        size_t all[2 * Rank] = {static_cast<size_t>(args)...};
        size_t l[Rank], r[Rank];
        for (size_t d = 0; d < Rank; ++d)
        {
            l[d] = all[2 * d] + 1;
            r[d] = all[2 * d + 1] + 1;
            if (l[d] < 1)
                l[d] = 1;
            if (r[d] > dims[d])
                r[d] = dims[d];
            if (l[d] > r[d])
                return 0; // 无效区间
        }
        size_t cur[Rank] = {};
        return inclusion_exclusion_rec<0>(l, r, 1, cur);
    }
};

// ============================================================================
// 一维特化（性能优化）
// ============================================================================
template <size_t Dim>
class Fenwick<1, Dim>
{
private:
    int tree[Dim + 1]{};

public:
    void add(int delta, size_t idx)
    {
        for (size_t i = idx + 1; i <= Dim; i += i & -i)
            tree[i] += delta;
    }

    int prefix(size_t idx) const
    {
        if (idx >= Dim)
            idx = Dim - 1;
        int res = 0;
        for (size_t i = idx + 1; i > 0; i -= i & -i)
            res += tree[i];
        return res;
    }

    int range(size_t l, size_t r) const
    {
        if (l > r || r >= Dim)
            return 0;
        return prefix(r) - (l > 0 ? prefix(l - 1) : 0);
    }
};
