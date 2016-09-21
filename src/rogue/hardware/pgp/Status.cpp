/**
 *-----------------------------------------------------------------------------
 * Title      : PGP Card Status Class
 * ----------------------------------------------------------------------------
 * File       : Status.cpp
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2017-09-17
 * Last update: 2017-09-17
 * ----------------------------------------------------------------------------
 * Description:
 * Wrapper for PgpStatus structure
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
#include <rogue/hardware/pgp/Status.h>
#include <boost/make_shared.hpp>

namespace rhp = rogue::hardware::pgp;
namespace bp  = boost::python;

//! Create the info class with pointer
rhp::StatusPtr rhp::Status::create() {
   rhp::StatusPtr r = boost::make_shared<rhp::Status>();
   return(r);
}

void rhp::Status::setup_python () {

   bp::class_<rhp::Status, rhp::StatusPtr>("Status",bp::no_init)
      .def("create",                  &rhp::Status::create)
      .staticmethod("create")
      .def_readwrite("lane",          &rhp::Status::lane)
      .def_readwrite("loopBack",      &rhp::Status::loopBack)
      .def_readwrite("locLinkReady",  &rhp::Status::locLinkReady)
      .def_readwrite("remLinkReady",  &rhp::Status::remLinkReady)
      .def_readwrite("rxReady",       &rhp::Status::rxReady)
      .def_readwrite("txReady",       &rhp::Status::txReady)
      .def_readwrite("rxCount",       &rhp::Status::rxCount)
      .def_readwrite("cellErrCnt",    &rhp::Status::cellErrCnt)
      .def_readwrite("linkDownCnt",   &rhp::Status::linkDownCnt)
      .def_readwrite("linkErrCnt",    &rhp::Status::linkErrCnt)
      .def_readwrite("fifoErr",       &rhp::Status::fifoErr)
      .def_readwrite("remData",       &rhp::Status::remData)
      .def_readwrite("remBuffStatus", &rhp::Status::remBuffStatus)
   ;

}
