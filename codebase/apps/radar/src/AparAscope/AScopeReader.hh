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
#include <deque>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/apar_ts_data.h>
#include <radar/AparTsInfo.hh>
#include <radar/AparTsPulse.hh>
#include <radar/AparTsReader.hh>

#include "AScope.hh"
#include "Params.hh"

/// A Time series reader for the AScope. It reads APAR data and translates
/// DDS samples to AScope::TimeSeries.

Q_DECLARE_METATYPE(AScope::TimeSeries)
  
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
  AScope &_scope;
  
  std::string _serverHost;
  int _serverPort;
  std::string _serverFmq;

  // read in data

  AparTsReader *_pulseReader;
  bool _haveChan1;
  int _dataTimerId;
  int _radarId;
  
  // pulse stats

  size_t _nSamples;
  int _pulseCount;
  
  // info and pulses

  deque<AparTsPulse *> _pulsesHCo;
  deque<AparTsPulse *> _pulsesVCo;
  deque<AparTsPulse *> _pulsesHx;
  deque<AparTsPulse *> _pulsesVx;
  
  // sequence number for time series to ascope

  size_t _tsSeqNum;

  // methods
  
  int _readData();
  AparTsPulse *_getNextPulse();
  void _trimPulseQueues();
  void _sendDataToAScope();
  int _loadTs(deque<AparTsPulse *> &pulses,
              int channelOut,
              AScope::FloatTimeSeries &ts);
};


#endif /*ASCOPEREADER_H_*/
