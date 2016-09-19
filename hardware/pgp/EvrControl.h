/**
 *-----------------------------------------------------------------------------
 * Title      : PGP Card EVR Control Class
 * ----------------------------------------------------------------------------
 * File       : EvrControl.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2017-09-17
 * Last update: 2017-09-17
 * ----------------------------------------------------------------------------
 * Description:
 * Wrapper for PgpEvrControl structure
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
#ifndef __ROGUE_HARDWARE_PGP_EVR_CONTROL_H__
#define __ROGUE_HARDWARE_PGP_EVR_CONTROL_H__
#include <hardware/pgp/PgpDriver.h>
#include <boost/python.hpp>
#include <stdint.h>

namespace rogue {
   namespace hardware {
      namespace pgp {

         //! Wrapper for PgpInfo class. 
         class EvrControl : public PgpEvrControl {
            public:

               //! Create the info class with pointer
               static boost::shared_ptr<rogue::hardware::pgp::EvrControl> create();

         };

         //! Convienence
         typedef boost::shared_ptr<rogue::hardware::pgp::EvrControl> EvrControlPtr;
      }
   }
}

#endif

