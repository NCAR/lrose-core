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
#ifndef TSREADER_HH_
#define TSREADER_HH_

#include <QObject>
#include <QMetaType>
#include <QTimerEvent>

#include <string>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_data.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsBurst.hh>
#include <radar/IwrfTsReader.hh>

#include "AScopeManager.hh"

class TsReader : public QObject
{
  
  Q_OBJECT
  
public:
  
  /// The timeseries type for importing data. The actual data
  /// are passed by reference, hopefully eliminating an
  /// unnecessary copy.
  class TimeSeries {
  public:
    // Data types we deal with. 
    enum TsDataTypeEnum { VOIDDATA, FLOATDATA, SHORTDATA };
    /*
     * The default constructor sets dataType to VOIDDATA, and this 
     * value must be set to the correct type by the user before trying 
     * to extract data using the i() and q() methods.
     */
    TimeSeries();
    TimeSeries(TsDataTypeEnum type);
    // Get I values by pulse number and gate.
    inline double i(int pulse, int gate) const;
    // Get I values by pulse number and gate.
    inline double q(int pulse, int gate) const;
    /// I and Q for each beam is in a vector containing I,Q for each gate.
    /// IQbeams contains pointers to each IQ vector for all
    /// of the beams in the timeseries. The length of the timeseries
    /// can be found from IQbeams.size(). The data types pointed to
    /// are defined by our dataType.
    std::vector<void*> IQbeams;
    /// Data type of the pointers in IQbeams
    TsDataTypeEnum dataType;
    /// The number of gates
    int gates;
    /// The channel id
    int chanId;
    /// The sample rate, in Hz
    double sampleRateHz;
    /// An opaque pointer that can be used to store
    /// anything that the caller wants to track along 
    /// with the TimeSeries. This will be useful when 
    /// the TimeSeries is returned to the owner,
    /// if for example when an associated object such as a
    /// DDS sample needs to be returned to DDS.
    void* handle;
  };
  
  /// TimeSeries subclasses for short* and float* data pointers
  class ShortTimeSeries : public TimeSeries {
  public:
    ShortTimeSeries() : TimeSeries(TimeSeries::SHORTDATA) {}
  };
  
  class FloatTimeSeries : public TimeSeries {
  public:
    FloatTimeSeries() : TimeSeries(TimeSeries::FLOATDATA) {}
  };
  
  /// Constructor
  /// @param host The server host
  /// @param port The server port
  /// @param fmqPath - set in FMQ mode
  TsReader(const std::string &host,
           int port,
           const std::string &fmqPath,
           bool simulMode,
           AScopeManager &ascope,
           int radarId,
           int burstChan,
           int debugLevel);

  /// Destructor
  virtual ~TsReader();
  
  signals:

  /// This signal provides an item that falls within
  /// the desired bandwidth specification.
  /// @param pItem A pointer to the item.
  /// It must be returned via returnItem().
    
  void newItem(TimeSeries pItem);

public slots:

  /// Use this slot to return an item
  /// @param pItem the item to be returned.

  void returnItemSlot(TimeSeries pItem);
  
  // respond to timer events
  
  void timerEvent(QTimerEvent *event);
    
protected:

private:

  int _radarId;
  int _burstChan;
  int _debugLevel;

  std::string _serverHost;
  int _serverPort;
  std::string _serverFmq;
  bool _simulMode;

  AScopeManager &_scope;
  
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
              FloatTimeSeries &ts);

  int _loadBurst(const IwrfTsBurst &burst,
                 int channelOut,
                 FloatTimeSeries &ts);

};

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to TsReader::TimeSeries.

Q_DECLARE_METATYPE(TsReader::TimeSeries)
  
#endif /* TSREADER_HH_*/
