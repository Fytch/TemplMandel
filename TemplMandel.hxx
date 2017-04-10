#ifndef TEMPLMANDEL_HXX_INCLUDED
#define TEMPLMANDEL_HXX_INCLUDED

#include <cstdint>
#include <limits>
#include <utility>

namespace tmandel {
	using int_t = std::int64_t;
	static_assert( std::numeric_limits< int_t >::digits == 63, "" );
	static constexpr int_t int_t_min = std::numeric_limits< int_t >::min();
	static constexpr int_t int_t_max = std::numeric_limits< int_t >::max();
	static_assert( ~int_t{} == int_t{ -1 }, "two's complement required" );
	static_assert( -int_t_max - 1 == int_t_min, "two's complement required" );

	template< int_t x >
	static constexpr int_t abs = x < 0 ? -x : x;
	template< int_t x, int_t y >
	static constexpr int_t min = x < y ? x : y;
	template< int_t x, int_t y >
	static constexpr int_t max = x > y ? x : y;
	template< int_t x, int_t y >
	static constexpr int_t smaller = abs< x > < abs< y > ? x : y;
	template< int_t x, int_t y >
	static constexpr int_t bigger = abs< x > > abs< y > ? x : y;

#define SMEAR_RIGHT_1( x )  (                 x   |                 x   >> 1  )
#define SMEAR_RIGHT_2( x )  ( SMEAR_RIGHT_1 ( x ) | SMEAR_RIGHT_1 ( x ) >> 2  )
#define SMEAR_RIGHT_4( x )  ( SMEAR_RIGHT_2 ( x ) | SMEAR_RIGHT_2 ( x ) >> 4  )
#define SMEAR_RIGHT_8( x )  ( SMEAR_RIGHT_4 ( x ) | SMEAR_RIGHT_4 ( x ) >> 8  )
#define SMEAR_RIGHT_16( x ) ( SMEAR_RIGHT_8 ( x ) | SMEAR_RIGHT_8 ( x ) >> 16 )
#define SMEAR_RIGHT( x )    ( SMEAR_RIGHT_16( x ) | SMEAR_RIGHT_16( x ) >> 32 )

	namespace detail {
		template< int_t x >
		struct hsb_helper {
			static constexpr std::size_t v = 1 + hsb_helper< ( x >> 1 ) >::v;
		};
		template<>
		struct hsb_helper< 0 > {
			static constexpr std::size_t v = 0;
		};
	}
	template< int_t x >
	static constexpr std::size_t hsb = detail::hsb_helper< SMEAR_RIGHT( x ) >::v;
	// before calculating, smear right -> way more instantiation table hits, ~7% speedup in compile time

	namespace detail {
		template< int_t lhs, int_t rhs, bool = ( rhs >= 0 ) >
		struct overflow_add_helper {
			static constexpr bool v = ( lhs > int_t_max - rhs );
		};
		template< int_t lhs, int_t rhs >
		struct overflow_add_helper< lhs, rhs, false > {
			static constexpr bool v = ( lhs < int_t_min - rhs );
		};
	}
	template< int_t lhs, int_t rhs >
	static constexpr bool overflow_add = detail::overflow_add_helper< lhs, rhs >::v;
	template< int_t lhs, int_t rhs >
	static constexpr bool overflow_sub = overflow_add< lhs, -rhs >;
	template< int_t lhs, int_t rhs >
	static constexpr bool overflow_mul = hsb< abs< lhs > > + hsb< abs< rhs > > > std::numeric_limits< int_t >::digits;
	template< int_t lhs, int_t rhs >
	static constexpr int overflow_mul_digs = max< 0, static_cast< int >( hsb< abs< lhs > > + hsb< abs< rhs > > ) - std::numeric_limits< int_t >::digits >;

	namespace detail {
		template< int_t a, int_t b >
		struct gcd_helper {
			static constexpr int_t v = gcd_helper< b, a % b >::v;
		};
		template< int_t a >
		struct gcd_helper< a, 0 > {
			static constexpr int_t v = a;
		};
	}
	template< int_t a, int_t b >
	static constexpr int_t gcd = detail::gcd_helper< max< a, b >, min< a, b > >::v;

