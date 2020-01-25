#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PyRogue base module - Block Class
#-----------------------------------------------------------------------------
# File       : pyrogue/_Block.py
# Created    : 2017-05-16
#-----------------------------------------------------------------------------
# This file is part of the rogue software platform. It is subject to 
# the license terms in the LICENSE.txt file found in the top-level directory 
# of this distribution and at: 
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
# No part of the rogue software platform, including this file, may be 
# copied, modified, propagated, or distributed except according to the terms 
# contained in the LICENSE.txt file.
#-----------------------------------------------------------------------------
import rogue.interfaces.memory as rim
import threading
import time
import math
import textwrap
import pyrogue as pr
import inspect


class MemoryError(Exception):
    """ Exception for memory access errors."""

    def __init__(self, *, name, address, msg=None, size=0):

        self._value = f"Memory Error for {name} at address {address:#08x}"

        if msg is not None:
            self._value += " " + msg

    def __str__(self):
        return repr(self._value)


class LocalBlock(object):
    def __init__(self, *, variable, localSet, localGet, value):
        self._path      = variable.path
        self._mode      = variable.mode
        self._device    = variable.parent
        self._localSet  = localSet
        self._localGet  = localGet
        self._variable  = variable
        self._variables = [variable] # Used by poller
        self._value     = value
        self._lock      = threading.RLock()
        self._doUpdate  = False

        # Setup logging
        self._log = pr.logInit(cls=self,name=self._path)

    def __repr__(self):
        return repr(self._path)

    @property
    def path(self):
        return self._path

    @property
    def mode(self):
        return self._mode

    @property
    def bulkEn(self):
        return True

    @property
    def stale(self):
        return False

    def forceStale(self):
        pass

    def set(self, var, value):
        with self._lock:

            if isinstance(value, list) or isinstance(value,dict):
                changed = True
            else:
                changed = self._value != value
            self._value = value

            # If a setFunction exists, call it (Used by local variables)
            if self._localSet is not None:

                # Possible args
                pargs = {'dev' : self._device, 'var' : self._variable, 'value' : self._value, 'changed' : changed}

                pr.varFuncHelper(self._localSet, pargs, self._log, self._variable.path)

    def get(self, var):
        if self._localGet is not None:
            with self._lock:

                # Possible args
                pargs = {'dev' : self._device, 'var' : self._variable}

                self._value = pr.varFuncHelper(self._localGet,pargs, self._log, self._variable.path)

        return self._value

    def startTransaction(self, type, check=False, lowByte=None, highByte=None):
        """
        Start a transaction.
        """
        with self._lock:
            self._doUpdate = True

    def checkTransaction(self):
        """
        Check status of block.
        If update=True notify variables if read
        """
        doUpdate = False
        with self._lock:
            doUpdate = self._doUpdate
            self._doUpdate = False

        # Update variables outside of lock
        if doUpdate: 
            self._variable._queueUpdate()

    def _setTimeout(self,value):
        pass

    def _iadd(self, other):
        with self._lock:
            self.set(None, self.get(None) + other)
            self._variable._queueUpdate()

    def _isub(self, other):
        with self._lock:
            self.set(None, self.get(None) - other)
            self._variable._queueUpdate()

    def _imul(self, other):
        with self._lock:
            self.set(None, self.get(None) * other)
            self._variable._queueUpdate()

    def _imatmul(self, other):
        with self._lock:
            self.set(None, self.get(None) @ other)
            self._variable._queueUpdate()

    def _itruediv(self, other):
        with self._lock:
            self.set(None, self.get(None) / other)
            self._variable._queueUpdate()

    def _ifloordiv(self, other):
        with self._lock:
            self.set(None, self.get(None) // other)
            self._variable._queueUpdate()

    def _imod(self, other):
        with self._lock:
            self.set(None, self.get(None) % other)
            self._variable._queueUpdate()

    def _ipow(self, other):
        with self._lock:
            self.set(None, self.get(None) ** other)
            self._variable._queueUpdate()

    def _ilshift(self, other):
        with self._lock:
            self.set(None, self.get(None) << other)
            self._variable._queueUpdate()

    def _irshift(self, other):
        with self._lock:
            self.set(None, self.get(None) >> other)
            self._variable._queueUpdate()

    def _iand(self, other):
        with self._lock:
            self.set(None, self.get(None) & other)
            self._variable._queueUpdate()

    def _ixor(self, other):
        with self._lock:
            self.set(None, self.get(None) ^ other)
            self._variable._queueUpdate()

    def _ior(self, other):
        with self._lock:
            self.set(None, self.get(None) | other)
            self._variable._queueUpdate()


class RemoteBlock(rim.Master):

    def __init__(self, *, memBase, offset, size):
        rim.Master.__init__(self)
        self._setSlave(memBase)
        
        self._variables   = []
        self._path        = "Undefined"
        self._mode        = "RW"
        self._bulkEn      = False
        self._verifyEn    = False
        self._verifyWr    = False
        self._verifyRange = None
        self._sData       = bytearray(size)  # Set data
        self._sDataMask   = bytearray(size)  # Set data mask
        self._bData       = bytearray(size)  # Block data
        self._vData       = bytearray(size)  # Verify data
        self._vDataMask   = bytearray(size)  # Verify data mask
        self._size        = size
        self._offset      = offset
        self._lock        = threading.RLock()
        self._doUpdate    = False

        # Setup Initial logging
        self._log = pr.logInit(cls=self,name=self._path)

    def __repr__(self):
        return repr(self._variables)

    @property
    def path(self):
        return self._path

    @property
    def mode(self):
        return self._mode

    @property
    def bulkEn(self):
        return self._bulkEn

    @property
    def stale(self):
        return any([b != 0 for b in self._sDataMask])

    def forceStale(self):
        for b in self._sDataMask: b = 0xFF

    @property
    def offset(self):
        return self._offset

    @property
    def address(self):
        return self._offset | self._reqAddress()

    @property
    def size(self):
        return self._size

    @property
    def memBaseId(self):
        return self._reqSlaveId()

    def set(self, var, ba):
        """
        Update block with bitSize bits from passed byte array.
        Offset sets the starting point in the block array.
        """
        with self._lock:
            srcBit = 0
            for x in range(len(var.bitOffset)):
                self._copyBits(self._sData, var.bitOffset[x], ba, srcBit, var.bitSize[x])
                self._setBits(self._sDataMask, var.bitOffset[x], var.bitSize[x])
                srcBit += var.bitSize[x]

    def get(self, var, ba):
        """
        Get bitSize bytes from block data.
        Offset sets the starting point in the block array.
        bytearray is returned
        """
        with self._lock:

            dstBit = 0
            for x in range(len(var.bitOffset)):
                if self._anyBits(self._sDataMask, var.bitOffset[x], var.bitSize[x]):
                    self._copyBits(ba, dstBit, self._sData, var.bitOffset[x], var.bitSize[x])
                else:
                    self._copyBits(ba, dstBit, self._bData, var.bitOffset[x], var.bitSize[x])
                dstBit += var.bitSize[x]

    def startTransaction(self, type, check=False, lowByte=None, highByte=None):
        """
        Start a transaction.
        """
        with self._lock:

            #print(f'Called {self.name}.startTransaction(check={check})')

            # Check for invalid combinations
            if (type == rim.Write  and (self.mode == 'RO')) or \
               (type == rim.Post   and (self.mode == 'RO')) or \
               (type == rim.Read   and (self.mode == 'WO')) or \
               (type == rim.Verify and (self.mode == 'WO' or \
                                        self.mode == 'RO' or \
                                        self._verifyWr == False)):
                return

            self._waitTransaction(0)
            self._clearError()

            # Set default low and high bytes
            if lowByte is None or highByte is None:
                lowByte  = 0
                highByte = self._size - 1

            # Move staged write data to block. Clear stale.
            if type == rim.Write or type == rim.Post:
                for x in range(self._size):
                    self._bData[x] = self._bData[x] & (self._sDataMask[x] ^ 0xFF)
                    self._bData[x] = self._bData[x] | (self._sDataMask[x] & self._sData[x])

                    # Override min and max bytes based upon stale
                    if self._sDataMask[x] != 0:
                        if x < lowByte:  lowByte  = x
                        if x > highByte: highByte = x

                self._sData = bytearray(self._size)
                self._sDataMask = bytearray(self._size)

            self._log.debug(f'startTransaction type={type}')
            self._log.debug(f'len bData = {len(self._bData)}, vData = {len(self._vData)}, vDataMask = {len(self._vDataMask)}')

            # Track verify after writes. 
            # Only verify blocks that have been written since last verify
            if type == rim.Write:
                self._verifyWr = self._verifyEn

            # Derive offset and size based upon min transaction size
            offset = math.floor(lowByte / self._reqMinAccess()) * self._reqMinAccess()
            size   = math.ceil((highByte-lowByte+1) / self._reqMinAccess()) * self._reqMinAccess()

            # Setup transaction
            self._verifyRange = [offset,offset+size] if  (type == rim.Verify) else None
            self._doUpdate = True

            # Set data pointer
            tData = self._vData if self._verifyRange is not None else self._bData

            # Start transaction
            self._reqTransaction(self.offset,tData,size,offset,type)

        if check:
            #print(f'Checking {self.path}.startTransaction(check={check})')
            self._checkTransaction()

    def checkTransaction(self):
        
        doUpdate = False
        with self._lock:
            self._waitTransaction(0)

            #print(f'Checking {self.path}._checkTransaction()')            

            # Error
            err = self._getError()
            self._clearError()

            if err != "":
                raise MemoryError(name=self.path, address=self.address, msg=err, size=self._size)

            if self._verifyRange is not None:
                self._verifyWr = False

                for x in range(self._verifyRange[0],self._verifyRange[1]):
                    if (self._vData[x] & self._vDataMask[x]) != (self._bData[x] & self._vDataMask[x]):
                        msg  = "Verify Error: "
                        msg += ('Local='    + ''.join(f'{x:#02x}' for x in self._bData))
                        msg += ('. Verify=' + ''.join(f'{x:#02x}' for x in self._vData))
                        msg += ('. Mask='   + ''.join(f'{x:#02x}' for x in self._vDataMask))

                        raise MemoryError(name=self.path, address=self.address, msg=msg, size=self._size)

               # Updated
            doUpdate = self._doUpdate
            self._doUpdate = False

        # Update variables outside of lock
        if doUpdate:
            for v in self._variables:
                v._queueUpdate()

    def setDefault(self, var, value):
        with self._lock:
            # Stage the default data        
            self.set(var, value)

            # Move stage data to block, but keep it staged as well
            for x in range(self._size):
                self._bData[x] = self._bData[x] & (self._sDataMask[x] ^ 0xFF)
                self._bData[x] = self._bData[x] | (self._sDataMask[x] & self._sData[x])


    def addVariables(self, variables):
        self._variables = variables
        self._path      = variables[0].path
        self._mode      = variables[0].mode

        # Re-setup logging
        self._log = pr.logInit(cls=self,name=self._path)

        # Temp bit masks
        excMask = bytearray(self._size)  # Variable bit mask for exclusive variables
        oleMask = bytearray(self._size)  # Variable bit mask for overlap enabled variables

        # Go through variables
        for var in variables:

            # Variable supports bulk operations
            if var._bulkEn: self._bulkEn = True

            self._log.debug(f"Adding variable {var.name} to block {self.path} at offset {self.offset:#02x}")

            # If variable modes mismatch, set block to read/write
            if var.mode != self._mode:
                self._mode = 'RW'

            # Update variable masks
            for x in range(len(var.bitOffset)):

                # Variable allows overlaps, add to overlap enable mask
                if var._overlapEn:
                    self._setBits(oleMask,var.bitOffset[x],var.bitSize[x])

                # Otherwise add to exclusive mask and check for existing mapping
                else:
                    if self._anyBits(excMask,var.bitOffset[x],var.bitSize[x]):
                        raise MemoryError(name=self.path, address=self.address, msg="Variable bit overlap detected.")
                    self._setBits(excMask,var.bitOffset[x],var.bitSize[x])

                # update verify mask
                if var.mode == 'RW' and var.verify is True:
                    self._verifyEn = True
                    self._setBits(self._vDataMask,var.bitOffset[x],var.bitSize[x])

        # Check for overlaps by anding exclusive and overlap bit vectors
        for b1, b2 in zip(oleMask, excMask):
            if b1 & b2 != 0:
                raise MemoryError(name=self.path, address=self.address, msg="Variable bit overlap detected.")

        # Set exclusive flag
        self._overlapEn = all (excMask[i] == 0 for i in range(self._size))

        # Force block to be stale at startup
        self.forceStale()

