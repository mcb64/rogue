/**
 *-----------------------------------------------------------------------------
 * Title      : Align Exception
 * ----------------------------------------------------------------------------
 * File       : AlignException.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2017-09-17
 * Last update: 2017-09-17
 * ----------------------------------------------------------------------------
 * Description:
 * Align denied exception for Rogue
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
#ifndef __ROGUE_EXCEPTIONS_ALIGN_EXCEPTION_H__
#define __ROGUE_EXCEPTIONS_ALIGN_EXCEPTION_H__
#include <exception>
#include <boost/python.hpp>
#include <stdint.h>

namespace rogue {
   namespace exceptions {

      extern PyObject * alignExceptionObj;

      //! Align exception
      class AlignException : public std::exception {
            char text_[100];
         public:
            AlignException (uint32_t index, uint32_t size);
            char const * what() const throw();
            static void setup_python();
            static void translate(AlignException const &e);
      };
   }
}

#endif

