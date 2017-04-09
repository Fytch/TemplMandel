#include "TemplMandel.hxx"
#include "Image.hxx"
#include <utility>
#include <cstdint>
#include <iostream>

namespace tmandel {
	template< typename... args_t >
	constexpr void discard( args_t&&... ) {
	}

	constexpr unsigned int max_iters = 8;

	template< std::size_t w, std::size_t h, typename topleft, typename pixel, std::size_t x, std::size_t y >
	constexpr unsigned int mandelbrot_pixel() {
		return mandel< Cadd< topleft, C< Qmul< re< pixel >, Q< x > >, Qmul< im< pixel >, Q< y > > > >, max_iters >;
	}
	template< std::size_t w, std::size_t h, typename center, typename span, std::size_t... i >
	constexpr void mandelbrot( unsigned int( &arr )[ w * h ], std::integer_sequence< std::size_t, i... > ) {
		using topleft = C< Qsub< re< center >, Qdiv< re< span >, Q< 2 > > >, Qadd< im< center >, Qdiv< im< span >, Q< 2 > > > >;
		using pixel = C< Qdiv< re< span >, Q< w > >, Qdiv< im< span >, Q< -static_cast< int_t >( h ) > > >;
		discard( arr[ i ] = mandelbrot_pixel< w, h, topleft, pixel, i % w, i / w >()... );
	}
	template< std::size_t w, std::size_t h, typename center, typename span >
	constexpr void mandelbrot( unsigned int( &arr )[ w * h ] ) {
		mandelbrot< w, h, center, span >( arr, std::make_index_sequence< w * h >{} );
	}
}

template< typename T >
constexpr T clamp( T min, T max, T val ) {
	return val < min ? min : ( val > max ? max : val );
}

int main( int argc, char** argv ) {
	using namespace tmandel;

	constexpr std::size_t w = 32, h = 32;
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
	char const* const filename = argc == 2 ? argv[ 1 ] : "output.bmp";
	if( !img.to_file( filename ) )
		std::cerr << "couldn't write file '" << filename << "'\n";
}
