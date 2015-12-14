#include <iostream>
#include <cstdint>
#include <climits>
#include <cstring>

template <typename T, int EBITS, int MBITS>
class Float
{
public:
    static const int expBits = EBITS;
    static const int expOffset = (1 << (EBITS-1));
    static const int mantBits = MBITS;
    static const T mantMask = (1 << mantBits)-1;
    typedef T intType;
    Float(T v)
	{
	    val.whole = v;
	}
    Float() { val.whole = 0xdeaddead; }
    Float(const Float& rhs) { val.whole = rhs.val.whole; }
    static_assert((EBITS + MBITS + 1) / CHAR_BIT == sizeof(T),
		  "Expect bits in T to match EBITS + MBITS + 1");
    union FloatValue
    {
	T whole;
	struct
	{
	    T mant: MBITS;
	    T exp: EBITS;
	    T sign: 1;
	} bits;
    };

    Float operator+=(const Float& rhs);
    Float operator-=(const Float& rhs);

    Float operator-() const
	{
	    Float tmp = *this;
	    tmp.Sign(!Negative());
	    return tmp;
	}

    int Exp() const { return val.bits.exp - expOffset; }
    T Mant() const;
    bool Negative() const { return !!val.bits.sign; }

    void Mant(T m) { val.bits.mant = m & mantMask; }
    void Exp(int e) { val.bits.exp = e + expOffset; }
    void Sign(bool s) { val.bits.sign = s; }
//private:
    FloatValue val;
};

template<typename T, int EBITS, int MBITS>
T Float<T, EBITS, MBITS>::Mant() const
{
    typedef Float<T, EBITS, MBITS> FP;
    if ((val.bits.mant|val.bits.exp) == 0)
    {
	return 0;
    }
    if (val.bits.exp)
    {
	return val.bits.mant | (1 << FP::mantBits);
    }
    // Denormal.
    return val.bits.mant;
}

template<typename T, int EBITS, int MBITS>
Float<T, EBITS, MBITS> Float<T, EBITS, MBITS>::operator+=(const Float<T, EBITS, MBITS>& rhs)
{
    if (rhs.Negative())
    {
	return (*this) -= -rhs;
    }
    int newExp = Exp();
    int re = rhs.Exp();
    intType lm = Mant();
    intType rm = rhs.Mant();
    int diff = re - newExp;
    if (diff > 0)
    {
	newExp = re;
	lm >>= diff;
    }
    else if (diff < 0)
    {
	rm >>= -diff;
    }

    intType m = rm + lm;
    while ((m & ~mantMask) > (1 << mantBits))
    {
	newExp ++;
	m >>= 1;
    }
    Mant(m);
    Exp(newExp);

    return *this;
}

template<typename T, int EBITS, int MBITS>
Float<T, EBITS, MBITS> Float<T, EBITS, MBITS>::operator-=(const Float<T, EBITS, MBITS>& rhs)
{
    if (rhs.Negative())
    {
	return (*this) += -rhs;
    }
    int newExp = Exp();
    int re = rhs.Exp();
    intType lm = Mant();
    intType rm = rhs.Mant();
    int diff = re - newExp;
    if (diff > 0)
    {
	newExp = re;
	lm >>= diff;
    }
    else if (diff < 0)
    {
	rm >>= -diff;
    }

    intType m;
    if (rm > lm)
    {
	m = rm - lm;
	Sign(!Negative());
    }
    else
    {
	m = lm - rm;
    }
    while ((m & ~mantMask) < (1 << mantBits))
    {
	newExp --;
	m <<= 1;
    }
    Mant(m);
    Exp(newExp);

    return *this;
}

template<typename T>
T operator+(T lhs, T rhs)
{
    T res = lhs;
    res += rhs;
    return res;
}

template<typename T>
T operator-(T lhs, T rhs)
{
    T res = lhs;
    res -= rhs;
    return res;
}

using FP16 = Float<uint16_t, 5, 10>;
using FP32 = Float<uint32_t, 8, 23>;

float FP32ToFloat(FP32 x)
{
    float r;
    memcpy(&r, &x, sizeof(float));
    return r;
}

FP32 FloatToFP32(float x)
{
    FP32 r;
    memcpy(&r, &x, sizeof(float));
    return r;
}

int main()
{
    FP16 a(0x3C00);
    FP16 b(0x4000);

    FP16 c = a + b;
    std::cout << std::hex << a.val.whole << std::endl;
    std::cout << std::hex << b.val.whole << std::endl;
    c = c + a;
    std::cout << std::hex << c.val.whole << std::endl;
    c = c + b;
    std::cout << std::hex << c.val.whole << std::endl;
    c += a;
    std::cout << std::hex << c.val.whole << std::endl;

    FP32 d(FloatToFP32(0.77f));
    FP32 e(FloatToFP32(1.82f));

    FP32 f = d + e;
    std::cout << std::hex << f.val.whole << std::endl;

    std::cout << FP32ToFloat(f) << std::endl;

    f -= e;
    std::cout << FP32ToFloat(f) << std::endl;

    FP32 g(FloatToFP32(1.0));
    FP32 h(FloatToFP32(1.5));

    FP32 i = g - h;

    std::cout << FP32ToFloat(i) << std::endl;
}
