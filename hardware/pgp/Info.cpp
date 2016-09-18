/**
 *-----------------------------------------------------------------------------
 * Title      : PGP Card Info Class
 * ----------------------------------------------------------------------------
 * File       : Info.cpp
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2017-09-17
 * Last update: 2017-09-17
 * ----------------------------------------------------------------------------
 * Description:
 * Wrapper for PgpInfo structure
 * ----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to 
 * the license terms in the LICENSE.txt file found in the top-level directory 
 * of this distribution and at: 
 *    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
 * No part of the rogue software platform, including this file, may be 
 * copied, modified, propagated, or distributed except according to the terms 
 * contained in the LICENSE.txt file.
 * ----------------------------------------------------------------------------
**/

#include <hardware/pgp/Info.h>

namespace rhp = rogue::hardware::pgp;

//! Create the info class with pointer
rhp::InfoPtr rhp::Info::create() {
   rhp::InfoPtr r = boost::make_shared<rhp::Info>();
   return(r);
}

//! Return buildstring in string format
std::string rhp::Info::buildString() {
   return(std::string(buildStamp));
}
