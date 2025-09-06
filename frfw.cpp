char buf[1 << 20], *p1, *p2;
#define getchar() (p1 == p2 && (p2 = (p1 = buf) + fread(buf, 1, 1 << 20, stdin), p1 == p2) ? EOF : *p1++)
inline void read(int &x)
{
    register bool f{true};
    register char ch = getchar();
    x ^= x;
    while (ch < 48 || ch > 57)
    {
        if (ch == 45)
            f = false;
        ch = getchar();
    }
    while (ch > 47 && ch < 58)
        x = (x << 3) + (x << 1) + (ch ^ 48), ch = getchar();
    x *= (f << 1) - 1;
}
char obuf[1 << 20], *p3 = obuf;
#define putchar(x) (p3 - obuf < 1 << 20) ? (*p3++ = x) : (fwrite(obuf, p3 - obuf, 1, stdout), p3 = obuf, *p3++ = x)
inline void write(register int x)
{
    if (!x)
    {
        putchar(48);
        return;
    }
    static int_fast8_t c[20];
    register int_fast64_t len{};
    if (x < 0)
        x = -x, putchar(45);
    while (x)
        c[len++] = x % 10 ^ 48, x /= 10;
    while (len--)
        putchar(c[len]);
}
struct _FAST_
{
    ~_FAST_()
    {
        fwrite(obuf, p3 - obuf, 1, stdout);
    }
} _fast_;
