// ============================================================================
// 可实例化的可持久化 WBLT
// 所有对象共享同一个静态节点池，每个对象只保存一个根索引。
// 插入、删除均返回新对象，旧对象不受影响。
// ============================================================================

template <typename Key, typename Value>
class PersistentWBLT
{
private:
    // ---------- 节点结构 ----------
    struct Node
    {
        int left, right;
        int weight;          // 子树叶子数
        Key max_key;         // 整个子树的最大键（叶子即自己的键，内部为右子树最大键）
        Key key;             // 叶子节点的键
        Value value;         // 叶子节点的值

        // 叶子构造
        Node(Key k, Value v)
            : left(0), right(0), weight(1), max_key(k), key(k), value(v) {}

        // 内部节点构造（由左右孩子创建）
        Node(int l, int r)
            : left(l), right(r), weight(0), max_key(Key{}), key(Key{}), value(Value{})
        {
            weight = (l ? nodes[l].weight : 0) + (r ? nodes[r].weight : 0);
            // 整个子树的最大键 = 右子树的最大键（如果右子树存在），否则左子树的最大键
            if (r)
                max_key = nodes[r].max_key;
            else if (l)
                max_key = nodes[l].max_key;
            else
                max_key = Key{};
        }

        Node() : left(0), right(0), weight(0), max_key(Key{}), key(Key{}), value(Value{}) {}
    };

    // ---------- 静态节点池 ----------
    static constexpr size_t MAX_NODES = 2000000;  // 根据需求调整
    static Node nodes[MAX_NODES];
    static int pool_ptr;

    // ---------- 辅助函数 ----------
    static int new_leaf(Key k, Value v)
    {
        nodes[++pool_ptr] = Node(k, v);
        return pool_ptr;
    }

    static int new_internal(int l, int r)
    {
        nodes[++pool_ptr] = Node(l, r);
        return pool_ptr;
    }

    static int clone_node(int p)
    {
        nodes[++pool_ptr] = nodes[p];
        return pool_ptr;
    }

    static int weight(int p) { return p ? nodes[p].weight : 0; }
    static Key max_key(int p) { return p ? nodes[p].max_key : Key{}; }

    // ---------- 旋转与平衡 ----------
    static constexpr double ALPHA = 3.0;

    static int rotate_right(int p)
    {
        int l = nodes[p].left;
        int new_l = clone_node(l);
        int new_p = clone_node(p);
        nodes[new_p].left = nodes[new_l].right;
        nodes[new_l].right = new_p;
        // 更新权重和 max_key
        nodes[new_p].weight = weight(nodes[new_p].left) + weight(nodes[new_p].right);
        nodes[new_p].max_key = max_key(nodes[new_p].right);
        nodes[new_l].weight = weight(nodes[new_l].left) + weight(nodes[new_l].right);
        nodes[new_l].max_key = max_key(nodes[new_l].right);
        return new_l;
    }

    static int rotate_left(int p)
    {
        int r = nodes[p].right;
        int new_r = clone_node(r);
        int new_p = clone_node(p);
        nodes[new_p].right = nodes[new_r].left;
        nodes[new_r].left = new_p;
        nodes[new_p].weight = weight(nodes[new_p].left) + weight(nodes[new_p].right);
        nodes[new_p].max_key = max_key(nodes[new_p].right);
        nodes[new_r].weight = weight(nodes[new_r].left) + weight(nodes[new_r].right);
        nodes[new_r].max_key = max_key(nodes[new_r].right);
        return new_r;
    }

    static int balance(int p)
    {
        if (!p) return 0;
        if (nodes[p].left == 0 || nodes[p].right == 0) return p;

        int l = nodes[p].left, r = nodes[p].right;
        int wl = weight(l), wr = weight(r);
        if (wl == 0 || wr == 0) return p;

        if (wl > ALPHA * wr)
        {
            int ll = nodes[l].left, lr = nodes[l].right;
            int wll = weight(ll), wlr = weight(lr);
            if (wlr > wll)
            {
                int new_l = rotate_left(l);
                int new_p = clone_node(p);
                nodes[new_p].left = new_l;
                return rotate_right(new_p);
            }
            else
            {
                return rotate_right(p);
            }
        }
        else if (wr > ALPHA * wl)
        {
            int rl = nodes[r].left, rr = nodes[r].right;
            int wrl = weight(rl), wrr = weight(rr);
            if (wrl > wrr)
            {
                int new_r = rotate_right(r);
                int new_p = clone_node(p);
                nodes[new_p].right = new_r;
                return rotate_left(new_p);
            }
            else
            {
                return rotate_left(p);
            }
        }
        return p;
    }

    // ---------- 插入递归 ----------
    static int insert_rec(int p, Key k, Value v)
    {
        if (p == 0) return new_leaf(k, v);

        if (nodes[p].left == 0 && nodes[p].right == 0)  // leaf
        {
            if (nodes[p].key == k)
            {
                int new_p = clone_node(p);
                nodes[new_p].value = v;
                return new_p;
            }
            int new_leaf_node = new_leaf(k, v);
            int new_internal_node;
            if (k < nodes[p].key)
                new_internal_node = new_internal(new_leaf_node, p);
            else
                new_internal_node = new_internal(p, new_leaf_node);
            return balance(new_internal_node);
        }

        // internal node: 根据左子树的最大键决定去向
        int l = nodes[p].left, r = nodes[p].right;
        int new_left, new_right;
        if (k <= max_key(l))  // 插入到左子树
        {
            new_left = insert_rec(l, k, v);
            new_right = r;
        }
        else
        {
            new_left = l;
            new_right = insert_rec(r, k, v);
        }
        int new_p = new_internal(new_left, new_right);
        return balance(new_p);
    }

