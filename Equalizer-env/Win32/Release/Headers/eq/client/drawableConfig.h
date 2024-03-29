
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_DRAWABLECONFIG_H
#define EQ_DRAWABLECONFIG_H

#include <eq/base/base.h>

#include <iostream>

namespace eq
{
    /** Stores characteristics of an OpenGL drawable. */
    struct DrawableConfig
    {
        int32_t stencilBits;    //!< No of stencil bits
        int32_t alphaBits;      //!< No of alpha bits
        int32_t accumBits;      //!< No of accum bits
        float   glVersion;      //!< OpenGL version
        bool    stereo;         //!< Active stereo supported
        bool    doublebuffered; //!< Doublebuffering supported
    };

    inline std::ostream& operator << ( std::ostream& os,
                                       const DrawableConfig& config )
    {
        os << "GL" << config.glVersion;
        if( config.stereo )
            os << "|ST";
        if( config.doublebuffered )
            os << "|DB";
        if( config.stencilBits )
            os << "|st" << config.stencilBits;
        if( config.alphaBits )
            os << "|a" << config.alphaBits;
        if( config.accumBits )
            os << "|acc" << config.accumBits;
        return os;
    }

}

#endif // EQ_DRAWABLECONFIG_H

