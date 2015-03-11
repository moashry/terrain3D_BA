
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQBASE_RNG_H
#define EQBASE_RNG_H

#include <eq/base/debug.h> // for EQASSERT
#include <eq/base/nonCopyable.h>

#include <fcntl.h>
#include <limits>
#include <stdio.h>

namespace eq
{
namespace base
{
    /**
     * A random number generator.
     *
     * Generates a set of random, or if not supported by the operating system,
     * pseudo-random numbers. Each instance creates its own series of numbers.
     */
    class RNG : public NonCopyable
    {
    public:
        /** Construct a new random number generator. @version 1.0 */
        RNG()
        {
#ifdef Linux
            _fd = ::open( "/dev/urandom", O_RDONLY );
            EQASSERT( _fd != -1 );
#endif
            reseed();
        }

        /** Destruct the random number generator. @version 1.0 */
        ~RNG()
        {
#ifdef Linux
            if( _fd > 0 )
                close( _fd );
#endif
        }

        /** Re-initialize the seed value for pseudo RNG's. @version 1.0 */
        void reseed()
        {
#ifdef Linux
            // NOP
#elif defined (WIN32)
            LARGE_INTEGER seed;
            QueryPerformanceCounter( &seed );
            srand( seed.LowPart );
#else // Darwin
            srandomdev();
#endif
        }

        /**
          * Generate a random number.
          * 
          * The returned number is between min..max for integer types, and 
          * between 0..1 for floating-point types.
          * @return a random number.
          * @version 1.0 
          */
        template< typename T >
        T get()
        {
            T              value;
#ifdef Linux
            int read = ::read( _fd, &value, sizeof( T ));
            EQASSERT( read == sizeof( T ));
            if( read != sizeof( T ))
            {
                EQERROR << "random number generator not working" << std::endl;
                return 0;
            }

#elif defined (WIN32)

            EQASSERTINFO( RAND_MAX >= 32767, RAND_MAX );

            uint8_t* bytes = reinterpret_cast< uint8_t* >( &value );
            for( size_t i=0; i<sizeof( T ); ++i )
                bytes[i] = ( rand() & 255 );
#else // Darwin
            uint8_t* bytes = reinterpret_cast< uint8_t* >( &value );
            for( size_t i=0; i<sizeof( T ); ++i )
                bytes[i] = ( random() & 255 );
#endif
            return value;
        }

    private:
#ifdef Linux
        int _fd;
#endif
    };

    template< > inline float RNG::get()
    {
        const float max_limits =
            static_cast< float >( std::numeric_limits< uint32_t >::max( ));
        return ( get< uint32_t >() / max_limits);
    }
    
    template< > inline double RNG::get()
    {
        const double max_limits =
            static_cast< double >( std::numeric_limits< uint64_t >::max( ));
        return ( get< uint64_t >() / max_limits);
    }
}
}
#endif  // EQBASE_RNG_H