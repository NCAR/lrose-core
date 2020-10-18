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
#ifndef SCOPEPLOT_H_
#define SCOPEPLOT_H_ 1

#ifndef QT_STATIC_CONST
#define QT_STATIC_CONST static const
#endif

#include <qwt/qwt.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_scale_engine.h>
#include "ui_ScopePlot.hh"
#include "ScrollZoomer.hh"

#include <vector>

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

/// Render time series and power spectrum data, using Qwt.
/// Three functions are provided, to display passed data 
/// as either time series, I versus Q, or power spectra.
/// The display will reconfigure itself when the two different
/// display functions are called in succesion. Thus no configuration
/// or setup calls are necessary.
/// 
/// This code was unabashedly lifted from the Qwt examples/realtime_plot
/// code.
class DLL_EXPORT ScopePlot: public QWidget, private Ui::ScopePlot
{
	Q_OBJECT
public:
   /// Plot type
   enum PLOTTYPE {TIMESERIES=0, IANDQ=1, IVSQ=2, SPECTRUM=3, PRODUCT=4};

   ScopePlot(QWidget *parent);

   virtual ~ScopePlot();

   /// The size hint.
   virtual QSize sizeHint() const;

   /// Draw a time series plot.
   /// @param Y The vector of y values for display.
   /// @param scaleMin The minimum value to set the y scale to.
   /// @param scaleMax The maximum value to set the y scale to.
   /// @param sampleRateHz The rate of the data samples, in Hz
   /// @param xLabel The label for the x axis. Leave
   /// empty if no label is required.
   /// @param yLabel The label for the y axis. Leave
   /// empty if no label is required.
   void TimeSeries(std::vector<double>& Y,
      double scaleMin,
      double scaleMax,
      double sampleRateHz,
      std::string xLabel="",
      std::string yLabel="");

   /// Draw a time series plot  of I and Q traces.
   /// @param I The I data.
   /// @param Q The Q data
   /// @param scaleMin The minimum value to set the y scale to.
   /// @param scaleMax The maximum value to set the y scale to.
   /// @param sampleRateHz The rate of the data samples, in Hz
   /// @param xLabel The label for the x axis. Leave 
   /// empty if no label is required.
   /// @param yLabel The label for the y axis. Leave 
   /// empty if no label is required.
   void IandQ(std::vector<double>& I, 
      std::vector<double>& Q, 
      double scaleMin, 
      double scaleMax,
      double sampleRateHz,
      std::string xLabel="", 
      std::string yLabel="");

   /// Draw a plot of I versus Q.
   /// @param I The I data.
   /// @param Q The Q data
   /// @param scaleMin The minimum value to set the y scale to.
   /// @param scaleMax The maximum value to set the y scale to.
   /// @param sampleRateHz The rate of the data samples, in Hz
   /// @param xLabel The label for the x axis. Leave 
   /// empty if no label is required.
   /// @param yLabel The label for the y axis. Leave 
   /// empty if no label is required.
   void IvsQ(std::vector<double>& I, 
      std::vector<double>& Q, 
      double scaleMin, 
      double scaleMax,
      double sampleRateHz,
      std::string xLabel="", 
      std::string yLabel="");

   /// Draw a plot of the power spectrum. The data values
   /// are expected to be in dB, and so both x and y scales are
   /// linear.
   /// @param power The power values, ranging from negative through
   /// zero to positive frequencies.
   /// @param scaleMin The minimum value to set the y scale to.
   /// @param scaleMax The maximum value to set the y scale to.
   /// @param sampleRateHz The rate of the data samples, in Hz
   /// @param logYaxis The scale of the y-axis. TRUE=log, FALSE= linear 
   /// @param xLabel The label for the x axis. Leave 
   /// empty if no label is required.
   /// @param yLabel The label for the y axis. Leave 
   /// empty if no label is required.
   void Spectrum(std::vector<double>& power, 
      double scaleMin, 
      double scaleMax,
      double sampleRateHz,
	  bool logYaxis,	
      std::string xLabel="", 
      std::string yLabel="");

   /// Draw a plot of a product. The data values
   /// are expected to be linear, and so both x and y scales are
   /// linear.
   /// @param productData The product data values, ranging from negative through
   /// zero to positive depending on the particular product.
   /// @param productType used by application: here used solely to signal redraw of axis labels
   /// @param scaleMin The minimum value to set the y scale to.
   /// @param scaleMax The maximum value to set the y scale to.
   /// @param sampleRateHz The rate of the data samples, in Hz
   /// @param xLabel The label for the x axis. Leave 
   /// empty if no label is required.
   /// @param yLabel The label for the y axis. Leave 
   /// empty if no label is required.
   void Product(std::vector<double>& productData, 
      int productType,
      double scaleMin, 
      double scaleMax,
      double sampleRateHz,
      std::string xLabel="", 
      std::string yLabel="");

