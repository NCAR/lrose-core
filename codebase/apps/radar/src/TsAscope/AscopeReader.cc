// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include "AScopeReader.h"
#include <cerrno>
#include <radar/iwrf_functions.hh>
using namespace std;

// Note that the timer interval for QtTSReader is 0

AScopeReader::AScopeReader(const string &host,
                           int port,
                           AScope &scope,
                           int debugLevel):
        _debugLevel(debugLevel),
        _serverHost(host),
        _serverPort(port),
        _scope(scope),
        _lastTryConnectTime(0),
        _pulseCount(0),
        _pulseCountSinceSync(0),
        _tsSeqNum(0)
{
  
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();
  
  // start timer for checking socket every 50 msecs
  _sockTimerId = startTimer(50);

  // pulse mode

  _xmitMode = XMIT_MODE_H_ONLY;

}

AScopeReader::~AScopeReader()

{

  // close socket if open

  if (_sock.isOpen()) {
    if (_debugLevel) {
      cerr << "Closing socket to IWRF data server" << endl;
    }
    _sock.close();
  }

}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void AScopeReader::timerEvent(QTimerEvent *event)
{

  if (event->timerId() == _sockTimerId) {

    if (_debugLevel > 0) {
      cerr << "Servicing socket timer event" << endl;
    }

    if (!_sock.isOpen()) {
      // try opening, once per second
      time_t now = time(NULL);
      if (now == _lastTryConnectTime) {
        return;
      }
      _lastTryConnectTime = now;
      if (_sock.open(_serverHost.c_str(), _serverPort)) {
        int errNum = errno;
        cerr << "ERROR AScopeReader::timerEvent" << endl;
        cerr << "  Cannot open socket to IWRF data server" << endl;
        cerr << "  host: " << _serverHost << endl;
        cerr << "  port: " << _serverPort << endl;
        cerr << "  " << strerror(errNum) << endl;
        return;
      }
      if (_debugLevel) {
        cerr << "INFO - AScopeReader::timerEvent" << endl;
        cerr << "Opened socket to IWRF data server" << endl;
        cerr << "  host: " << _serverHost << endl;
        cerr << "  port: " << _serverPort << endl;
      }
    }

    // read data from server, until enough data is gathered

    if (_readFromServer() == 0) {
      _sendDataToAScope();
    }

  } // if (event->timerId() == _sockTimerId)
    
}

/////////////////////////////
// read data from the server
// returns 0 on succes, -1 on failure (not enough data)

int AScopeReader::_readFromServer()

