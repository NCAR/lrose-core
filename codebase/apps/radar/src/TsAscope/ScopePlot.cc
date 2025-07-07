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
#include "ScopePlot.hh"
#include <QPainter>
#include <QPixmap>
#include <QStack>

#include <iostream>
#include <stdlib.h>
#include <qpen.h>

#include <qwt/qwt_math.h>
#include <qwt/qwt_symbol.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_plot_marker.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_text.h>
#include <qwt/qwt_scale_widget.h>


//////////////////////////////////////////////////////////////////////////////////

ScopePlot::ScopePlot(QWidget *parent):
        QWidget(parent),
        _curveId1(0),
        _curveId2(0),
        _plotType(IANDQ),
        _scaleMin(0.0),
        _scaleMax(0.0),
        _paused(false)
{
  setupUi(this);

  _qwtPlot->setFrameStyle(QFrame::NoFrame);
  _qwtPlot->setLineWidth(0);
#if (QWT_VERSION < 0x060100)
  _qwtPlot->setCanvasLineWidth(2);
#endif
  _qwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());

  _grid = new QwtPlotGrid();
  _grid->attach(_qwtPlot);

  _qwtPlot->setCanvasBackground(QColor(29, 100, 141)); // nice blue

  // enable zooming

  _zoomer = new ScrollZoomer(_qwtPlot->canvas());
  _zoomer->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
  _zoomer->setTrackerPen(QPen(Qt::white));

}

//////////////////////////////////////////////////////////////////////////////////

ScopePlot::~ScopePlot()
{
  delete _zoomer;
}

//////////////////////////////////////////////////////////////////////////////////

