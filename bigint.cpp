struct bigint final : std::vector<bool> {
    bool is_negative = false;

    // ==========================================
    // 0. 内部辅助
    // ==========================================
    void normalize();

    // ==========================================
    // 1. 构造、析构与基础赋值
    // ==========================================
    bigint();
    bigint(const bigint&);
    bigint(bigint&&);
    bigint& operator=(const bigint&);
    bigint& operator=(bigint&&);
    ~bigint();

    // ==========================================
    // 2. 复合赋值运算符 (10个)
    // ==========================================
    // 算术
    bigint& operator+=(const bigint&);
    bigint& operator-=(const bigint&);
    bigint& operator*=(const bigint&);
    bigint& operator/=(const bigint&);
    bigint& operator%=(const bigint&);
    // 位运算
    bigint& operator&=(const bigint&);
    bigint& operator|=(const bigint&);
    bigint& operator^=(const bigint&);
    bigint& operator<<=(const bigint&);
    bigint& operator>>=(const bigint&);

    // ==========================================
    // 3. 双目算术运算符 (5个)
    // ==========================================
    bigint operator+(const bigint&) const;
    bigint operator-(const bigint&) const;
    bigint operator*(const bigint&) const;
    bigint operator/(const bigint&) const;
    bigint operator%(const bigint&) const;

    // ==========================================
    // 4. 双目位运算符 (5个)
    // ==========================================
    bigint operator&(const bigint&) const;
    bigint operator|(const bigint&) const;
    bigint operator^(const bigint&) const;
    bigint operator<<(const bigint&) const;
    bigint operator>>(const bigint&) const;

    // ==========================================
    // 5. 一元运算符 (9个)
    // ==========================================
    bigint operator+() const;       // 一元正
    bigint operator-() const;       // 一元负
    bigint operator~() const;       // 按位取反
    
    bigint& operator++();           // 前置 ++
    bigint  operator++(int);        // 后置 ++
    bigint& operator--();           // 前置 --
    bigint  operator--(int);        // 后置 --
    
    bool operator!() const;         // 逻辑非

    // ==========================================
    // 6. 比较运算符 (C++20)
    // ==========================================
    bool operator==(const bigint&) const;
    std::strong_ordering operator<=>(const bigint&) const;

    // ==========================================
    // 7. 类型转换
    // ==========================================
    explicit operator bool() const;
};