{

  // read data until nSamples pulses have been gathered
  
  _nSamples = _scope.getBlockSize();

  MemBuf buf;
  while (true) {
    
    // read packet from time series server
    
    int packetId, packetLen;
    if (_readPacket(packetId, packetLen, buf)) {
      cerr << "ERROR - AScopeReader::_readFromServer" << endl;
      return -1;
    }
    if (_timedOut) {
      return -1;
    }
    
    // handle packet types

    if (packetId == IWRF_PULSE_HEADER_ID) {

      // add pulse to vector

      _addPulse(buf);

    } else if (packetId == IWRF_BURST_HEADER_ID) {

      // add pulse to vector
      
      _setBurst(buf);

    } else {

      // set the ops info appropriately

      _info.setFromBuffer(buf.getPtr(), buf.getLen());

    }

    // check we have enough data
    
    if ((int) _pulsesH.size() >= _nSamples) {

      if (_pulsesV.size() == 0) {
        // H data only
        _xmitMode = XMIT_MODE_H_ONLY;
        return 0;
      } else if ((int) _pulsesV.size() >= _nSamples) {
        // Equal number H and V implies alternating data
        _xmitMode = XMIT_MODE_ALTERNATING;
        return 0;
      }

    } else if ((int) _pulsesV.size() >= _nSamples) {
      
      if (_pulsesH.size() == 0) {
        // V data only
        _xmitMode = XMIT_MODE_V_ONLY;
        return 0;
      } else if ((int) _pulsesH.size() >= _nSamples) {
        // Equal number H and V implies alternating data
        _xmitMode = XMIT_MODE_ALTERNATING;
        return 0;
      }

    }

  } // while 

  return -1;

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int AScopeReader::_readPacket(int &id, int &len, MemBuf &buf)

{
  bool haveGoodHeader = false;
  si32 packetId;
  si32 packetLen;
  si32 packetTop[2];
  _timedOut = false;

  while (!haveGoodHeader) {

    // peek at the first 8 bytes

    if (_peekAtBuffer(packetTop, sizeof(packetTop))) {
      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      _sock.close();
      return -1;
    }

    if (_timedOut) {
      return 0;
    }
    
    // check ID for packet, and get its length
    
    packetId = packetTop[0];
    packetLen = packetTop[1];

    if (iwrf_check_packet_id(packetId, packetLen)) {

      // out of order, so close and return error
      // this will force a reconnection and resync

      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      cerr << " Incoming data stream out of sync" << endl;
      cerr << " Closing socket" << endl;
      _sock.close();
      return -1;

    } else {

      haveGoodHeader = true;
      id = packetId;
      len = packetLen;

    }

  } // while (!haveGoodHeader)
    
  // read packet in

  buf.reserve(packetLen);
  if (_sock.readBuffer(buf.getPtr(), packetLen, 50)) {
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      _timedOut = true;
      return 0;
    } else {
      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      _sock.close();
      return -1;
    }
  }
  
  if (id == IWRF_PULSE_HEADER_ID) {
    _pulseCount++;
    _pulseCountSinceSync++;
  }
    
  if (_debugLevel > 2) {

    iwrf_packet_print(stderr, buf.getPtr(), buf.getLen());

  } else if (_debugLevel > 1) {

    if (id == IWRF_PULSE_HEADER_ID) {
      cerr << "Read in PULSE packet, id, len, count: " << ", "
           << iwrf_packet_id_to_str(id) << ", "
           << packetLen << ", "
           << _pulseCount << endl;
    } else {
      cerr << "Read in TCP packet, id, len: "
           << iwrf_packet_id_to_str(id) << ", "
           << packetLen << endl;
      if(id == IWRF_SYNC_ID) {
        if(_pulseCount > 0) {
          cerr << "N pulses since last sync: " << _pulseCountSinceSync << endl;
          _pulseCountSinceSync = 0;  
        }
      } else {
        iwrf_packet_print(stderr, buf.getPtr(), buf.getLen());
      }
      
    } // if (id == IWRF_PULSE_HEADER_ID)

  } // if (_debugLevel > 2) 
    
  return 0;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure
// _timedOut set in case of timeout

int AScopeReader::_peekAtBuffer(void *buf, int nbytes)

{

  _timedOut = false;

  // peek with no wait
  if (_sock.peek(buf, nbytes, 50) == 0) {
    return 0;
  } else {
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      // no data available
      _timedOut = true;
      return 0;
    } else {
      cerr << "ERROR - AScopeReader::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////
// add pulse to queue

void AScopeReader::_addPulse(const MemBuf &buf)

{
  
  // create a new pulse
  
  IwrfTsPulse *pulse = new IwrfTsPulse(_info);
  
  // set the data on the pulse, as floats

  pulse->setFromBuffer(buf.getPtr(), buf.getLen(), true);

  // add to vector based on H/V flag

  int hvFlag = pulse->get_hv_flag();
  if (hvFlag) {
    _pulsesH.push_back(pulse);
  } else {
    _pulsesV.push_back(pulse);
  }

}
      
///////////////////////////////////////////////////////
// set the burst

void AScopeReader::_setBurst(const MemBuf &buf)

{
  
  // set the data on the pulse, as floats
  
  _burst.setFromBuffer(buf.getPtr(), buf.getLen(), true);

}
      
///////////////////////////////////////////////////////
// send data to the AScope

void AScopeReader::_sendDataToAScope()

{

  // compute max gates and channels
  
  int nGates = 0;
  int nChannels = 0;
  
  for (size_t ii = 0; ii < _pulsesH.size(); ii++) {
    const IwrfTsPulse *pulse = _pulsesH[ii];
    int nGatesPulse = pulse->getNGates();
    if (nGatesPulse > nGates) {
      nGates = nGatesPulse;
    }
    int nChannelsPulse = pulse->getNChannels();
    if (nChannelsPulse > nChannels) {
      nChannels = nChannelsPulse;
    }
  } // ii
  
  for (size_t ii = 0; ii < _pulsesV.size(); ii++) {
    const IwrfTsPulse *pulse = _pulsesV[ii];
    int nGatesPulse = pulse->getNGates();
    if (nGatesPulse > nGates) {
      nGates = nGatesPulse;
    }
    int nChannelsPulse = pulse->getNChannels();
    if (nChannelsPulse > nChannels) {
      nChannels = nChannelsPulse;
    }
  } // ii

  if (_xmitMode == XMIT_MODE_H_ONLY) {

    // load H chan 0, send to scope

    AScope::FloatTimeSeries tsChan0;
    _loadTs(nGates, 0, _pulsesH, 0, tsChan0);
    emit newItem(tsChan0);

    // load H chan 1, send to scope

    AScope::FloatTimeSeries tsChan1;
    _loadTs(nGates, 1, _pulsesH, 1, tsChan1);
    emit newItem(tsChan1);

    // load H burst, send to scope as chan 2

    AScope::FloatTimeSeries tsChan2;
    _loadTs(nGates, 2, _pulsesH, 2, tsChan2);
    emit newItem(tsChan2);

  } else if (_xmitMode == XMIT_MODE_V_ONLY) {

    // load V chan 0, send to scope
    
    AScope::FloatTimeSeries tsChan0;
    _loadTs(nGates, 0, _pulsesV, 0, tsChan0);
    emit newItem(tsChan0);

    // load V chan 1, send to scope

    AScope::FloatTimeSeries tsChan1;
    _loadTs(nGates, 1, _pulsesV, 1, tsChan1);
    emit newItem(tsChan1);
    
    // load V burst, send to scope as chan 3
    
    AScope::FloatTimeSeries tsChan3;
    _loadTs(nGates, 3, _pulsesV, 3, tsChan3);
    emit newItem(tsChan3);

  } else {

    // alternating mode, 4 channels

    // load H chan 0, send to scope

    AScope::FloatTimeSeries tsChan0;
    _loadTs(nGates, 0, _pulsesH, 0, tsChan0);
    emit newItem(tsChan0);

    // load H chan 1, send to scope
    
    AScope::FloatTimeSeries tsChan1;
    _loadTs(nGates, 1, _pulsesH, 1, tsChan1);
    emit newItem(tsChan1);

    // load V chan 0, send to scope as chan 2

    AScope::FloatTimeSeries tsChan2;
    _loadTs(nGates, 0, _pulsesV, 2, tsChan2);
    emit newItem(tsChan2);

    // load V chan 1, send to scope as chan 3

    AScope::FloatTimeSeries tsChan3;
    _loadTs(nGates, 1, _pulsesV, 3, tsChan3);
    emit newItem(tsChan3);
    
  }

  // free up the pulses
  
  for (size_t ii = 0; ii < _pulsesH.size(); ii++) {
    delete _pulsesH[ii];
  }
  _pulsesH.clear();

  for (size_t ii = 0; ii < _pulsesV.size(); ii++) {
    delete _pulsesV[ii];
  }
  _pulsesV.clear();

}

///////////////////////////////////////////////
// load up time series object

void AScopeReader::_loadTs(int nGates,
                           int channelIn,
                           const vector<IwrfTsPulse *> &pulses,
                           int channelOut,
                           AScope::FloatTimeSeries &ts)

{
  
  if (pulses.size() < 2) return;

  // set header

  ts.gates = nGates;
  ts.chanId = channelOut;
  ts.sampleRateHz = 1.0 / pulses[0]->get_prt();
  
  // set sequence number

  size_t *seq0 = new size_t;
  *seq0 = _tsSeqNum;
  ts.handle = seq0;
  if (_debugLevel > 1) {
    cerr << "Creating ts data, seq num: " << _tsSeqNum << endl;
  }
  _tsSeqNum++;
  
  // load IQ data

  for (size_t ii = 0; ii < pulses.size(); ii++) {
    
    const IwrfTsPulse *pulse = pulses[ii];
    int nGatesPulse = pulses[ii]->getNGates();
    
    fl32 *iq = new fl32[nGates * 2];
    memset(iq, 0, nGates * 2 * sizeof(fl32));
    if (channelIn == 0) {
      memcpy(iq, pulse->getIq0(), nGatesPulse * 2 * sizeof(fl32));
    } else if (channelIn == 1) {
      if (pulse->getIq1() != NULL) {
        memcpy(iq, pulse->getIq1(), nGatesPulse * 2 * sizeof(fl32));
      }
    } else if (channelIn == 2) {
      if (pulse->getIq2() != NULL) {
        memcpy(iq, pulse->getIq2(), nGatesPulse * 2 * sizeof(fl32));
      }
    }
    ts.IQbeams.push_back(iq);
    
  } // ii
  
}
    
//////////////////////////////////////////////////////////////////////////////
// Clean up when iq data is returned from display

void AScopeReader::returnItemSlot(AScope::TimeSeries ts)

{

  size_t *seqNum = (size_t *) ts.handle;
  if (_debugLevel > 1) {
    cerr << "--->> Freeing ts data, seq num: " << *seqNum << endl;
  }
  delete seqNum;
  
  for (size_t ii = 0; ii < ts.IQbeams.size(); ii++) {
    delete[] (fl32 *) ts.IQbeams[ii];
  }
  
}