QSize 
  ScopePlot::sizeHint() const
{
  return QSize(540,400);
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::initCurve()
{

  if ( _curveId1 )
  {
    delete _curveId1;
    _curveId1 = 0;
  }

  if ( _curveId2 )
  {
    delete _curveId2;
    _curveId2 = 0;
  }

  _curveId1 = new QwtPlotCurve("Data1");
  _curveId1->attach(_qwtPlot);
  _curveId1->setStyle(QwtPlotCurve::Lines);
  _curveId1->setPen(QPen(Qt::cyan));

  // if we are doing I and Q, then make two curves
  if (_plotType == IANDQ) {
    _curveId2 = new QwtPlotCurve("Data2");
    _curveId2->attach(_qwtPlot);
    _curveId2->setStyle(QwtPlotCurve::Lines);
    _curveId2->setPen(QPen(Qt::red));
  }
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::TimeSeries(std::vector<double>& Y,
                        double scaleMin,
                        double scaleMax,
                        double sampleRateHz,
                        std::string xLabel,
                        std::string yLabel)
{
  if (_paused)
    return;

  if (_plotType     != TIMESERIES ||
      _xdata.size() != Y.size()   ||
      scaleMin      != _scaleMin  ||
      scaleMax      != _scaleMax  ||
      sampleRateHz  != _sampleRateHz ||
      xLabel        != _timeSeriesXlabel ||
      yLabel        != _timeSeriesYlabel )
  {
    configureForTimeSeries(Y.size(), scaleMin, scaleMax, sampleRateHz);
    _timeSeriesXlabel = xLabel;
    _timeSeriesYlabel = yLabel;
    labelAxes(_timeSeriesXlabel, _timeSeriesYlabel);
  }

  initCurve();

  // QwtCurve::setData() API changes a bit as of Qwt 5.3...
#if QWT_VERSION < 0x050300
  _curveId1->setData(&_xdata[0], &Y[0], Y.size());
#else
  QVector<QPointF> ypoints;
  for (unsigned int p = 0; p < Y.size(); p++) {
    ypoints.push_back(QPointF(_xdata[p], Y[p]));
  }
  _curveId1->setData(new QwtPointSeriesData(ypoints));
#endif

  _qwtPlot->replot();
}

//////////////////////////////////////////////////////////////////////////////////

void
  ScopePlot::IandQ(std::vector<double>& I, 
                   std::vector<double>& Q,
                   double scaleMin,
                   double scaleMax,
                   double sampleRateHz,
                   std::string xLabel, 
                   std::string yLabel)
{
  if (_paused)
    return;

  if (_plotType     != IANDQ ||
      _xdata.size() != I.size()   || 
      scaleMin      != _scaleMin  || 
      scaleMax      != _scaleMax  ||
      sampleRateHz  != _sampleRateHz ||
      xLabel        != _timeSeriesXlabel ||
      yLabel        != _timeSeriesYlabel ) 
  {
    configureForIandQ(I.size(), scaleMin, scaleMax, sampleRateHz);
    _timeSeriesXlabel = xLabel;
    _timeSeriesYlabel = yLabel;
    labelAxes(_timeSeriesXlabel, _timeSeriesYlabel);
  }

  initCurve();

  // QwtCurve::setData() API changes a bit as of Qwt 5.3...
#if QWT_VERSION < 0x050300
  _curveId1->setData(&_xdata[0], &I[0], I.size());
  _curveId2->setData(&_xdata[0], &Q[0], Q.size());
#else
  QVector<QPointF> ipoints;
  QVector<QPointF> qpoints;
  for (unsigned int p = 0; p < I.size(); p++) {
    ipoints.push_back(QPointF(_xdata[p], I[p]));
    qpoints.push_back(QPointF(_xdata[p], Q[p]));
  }
  _curveId1->setData(new QwtPointSeriesData(ipoints));
  _curveId2->setData(new QwtPointSeriesData(qpoints));
#endif

  _qwtPlot->replot();
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::IvsQ(std::vector<double>& I, 
                  std::vector<double>& Q,
                  double scaleMin,
                  double scaleMax,
                  double /*sampleRateHz*/,
                  std::string xLabel, 
                  std::string yLabel)
{

  if (_paused)
    return;

  if (_plotType != IVSQ      ||
      scaleMin  != _scaleMin || 
      scaleMax  != _scaleMax ||
      xLabel        != _iqXlabel ||
      yLabel        != _iqYlabel) {
    configureForIvsQ(scaleMin, scaleMax);
    _iqXlabel = xLabel;
    _iqYlabel = yLabel;
    labelAxes(_iqXlabel, _iqYlabel);
  }

  initCurve();

  // QwtCurve::setData() API changes a bit as of Qwt 5.3...
#if QWT_VERSION < 0x050300
  _curveId1->setData(&I[0], &Q[0], I.size());
#else
  QVector<QPointF> points;
  for (unsigned int p = 0; p < I.size(); p++)
    points.push_back(QPointF(I[p], Q[p]));
  _curveId1->setData(new QwtPointSeriesData(points));
#endif

  _qwtPlot->replot();
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::Spectrum(std::vector<double>& power,
                      double scaleMin,
                      double scaleMax,
                      double sampleRateHz,
                      bool logYaxis,
                      std::string xLabel, 
                      std::string yLabel)
{

  if (_paused)
    return;
  if (_plotType != SPECTRUM ||
      _xdata.size() != power.size() || 
      scaleMin      != _scaleMin    || 
      scaleMax      != _scaleMax    ||
      sampleRateHz  != _sampleRateHz ||
      xLabel        != _spectrumXlabel ||
      yLabel        != _spectrumYlabel) {
    configureForSpectrum(power.size(), scaleMin, scaleMax, sampleRateHz, logYaxis);
    _spectrumXlabel = xLabel;
    _spectrumYlabel = yLabel;
    labelAxes(_spectrumXlabel, _spectrumYlabel);
  }

  initCurve();

  // QwtCurve::setData() API changes a bit as of Qwt 5.3...
#if QWT_VERSION < 0x050300
  _curveId1->setData(&_xdata[0], &power[0], power.size());
#else
  QVector<QPointF> points;
  for (unsigned int p = 0; p < power.size(); p++)
    points.push_back(QPointF(_xdata[p], power[p]));
  _curveId1->setData(new QwtPointSeriesData(points));
#endif

  _qwtPlot->replot();
}


//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::Product(std::vector<double>& productData,
                     int productType,
                     double scaleMin,
                     double scaleMax,
                     double sampleRateHz,
                     std::string xLabel, 
                     std::string yLabel)
{

  if (_paused)
    return;
  if (_plotType != PRODUCT ||
      productType	!= _productType || 
      _xdata.size() != productData.size() || 
      scaleMin      != _scaleMin    || 
      scaleMax      != _scaleMax    ||
      sampleRateHz  != _sampleRateHz ||
      xLabel        != _productXlabel ||
      yLabel        != _productYlabel) {
    configureForProduct(productData.size(), scaleMin, scaleMax, sampleRateHz, productType);
    _productXlabel = xLabel;
    _productYlabel = yLabel;
    labelAxes(_productXlabel, _productYlabel);
  }

  initCurve();

  // QwtCurve::setData() API changes a bit as of Qwt 5.3...
#if QWT_VERSION < 0x050300
  _curveId1->setData(&_xdata[0], &productData[0], productData.size());
#else
  QVector<QPointF> points;
  for (unsigned int p = 0; p < productData.size(); p++)
    points.push_back(QPointF(_xdata[p], productData[p]));
  _curveId1->setData(new QwtPointSeriesData(points));
#endif

  _qwtPlot->replot();
}


//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::configureForIandQ(
          int n, 
          double scaleMin, 
          double scaleMax, 
          double sampleRateHz)
{
  // we need to reset the zoom base when the plot type changes
  bool reZoom = (_plotType != IANDQ);

  _plotType = IANDQ;
  _scaleMin = scaleMin;
  _scaleMax = scaleMax;
  _sampleRateHz = sampleRateHz;

  _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());

  _qwtPlot->setAxisScale(QwtPlot::xBottom, 0, n/_sampleRateHz);
  _qwtPlot->setAxisScale(QwtPlot::yLeft, _scaleMin, _scaleMax);

  _xdata.resize(n);

  for (int i = 0; i < n; i++)
    _xdata[i] = i/_sampleRateHz;

  initCurve();

  _qwtPlot->replot();

  if((_zoomer->zoomStack().size() == 1) || reZoom )
    _zoomer->setZoomBase();
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::configureForTimeSeries(
          int n,
          double scaleMin,
          double scaleMax,
          double sampleRateHz)
{
  // we need to reset the zoom base when the plot type changes
  bool reZoom = (_plotType != IANDQ);

  _plotType = TIMESERIES;
  _scaleMin = scaleMin;
  _scaleMax = scaleMax;
  _sampleRateHz = sampleRateHz;

  _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());

  _qwtPlot->setAxisScale(QwtPlot::xBottom, 0, n/_sampleRateHz);
  _qwtPlot->setAxisScale(QwtPlot::yLeft, _scaleMin, _scaleMax);

  _xdata.resize(n);

  for (int i = 0; i < n; i++)
    _xdata[i] = i/_sampleRateHz;

  initCurve();

  _qwtPlot->replot();

  if((_zoomer->zoomStack().size() == 1) || reZoom )
    _zoomer->setZoomBase();
}

//////////////////////////////////////////////////////////////////////////////////

void
  ScopePlot::configureForIvsQ(double scaleMin, 
                              double scaleMax)
{
  // we need to reset the zoom base when the plot type changes
  bool reZoom = (_plotType != IVSQ);

  _plotType = IVSQ;
  _scaleMin = scaleMin;
  _scaleMax = scaleMax;

  _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());

  _qwtPlot->setAxisScale(QwtPlot::xBottom, _scaleMin, _scaleMax);
  _qwtPlot->setAxisScale(QwtPlot::yLeft,   _scaleMin, _scaleMax);

  initCurve();

  _qwtPlot->replot();
  if((_zoomer->zoomStack().size() == 1) || reZoom)
    _zoomer->setZoomBase();

}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::configureForSpectrum(int n, 
                                  double scaleMin, 
                                  double scaleMax, 
                                  double sampleRateHz,
                                  bool logYaxis)
{
  // we need to reset the zoom base when the plot type changes
  bool reZoom = (_plotType != SPECTRUM);

  _plotType = SPECTRUM;
  _scaleMin = scaleMin;
  _scaleMax = scaleMax;
  _sampleRateHz = sampleRateHz;

  if (logYaxis)
#if (QWT_VERSION < 0x060100)
    _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine());