	namespace detail {
		template< template< typename, typename> class bin_op, typename... T >
		struct fold_helper;
		template< template< typename, typename> class bin_op, typename T >
		struct fold_helper< bin_op, T > {
			using t = T;
		};
		template< template< typename, typename> class bin_op, typename lhs, typename rhs, typename... T >
		struct fold_helper< bin_op, lhs, rhs, T... > {
			using t = typename fold_helper< bin_op, bin_op< lhs, rhs >, T... >::t;
		};
	}
	template< template< typename, typename> class bin_op, typename... T >
	using fold = typename detail::fold_helper< bin_op, T... >::t;

	template< int_t numerator, int_t denominator = 1 >
	struct Q {
		static constexpr int_t _gcd = gcd< abs< numerator >, abs< denominator > >;
		static constexpr int_t _sgn = ( numerator < 0 ) ^ ( denominator < 0 );
		static constexpr int_t num = abs< numerator > / _gcd * ( _sgn ? -1 : 1 );
		static constexpr int_t den = abs< denominator > / _gcd;

		template< typename T >
		static constexpr T to_floating = static_cast< T >( num ) / static_cast< T >( den );
	};

	template< typename lhs, typename rhs >
	static constexpr bool Qeq = lhs::num == rhs::num && lhs::den == rhs::den;
	template< typename lhs, typename rhs >
	static constexpr bool Qneq = !Qeq< lhs, rhs >;
	template< typename lhs, typename rhs >
	static constexpr bool Qgr = lhs::template to_floating< double > > rhs::template to_floating< double >;

	template< typename q >
	using Qneg = Q< -q::num, q::den >;
	template< typename q >
	using Qreci = Q< q::den, q::num >;
	template< typename q >
	using Qabs = Q< abs< q::num >, q::den >;

	namespace detail {
		template< int_t a, int_t b, int_t c, int_t d, int of = max< overflow_mul_digs< a, b >, overflow_mul_digs< c, d > > >
		struct Qadd_op_safe_helper {
			static constexpr int_t correction = ( 1 << ( of - 1 ) );
			using rec = Qadd_op_safe_helper< ( ( bigger< a, b > + correction ) >> of ), smaller< a, b >, ( ( bigger< c, d > + correction ) >> of ), smaller< c, d > >;
			static constexpr int_t div = ( 1 << of ) * rec::div;
			static constexpr int_t v = rec::v;
		};
		template< int_t a, int_t b, int_t c, int_t d >
		struct Qadd_op_safe_helper< a, b, c, d, 0 > {
			static constexpr int_t div = overflow_add< a * b, c * d > ? 2 : 1;
			static constexpr int_t v = a * b / div + c * d / div;
		};
		template< typename lhs, typename rhs >
		struct Qadd_op_safe {
			static constexpr int_t _gcd = gcd< lhs::den, rhs::den >;
			static constexpr int_t expansion_l = rhs::den / _gcd;
			static constexpr int_t expansion_r = lhs::den / _gcd;
			static constexpr int_t lcm = expansion_r * rhs::den;
			using result = Qadd_op_safe_helper< lhs::num, expansion_l, rhs::num, expansion_r>;
			using t = Q< result::v, lcm / result::div >;
		};
		template< typename lhs, typename rhs >
		using Qadd_op = typename Qadd_op_safe< lhs, rhs >::t;
		template< typename lhs, typename rhs >
		using Qsub_op = Qadd_op< lhs, Qneg< rhs > >;

		template< typename lhs, typename rhs, bool of = overflow_mul< lhs::num, rhs::num > || overflow_mul< lhs::den, rhs::den >, bool = Qgr< Qabs< lhs >, Qabs< rhs > > >
		struct Qmul_op_safe {
			using t = Q< lhs::num * rhs::num, lhs::den * rhs::den >;
		};
		template< typename lhs, typename rhs >
		struct Qmul_op_safe< lhs, rhs, true, false > {
			using t = typename Qmul_op_safe< lhs, Q< rhs::num / 2, rhs::den / 2 > >::t;
		};
		template< typename lhs, typename rhs >
		struct Qmul_op_safe< lhs, rhs, true, true > {
			using t = typename Qmul_op_safe< Q< lhs::num / 2, lhs::den / 2 >, rhs >::t;
		};
		template< typename lhs, typename rhs >
		using Qmul_op = typename Qmul_op_safe< lhs, rhs >::t;
		template< typename lhs, typename rhs >
		using Qdiv_op = Qmul_op< lhs, Qreci< rhs > >;
	}
	template< typename... q >
	using Qadd = fold< detail::Qadd_op, q... >;
	template< typename... q >
	using Qsub = fold< detail::Qsub_op, q... >;
	template< typename... q >
	using Qmul = fold< detail::Qmul_op, q... >;
	template< typename... q >
	using Qdiv = fold< detail::Qdiv_op, q... >;
	template< typename q >
	using Qsq = Qmul< q, q >;

