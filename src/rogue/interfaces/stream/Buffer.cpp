/**
 *-----------------------------------------------------------------------------
 * Title      : Stream Buffer Container
 * ----------------------------------------------------------------------------
 * File       : Buffer.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2016-09-16
 * Last update: 2016-09-16
 * ----------------------------------------------------------------------------
 * Description:
 * Stream frame container
 * Some concepts borrowed from CPSW by Till Straumann
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

#include <rogue/interfaces/stream/Buffer.h>
#include <rogue/interfaces/stream/Slave.h>
#include <boost/make_shared.hpp>

namespace ris = rogue::interfaces::stream;

//! Class creation
/*
 * Pass owner, raw data buffer, and meta data
 */
ris::BufferPtr ris::Buffer::create ( ris::SlavePtr source, void * data, uint32_t meta, uint32_t rawSize) {
   ris::BufferPtr buff = boost::make_shared<ris::Buffer>(source,data,meta,rawSize);
   return(buff);
}

//! Create a buffer.
/*
 * Pass owner, raw data buffer, and meta data
 */
ris::Buffer::Buffer(ris::SlavePtr source, void *data, uint32_t meta, uint32_t rawSize) {
   source_   = source;
   data_     = (uint8_t *)data;
   meta_     = meta;
   rawSize_  = rawSize;
   headRoom_ = 0;
   count_    = 0;
   error_    = 0;
}

//! Destroy a buffer
/*
 * Owner return buffer method is called
 */
ris::Buffer::~Buffer() {
   source_->retBuffer(data_,meta_,rawSize_);
   data_ = NULL;
}

//! Get raw data pointer
uint8_t * ris::Buffer::getRawData() {
   return(data_);
}

//! Get payload data pointer
uint8_t * ris::Buffer::getPayloadData() {
   if ( data_ == NULL ) return(NULL);
   else return(data_ + headRoom_);
}

//! Get meta data
uint32_t ris::Buffer::getMeta() {
   return(meta_);
}

//! Set meta data
void ris::Buffer::setMeta(uint32_t meta) {
   meta_ = meta;
}

//! Get raw size
uint32_t ris::Buffer::getRawSize() {
   return(rawSize_);
}

//! Get buffer data count (payload + headroom)
uint32_t ris::Buffer::getCount() {
   return(count_);
}

//! Get header space
uint32_t ris::Buffer::getHeadRoom() {
   return(headRoom_);
}

//! Get available size for payload
uint32_t ris::Buffer::getAvailable() {
   uint32_t temp;

   temp = rawSize_ - count_;

   if ( temp < headRoom_) return(0);
   else return(temp - headRoom_);
}

//! Get real payload size
uint32_t ris::Buffer::getPayload() {
   if ( count_ < headRoom_ ) return(0);
   else return(count_ - headRoom_);
}

//! Get flags
uint32_t ris::Buffer::getFlags() {
   return(flags_);
}

//! Set error state
void ris::Buffer::setFlags(uint32_t flags) {
   error_ = flags;
}

//! Get error state
uint32_t ris::Buffer::getError() {
   return(error_);
}

//! Set error state
void ris::Buffer::setError(uint32_t error) {
   error_ = error;
}

//! Set size including header
void ris::Buffer::setSize(uint32_t size) {
   count_ = size;
}

//! Set head room
void ris::Buffer::setHeadRoom(uint32_t offset) {
   headRoom_ = offset;
}

//! Read up to count bytes from buffer, starting from offset.
uint32_t ris::Buffer::read  ( void *p, uint32_t offset, uint32_t count ) {
   uint32_t rcnt;

   // Empty buffer
   if ( data_ == NULL ) return(0);

   // No data in buffer
   if ( count_ < headRoom_ ) return(0);

   // Read offset is larger than payload
   if ( offset >= (count_ - headRoom_) ) return(0);

   // Adjust read count for payload size and offset
   if ( count > ((count_ - headRoom_) - offset) ) 
      rcnt = ((count_ - headRoom_) - offset);
   else rcnt = count;

   // Copy data
   memcpy(p,data_+headRoom_+offset,rcnt);
   return(rcnt);
}

//! Write count bytes to frame, starting at offset
uint32_t ris::Buffer::write ( void *p, uint32_t offset, uint32_t count ) {
   uint32_t wcnt;

   // Empty buffer
   if ( data_ == NULL ) return(0);

   if ( offset >= (rawSize_ - headRoom_) ) return(0);

   if ( count > ((rawSize_ - headRoom_) - offset)) 
      wcnt = ((rawSize_ - headRoom_) - offset);
   else wcnt = count;

   // Set payload size to last write
   count_ = offset + headRoom_ + wcnt;
   memcpy(data_+headRoom_+offset,p,wcnt);
   return(wcnt);
}

void ris::Buffer::setup_python() {
   // Nothing to do
}