#else
  _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
#endif
  else
    _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());

  _qwtPlot->setAxisScale(QwtPlot::xBottom, -_sampleRateHz/2.0, _sampleRateHz/2.0);
  _qwtPlot->setAxisScale(QwtPlot::yLeft, scaleMin, scaleMax);

  _xdata.resize(n);

  for (int i = 0; i < n; i++)
    _xdata[i] = -(_sampleRateHz/n)*(i - n/2);

  initCurve();

  _qwtPlot->replot();

  if((_zoomer->zoomStack().size() == 1) || reZoom)
    _zoomer->setZoomBase();

}
//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::configureForProduct(int n, 
                                 double scaleMin, 
                                 double scaleMax, 
                                 double sampleRateHz,
                                 int newProductType)
{
  // we need to reset the zoom base when the plot type changes
  bool reZoom = (_plotType != PRODUCT) || (_productType != newProductType);

  _plotType = PRODUCT;
  _productType = newProductType;
  _scaleMin = scaleMin;
  _scaleMax = scaleMax;
  _sampleRateHz = sampleRateHz;

  _qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
  _qwtPlot->setAxisScale(QwtPlot::xBottom, 0, _sampleRateHz);
  _qwtPlot->setAxisScale(QwtPlot::yLeft, scaleMin, scaleMax);

  _xdata.resize(n);

  for (int i = 0; i < n; i++)
    _xdata[i] = i;

  initCurve();

  _qwtPlot->replot();

  if((_zoomer->zoomStack().size() == 1) || reZoom)
    _zoomer->setZoomBase();

}
//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::labelAxes(std::string xLabel, std::string yLabel) {
  _qwtPlot->setAxisTitle(QwtPlot::xBottom, xLabel.c_str());	
  _qwtPlot->setAxisTitle(QwtPlot::yLeft,   yLabel.c_str());
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::enableXgrid(bool tf) {
  _grid->enableX(tf);
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::enableYgrid(bool tf) {
  _grid->enableY(tf);
}

//////////////////////////////////////////////////////////////////////////////////

void 
  ScopePlot::pause(bool tf) {
  _paused = tf;
}
////////////////////////////////////////////////////////////////////////
void
  ScopePlot::saveImageToFile(std::string filePath) {
  QPixmap pixmap = grab();

  pixmap.save(filePath.c_str(), "PNG", 100);
}

