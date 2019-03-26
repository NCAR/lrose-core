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
// AScopePlot.hh
//
// Plotting for power vs range in an ascope
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
#ifndef AScopePlot_HH
#define AScopePlot_HH

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

#include <string>
#include <vector>

#include <QDialog>
#include <QWidget>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>
#include <QRubberBand>
#include <QPoint>
#include <QTransform>

#include "Params.hh"
#include "ScaledLabel.hh"
#include "WorldPlot.hh"

class Beam;

/// AScope plotting

class DLL_EXPORT AScopePlot
{

  // must include this if you use Qt signals/slots
  Q_OBJECT
  
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  AScopePlot(QWidget *parent,
             const Params &params);
  
  /**
   * @brief Destructor.
   */

  virtual ~AScopePlot();

  // configure the axes
  
  void configure(int width,
                 int height,
                 int xOffset,
                 int yOffset,
                 const Beam &beam);
  
  /**
   * @brief Specify the background color.
   * @param[in] color     The background color.
   * @notes This method is not currently called anywhere.
   */

  void setBackgroundColor(const QColor &color);

  // plot a beam

  void plotBeam(Beam *beam);

  // get the world coords
  
  const WorldPlot &getFullWorld() const { return _fullWorld; }
  const WorldPlot &getZoomWorld() const { return _zoomWorld; }
  bool getIsZoomed() const { return _isZoomed; }
  
  /**
   * @brief Capture an image of the display.
   *
   * @return Returns the image. The caller must delete it when finished
   *         with it.
   *
   * @notes This method is not currently called anywhere.
   */

  QImage *getImage();

  /**
   * @brief Capture a pixmap of the display.
   *
   * @return Returns the pixmap. The caller must delete it when finished
   *         with it.
   *
   * @notes This method is not currently called anywhere.
   */

  QPixmap *getPixmap();
		      
  // initalize the plot start time
  
  void setPlotStartTime(const RadxTime &plot_start_time,
                        bool clearBeams = false);

  // reset the plot start time
  
  void resetPlotStartTime(const RadxTime &plot_start_time);

  // set mouse click point from external routine, to simulate and actual
  // mouse release event

  void setMouseClickPoint(double worldX, double worldY);

  // was the mouse clicked in the data area?

  bool getPointClicked() const { return _pointClicked; }

  ////////////////
  // Qt signals //
  ////////////////

 signals:

  void locationClicked(double xkm, double ykm);

  //////////////
  // Qt slots //
  //////////////

 public slots:

  /**
   * @brief Reset the view to unzoomed.
   */

  void unzoomView();

  /**
   * @brief Clear the data in the view.
   */

  void clear();

  /*************************************************************************
   * refresh()
   */
  
  void refresh();

  /**
   * @brief Resize the window.
   *
   */

  void resize(int width, int height);

  /**
   * @brief Set overlay visibility.
   *
   * @param[in] state True to show them, false otherwise.
   */

  void setXGridEnabled(bool state);
  void setYGridEnabled(bool state);

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Parent plot.
   */
  
  QWidget *_parent;

  /**
   * @brief TDRP params.
   */

  const Params &_params;

  /**
   * @brief The brush for the background.
   */

  QBrush _backgroundBrush;

  /**
   * Overlays
   */

  bool _xGridEnabled;
  bool _yGridEnabled;

  /**
   * @brief This will create labels wiith nicely scaled values and
   *        approriate units.
   */

  ScaledLabel _scaledLabel;
  
  // time of plot

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  double _timeSpanSecs;

  // amplitude of plot

  double _minAmplitude;
  double _maxAmplitude;

  /**
   * @brief Last X,Y location of the mouse during mouse move events; used for
   *        panning.
   */

  bool _pointClicked;
  int _mousePressX, _mousePressY;
  int _mouseReleaseX, _mouseReleaseY;

  /**
   * @brief Location world of the latest click point.
   */
  
  double _worldPressX, _worldPressY;

  /**
   * @brief The rubber band origin.
   */

  QPoint _rubberBandOrigin;

  /**
   * @brief Transform for unzoomed state
   */

  QTransform _fullTransform;
  WorldPlot _fullWorld;
  
  /**
   * @brief Transformed for zoomed state
   */

  bool _isZoomed;
  QTransform _zoomTransform;
  WorldPlot _zoomWorld;
  
  /**
   * @brief The width of the color scale
   */

  int _colorScaleWidth;
  
  // vertical scale state
  
  // Params::range_axis_mode_t _rangeAxisMode;
  
  // time since last rendered
  
  RadxTime _timeLastRendered;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Refresh the images.  Note that this is an expensive method and
   *        should only be called where needed.
   */

  void _refreshImages();

  /**
   * @brief Render the axes, grids, labels and other overlays
   *
   * @param[in] painter    Painter to use for rendering.
   */
  
  void _drawOverlays(QPainter &painter);
  
  /**
   * @brief Initialize the full window transform to use for the plot.
   *
   * @param[in] window    The full window to use for the plot.
   */

  void _setTransform(const QTransform &transform);
  
  // update the renderers

  void _updateRenderers();

  /////////////////////////////////
  // Overridden QtPlot methods //
  /////////////////////////////////

  /**
   * @brief Capture mouse move event for panning/zooming.
   *
   * @param[in] event   The mouse event.
   */

  virtual void mouseMoveEvent(QMouseEvent* event);

  /**
   * @brief Capture mouse press event which signals the start of
   *        panning/zooming.
   *
   * @param[in] event    The mouse press event.
   */

  virtual void mousePressEvent(QMouseEvent* event);

  /**
   * @brief Capture mouse release event which signals the start of
   * panning/zooming.
   *
   * @param[in] event    The mouse event.
   */

  virtual void mouseReleaseEvent(QMouseEvent* event);

  /**
   * @brief The method that is called when a repaint event is triggered.
   *
   * @param[in] event   The repaint event.
   */

  void paintEvent(QPaintEvent *event);

  /**
   * @brief Handle a resize event. A timer is used to prevent refreshes until
   *        the resize is finished.
   *
   * @brief event   The resize event.
   */

  virtual void resizeEvent(QResizeEvent * event);

  // reset the pixel size of the world view

  void _resetWorld(int width, int height);

  // call the renderers for each field

  void _performRendering();

};

#endif
