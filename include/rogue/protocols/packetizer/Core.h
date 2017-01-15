/**
 *-----------------------------------------------------------------------------
 * Title      : Packetizer Core Class
 * ----------------------------------------------------------------------------
 * File       : Core.h
 * Created    : 2017-01-07
 * Last update: 2017-01-07
 * ----------------------------------------------------------------------------
 * Description:
 * Packetizer Core
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
#ifndef __ROGUE_PROTOCOLS_PACKETIZER_CORE_H__
#define __ROGUE_PROTOCOLS_PACKETIZER_CORE_H__
#include <boost/python.hpp>
#include <boost/thread.hpp>
#include <stdint.h>

namespace rogue {
   namespace protocols {
      namespace packetizer {

         class Transport;
         class Application;
         class Controller;

         //! Core Class
         class Core {

               //! Transport module
               boost::shared_ptr<rogue::protocols::packetizer::Transport> tran_;

               //! Application module
               boost::shared_ptr<rogue::protocols::packetizer::Application> app_;

               //! Core module
               boost::shared_ptr<rogue::protocols::packetizer::Controller> cntl_;

            public:

               //! Class creation
               static boost::shared_ptr<rogue::protocols::packetizer::Core> create ();

               //! Setup class in python
               static void setup_python();

               //! Creator
               Core();

               //! Destructor
               ~Core();

               //! Get transport interface
               boost::shared_ptr<rogue::protocols::packetizer::Transport> transport();

               //! Application module
               boost::shared_ptr<rogue::protocols::packetizer::Application> application();
         };

         // Convienence
         typedef boost::shared_ptr<rogue::protocols::packetizer::Core> CorePtr;

      }
   }
};

#endif