	static_assert( Qeq< Qadd< Q< 2, 3 >, Q< 2, 5 >, Q< 1, -15 > >, Q< 1 > >, "" );

	template< typename real, typename imaginary = Q< 0 > >
	struct C {
		using re = real;
		using im = imaginary;
	};

	template< typename T >
	using re = typename T::re;
	template< typename T >
	using im = typename T::im;

	template< typename lhs, typename rhs >
	static constexpr bool Ceq = Qeq< re< lhs >, re< rhs > > && Qeq< im< lhs >, im< rhs > >;
	template< typename lhs, typename rhs >
	static constexpr bool Cneq = !Ceq< lhs, rhs >;

	namespace detail {
		template< typename lhs, typename rhs >
		using Cadd_op = C< Qadd< re< lhs >, re< rhs > >, Qadd< im< lhs >, im< rhs > > >;
		template< typename lhs, typename rhs >
		using Csub_op = C< Qsub< re< lhs >, re< rhs > >, Qsub< im< lhs >, im< rhs > > >;
		template< typename lhs, typename rhs >
		using Cmul_op = C< Qsub< Qmul< re< lhs >, re< rhs > >, Qmul< im< lhs >, im< rhs > > >, Qadd< Qmul< im< lhs >, re< rhs > >, Qmul< re< lhs >, im< rhs > > > >;
		template< typename lhs, typename rhs >
		using Cdiv_op = C< Qdiv< Qadd< Qmul< re< lhs >, re< rhs > >, Qmul< im< lhs >, im< rhs > > >, Qadd< Qsq< re< rhs > >, Qsq< im< rhs > > > >, Qdiv< Qsub< Qmul< im< lhs >, re< rhs > >, Qmul< re< lhs >, im< rhs > > >, Qadd< Qsq< re< rhs > >, Qsq< im< rhs > > > > >;
	}
	template< typename... c >
	using Cadd = fold< detail::Cadd_op, c... >;
	template< typename... c >
	using Csub = fold< detail::Csub_op, c... >;
	template< typename... c >
	using Cmul = fold< detail::Cmul_op, c... >;
	template< typename... c >
	using Cdiv = fold< detail::Cdiv_op, c... >;
	template< typename c >
	using Cneg = C< Qneg< re< c > >, Qneg< im< c > > >;
	template< typename c >
	using Creci = C< Qdiv< re< c >, Qadd< Qsq< re< c > >, Qsq< im< c > > > >, Qdiv< Qneg< im< c > >, Qadd< Qsq< re< c > >, Qsq< im< c > > > > >;
	template< typename c >
	using Csq = Cmul< c, c >;
	template< typename c >
	using Cnormsq = Qadd< Qsq< re< c > >, Qsq< im< c > > >;

	static_assert( Ceq< Csq< C< Q< 0 >, Q< 1 > > >, C< Q< -1 > > >, "i^2 != -1" );
	static_assert( Ceq< Creci< Creci< C< Q< 3, 4 >, Q< -2, 5 > > > >, C< Q< 3, 4 >, Q< -2, 5 > > >, "" );

	namespace detail {
		template< typename z, typename c >
		using mandel_iter = Cadd< Csq< z >, c >;

		template< typename c, unsigned int n, typename z = C< Q< 0 > >, bool bailout = Qgr< Cnormsq< z >, Qsq< Q< 2 > > > >
		struct mandel {
			static constexpr unsigned int v = 1 + mandel< c, n - 1, mandel_iter< z, c > >::v;
		};
		template< typename c, unsigned int n, typename z >
		struct mandel< c, n, z, true > {
			static constexpr unsigned int v = 0;
		};
		template< typename c, typename z >
		struct mandel< c, 0, z, false > {
			static constexpr unsigned int v = 0;
		};
	}
	template< typename c, unsigned int n = 16 >
	static constexpr unsigned int mandel = detail::mandel< c, n >::v;
}

#endif // !TEMPLMANDEL_HXX_INCLUDED
