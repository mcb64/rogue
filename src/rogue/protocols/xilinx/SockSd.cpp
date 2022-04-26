//-----------------------------------------------------------------------------
// Title      : JTAG Support
//-----------------------------------------------------------------------------
// Company    : SLAC National Accelerator Laboratory
//-----------------------------------------------------------------------------
// Description: SockSd.cpp
//-----------------------------------------------------------------------------
// This file is part of 'SLAC Firmware Standard Library'.
// It is subject to the license terms in the LICENSE.txt file found in the
// top-level directory of this distribution and at:
//    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
// No part of 'SLAC Firmware Standard Library', including this file,
// may be copied, modified, propagated, or distributed except according to
// the terms contained in the LICENSE.txt file.
//-----------------------------------------------------------------------------

#include <rogue/protocols/xilinx/SockSd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <pthread.h>
#include <math.h>
#include <string>

namespace rpx = rogue::protocols::xilinx;

rpx::SockSd::SockSd(bool stream)
{
   if ((sd_ = ::socket(AF_INET, stream ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0)
      throw(rogue::GeneralError::create("SockSd::SockSd()", "Unable to create socket"));
}

rpx::SockSd::~SockSd()
{
   ::close(sd_);
}

int rpx::SockSd::getSd()
{
   return sd_;
}
