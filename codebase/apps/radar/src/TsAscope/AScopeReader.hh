/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
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
#include <radar/IwrfTsReader.hh>

#include "AScope.hh"
#include "Params.hh"

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

//Q_DECLARE_METATYPE(AScope::TimeSeries)
  
class AScopeReader : public QObject
{

  Q_OBJECT

public:
    
  /// Constructor
  /// @param host The server host
  /// @param port The server port
  /// @param fmqPath - set in FMQ mode
  AScopeReader(const Params &params,
               AScope &scope);
  
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

  const Params &_params;

  int _radarId;
  int _burstChan;

  std::string _serverHost;
  int _serverPort;
  std::string _serverFmq;
  bool _simulMode;

  AScope &_scope;
  
  // read in data

  IwrfTsReader *_pulseReader;
  bool _haveChan1;
  int _dataTimerId;

  // pulse stats

  int _nSamples;
  int _pulseCount;
  
  // info and pulses

  vector<IwrfTsPulse *> _pulses; // SIM mode, or when H/V flag is 1
  vector<IwrfTsPulse *> _pulsesV; // when H/V flag is 0

  // xmit mode

  typedef enum {
    CHANNEL_MODE_HV_SIM,
    CHANNEL_MODE_V_ONLY,
    CHANNEL_MODE_ALTERNATING
  } channelMode_t;
  channelMode_t _channelMode;

  // sequence number for time series to ascope

  size_t _tsSeqNum;

  // methods
  
  int _readData();
  IwrfTsPulse *_getNextPulse();
  void _sendDataToAScope();
  int _loadTs(int nGates,
              int channelIn,
              const vector<IwrfTsPulse *> &pulses,
              int channelOut,
              AScope::FloatTimeSeries &ts);
  int _loadBurst(const IwrfTsBurst &burst,
                 int channelOut,
                 AScope::FloatTimeSeries &ts);

};


#endif /*ASCOPEREADER_H_*/
