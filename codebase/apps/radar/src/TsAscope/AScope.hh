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

/////////////////////////////////////////////////////////////
// AScope.cc
//
// Originally from Charlie Martin
//
// EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////

#ifndef ASCOPE_HH_
#define ASCOPE_HH_

#include <iostream>
#include <cstdlib>

#include <QWidget>
#include <QPalette>
#include <QButtonGroup>

#include <qevent.h>
#include <deque>
#include <set>
#include <map>
#include <fftw3.h>

// Components from the QtToolbox

#include "ScopePlot.hh"
#include "Knob.hh"

// The designer generated header file.
#include "ui_AScope.hh"

// PlotInfo knows the characteristics of a plot
#include "PlotInfo.hh"

#include "Params.hh"

// AScope provides a traditional real-time Ascope display of
// eldora time series data and computed products. It is implemented
// with Qt, and uses the QtToolbox::ScopePlot as the primary display.
// I&Q, I versus Q, IQ power spectrum, and computed product displays
// are available. The data can be displayed either along the beam
// for all gates, or in time for one gate. Users may select the
// fft block size and the gate to be displayed.

// AScope is simply a data consumer; it does not know
// anything about the data provider. Signals and slots
// used to coordinate with other components.

// It is the responsibility of the data provider to feed data
// at a desired rate. AScope will attempt to render all data
// delivered.

// There are two data areas, _I/_Q and _fftwData, that are dynamically
// resized depending upon the operational mode of the scope.

// _I and _Q contain the I/Q values that are being analyzed. They are
// filled as data is delivered to newTSItemSlot. They will be either
// be the same as the selected block size, if operating in fixed gate
// mode, or the number of gates if operating along beam mode.

// _fftwData is sized to the currently selected block size. Thus this will
// match _I/_Q sizes when operating in fixed gate mode. When in along beam mode,
// this probably not match the number of gates. The _fftwData will be zero padded
// if it is larger than _I/_Q size. If smaller, it will just be filled with
// the leading data from _I/_Q.

// Thus, _I/Q gets resized when the block size is changed, if not operating
// in along beam mode. It gets resized to the number of gates,
// when switching into along beam mode. It gets resized to the block size
// when leaving along beam mode.

// Likewise, _fftwData gets resized when the block size changes. A new
// fftwPlan gets created at the same time.

// The processing of incoming data will be handled differently depending
// upon the type of plot that is currently selected. For instance, if
// a time series or I vs Q  plot by gate is chosen, I and Q are collected
// along the specified gate and displayed. If a power spectrum plot is
// chosen, the I and Q data are collected and then a power spectrum is
// computed. And so on. This work is done in newTSItemSlot(), and then
// the collected data are sent on to proper display method.

// A small QFrame in the controls area is provided for users to add their
// own status widgets, branding, etc.

class AScope : public QWidget, private Ui::AScope {
  
Q_OBJECT

  /// Time series plot types.Exactly one of these type
  /// plots will be created.
  enum TS_PLOT_TYPES {
    TS_AMPLITUDE_PLOT,  ///<  time series amplitude plot
    TS_IANDQ_PLOT,      ///<  time series I and Q plot
    TS_IVSQ_PLOT,       ///<  time series I versus Q plot
    TS_SPECTRUM_PLOT    ///<  time series power spectrum plot
  };
  
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
  /// params and parent are passed in

  AScope(const Params &params,
         QWidget* parent = 0);

  /// Destructor
  virtual ~AScope();

  /// @return The user frame, available for adding your
  /// own interface elements. Putting large widgets
  /// in this area will really mess up the overall
  /// layout of the scope.

  QFrame* userFrame();

signals:

  /// emit this signal to alert the client that we
  /// are finished with this item. AScope::TimeSeries
  /// contains an opaque handle that the client can
  /// use to keep track of this item between the 
  /// triggering of newTSItemSlot() and the emitting
  /// of returnTSItem().
  void returnTSItem(AScope::TimeSeries pItem);

public slots:

  /// Feed new timeseries data via this slot.
  /// @param pItem This contains some metadata and pointers to I/Q data
  void newTSItemSlot(AScope::TimeSeries pItem);

  /// Call when the plot type is changed. This function
  /// must determine which of the two families of
  /// plots, _tsPlotInfo, or _productPlotInfo, the
  /// previous and new plot types belong to.
  void plotTypeSlot();

