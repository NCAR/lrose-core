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
#include <radar/iwrf_functions.hh>
#include <toolsa/uusleep.h>
using namespace std;

// Note that the timer interval for QtTSReader is 0

AScopeReader::AScopeReader(const Params &params,
                           AScope &scope):
        _params(params),
        _scope(scope)
        
{
  
  _radarId = _params.radar_id;
  _burstChan = _params.burst_chan;
  _serverHost = _params.input_tcp_host;
  _serverPort = _params.input_tcp_port;
  _serverFmq = _params.input_fmq_url;
  _simulMode = _params.simultaneous_mode;
  _pulseCount = 0;
  _tsSeqNum = 0;
  
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();
  
  // start timer for checking socket every 50 msecs
  _dataTimerId = startTimer(50);

  // pulse mode

  _channelMode = CHANNEL_MODE_HV_SIM;

  // pulse reader

  if (_serverFmq.size() > 0) {
    _pulseReader = new IwrfTsReaderFmq(_serverFmq.c_str());
  } else {
    _pulseReader = new IwrfTsReaderTcp(_serverHost.c_str(), _serverPort);
  }
  if (_radarId != 0) {
    _pulseReader->setRadarId(_radarId);
  }
  _haveChan1 = false;
  _pulseReader->setNonBlocking(50);

}

AScopeReader::~AScopeReader()

