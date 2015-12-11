#include <iostream>
#include <cstdint>
#include <climits>

template <typename T, int EBITS, int MBITS>
class Float
{
public:
    static const int expBits = EBITS;
    static const int expOffset = EBITS >> 1;
    static const int mantBits = MBITS;
    typedef T intType;
    Float(T v)
	{
	    val.whole = v;
	}
    Float(int s, int e, T m);
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

    int Exp() { return val.bits.exp - expOffset; }
    T Mant();
    
//private:
    FloatValue val;
};

template<typename T, int EBITS, int MBITS>
T Float<T, EBITS, MBITS>::Mant()
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
    return val.bits.mant;
}

template<typename T, int EBITS, int MBITS>
Float<T, EBITS, MBITS>::Float(int s, int e, T m)
{
    val.bits.sign = s;
    val.bits.exp = e + expOffset;
    val.bits.mant = m;
}


template<typename T>
T operator+(T lhs, T rhs)
{
    int le = lhs.Exp();
    int re = rhs.Exp();
    typename T::intType lm = lhs.Mant();
    typename T::intType rm = rhs.Mant();
    int diff = re - le;
    if ( diff > 0 )
    {
	lm >>= diff;
    }
    else if (diff < 0)
    {
	rm >>= -diff;
    }
    int newExp = std::max(le, re);
    typename T::intType m = rm + lm;
    typename T::intType mask = (1 << T::mantBits)-1;
    while ((m & ~((1 << T::mantBits)-1)) > 1 << T::mantBits)
    {
	newExp ++;
	m >>= 1;
    }
    m &= mask;
    T res(0, newExp, m);
    return res;
}

using FP16 = Float<uint16_t, 5, 10>;

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
}