   /// Save a screenshot of the image
   /// @param filePath The path where the image wil be saved.
   void saveImageToFile(std::string filePath);

public slots:

   /// Enable the X grid
   /// @param tf  True to enable, false otherwise
   void enableXgrid(bool tf);

   /// Enable the Y grid
   /// @param tf  True to enable, false otherwise
   void enableYgrid(bool tf);

   /// Stop updating the display.
   /// @param tf True to enable, false otherwise
   void pause(bool tf);

protected:

   /// Remove existing curves, create (empty) new ones,
   /// add redraw the plot.
   void initCurve();

   /// Reconfigure plot to display time series.
   /// @param n The number of points in the time series.
   /// @param scaleMin The y scale minimum.
   /// @param scaleMax The y scale maximum.
   /// @param sampleRateHz The sample rate in Hz
   void configureForTimeSeries(int n, 
      double scaleMin, 
      double scaleMax,
      double sampleRateHz);

   /// Reconfigure plot to display I and Q plot.
   /// @param n The number of points in the time series.
   /// @param scaleMin The y scale minimum.
   /// @param scaleMax The y scale maximum.
   /// @param sampleRateHz The sample rate in Hz
   void configureForIandQ(int n,
      double scaleMin,
      double scaleMax,
      double sampleRateHz);

   /// Reconfigure plot to display I versus Q.
   /// @param scaleMin The x and y scale minimum.
   /// @param scaleMax The x and y scale maximum.
   void configureForIvsQ(double scaleMin, double scaleMax);

   /// Reconfigure plot to display I versus Q.
   /// @param n The number of points in the time series.
   /// @param scaleMin The y scale minimum.
   /// @param scaleMax The y scale maximum.
   /// @param sampleRateHz The sample rate, in Hz
   /// @param logYaxis If true, display using a log y axis.
   void configureForSpectrum(int n,
      double scaleMin, 
      double scaleMax,
      double sampleRateHz,
      bool logYaxis = true);

   /// Reconfigure plot to display a product.
   /// @param n The number of points in the power data.
   /// @param scaleMin The y scale minimum.
   /// @param scaleMax The y scale maximum.
   /// @param sampleRateHz The sample rate, in Hz
   /// @param newProductType The requested product type. Keep track 
   /// of this so that we can set the zoom base if the product type changes.
   void configureForProduct(int n,
      double scaleMin, 
      double scaleMax,
      double sampleRateHz,
	  int newProductType);

   /// Label the axes
   /// @param xLabel Label for x
   /// @param yLabel Label for y
   void labelAxes(std::string xLabel, std::string yLabel);

   /// Curve id for the main plot.
   QwtPlotCurve* _curveId1;

   /// Curve id if we have a second curve, such as 
   /// the time series plot.
   QwtPlotCurve* _curveId2;

   /// The grid
   QwtPlotGrid* _grid;

   // The zoomer
   ScrollZoomer* _zoomer;

   /// Type of current plot display.
   PLOTTYPE _plotType;

   /// Product type to display
   int _productType; 

   /// The current minimum scale. Use it to determine if
   /// we need reconfigure the display when the requested
   /// scale changes with the incoming plot request.
   double _scaleMin;

   /// The current maximum scale. Use it to determine if
   /// we need reconfigure the display when the requested
   /// scale changes with the incoming plot request.
   double _scaleMax;

   /// The current sample rate in Hz, Used to determine
   /// axis scaling
   double _sampleRateHz;

   /// Pre-calculated xaxis data. For repeating plots of
   /// time series and power spectra, when the number of
   /// data points doesn't change, we can use the x axis values
   /// over again.
   std::vector<double> _xdata;
	/// set true to pause the plot
   bool _paused;
   /// Current timeseries x axis label
   std::string _timeSeriesXlabel;
   /// Current timeseries y axis label
   std::string _timeSeriesYlabel;
   /// Current I/Q x axis label
   std::string _iqXlabel;
   /// Current I/Q y axis label
   std::string _iqYlabel;
   /// Current I/Q x axis label
   std::string _spectrumXlabel;
   /// Current I/Q y axis label
   std::string _spectrumYlabel;
   /// Current product x axis label
   std::string _productXlabel;
   /// Current product y axis label
   std::string _productYlabel;

};

#endif // SCOPEPLOT_H_