  /// call to save the current plotting parameters for the
  /// current plot type, and reload the parameters for the
  /// the new plot type.
  void plotTypeChange(PlotInfo* pi, TS_PLOT_TYPES plotType);

  /// A different tab has been selected. Change the plot type to the
  /// currently selected button on that tab.
  void tabChangeSlot(int index);

  /// The gain knob value has changed.
  virtual void gainChangeSlot(double);

  /// slide the plot up.
  virtual void upSlot();

  /// Slide the plot down.
  virtual void dnSlot();

  /// Initiate an autoscale. A flag is set; during the next
  /// pulse reception an autoscale computation is made.
  virtual void autoScaleSlot();

  /// Save the scope display to a PNG file.
  void saveImageSlot();

  /// Pause the plotting. Any received data are ignored.
  /// @param p True to enable pause.
  void pauseSlot(bool p);

  /// Select the channel
  /// @param b The button
  void channelSlot(QAbstractButton *b);

  /// Select the gate
  /// @param index The index from the combo box of the selected gate.
  void setGateNumber();
  void setGateNumber(int val);

  /// Select the block size
  /// @param size The block size. It must be a power of two.
  void blockSizeSlot(int size);

  /// Enable/disable windowing
  void windowSlot(bool);

  /// Select long beam display
  void alongBeamSlot(bool);

  /// Get the current block size
  unsigned int getBlockSize() const { return _blockSize; }

protected:

  /// Initialize the block size choices. The minimum
  /// size will be 32. The max size will be 4096.
  /// @todo allow the max (and min?) block sizes to
  /// be confiurable.
  void initBlockSizes();

  /// Allocate the fftw space and create then plan.
  /// Existing space and plan are returned first.
  /// Set up the hammimg window coefficients.
  /// @param size The fft length.
  void initFFT(int size);

  /// Set the number of gates
  /// @param gates The number of gates.
  void setNGates(int gates);

  /// Initialize the channel selection
  /// @param channels The number of channels.
  void initChans(int channels);

  /// Emit a signal announcing the desired gate mode,
  /// either along beam, or one gate. The channel select,
  /// gate choice and (for one gate mode) data block
  /// size will be part of the emitted signal.
  void dataMode();

  /// Send the data for the current plot type to the ScopePlot.
  void displayData();

  /// Setup the hamming coefficients.
  /// @param size The number of coefficients.
  void hammingSetup(int size);

  /// Apply the hamming filter.
  void doHamming();

  /// Autoscale based on a set of data.
  /// @param data The data series to be analyzed.
  /// @param displayType The type of plot that the data is scaled for.
  void autoScale(std::vector<double>& data,
                 TS_PLOT_TYPES displayType);

  /// Autoscale based on two sets of data.
  /// @param data1 The first data series to be analyzed.
  /// @param data2 The second data series to be analyzed.
  /// @param displayType The type of plot that the data is scaled for.
  void autoScale(std::vector<double>& data1,
                 std::vector<double>& data2,
                 TS_PLOT_TYPES displayType);

  /// Initialize the combo box choices and FFTs.
  /// @param channels The number of channels,
  /// @param gates The number of gates
  void initCombos(int channels);

  /// Adjust the _graphRange and _graphOffset values.
  /// @param min Desired scale minimum
  /// @param max Desired scale maximum
  /// @param displayType The type of plot that the data is scaled for.
  void adjustGainOffset(double min,
                        double max,
                        TS_PLOT_TYPES displayType);

  /// Process time series data.
  /// @param Idata The I values
  /// @param Qdata The Q values
  void processTimeSeries(std::vector<double>& Idata,
                         std::vector<double>& Qdata);

  /// Compute the power spectrum. The input values will come
  /// I[]and Q[], the power spectrum will be written to
  /// _spectrum[]
  /// @param Idata The I time series.
  /// @param Qdata The Q time series.
  /// @return The zero moment
  double powerSpectrum(std::vector<double>& Idata,
                       std::vector<double>& Qdata);

  /// initialize all of the book keeping structures
  /// for the various plots.
  void initPlots();

