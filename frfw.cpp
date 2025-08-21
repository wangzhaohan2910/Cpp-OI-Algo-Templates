char ibuf[1 << 20], *p1, *p2;

inline void gc(char &ch)
{
    if (p1 == p2)
    {
        p1 = ibuf;
        p2 = ibuf + fread(ibuf, 1, 1 << 20, stdin);
        if (p1 == p2)
        {
            ch = EOF;
            return;
        }
    }
    ch = *p1++;
}

inline void read(int &x)
{
    register bool f{};
    register char ch;
    gc(ch);
    x = 0;
    for (;;)
    {
        if (ch > 47 && ch < 58)
            break;
        if (ch == 45)
            f = true;
        gc(ch);
        if (ch > 47 && ch < 58)
            break;
        if (ch == 45)
            f = true;
        gc(ch);
        if (ch > 47 && ch < 58)
            break;
        if (ch == 45)
            f = true;
        gc(ch);
    }
#define get1char if (ch < 48 || ch > 57) goto end; x = x * 10 + (ch ^ 48), gc(ch);
#define get2char get1char get1char
#define get4char get2char get2char
    get4char
    get4char
    get4char
    get4char
    get4char
#undef get4char
#undef get2char
#undef get1char
end:
    if (f)
        x = -x;
}

char obuf[1 << 20], *p3{obuf};

inline void pc(register char ch)
{
    if (p3 - obuf >= 1 << 20)
        fwrite(obuf, p3 - obuf, 1, stdout), p3 = obuf;
    *p3++ = ch;
}

inline void write(register int x)
{
    if (!x)
    {
        pc(48);
        return;
    }
    static int_fast8_t c[20];
    register int_fast64_t len{};
    if (x < 0)
        x = -x, pc(45);
    while (x)
        c[len++] = x % 10 ^ 48, x /= 10;
#define write1char if (!len--) return; pc(c[len]);
#define write2char write1char write1char
#define write4char write2char write2char
    write4char
    write4char
    write4char
    write4char
    write4char
#undef write4char
#undef write2char
#undef write1char
}

struct FAST_
{
    ~FAST_()
    {
        fwrite(obuf, p3 - obuf, 1, stdout);
    }
} fast_;
