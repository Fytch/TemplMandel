#include "TemplMandel.hxx"
#include "Image.hxx"
#include <utility>
#include <cstdint>
#include <iostream>

namespace tmandel {
	constexpr unsigned int max_iters = 8;

	template< std::size_t w, std::size_t h, typename topleft, typename pixel, std::size_t x, std::size_t y >
	constexpr unsigned int mandelbrot() {
		return mandel< Cadd< topleft, C< Qmul< re< pixel >, Q< x > >, Qmul< im< pixel >, Q< y > > > >, max_iters >;
	}
	template< std::size_t w, std::size_t h, typename topleft, typename pixel, std::size_t... i >
	constexpr void mandelbrot( unsigned int( &arr )[ w * h ], std::integer_sequence< std::size_t, i... > ) {
		using T = unsigned int[];
		T result{ mandelbrot< w, h, topleft, pixel, i % w, i / w >()... };
		std::copy( std::begin( result ), std::end( result ), std::begin( arr ) );
	}
	template< std::size_t w, std::size_t h, typename center, typename span >
	constexpr void mandelbrot( unsigned int( &arr )[ w * h ] ) {
		mandelbrot<
			w, h,
			C< Qsub< re< center >, Qdiv< re< span >, Q< 2 > > >, Qadd< im< center >, Qdiv< im< span >, Q< 2 > > > >,
			C< Qdiv< re< span >, Q< w > >, Qdiv< im< span >, Q< -static_cast< int_t >( h ) > > >
		>( arr, std::make_index_sequence< w * h >{} );
	}
}

template< typename T >
T clamp( T min, T max, T val ) {
	return val < min ? min : ( val > max ? max : val );
}

int main( int argc, char** argv ) {
	using namespace tmandel;

	constexpr std::size_t w = 64, h = 64;
	unsigned int iters[ w * h ];
	mandelbrot< w, h, C< Q< -3, 4 > >, C< Q< 3 >, Q< 3 > > >( iters );
	tmandel::image img{ w, h };
	for( std::size_t y = 0; y < h; ++y ) {
		for( std::size_t x = 0; x < w; ++x ) {
			const std::uint8_t intensity = clamp< unsigned int >( 0x00, 0xFF, iters[ x + y * w ] * ( 0x100 / max_iters ) );
			img( x, y ).r = intensity;
			img( x, y ).g = intensity;
			img( x, y ).b = intensity;
		}
	}
	char const* const filename = argc == 2 ? argv[ 1 ] : "mandelbrot.bmp";
	if( !img.to_file( filename ) )
		std::cerr << "couldn't write file '" << filename << "'\n";
}
