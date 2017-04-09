#ifndef IMAGE_HXX_INCLUDED
#define IMAGE_HXX_INCLUDED

#include <cstdint>
#include <limits>
#include <type_traits>
#include <string>
#include <vector>
#include <fstream>
#include <exception>

namespace tmandel {
	struct color {
		std::uint8_t r, g, b;
	};

	static_assert( std::numeric_limits< unsigned char >::digits == 8, "" );
	template< typename T, typename = std::enable_if_t< std::is_integral< T >::value > >
	std::string to_little_endian( T value ) {
		std::string result( sizeof( T ), '\0' );
		for( std::size_t i = 0; i < sizeof( T ); ++i ) {
			result[ i ] = value & ( ( 1 << 8 ) - 1 );
			value >>= 8;
		}
		return result;
	}

	class image {
		std::size_t m_width;
		std::size_t m_height;
		std::vector< color > m_array;

	public:
		inline image( std::size_t width, std::size_t height )
			: m_width{ width }, m_height{ height }, m_array( size() ) {
		}

		inline std::size_t width() const {
			return m_width;
		}
		inline std::size_t height() const {
			return m_height;
		}
		inline std::size_t size() const {
			return width() * height();
		}

		inline decltype( auto ) operator()( std::size_t x, std::size_t y ) const {
			return m_array[ x + y * width() ];
		}
		inline decltype( auto ) operator()( std::size_t x, std::size_t y ) {
			return m_array[ x + y * width() ];
		}

		inline bool to_file( char const* filename ) const {
			try {
				std::ofstream file{ filename, std::ofstream::trunc };
				if( !file )
					return false;
				file.exceptions( std::ofstream::badbit | std::ofstream::failbit );
				file << 'B' << 'M';
				std::size_t padding = 4 - ( 3 * width() & 0x03 );
				padding *= ( padding != 4 );
				const std::size_t size = ( 3 * width() + padding ) * height();
				file << to_little_endian< std::uint32_t >( 54 + size );
				file << to_little_endian< std::uint32_t >( 0 );
				file << to_little_endian< std::uint32_t >( 54 );
				file << to_little_endian< std::uint32_t >( 40 );
				file << to_little_endian< std::int32_t >( width() );
				file << to_little_endian< std::int32_t >( height() );
				file << to_little_endian< std::uint16_t >( 1 );
				file << to_little_endian< std::uint16_t >( 24 );
				file << to_little_endian< std::uint32_t >( 0 );
				file << to_little_endian< std::uint32_t >( size );
				file << to_little_endian< std::int32_t >( 0 );
				file << to_little_endian< std::int32_t >( 0 );
				file << to_little_endian< std::uint32_t >( 0 );
				file << to_little_endian< std::uint32_t >( 0 );
				for( std::size_t y = 0; y < height(); ++y ) {
					for( std::size_t x = 0; x < width(); ++x ) {
						auto const& p = operator()( x, height() - y - 1 );
						file << p.b << p.g << p.r;
					}
					for( std::size_t i = 0; i < padding; ++i )
						file << '\0';
				}
				return true;
			} catch( std::exception const& e ) {
				return false;
			}
		}
	};
}

#endif // !IMAGE_HXX_INCLUDED
