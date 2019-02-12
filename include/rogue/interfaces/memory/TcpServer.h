/**
 *-----------------------------------------------------------------------------
 * Title      : Memory Server Network Bridge
 * ----------------------------------------------------------------------------
 * File       : TcpServer.h
 * Created    : 2019-01-30
 * ----------------------------------------------------------------------------
 * Description:
 * Memory Server Network Bridge
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
#ifndef __ROGUE_INTERFACES_MEMORY_TCP_SERVER_H__
#define __ROGUE_INTERFACES_MEMORY_TCP_SERVER_H__
#include <rogue/interfaces/memory/Master.h>
#include <rogue/Logging.h>
#include <boost/thread.hpp>
#include <stdint.h>

namespace rogue {
   namespace interfaces {
      namespace memory {

         class TcpServer : public rogue::interfaces::memory::Master {

               //! Inbound Address
               std::string reqAddr_;

               //! Outbound Address
               std::string respAddr_;

               //! Zeromq Context
               void * zmqCtx_;

               //! Zeromq inbound port
               void * zmqReq_;

               //! Zeromq outbound port
               void * zmqResp_;

               //! Thread background
               void runThread();

               //! Log
               boost::shared_ptr<rogue::Logging> bridgeLog_;

               //! Thread
               boost::thread * thread_;

            public:

               //! Class creation
               static boost::shared_ptr<rogue::interfaces::memory::TcpServer> 
                      create (std::string addr, uint16_t port);

               //! Setup class in python
               static void setup_python();

               //! Creator
               TcpServer(std::string addr, uint16_t port);

               //! Destructor
               ~TcpServer();

         };

         // Convienence
         typedef boost::shared_ptr<rogue::interfaces::memory::TcpServer> TcpServerPtr;

      }
   }
};

#endif

