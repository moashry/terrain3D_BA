
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

#ifndef EQ_H
#define EQ_H

#include <eq/base/base.h>

#include <eq/client/canvas.h>
#include <eq/client/channelStatistics.h>
#include <eq/client/client.h>
#include <eq/client/compositor.h>
#include <eq/client/config.h>
#include <eq/client/configEvent.h>
#include <eq/client/configParams.h>
#include <eq/client/configVisitor.h>
#include <eq/client/event.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/global.h>
#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/layout.h>
#include <eq/client/log.h>
#include <eq/client/node.h>
#include <eq/client/nodeFactory.h>
#include <eq/client/observer.h>
#include <eq/client/packets.h>
#include <eq/client/pipe.h>
#include <eq/client/server.h>
#include <eq/client/task.h>
#include <eq/client/types.h>
#include <eq/client/version.h>
#include <eq/client/view.h>
#include <eq/client/windowSystem.h>

#ifdef AGL
#  include <eq/client/aglEventHandler.h>
#  include <eq/client/aglPipe.h>
#  include <eq/client/aglWindow.h>
#endif
#ifdef GLX
#  include <eq/client/glXEventHandler.h>
#  include <eq/client/glXPipe.h>
#  include <eq/client/glXWindow.h>
#endif
#ifdef WGL
#  include <eq/client/wglEventHandler.h>
#  include <eq/client/wglPipe.h>
#  include <eq/client/wglWindow.h>
#endif

#include <eq/base/file.h>
#include <eq/base/rng.h>
#include <eq/base/sleep.h>
#include <eq/net/net.h>
#include <eq/util/util.h>

#include <vmmlib/vmmlib.hpp>

#ifdef EQ_USE_DEPRECATED
namespace eqBase = ::eq::base;
namespace eqNet  = ::eq::net;
#endif

/** \mainpage Equalizer API Documentation
    \htmlinclude "RelNotes.dox"
*/

#endif // EQ_H