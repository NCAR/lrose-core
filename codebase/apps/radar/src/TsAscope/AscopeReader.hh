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
#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include <QObject>
#include <QMetaType>

#include <string>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_data.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsBurst.hh>

#include "AScope.h"

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

Q_DECLARE_METATYPE(AScope::TimeSeries)
  
class AScopeReader : public QObject
{

  Q_OBJECT

public:
    
  /// Constructor
  /// @param host The server host
  /// @param port The server port
    AScopeReader(const std::string &host, int port,
                 AScope &scope, int debugLevel);

  /// Destructor
  virtual ~AScopeReader();
  
  signals:

  /// This signal provides an item that falls within
  /// the desired bandwidth specification.
  /// @param pItem A pointer to the item.
  /// It must be returned via returnItem().
    
  void newItem(AScope::TimeSeries pItem);

public slots:

  /// Use this slot to return an item
  /// @param pItem the item to be returned.

  void returnItemSlot(AScope::TimeSeries pItem);
  
  // respond to timer events
  
  void timerEvent(QTimerEvent *event);
    
protected:

private:

  int _debugLevel;

  std::string _serverHost;
  int _serverPort;

  AScope &_scope;
  
  // communication via socket

  Socket _sock;
  time_t _lastTryConnectTime;
  int _sockTimerId;
  bool _timedOut;
  
  // pulse stats

  int _nSamples;
  int _pulseCount;
  int _pulseCountSinceSync;
  
  // info and pulses

  IwrfTsInfo _info;
  IwrfTsBurst _burst;
  vector<IwrfTsPulse *> _pulsesH; // when H/V flag is 1
  vector<IwrfTsPulse *> _pulsesV; // when H/V flag is 0

  // xmit mode

  typedef enum {
    XMIT_MODE_H_ONLY,
    XMIT_MODE_V_ONLY,
    XMIT_MODE_ALTERNATING
  } xmitMode_t;
  xmitMode_t _xmitMode;

  // sequence number for time series to ascope

  size_t _tsSeqNum;

  // methods
  
  int _readFromServer();
  int _readPacket(int &id, int &len, MemBuf &buf);
  int _peekAtBuffer(void *buf, int nbytes);
  void _addPulse(const MemBuf &buf);
  void _setBurst(const MemBuf &buf);
  void _sendDataToAScope();
  void _loadTs(int nGates,
               int channelIn,
               const vector<IwrfTsPulse *> &pulses,
               int channelOut,
               AScope::FloatTimeSeries &ts);


};


#endif /*ASCOPEREADER_H_*/
