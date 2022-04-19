/**
 *-----------------------------------------------------------------------------
 * Title      : XVC Server Class
 * ----------------------------------------------------------------------------
 * Description:
 * Rogue implementation of the XVC Server 
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
#ifndef __ROGUE_PROTOCOLS_XILINX_XVC_H__
#define __ROGUE_PROTOCOLS_XILINX_XVC_H__

#include <rogue/interfaces/stream/Master.h>
#include <rogue/interfaces/stream/Slave.h>
#include <rogue/protocols/xilinx/XvcServer.h>
#include <rogue/protocols/xilinx/XvcConnection.h>
#include <rogue/protocols/xilinx/JtagDriver.h>
#include <rogue/protocols/xilinx/JtagDriverAxisToJtag.h>
#include <rogue/EnableSharedFromThis.h>
#include <rogue/Logging.h>
#include <thread>
#include <stdint.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <memory>

namespace rogue
{
   namespace protocols
   {
      namespace xilinx
      {
         const unsigned int kMaxArgs = 3;

         class Xvc : public rogue::interfaces::stream::Master,
                     public rogue::interfaces::stream::Slave,
                     public rogue::protocols::xilinx::JtagDriverAxisToJtag,
                     public rogue::EnableSharedFromThis<rogue::protocols::xilinx::Xvc>
         {
         protected:

            //! Pointers to JTAG driver and XVC server
            XvcServer  *s_;
            JtagDriver *drv_;

            int timeoutMs_;
            unsigned mtu_;

            // Use rogue frames to exchange data with other rogue objects
            std::shared_ptr<rogue::interfaces::stream::Frame> frame_;

            // Log
            std::shared_ptr<rogue::Logging> xvcLog_;

            //! Thread background
            bool threadEn_;            
            std::thread* thread_;

            // Lock
            std::mutex mtx_;

            // TCP server for Vivado client
            void runThread(std::weak_ptr<int> lock);

         public:

            //! Class creation
            static std::shared_ptr<rogue::protocols::xilinx::Xvc>
            create(std::string host, uint16_t port);

            //! Setup class in python
            static void setup_python();

            //! Creator
            Xvc(std::string host, uint16_t port);

            //! Destructor
            ~Xvc();

            //! Stop the interface
            void stop();

            // Receive frame
            void acceptFrame (std::shared_ptr<rogue::interfaces::stream::Frame> frame);

            virtual unsigned long getMaxVectorSize() final;

            virtual int
            xfer(uint8_t *txBuffer, unsigned txBytes, uint8_t *hdBuffer, unsigned hdBytes, uint8_t *rxBuffer, unsigned rxBytes) final;
         };

         // Convenience
         typedef std::shared_ptr<rogue::protocols::xilinx::Xvc> XvcPtr;
      }
   }
}

#endif
