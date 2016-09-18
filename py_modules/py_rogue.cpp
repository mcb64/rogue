/**
 *-----------------------------------------------------------------------------
 * Title      : Python Classes
 * ----------------------------------------------------------------------------
 * File       : py_rogue.cpp
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2016-08-08
 * Last update: 2016-08-08
 * ----------------------------------------------------------------------------
 * Description:
 * Python class wrapper
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

#include <interfaces/stream/Slave.h>
#include <interfaces/stream/Master.h>
#include <interfaces/stream/Frame.h>
#include <boost/python.hpp>
#include <utilities/Prbs.h>

using namespace boost::python;
namespace ris = rogue::interfaces::stream;
namespace ru  = rogue::utilities;

BOOST_PYTHON_MODULE(py_rogue)
{

   PyEval_InitThreads();

   /////////////////////////////////
   // Interfaces
   /////////////////////////////////

   class_<ris::Frame, ris::FramePtr>("Frame",no_init)
      .def("getAvailable", &ris::Frame::getAvailable)
      .def("getPayload",   &ris::Frame::getPayload)
      .def("read",         &ris::Frame::readPy)
      .def("write",        &ris::Frame::writePy)
      .def("setError",     &ris::Frame::setError)
      .def("getError",     &ris::Frame::getError)
      .def("setFlags",     &ris::Frame::setFlags)
      .def("getFlags",     &ris::Frame::getFlags)
   ;

   class_<ris::Master, ris::MasterPtr>("Master",init<>())
      .def("create",         &ris::Master::create)
      .staticmethod("create")
      .def("setSlave",       &ris::Master::setSlave)
      .def("addSlave",       &ris::Master::addSlave)
      .def("reqFrame",       &ris::Master::reqFrame)
      .def("sendFrame",      &ris::Master::sendFrame)
   ;

   class_<ris::SlaveWrap, ris::SlaveWrapPtr, boost::noncopyable>("Slave",init<>())
      .def("create",         &ris::Slave::create)
      .staticmethod("create")
      .def("acceptFrame",    &ris::Slave::acceptFrame, &ris::SlaveWrap::defAcceptFrame)
      .def("getAlloc",       &ris::Slave::getAlloc)
   ;

   /////////////////////////////////
   // Utilities
   /////////////////////////////////

   class_<ru::Prbs, bases<ris::Master,ris::Slave>, ru::PrbsPtr >("Prbs",init<>())
      .def("create",         &ru::Prbs::create)
      .staticmethod("create")
      .def("genFrame",       &ru::Prbs::genFrame)
      .def("enable",         &ru::Prbs::enable)
      .def("disable",        &ru::Prbs::disable)
      .def("getErrors",      &ru::Prbs::getErrors)
      .def("getCount",       &ru::Prbs::getCount)
      .def("getBytes",       &ru::Prbs::getBytes)
      .def("resetCount",     &ru::Prbs::resetCount)
      .def("enMessages",     &ru::Prbs::enMessages)
   ;

   implicitly_convertible<ru::PrbsPtr, ris::SlavePtr>();
   implicitly_convertible<ru::PrbsPtr, ris::MasterPtr>();

   /////////////////////////////////
   // PGP
   /////////////////////////////////

/*
   class_<PgpInfo,boost::noncopyable>("PgpInfo")
      .def_readwrite("serial",     &PgpInfo::serial)
      .def_readwrite("type",       &PgpInfo::type)
      .def_readwrite("version",    &PgpInfo::version)
      .def_readwrite("laneMask",   &PgpInfo::laneMask)
      .def_readwrite("vcPerMask",  &PgpInfo::vcPerMask)
      .def_readwrite("pgpRate",    &PgpInfo::pgpRate)
      .def_readwrite("promPrgEn",  &PgpInfo::promPrgEn)
      .def_readwrite("evrSupport", &PgpInfo::evrSupport)
      //.def_readwrite("buildStamp", &PgpInfo::buildStamp, 
            //return_value_policy<copy_non_const_reference>())
   ; 
   
   class_<PciStatus,boost::noncopyable>("PciStatus")
      .def_readwrite("pciCommand",   &PciStatus::pciCommand)
      .def_readwrite("pciStatus",    &PciStatus::pciStatus)
      .def_readwrite("pciDCommand",  &PciStatus::pciDCommand)
      .def_readwrite("pciDStatus",   &PciStatus::pciDStatus)
      .def_readwrite("pciLCommand",  &PciStatus::pciLCommand)
      .def_readwrite("pciLStatus",   &PciStatus::pciLStatus)
      .def_readwrite("pciLinkState", &PciStatus::pciLinkState)
      .def_readwrite("pciFunction",  &PciStatus::pciFunction)
      .def_readwrite("pciDevice",    &PciStatus::pciDevice)
      .def_readwrite("pciBus",       &PciStatus::pciBus)
      .def_readwrite("pciLanes",     &PciStatus::pciLanes)
   ; 
   
   class_<PgpStatus,boost::noncopyable>("PgpStatus")
      .def_readwrite("lane",          &PgpStatus::lane)
      .def_readwrite("loopBack",      &PgpStatus::loopBack)
      .def_readwrite("locLinkReady",  &PgpStatus::locLinkReady)
      .def_readwrite("remLinkReady",  &PgpStatus::remLinkReady)
      .def_readwrite("rxReady",       &PgpStatus::rxReady)
      .def_readwrite("txReady",       &PgpStatus::txReady)
      .def_readwrite("rxCount",       &PgpStatus::rxCount)
      .def_readwrite("cellErrCnt",    &PgpStatus::cellErrCnt)
      .def_readwrite("linkDownCnt",   &PgpStatus::linkDownCnt)
      .def_readwrite("linkErrCnt",    &PgpStatus::linkErrCnt)
      .def_readwrite("fifoErr",       &PgpStatus::fifoErr)
      .def_readwrite("remData",       &PgpStatus::remData)
      .def_readwrite("remBuffStatus", &PgpStatus::remBuffStatus)
   ;

   class_<PgpEvrControl,boost::noncopyable>("PgpEvrControl")
      .def_readwrite("lane",        &PgpEvrControl::lane)
      .def_readwrite("evrEnable",   &PgpEvrControl::evrEnable)
      .def_readwrite("laneRunMask", &PgpEvrControl::laneRunMask)
      .def_readwrite("evrSyncEn",   &PgpEvrControl::evrSyncEn)
      .def_readwrite("evrSyncSel",  &PgpEvrControl::evrSyncSel)
      .def_readwrite("headerMask",  &PgpEvrControl::headerMask)
      .def_readwrite("evrSyncWord", &PgpEvrControl::evrSyncWord)
      .def_readwrite("runCode",     &PgpEvrControl::runCode)
      .def_readwrite("runDelay",    &PgpEvrControl::runDelay)
      .def_readwrite("acceptCode",  &PgpEvrControl::acceptCode)
      .def_readwrite("acceptDelay", &PgpEvrControl::acceptDelay)
   ;

   class_<PgpEvrStatus,boost::noncopyable>("PgpEvrStatus")
      .def_readwrite("lane",          &PgpEvrStatus::lane)
      .def_readwrite("linkErrors",    &PgpEvrStatus::linkErrors)
      .def_readwrite("linkUp",        &PgpEvrStatus::linkUp)
      .def_readwrite("runStatus",     &PgpEvrStatus::runStatus)
      .def_readwrite("evrSeconds",    &PgpEvrStatus::evrSeconds)
      .def_readwrite("runCounter",    &PgpEvrStatus::runCounter)
      .def_readwrite("acceptCounter", &PgpEvrStatus::acceptCounter)
   ;

   class_<PgpData,boost::noncopyable>("PgpData",no_init)
      .def("retBuffer",  &PgpData::retBuffer)
      .def("write",      &PgpData::write)
      .def("getData",    &PgpData::getDataPy)
      .def("getIndex",   &PgpData::getIndex)
      .def("getMaxSize", &PgpData::getMaxSize)

      .def_readwrite("lane",  &PgpData::lane)
      .def_readwrite("vc",    &PgpData::vc)
      .def_readwrite("cont",  &PgpData::cont)
      .def_readwrite("size",  &PgpData::size)
      .def_readwrite("error", &PgpData::error)
   ;

   class_<PgpCard,boost::noncopyable>("PgpCard",init<>())
      .def("openWo",         &PgpCard::openWo)
      .def("open",           &PgpCard::open)
      .def("openMask",       &PgpCard::openMask)
      .def("close",          &PgpCard::close)
      .def("getWriteBuffer", &PgpCard::getWriteBuffer, return_value_policy<reference_existing_object>())
      .def("write",          &PgpCard::write)
      .def("read",           &PgpCard::read, return_value_policy<reference_existing_object>())
      .def("retBuffer",      &PgpCard::retBuffer)
      .def("getInfo",        &PgpCard::getInfo, return_value_policy<reference_existing_object>())
      .def("getPciStatus",   &PgpCard::getPciStatus, return_value_policy<reference_existing_object>())
      .def("getLaneStatus",  &PgpCard::getLaneStatus, return_value_policy<reference_existing_object>())
      .def("getEvrControl",  &PgpCard::getEvrControl, return_value_policy<reference_existing_object>())
      .def("setEvrControl",  &PgpCard::setEvrControl)
      .def("getEvrStatus",   &PgpCard::getEvrStatus, return_value_policy<reference_existing_object>())
      .def("setLoop",        &PgpCard::setLoop)
      .def("setData",        &PgpCard::setData)
      .def("sendOpCode",     &PgpCard::sendOpCode)
   ;

   class_<PgpCardStream, bases<StreamSrc,StreamDest,PgpCard>, boost::noncopyable >("PgpCardStream",init<>());

*/

};

