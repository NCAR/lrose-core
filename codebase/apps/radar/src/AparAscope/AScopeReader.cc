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

#include "AScopeReader.hh"
#include <cerrno>
#include <radar/apar_ts_functions.hh>
#include <toolsa/uusleep.h>
using namespace std;

// Note that the timer interval for QtTSReader is 0

AScopeReader::AScopeReader(const Params &params,
                           AScope &scope):
        _params(params),
        _scope(scope)
        
{
  
  _radarId = _params.radar_id;
  _serverHost = _params.input_tcp_host;
  _serverPort = _params.input_tcp_port;
  _serverFmq = _params.input_fmq_url;
  _pulseCount = 0;
  _tsSeqNum = 0;
  
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();
  
  // start timer for checking socket every 50 msecs
  _dataTimerId = startTimer(50);

  // pulse reader
  
  if (_serverFmq.size() > 0) {
    _pulseReader = new AparTsReaderFmq(_serverFmq.c_str());
  } else {
    _pulseReader = new AparTsReaderTcp(_serverHost.c_str(), _serverPort);
  }
  if (_radarId != 0) {
    _pulseReader->setRadarId(_radarId);
  }
  _haveChan1 = false;
  _pulseReader->setNonBlocking(50);

}

AScopeReader::~AScopeReader()

{

  // free up the pulses
  
  for (size_t ii = 0; ii < _pulsesHCo.size(); ii++) {
    delete _pulsesHCo[ii];
  }
  _pulsesHCo.clear();

  for (size_t ii = 0; ii < _pulsesVCo.size(); ii++) {
    delete _pulsesVCo[ii];
  }
  _pulsesVCo.clear();

  for (size_t ii = 0; ii < _pulsesHx.size(); ii++) {
    delete _pulsesHx[ii];
  }
  _pulsesHx.clear();

  for (size_t ii = 0; ii < _pulsesVx.size(); ii++) {
    delete _pulsesVx[ii];
  }
  _pulsesVx.clear();

  if (_pulseReader) {
    delete _pulseReader;
  }

}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void AScopeReader::timerEvent(QTimerEvent *event)
{

  if (event->timerId() == _dataTimerId) {

    if (_params.debug) {
      cerr << "Servicing socket timer event" << endl;
    }

    // read data from server, until enough data is gathered

    if (_readData() == 0) {
      _sendDataToAScope();
    }

  } // if (event->timerId() == _dataTimerId)
    
}

/////////////////////////////
// read data from the server
// returns 0 on succes, -1 on failure (not enough data)

int AScopeReader::_readData()

{

  // read data until nSamples pulses have been gathered
  
  _nSamples = _scope.getBlockSize();
  _trimPulseQueues();
  
  MemBuf buf;
  while (true) {
    
    // read in a pulse
    
    AparTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      // No data yet; return to event loop
      return -1;
    }
    _pulseCount++;
    if (pulse->getIq0() == NULL) {
      cerr << "WARNING - pulse has NULL data" << endl;
      continue;
    }

    if (pulse->getHvFlag()) {
      // Horizontal
      if (pulse->getChanIsCopol(0)) {
        _pulsesHCo.push_back(pulse);
      } else {
        _pulsesHx.push_back(pulse);
      }
    } else {
      // Vertical
      if (pulse->getChanIsCopol(0)) {
        _pulsesVCo.push_back(pulse);
      } else {
        _pulsesVx.push_back(pulse);
      }
    }

    // check we have enough data in the active channel
    
    int chanInUse = _scope.getChannel();
    if (chanInUse == 1) {
      if (_pulsesVCo.size() >= _nSamples) {
        return 0;
      }
    } else if (chanInUse == 2) {
      if (_pulsesHx.size() >= _nSamples) {
        return 0;
      }
    } else if (chanInUse == 3) {
      if (_pulsesVx.size() >= _nSamples) {
        return 0;
      }
    } else {
      if (_pulsesHCo.size() >= _nSamples) {
        return 0;
      }
    }

  } // while 

  return -1;

}

///////////////////////////////////////////////////////
// get next pulse
//
// returns NULL on failure

AparTsPulse *AScopeReader::_getNextPulse()

{
  
  AparTsPulse *pulse = NULL;
  
  while (pulse == NULL) {
    pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      if (_pulseReader->getTimedOut()) {
	// No data yet; return to event loop
	return NULL;
      }
      if (_pulseReader->endOfFile()) {
	cout << "# NOTE: end of file encountered" << endl;
      }
      return NULL;
    }
  }

  if (_pulseReader->endOfFile()) {
    cout << "# NOTE: end of file encountered" << endl;
  }
  if (pulse->getIq1() != NULL) {
    _haveChan1 = true;
  } else {
    _haveChan1 = false;
  }

  return pulse;

}

