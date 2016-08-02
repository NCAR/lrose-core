/** @page cp2scope-overview The CP2Scope Program

CP2Scope provides a traditional real-time Ascope display of CP2 pulse
and beam data. Both of thes products are read from the network.
CP2Scope is configured via CP2Scope.ini.
**/

#ifndef CP2SCOPE_H
#define CP2SCOPE_H

#ifdef WIN32
#include <winsock2.h>		//	no redefinition errors if before Qt includes?
#endif
#include <QDialog>
#include <QPalette>
#include <qevent.h>
#include <deque>
#include <set>
#include <map>

#include "CP2UdpSocket.h"
#include "CP2Config.h"

// request this much space for the socket receive buffer
#define CP2SCOPE_RCVBUF 25000000

// the fastest fft in the west; used for power spectrum calcs.
#include <fftw3.h>

// The base class created from the designer .ui specification
#include "ui_CP2scope.h"

// Coponents from the QtToolbox
#include <ScopePlot.h>

// CP2 timeseries network transfer protocol.
#include "CP2Net.h"
using namespace CP2Net;

// PlotInfo knows the characteristics of a plot
#include "PlotInfo.h"

#define PIRAQ3D_SCALE	1.0/(unsigned int)pow(2,31.0)	

// non-product plot types:
enum	PLOTTYPE {	
	S_TIMESERIES,	///< S time series
	XH_TIMESERIES,	///< Xh time series
	XV_TIMESERIES,	///< Xv time series
	S_IQ,			///< S IQ
	XH_IQ,			///< Xh IQ
	XV_IQ,			///< Xv IQ
	S_SPECTRUM,		///< S spectrum 
	XH_SPECTRUM,	///< Xh spectrum 
	XV_SPECTRUM,	///< Xv spectrum 
}; 

// types of plots available
enum SCOPEPLOTTYPE {
	TIMESERIES,
	IVSQ,
	SPECTRUM,
	PRODUCT
};


/// Provides a real time display of either pulse data or a product
/// extracted from beams. Only a fraction of the incoming data stream is
/// displayed, since the human eye could not discern the whole bandwidth,
/// and would use up the cpu anyway.
class CP2Scope : public QDialog, public Ui::CP2Scope {
	Q_OBJECT
public:
	CP2Scope(QDialog* parent = 0);
	~CP2Scope();

public slots:
	/// Call when data is available on the pulse data socket.
	void newPulseSlot();
	/// Call when data is available on the product data socket.
	void newProductSlot();
	/// Call when the plot type is changed. This function 
	/// must determine which of the two families of
	/// plots, _pulsePlotInfo, or _prodPlotInfo, the
	/// previous and new plot types belong to.
	virtual void plotTypeSlot(int plotType);
	/// call to save the current plotting parameters for the
	/// current plot type, and reload the parameters for the 
	/// the new plot type. It handles both pulse and beam
	/// displays. pulsePlot is used to differentiate between the
	/// two.
	void plotTypeChange(PlotInfo* pi, 
					   PLOTTYPE plotType, 
					   PRODUCT_TYPES prodType, 
					   bool pulsePlot);
	/// A different tab has been selected. Change the plot type to the
	/// currently selected button on that tab.
	void tabChangeSlot(QWidget* w);
    /// The gain knob value has changed.
	virtual void gainChangeSlot( double );	
	/// slide the plot up.
	virtual void upSlot();
	/// Slide the plot down.
	virtual void dnSlot();
	/// Initiate an autoscale. A flag is set; during the next 
	/// pulse reception an autoscale computation is made.
	virtual void autoScaleSlot();
	/// Save the scope display to a PNG file.
	void saveImageSlot();

protected:	
	/// Send the data for the current plot type to the ScopePlot.
	void displayData(); 
	/// Initialize the pulse and product sockets. The
	/// notifiers will be created, and connected
	/// to the data handling slots.
	void initSockets(); 
	/// The socket that pulse data is received on.
	CP2UdpSocket*      _pPulseSocket;
	/// The port for the pulse data.
	int				 _pulsePort;
	/// The buffer for incoming pulse datagrams
	std::vector<char> _pPulseSocketBuf;
	/// The socket that product data is received on.
	CP2UdpSocket*       _pProductSocket;
	/// The port for the product data.
	int				  _productPort;
	/// The buffer for incoming product datagrams
	std::vector<char> _pProductSocketBuf;
	///	board source of data CP2 PIRAQ 1-3 
	int				  _dataChannel;					
	///	prior cumulative pulse count, used for throughput calcs
	int				  _prevPulseCount[3];		
	///	cumulative pulse count
	int				  _pulseCount[3];		
	///	cumulative error count
	int				  _errorCount[3];		
	///  last pulse number
	long long		  _lastPulseNum[3];		
	///    set true when fifo eof occurs. Used so that we don't keep setting the fifo eof led.
	bool			  _eof[3];						
	///	decimation factor for along range (DATA_SET_PULSE) display: currently set 50
	unsigned int	  _pulseDecimation;
	///	decimation factor for products display: currently set 50
	unsigned int	  _productDecimation;	
 