{

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
  
  MemBuf buf;
  while (true) {
    
    // read in a pulse
    
    IwrfTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      // No data yet; return to event loop
      return -1;
    }
    _pulseCount++;
    if (pulse->getIq0() == NULL) {
      cerr << "WARNING - pulse has NULL data" << endl;
      continue;
    }

    if (pulse->getIq1() == NULL) {
      
      // single pol mode
      
      _pulses.push_back(pulse);
      _channelMode = CHANNEL_MODE_HV_SIM;
      
    } else {
      
      // add to vector based on H/V flag
      
      if (_simulMode) {
        _pulses.push_back(pulse);
      } else {
        int hvFlag = pulse->get_hv_flag();
        if (hvFlag) {
          _pulses.push_back(pulse);
        } else {
          _pulsesV.push_back(pulse);
        }
      }

    }

    // check we have enough data
    
    if ((int) _pulses.size() >= _nSamples) {
      
      if (_pulsesV.size() == 0) {
        // H data only
        _channelMode = CHANNEL_MODE_HV_SIM;
        return 0;
      } else if ((int) _pulsesV.size() >= _nSamples) {
        // Equal number H and V implies alternating data
        _channelMode = CHANNEL_MODE_ALTERNATING;
        return 0;
      }

    } else if ((int) _pulsesV.size() >= _nSamples) {
      
      if (_pulses.size() == 0) {
        // V data only
        _channelMode = CHANNEL_MODE_V_ONLY;
        return 0;
      } else if ((int) _pulses.size() >= _nSamples) {
        // Equal number H and V implies alternating data
        _channelMode = CHANNEL_MODE_ALTERNATING;
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

IwrfTsPulse *AScopeReader::_getNextPulse()

{
  
  IwrfTsPulse *pulse = NULL;

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
// send data to the AScope

void AScopeReader::_sendDataToAScope()

{

  // compute max gates and channels
  
  int nGates = 0;
  int nChannels = 0;
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    const IwrfTsPulse *pulse = _pulses[ii];
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

  if (_channelMode == CHANNEL_MODE_HV_SIM) {

    // load H chan 0, send to scope

    AScope::FloatTimeSeries tsChan0;
    if (_loadTs(nGates, 0, _pulses, 0, tsChan0) == 0) {
      emit newItem(tsChan0);
    }

    // load H chan 1, send to scope

    AScope::FloatTimeSeries tsChan1;
    if (_loadTs(nGates, 1, _pulses, 1, tsChan1) == 0) {
      emit newItem(tsChan1);
    }

    // load burst, send to scope as chan 2

    AScope::FloatTimeSeries tsChan2;
    if (_loadBurst(_pulseReader->getBurst(), 2, tsChan2) == 0) {
      emit newItem(tsChan2);
    }

  } else if (_channelMode == CHANNEL_MODE_V_ONLY) {

    // load V chan 0, send to scope
    
    AScope::FloatTimeSeries tsChan0;
    if (_loadTs(nGates, 0, _pulsesV, 0, tsChan0) == 0) {
      emit newItem(tsChan0);
    }

    // load V chan 1, send to scope

    AScope::FloatTimeSeries tsChan1;
    if (_loadTs(nGates, 1, _pulsesV, 1, tsChan1) == 0) {
      emit newItem(tsChan1);
    }
    
    // load V burst, send to scope as chan 3
    
    AScope::FloatTimeSeries tsChan3;
    if (_loadBurst(_pulseReader->getBurst(), 3, tsChan3) == 0) {
      emit newItem(tsChan3);
    }

  } else {

    // alternating mode, 4 channels
    // if burst chan is set (>= 0) use specified channel instead of
    // the default channels

    // load H chan 0, (h-co), into channel 0

    if (_burstChan == 0) {
      AScope::FloatTimeSeries tsChan0;
      if (_loadBurst(_pulseReader->getBurst(), 0, tsChan0) == 0) {
        emit newItem(tsChan0);
      }
    } else {
      AScope::FloatTimeSeries tsChan0;
      if (_loadTs(nGates, 0, _pulses, 0, tsChan0) == 0) {
        emit newItem(tsChan0);
      }
    }

    // load H chan 1, (v-cross), into channel 3
    
    if (_burstChan == 3) {
      AScope::FloatTimeSeries tsChan3;
      if (_loadBurst(_pulseReader->getBurst(), 3, tsChan3) == 0) {
        emit newItem(tsChan3);
      }
    } else {
      AScope::FloatTimeSeries tsChan3;
      if (_loadTs(nGates, 1, _pulses, 3, tsChan3) == 0) {
        emit newItem(tsChan3);
      }
    }

    // load V chan 0, (v-co) into channel 1
    
    if (_burstChan == 1) {
      AScope::FloatTimeSeries tsChan1;
      if (_loadBurst(_pulseReader->getBurst(), 1, tsChan1) == 0) {
        emit newItem(tsChan1);
      }
    } else {
      AScope::FloatTimeSeries tsChan1;
      if (_loadTs(nGates, 0, _pulsesV, 1, tsChan1) == 0) {
        emit newItem(tsChan1);
      }
    }

    // load V chan 1, (h_cross) into channel 2

    if (_burstChan == 2) {
      AScope::FloatTimeSeries tsChan2;
      if (_loadBurst(_pulseReader->getBurst(), 2, tsChan2) == 0) {
        emit newItem(tsChan2);
      }
    } else {
      AScope::FloatTimeSeries tsChan2;
      if (_loadTs(nGates, 1, _pulsesV, 2, tsChan2) == 0) {
        emit newItem(tsChan2);
      }
    }
    
  }

  // free up the pulses
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    delete _pulses[ii];
  }
  _pulses.clear();

  for (size_t ii = 0; ii < _pulsesV.size(); ii++) {
    delete _pulsesV[ii];
  }
  _pulsesV.clear();

}

///////////////////////////////////////////////
// load up time series object

int AScopeReader::_loadTs(int nGates,
                          int channelIn,
                          const vector<IwrfTsPulse *> &pulses,
                          int channelOut,
                          AScope::FloatTimeSeries &ts)
  
{

  if (pulses.size() < 2) return -1;

  // set header

  ts.gates = nGates;
  ts.chanId = channelOut;
  ts.sampleRateHz = 1.0 / pulses[0]->get_prt();
  
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
    
    const IwrfTsPulse* pulse = pulses[ii];
    int nGatesPulse = pulse->getNGates();
    
    fl32 *iq = new fl32[nGates * 2];
    memset(iq, 0, nGates * 2 * sizeof(fl32));
    if (channelIn == 0) {
      if (pulse->getIq0()) {
        memcpy(iq, pulse->getIq0(), nGatesPulse * 2 * sizeof(fl32));
      }
    } else if (channelIn == 1) {
      if (pulse->getIq1()) {
        memcpy(iq, pulse->getIq1(), nGatesPulse * 2 * sizeof(fl32));
      }
    }
    ts.IQbeams.push_back(iq);
    
  } // ii

  return 0;
  
}
    
///////////////////////////////////////////////
// load up burst data

int AScopeReader::_loadBurst(const IwrfTsBurst &burst,
                             int channelOut,
                             AScope::FloatTimeSeries &ts)

{
  
  if (burst.getNSamples() < 2) return -1;

  IwrfTsBurst copy(burst);
  copy.convertToFL32();
  if (copy.getIq() == NULL) {
    cerr << "WARNING - burst has null data" << endl;
    return -1;
  }

  // set header

  ts.gates = copy.getNSamples();
  ts.chanId = channelOut;
  ts.sampleRateHz = copy.getSamplingFreqHz();
  
  // set sequence number

  size_t *seq0 = new size_t;
  *seq0 = _tsSeqNum;
  ts.handle = seq0;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Creating burst data, seq num: " << _tsSeqNum << endl;
  }
  _tsSeqNum++;
  
  // load IQ data

  fl32 *iq = new fl32[copy.getNSamples() * 2];
  if (_params.debug >= Params::DEBUG_EXTRA) {
    copy.printHeader(stderr);
    copy.printData(stderr);
  }
  memcpy(iq, copy.getIq(), copy.getNSamples() * 2 * sizeof(fl32));
  ts.IQbeams.push_back(iq);

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

