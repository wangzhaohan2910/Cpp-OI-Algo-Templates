// ============================================================================
// 可持久化 WBLT（Weight Balanced Leafy Tree，集合版，允许重复键）
// 每次 insert / erase 返回新版本，旧版本不受影响。
// 要求：Key 可比较、可拷贝、可默认构造。
// ============================================================================
template <typename Key>
class PersistentWBLT
{
private:
    struct Node
    {
        int l, r;
        int sz;        // 子树叶子数
        Key max_key;   // 子树最大键；叶子即自己的键
        Key key;       // 仅叶子有效

        Node() : l(0), r(0), sz(0), max_key(), key() {}
    };

    static constexpr int MAXN = 5000000;   // 按内存限制调整
    static Node t[MAXN];
    static int pool;

    int root;

    static int sz(int p)
    {
        return p ? t[p].sz : 0;
    }

    static int new_leaf(const Key &k)
    {
        ++pool;
        t[pool].l = t[pool].r = 0;
        t[pool].sz = 1;
        t[pool].max_key = t[pool].key = k;
        return pool;
    }

    static int make_internal(int l, int r)
    {
        ++pool;
        t[pool].l = l;
        t[pool].r = r;
        t[pool].sz = t[l].sz + t[r].sz;
        t[pool].max_key = t[r].max_key;
        return pool;
    }

    // ---------- 旋转：全部返回新节点，不修改输入 ----------
    static constexpr double ALPHA = 3.0;

    static int rotate_right(int p)
    {
        int l = t[p].l, r = t[p].r;
        int ll = t[l].l, lr = t[l].r;
        int q = make_internal(lr, r);
        return make_internal(ll, q);
    }

    static int rotate_left(int p)
    {
        int l = t[p].l, r = t[p].r;
        int rl = t[r].l, rr = t[r].r;
        int q = make_internal(l, rl);
        return make_internal(q, rr);
    }

    static int balance(int p)
    {
        if (t[p].l == 0) return p;   // 叶子
        int l = t[p].l, r = t[p].r;
        int wl = t[l].sz, wr = t[r].sz;

        if (wl > ALPHA * wr)
        {
            int ll = t[l].l, lr = t[l].r;
            if (t[lr].sz > t[ll].sz)
            {
                // LR 型：先对 l 左旋，再对整体右旋
                int new_l = rotate_left(l);
                int q = make_internal(new_l, r);
                return rotate_right(q);
            }
            return rotate_right(p);
        }
        if (wr > ALPHA * wl)
        {
            int rl = t[r].l, rr = t[r].r;
            if (t[rl].sz > t[rr].sz)
            {
                // RL 型：先对 r 右旋，再对整体左旋
                int new_r = rotate_right(r);
                int q = make_internal(l, new_r);
                return rotate_left(q);
            }
            return rotate_left(p);
        }
        return p;
    }

    // ---------- 插入：允许重复键 ----------
    static int insert_rec(int p, const Key &k)
    {
        if (p == 0) return new_leaf(k);

        if (t[p].l == 0)   // 叶子
        {
            int n = new_leaf(k);
            int q;
            if (k < t[p].key) q = make_internal(n, p);
            else              q = make_internal(p, n);
            return balance(q);
        }

        if (k <= t[t[p].l].max_key)
        {
            int new_l = insert_rec(t[p].l, k);
            return balance(make_internal(new_l, t[p].r));
        }
        else
        {
            int new_r = insert_rec(t[p].r, k);
            return balance(make_internal(t[p].l, new_r));
        }
    }

    // ---------- 删除：删一个等于 k 的叶子 ----------
    // 没找到时 removed = false，原样返回 p，不浪费节点
    static int erase_rec(int p, const Key &k, bool &removed)
    {
        if (p == 0)
        {
            removed = false;
            return 0;
        }

        if (t[p].l == 0)   // 叶子
        {
            if (t[p].key == k)
            {
                removed = true;
                return 0;
            }
            removed = false;
            return p;
        }

        if (k <= t[t[p].l].max_key)
        {
            int new_l = erase_rec(t[p].l, k, removed);
            if (!removed) return p;
            if (new_l == 0) return t[p].r;           // 左孩子删空，提升右孩子
            return balance(make_internal(new_l, t[p].r));
        }
        else
        {
            int new_r = erase_rec(t[p].r, k, removed);
            if (!removed) return p;
            if (new_r == 0) return t[p].l;           // 右孩子删空，提升左孩子
            return balance(make_internal(t[p].l, new_r));
        }
    }

    // ---------- 第 k 小（1-indexed） ----------
    static int kth_impl(int p, int k)
    {
        while (t[p].l)
        {
            int ls = t[t[p].l].sz;
            if (k <= ls) p = t[p].l;
            else
            {
                k -= ls;
                p = t[p].r;
            }
        }
        return p;
    }

    // ---------- 小于等于 k 的元素个数 ----------
    static int count_le_impl(int p, const Key &k)
    {
        int cnt = 0;
        while (t[p].l)
        {
            int l = t[p].l;
            if (k > t[l].max_key)
            {
                cnt += t[l].sz;
                p = t[p].r;
            }
            else p = l;
        }
        if (t[p].key <= k) cnt++;
        return cnt;
    }

    // ---------- 是否包含 k ----------
    static bool contains_impl(int p, const Key &k)
    {
        while (t[p].l)
        {
            if (k <= t[t[p].l].max_key) p = t[p].l;
            else p = t[p].r;
        }
        return t[p].key == k;
    }

public:
    PersistentWBLT() : root(0) {}
    explicit PersistentWBLT(int r) : root(r) {}

    [[nodiscard]] PersistentWBLT insert(const Key &k) const
    {
        return PersistentWBLT(insert_rec(root, k));
    }

    [[nodiscard]] PersistentWBLT erase(const Key &k) const
    {
        bool removed = false;
        int new_root = erase_rec(root, k, removed);
        if (!removed) return PersistentWBLT(root);   // 没找到，返回原版本
        return PersistentWBLT(new_root);
    }

    [[nodiscard]] Key kth(int k) const
    {
        if (k < 1 || k > sz(root)) return Key{};
        return t[kth_impl(root, k)].key;
    }

    [[nodiscard]] int count_le(const Key &k) const
    {
        return count_le_impl(root, k);
    }

    [[nodiscard]] bool contains(const Key &k) const
    {
        return root && contains_impl(root, k);
    }

    [[nodiscard]] int size() const
    {
        return sz(root);
    }
    [[nodiscard]] bool empty() const
    {
        return root == 0;
    }
    [[nodiscard]] int get_root() const
    {
        return root;
    }
};

template <typename Key>
typename PersistentWBLT<Key>::Node PersistentWBLT<Key>::t[PersistentWBLT<Key>::MAXN];

template <typename Key>
int PersistentWBLT<Key>::pool = 0;
