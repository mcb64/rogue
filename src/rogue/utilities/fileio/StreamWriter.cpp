/**
 *-----------------------------------------------------------------------------
 * Title         : Data file writer utility.
 * ----------------------------------------------------------------------------
 * File          : StreamWriter.h
 * Author        : Ryan Herbst <rherbst@slac.stanford.edu>
 * Created       : 09/28/2016
 * Last update   : 09/28/2016
 *-----------------------------------------------------------------------------
 * Description :
 *    Class to coordinate data file writing.
 *    This class supports multiple stream slaves, each with the ability to
 *    write to a common data file. The data file is a series of banks.
 *    Each bank has a channel and frame flags. The channel is per source and the
 *    lower 24 bits of the frame flags are used as the flags.
 *    The bank is preceeded by 2 - 32-bit headers to indicate bank information
 *    and length:
 *
 *       headerA:
 *          [31:0] = Length of data block in bytes
 *       headerB
 *          31:24  = Channel ID
 *          23:o   = Frame flags
 *
 *-----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to 
 * the license terms in the LICENSE.txt file found in the top-level directory 
 * of this distribution and at: 
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
 * No part of the rogue software platform, including this file, may be 
 * copied, modified, propagated, or distributed except according to the terms 
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/
#include <rogue/utilities/fileio/StreamWriterChannel.h>
#include <rogue/utilities/fileio/StreamWriter.h>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/exceptions/OpenException.h>
#include <rogue/exceptions/GeneralException.h>
#include <rogue/exceptions/AllocationException.h>
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <fcntl.h>

namespace ris = rogue::interfaces::stream;
namespace ruf = rogue::utilities::fileio;
namespace bp  = boost::python;
namespace re  = rogue::exceptions;

//! Class creation
ruf::StreamWriterPtr ruf::StreamWriter::create () {
   ruf::StreamWriterPtr s = boost::make_shared<ruf::StreamWriter>();
   return(s);
}

//! Setup class in python
void ruf::StreamWriter::setup_python() {
   bp::class_<ruf::StreamWriter, ruf::StreamWriterPtr, boost::noncopyable >("StreamWriter",bp::init<>())
      .def("create",         &ruf::StreamWriter::create)
      .staticmethod("create")
      .def("open",           &ruf::StreamWriter::open)
      .def("close",          &ruf::StreamWriter::close)
      .def("setBufferSize",  &ruf::StreamWriter::setBufferSize)
      .def("setMaxSize",     &ruf::StreamWriter::setMaxSize)
      .def("getChannel",     &ruf::StreamWriter::getChannel)
      .def("getSize",        &ruf::StreamWriter::getSize)
      .def("getFrameCount",  &ruf::StreamWriter::getFrameCount)
   ;
}

//! Creator
ruf::StreamWriter::StreamWriter() {
   baseName_   = "";
   fd_         = -1;
   sizeLimit_  = 0;
   buffSize_   = 0;
   currSize_   = 0;
   totSize_    = 0;
   buffer_     = NULL;
   frameCount_ = 0;
   currBuffer_ = 0;
}

//! Deconstructor
ruf::StreamWriter::~StreamWriter() { 
   this->close();
}

//! Open a data file
void ruf::StreamWriter::open(std::string file) {
   std::string name;

   baseName_ = file;
   name   = file;
   fdIdx_ = 1;

   if ( sizeLimit_ > 0 ) name.append(".1");

   if ( (fd_ = ::open(name.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) < 0 )
      throw(re::OpenException("StreamWriter::open",name,0));

   totSize_    = 0;
   currSize_   = 0;
   frameCount_ = 0;
   currBuffer_ = 0;
}

//! Close a data file
void ruf::StreamWriter::close() {
   boost::lock_guard<boost::mutex> lock(mtx_);
   flush();
   if ( fd_ >= 0 ) ::close(fd_);
   fd_ = -1;
}

//! Set buffering size, 0 to disable
void ruf::StreamWriter::setBufferSize(uint32_t size) {
   boost::lock_guard<boost::mutex> lock(mtx_);

   // No change
   if ( size == buffSize_ ) return;

   // Flush data out of current buffer
   flush();

   // Free old buffer
   if ( buffer_ != NULL ) free(buffer_);
   buffSize_ = 0;

   // Buffer is not enabled
   if ( size == 0 ) return;

   // Create new buffer
   if ( (buffer_ = (uint8_t *)malloc(size)) == NULL )
      throw(re::AllocationException("StreamWriter::setBufferSize",size));

   buffSize_ = size;
}

//! Set max file size, 0 for unlimited
void ruf::StreamWriter::setMaxSize(uint32_t size) {
   boost::lock_guard<boost::mutex> lock(mtx_);
   sizeLimit_ = size;
}

//! Get a slave port
ruf::StreamWriterChannelPtr ruf::StreamWriter::getChannel(uint8_t channel) {
   return(ruf::StreamWriterChannel::create(shared_from_this(),channel));
}

//! Get current file size
uint32_t ruf::StreamWriter::getSize() {
   boost::lock_guard<boost::mutex> lock(mtx_);
   return(totSize_ + currBuffer_);
}

//! Get current frame count
uint32_t ruf::StreamWriter::getFrameCount() {
   boost::lock_guard<boost::mutex> lock(mtx_);
   return(frameCount_);
}

//! Write data to file. Called from StreamWriterChannel
void ruf::StreamWriter::writeFile ( uint8_t channel, boost::shared_ptr<rogue::interfaces::stream::Frame> frame) {
   ris::FrameIteratorPtr iter;
   uint32_t value;
   uint32_t size;

   boost::lock_guard<boost::mutex> lock(mtx_);

   if ( fd_ < 0 ) return;

   size = frame->getPayload() + 4;

   // Check file size
   checkSize(size);

   // First write size
   intWrite(&size,4);

   // Create EVIO header
   value  = frame->getFlags() & 0xFFFFFF;
   value |= (channel << 24);
   intWrite(&value,4);

   iter = frame->startRead(0,size-4);
   do {
      intWrite(iter->data(),iter->size());
   } while (frame->nextRead(iter));

   // Update counters
   frameCount_ ++;
}

//! Internal method for file writing with buffer and auto close and reopen
void ruf::StreamWriter::intWrite(void *data, uint32_t size) {

   if ( fd_ < 0 ) return;

   // New size is larger than buffer size, flush
   if ( (size + currBuffer_) > buffSize_ ) flush();

   // Attempted write is larger than buffer, raw write
   // This is called if bufer is disabled
   if ( size > buffSize_ ) {
      if (write(fd_,data,size) != size) 
         throw(re::GeneralException("StreamWriter::intWrite","Write failed"));
      currSize_ += size;
      totSize_  += size;
   }

   // Append to buffer if non zero
   else if ( buffSize_ > 0 && size > 0 ) {
      memcpy(buffer_ + currBuffer_, data, size);
      currBuffer_ += size;
   }
}

//! Check file size for next write
void ruf::StreamWriter::checkSize(uint32_t size) {
   std::string name;

   // No Limit
   if ( sizeLimit_ == 0 ) return;

   // Bad configuration
   if ( size > sizeLimit_ ) 
      throw(re::GeneralException("StreamWriter::checkSize","Frame size is larger than file size limit"));

   // File size (including buffer) is larger than max size
   if ( (size + currBuffer_ + currSize_) > sizeLimit_ ) {
      flush();

      // Close and update index
      ::close(fd_);
      fdIdx_++;

      name = baseName_ + "." + boost::lexical_cast<std::string>(fdIdx_);

      // Open new file
      if ( (fd_ = ::open(name.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) < 0 )
         throw(re::OpenException("StreamWriter::checkSize",name,0));

      currSize_ = 0;
   }
}

//! Flush file
void ruf::StreamWriter::flush() {
   if ( currBuffer_ > 0 ) {
      if ( write(fd_,buffer_,currBuffer_) != currBuffer_ )
         throw(re::GeneralException("StreamWriter::flush","Write failed"));
      currSize_ += currBuffer_;
      totSize_  += currBuffer_;
      currBuffer_ = 0;
   }
}