    // ---------- 分裂 (按键) ----------
    // 返回 pair<左树, 右树>，左树所有键 <= k，右树所有键 > k
    static pair<int, int> split_rec(int p, Key k)
    {
        if (p == 0) return {0, 0};
        if (nodes[p].left == 0 && nodes[p].right == 0)  // leaf
        {
            if (nodes[p].key <= k)
                return {p, 0};
            else
                return {0, p};
        }
        int l = nodes[p].left, r = nodes[p].right;
        if (k <= max_key(l))
        {
            auto [a, b] = split_rec(l, k);
            int new_right = merge(b, r);  // 注意：b 的键 <= k，r 的键 > k，所以 b 全部在 r 前面
            return {a, new_right};
        }
        else
        {
            auto [a, b] = split_rec(r, k);
            int new_left = merge(l, a);
            return {new_left, b};
        }
    }

    // ---------- 合并 (要求左树所有键 < 右树所有键) ----------
    static int merge_rec(int a, int b)
    {
        if (a == 0) return b;
        if (b == 0) return a;
        if (nodes[a].left == 0 && nodes[a].right == 0 &&
            nodes[b].left == 0 && nodes[b].right == 0)
        {
            // 两个叶子直接合并
            int new_node = new_internal(a, b);
            return balance(new_node);
        }
        // 否则，根据权重决定合并方向
        int wa = weight(a), wb = weight(b);
        if (wa > wb)
        {
            // 把 b 合并到 a 的右子树
            int new_right = merge_rec(nodes[a].right, b);
            int new_a = clone_node(a);
            nodes[new_a].right = new_right;
            nodes[new_a].weight = weight(nodes[new_a].left) + weight(new_right);
            nodes[new_a].max_key = max_key(new_right);
            return balance(new_a);
        }
        else
        {
            // 把 a 合并到 b 的左子树
            int new_left = merge_rec(a, nodes[b].left);
            int new_b = clone_node(b);
            nodes[new_b].left = new_left;
            nodes[new_b].weight = weight(new_left) + weight(nodes[new_b].right);
            nodes[new_b].max_key = max_key(nodes[new_b].right);
            return balance(new_b);
        }
    }

    // ---------- 删除 (基于 split + merge) ----------
    static int erase_rec(int p, Key k)
    {
        auto [left, right] = split_rec(p, k);
        // 从 left 中分离出等于 k 的节点（即 left 的最右元素）
        auto [left_part, mid] = split_rec(left, k - 1); // 左部分键 <= k-1，mid 键 == k
        // 现在 left_part 包含 < k 的，mid 包含 == k 的（如果有）
        // 直接合并 left_part 和 right（丢弃 mid）
        return merge_rec(left_part, right);
    }

    // ---------- 查找 ----------
    static Value* find_rec(int p, Key k)
    {
        while (p)
        {
            if (nodes[p].left == 0 && nodes[p].right == 0)
            {
                if (nodes[p].key == k)
                    return &nodes[p].value;
                return nullptr;
            }
            if (k <= max_key(nodes[p].left))
                p = nodes[p].left;
            else
                p = nodes[p].right;
        }
        return nullptr;
    }

    // ---------- 遍历输出（调试用） ----------
    static void print_rec(int p, int indent = 0)
    {
        if (!p) return;
        if (nodes[p].left == 0 && nodes[p].right == 0)
        {
            for (int i = 0; i < indent; ++i) cout << ' ';
            cout << "Leaf: key=" << nodes[p].key << " val=" << nodes[p].value << "\n";
        }
        else
        {
            for (int i = 0; i < indent; ++i) cout << ' ';
            cout << "Internal (w=" << nodes[p].weight << ", max_key=" << nodes[p].max_key << ")\n";
            print_rec(nodes[p].left, indent + 2);
            print_rec(nodes[p].right, indent + 2);
        }
    }

    // ---------- 成员变量：根索引 ----------
    int root;

public:
    // ---------- 构造 / 赋值 ----------
    PersistentWBLT() : root(0) {}
    explicit PersistentWBLT(int r) : root(r) {}

    // 拷贝构造、赋值默认即可（只复制一个 int）

    // ---------- 公开操作 ----------
    PersistentWBLT insert(Key k, Value v) const
    {
        int new_root = insert_rec(root, k, v);
        return PersistentWBLT(new_root);
    }

    PersistentWBLT erase(Key k) const
    {
        int new_root = erase_rec(root, k);
        return PersistentWBLT(new_root);
    }

    Value* find(Key k) const
    {
        return find_rec(root, k);
    }

    bool contains(Key k) const
    {
        return find_rec(root, k) != nullptr;
    }

    int size() const
    {
        return weight(root);
    }

    bool empty() const
    {
        return root == 0;
    }

    void print() const
    {
        print_rec(root);
    }

    // 返回根索引（可用于快速比较或底层操作）
    int get_root() const { return root; }
};

// ---------- 静态成员初始化 ----------
template <typename Key, typename Value>
typename PersistentWBLT<Key, Value>::Node PersistentWBLT<Key, Value>::nodes[MAX_NODES];

template <typename Key, typename Value>
int PersistentWBLT<Key, Value>::pool_ptr = 0;