///////////////////////////////////////////////////////
// trim the pulse queues

void AScopeReader::_trimPulseQueues()

{

  if (_pulsesHCo.size() > _nSamples - 1) {
    AparTsPulse *pulse = _pulsesHCo.front();
    delete pulse;
    _pulsesHCo.pop_front();
  }

  if (_pulsesVCo.size() > _nSamples - 1) {
    AparTsPulse *pulse = _pulsesVCo.front();
    delete pulse;
    _pulsesVCo.pop_front();
  }

  if (_pulsesHx.size() > _nSamples - 1) {
    AparTsPulse *pulse = _pulsesHx.front();
    delete pulse;
    _pulsesHx.pop_front();
  }

  if (_pulsesVx.size() > _nSamples - 1) {
    AparTsPulse *pulse = _pulsesVx.front();
    delete pulse;
    _pulsesVx.pop_front();
  }

}

///////////////////////////////////////////////////////
// send data to the AScope

void AScopeReader::_sendDataToAScope()

{

  // check we have enough data in the active channel
  
  int chanInUse = _scope.getChannel();
  if (chanInUse == 1) {
    AScope::FloatTimeSeries tsChan;
    if (_loadTs(_pulsesVCo, chanInUse, tsChan) == 0) {
      emit newItem(tsChan);
    }
  } else if (chanInUse == 2) {
    AScope::FloatTimeSeries tsChan;
    if (_loadTs(_pulsesHx, chanInUse, tsChan) == 0) {
      emit newItem(tsChan);
    }
  } else if (chanInUse == 3) {
    AScope::FloatTimeSeries tsChan;
    if (_loadTs(_pulsesVx, chanInUse, tsChan) == 0) {
      emit newItem(tsChan);
    }
  } else {
    AScope::FloatTimeSeries tsChan;
    if (_loadTs(_pulsesHCo, chanInUse, tsChan) == 0) {
      emit newItem(tsChan);
    }
  }

}

///////////////////////////////////////////////
// load up time series object

int AScopeReader::_loadTs(const deque<AparTsPulse *> &pulses,
                          int channelOut,
                          AScope::FloatTimeSeries &ts)
  
{

  if (pulses.size() < 2) return -1;

  // set max number of gates
  
  size_t maxNGates = 0;
  for (size_t ii = 0; ii < pulses.size(); ii++) {
    const AparTsPulse* pulse = pulses[ii];
    size_t nGatesPulse = pulse->getNGates();
    if (nGatesPulse > maxNGates) {
      maxNGates = nGatesPulse;
    }
  }
  
  // set header

  ts.gates = maxNGates;
  ts.chanId = channelOut;
  ts.sampleRateHz = 1.0 / pulses[0]->getPrt();
  
  // set sequence number
  
  size_t *seq0 = new size_t;
  *seq0 = _tsSeqNum;
  ts.handle = seq0;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Creating ts data, seq num: " << _tsSeqNum << endl;
  }
  _tsSeqNum++;

  // load IQ data

  for (size_t ii = 0; ii < pulses.size(); ii++) {
    
    const AparTsPulse* pulse = pulses[ii];
    int nGatesPulse = pulse->getNGates();
    
    fl32 *iq = new fl32[maxNGates * 2];
    memset(iq, 0, maxNGates * 2 * sizeof(fl32));
    if (pulse->getIq0()) {
      memcpy(iq, pulse->getIq0(), nGatesPulse * 2 * sizeof(fl32));
    } else if (pulse->getIq1()) {
      memcpy(iq, pulse->getIq1(), nGatesPulse * 2 * sizeof(fl32));
    }
    ts.IQbeams.push_back(iq);
    
  } // ii

  return 0;
  
}
    
//////////////////////////////////////////////////////////////////////////////
// Clean up when iq data is returned from display

void AScopeReader::returnItemSlot(AScope::TimeSeries ts)

{

  size_t *seqNum = (size_t *) ts.handle;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--->> Freeing ts data, seq num: " << *seqNum << endl;
  }
  delete seqNum;
  
  for (size_t ii = 0; ii < ts.IQbeams.size(); ii++) {
    delete[] (fl32 *) ts.IQbeams[ii];
  }
  
}