	double          _knobGain;
	double          _knobOffset;
	double			_xyGraphRange;	
	double			_xyGraphCenter;
	double			_specGraphRange;	
	double			_specGraphCenter;
	double			_xFullScale;	
	/// Set true to cause an autoscale 
	/// to take place on the next data series
	bool _performAutoScale;
	/// set the _graphRange and _graphOffset based
	/// on the single data series.
	/// @param data The data series to be analyzed.
	void autoScale(std::vector<double>& data);
	/// set the _graphRange and _graphOffset based
	/// on the two data series.
	/// @param data1 The first data series to be analyzed.
	/// @param data2 The second data series to be analyzed.
	void autoScale(std::vector<double>& data1, std::vector<double>& data2);
	/// set the _specGraphRange and _specGraphCenter based
	/// on the single data series.
	/// @param data The spectral series to be analyzed.
	void specAutoScale(std::vector<double>& data);
	/// Adjust the _graphRange and _graphOffset values.
	/// @param min Desired scale minimum
	/// @param max Desired scale maximum
	void adjustGainOffset(double min, double max);
	/// Holds I data from a pulse for time series and I vs. Q 	
	std::vector<double> I;
	/// Holds Q data from a pulse for time series and I vs. Q display
	std::vector<double> Q;
	/// Used to collect the spectrum values calculated from pulses
	std::vector<double> _spectrum;
	/// Used to collect product data from beams
	std::vector<double> _ProductData;
	// how often to update the statistics (in seconds)
	int _statsUpdateInterval;
	/// Set true if raw plots are selected, false for product type plots
	bool _pulsePlot;
	/// The current selected plot type.
	PLOTTYPE        _pulsePlotType;
	/// The current selected product type.
	PRODUCT_TYPES   _productPlotType;

	// The builtin timer will be used to calculate beam statistics.
	void timerEvent(QTimerEvent*);

	/// The hamming window coefficients
	std::vector<double> _hammingCoefs;

	///	The fftw plan. This is a handle used by
	///	the fftw routines.
	fftw_plan _fftwPlan;

	///	The fftw data array. The fft will
	//	be performed in place, so both input data 
	///	and results are stored here.
	fftw_complex* _fftwData;

	//	fixed block size for initial cut: 
	unsigned int _fftBlockSize;

	//	power correction factor applied to (uncorrected) powerSpectrum() output
	double	_powerCorrection;	///< approximate power correction to dBm 

	/// Set true if the Hamming window should be applied
	bool _doHamming;

	double _az;

	/// Process pulse data.
	/// @param pPulse The pulse to be processed. 
	void processPulse(CP2Pulse* pPulse);

	/// Process product data
	/// @param pPulse The pulse to be processed. 
	void processProduct(CP2Product* pProduct);
	/// Counter of time series, used for decimating 
	/// the timeseries (and I/Q and Spectrum)
	/// display updates.
	int _tsDisplayCount; 
	/// Counter of product cacluations, used
	/// for decimating the product display updates.
	int _productDisplayCount; 

	/// Compute the power spectrum. The input values will come
	/// I[]and Q[], the power spectrum will be written to 
	/// _spectrum[]
	/// @return The zero moment
	double powerSpectrum();

	/// For each PLOTTYPE, there will be an entry in this map.
	std::map<PLOTTYPE, PlotInfo> _pulsePlotInfo;

	/// For each PRODUCT_TYPES, there will be an entry in this map.
	std::map<PRODUCT_TYPES, PlotInfo> _prodPlotInfo;

	/// This set contains PLOTTYPEs for all timeseries plots
	std::set<PLOTTYPE> _timeSeriesPlots;

	/// This set contains PLOTTYPEs for all raw data plots
	std::set<PLOTTYPE> _pulsePlots;

	/// This set contains PLOTTYPEs for all S band moments plots
	std::set<PRODUCT_TYPES> _sMomentsPlots;

	/// This set contains PLOTTYPEs for all X band moments plots
	std::set<PRODUCT_TYPES> _xMomentsPlots;

	/// save the button group for each tab,
	/// so that we can find the selected button
	/// and change the plot type when tabs are switched.
	std::vector<QButtonGroup*> _tabButtonGroups;

	/// initialize all of the book keeping structures
	/// for the various plots.
	void initPlots();
	/// add a rw plot tab to the plot type selection tab widget.
	/// Radio buttons are created for all of specified
	/// plty types, and grouped into one button group.
	/// _pulsePlotInfo provides the label information for
	/// the radio buttons.
	/// @param tabName The title for the tab.
	/// @param types A set of the desired PLOTTYPE types 
	/// @return The button group that the inserted buttons
	/// belong to.
	QButtonGroup* addProductTypeTab(std::string tabName, std::set<PRODUCT_TYPES> types);
	/// add a products tab to the plot type selection tab widget.
	/// Radio buttons are created for all of specified
	/// plty types, and grouped into one button group.
	/// _pulsePlotInfo provides the label information for
	/// the radio buttons.
	/// @param tabName The title for the tab.
	/// @param types A set of the desired PLOTTYPE types 
	/// @return The button group that the inserted buttons
	/// belong to.
	QButtonGroup* addPlotTypeTab(std::string tabName, std::set<PLOTTYPE> types);
	/// The configuration for CP2Scope
	CP2Config _config;
	/// Palette for making the leds green
	QPalette _greenPalette;
	/// Platette for making the leds red
	QPalette _redPalette;

};

#endif