  /// add a ts tab to the plot type selection tab widget.
  /// Radio buttons are created for all of specified
  /// plty types, and grouped into one button group.
  /// _tsPlotInfo provides the label information for
  /// the radio buttons.
  /// @param tabName The title for the tab.
  /// @param types A set of the desired TS_PLOT_TYPES types
  /// @return The button group that the inserted buttons
  /// belong to.
  QButtonGroup* addTSTypeTab(std::string tabName,
                             std::set<TS_PLOT_TYPES> types);

  /// Calculate the zeroth moment, using the time
  /// series for input.
  double zeroMomentFromTimeSeries(std::vector<double>& I,
                                  std::vector<double>& Q);

  // The builtin timer will be used to calculate beam statistics.
  void timerEvent(QTimerEvent*);

  const Params &_params;
  
  /// For each TS_PLOT_TYPES, there will be an entry in this map.
  std::map<TS_PLOT_TYPES, PlotInfo> _tsPlotInfo;

  /// This set contains PLOTTYPEs for all timeseries plots
  std::set<TS_PLOT_TYPES> _timeSeriesPlots;

  /// save the button group for each tab,
  /// so that we can find the selected button
  /// and change the plot type when tabs are switched.
  std::vector<QButtonGroup*> _tabButtonGroups;

  /// This set contains PLOTTYPEs for all raw data plots
  std::set<TS_PLOT_TYPES> _pulsePlots;

  /// Holds Y data to display for  TimeSeries display
  std::vector<double> Y;

  /// Holds I data to display for  I vs. Q
  std::vector<double> I;
  /// Holds Q data to display for I vs. Q display
  std::vector<double> Q;

  /// Holds power spectrum values for display.
  std::vector<double> _spectrum;

  // how often to update the display
  double _refreshRateHz;

  /// Set true when a plot is chosen which shows results
  /// from IQ data. If a plot of products is chosen,
  /// it is false.
  bool _IQplot;

  /// The current selected plot type.
  TS_PLOT_TYPES _tsPlotType;

  /// The hamming window coefficients
  std::vector<double> _hammingCoefs;

  /// The possible block/fftw size choices.
  std::vector<int> _blockSizeChoices;

  ///	The fftw plan. This is a handle used by
  ///	the fftw routines.
  //std::vector<fftw_plan> _fftwPlan;
  fftw_plan _fftwPlan;

  ///	The fftw data array. The fft will
  //	be performed in place, so both input data
  ///	and results are stored here.
  //std::vector<fftw_complex*> _fftwData;
  fftw_complex* _fftwData;

  //	power correction factor applied to (uncorrected) powerSpectrum() output
  double _powerCorrection;

  /// The current block size
  unsigned int _blockSize;

  /// Set true if the Hamming window should be applied
  bool _doHamming;

  /// The button group for channel selection
  QButtonGroup* _chanButtonGroup;

  /// Palette for making the leds green
  QPalette _greenPalette;

  /// Platette for making the leds red
  QPalette _redPalette;

  /// Set true if the plot graphics are paused
  bool _paused;

  /// The signal power, computed directly from the I&Q
  /// data, or from the power spectrum
  double _zeroMoment;

  /// The choice of channels (0-3)
  int _channel;

  /// The selected gate, zero based.
  int _gateNum;

  /// Set false to cause initialization of blocksize and 
  /// gate choices when the first data is received.
  bool _combosInitialized;

  /// Set true if data are to be taken along the beam. Otherwise
  /// data are taken at the specified gate
  bool _alongBeam;

  ///	cumulative error count
  int _errorCount[3];

  ///  last pulse number
  long long _lastPulseNum[3];
  double _knobGain;
  double _knobOffset;
  double _xyGraphRange;
  double _xyGraphCenter;
  double _specGraphRange;
  double _specGraphCenter;

  // storage to collect incoming I values
  std::vector<double> _I;

  // storage to collect incoming Q values
  std::vector<double> _Q;

  // the next index of the incoming location to fill in _I and _Q
  unsigned int _nextIQ;

  /// The number of gates. Initially zero, it is diagnosed from the data stream
  int _nGates;

  /// set true when we want to start capturing the next incoming data
  bool _capture;

  /// The directory where images are saved.
  std::string _saveDir;

  /// The sample rate in Hz.
  double _sampleRateHz;

};


#endif /*PROFSCOPE_H_*/
