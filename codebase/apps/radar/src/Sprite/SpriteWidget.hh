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
#ifndef SpriteWidget_HH
#define SpriteWidget_HH

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
#include <QMenu>

#include "Params.hh"
#include "ScaledLabel.hh"
#include "WorldPlot.hh"

class Beam;
class AscopePlot;
class WaterfallPlot;
class IqPlot;
class RadxRay;
class SpriteMgr;

/// Widget class - Spectra mode
/// Plot the Spectra and spectra for a beam

class DLL_EXPORT SpriteWidget : public QWidget
{

  // must include this if you use Qt signals/slots
  Q_OBJECT

 public:

  typedef enum {
    PANEL_TITLE,
    PANEL_ASCOPE,
    PANEL_WATERFALL,
    PANEL_IQPLOT
  } panel_type_t;

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent         Parent widget.
   * @param[in] params         TDRP parameters.
   */

  SpriteWidget(QWidget* parent,
                const SpriteMgr &manager,
                const Params &params);
  
  /**
   * @brief Destructor.
   */

  virtual ~SpriteWidget();

  // configure the axes
  
  void configureAxes(double min_amplitude,
                     double max_amplitude,
                     double time_span_secs);
  
  /**
   * @brief Specify the background color.
   * @param[in] color     The background color.
   * @notes This method is not currently called anywhere.
   */

  void setBackgroundColor(const QColor &color);

  // prepare a beam for plotting

  void prepareBeam(Beam *beam);

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
		      
  // set mouse click point from external routine, to simulate and actual
  // mouse release event

  void setMouseClickPoint(double worldX, double worldY);

  // was the mouse clicked in the data area?

  bool getPointClicked() const { return _pointClicked; }

  // set the range

  void setRange(double rangeKm);

  // increment/decrement the range in response to up/down arrow keys

  void changeRange(int nGatesDelta);

  // get currently selected range

  double getSelectedRangeKm() const { return _selectedRangeKm; }
  int getSelectedGateNum() const { return _selectedGateNum; }
  
  ////////////////
  // Qt signals //
  ////////////////

 signals:

  void locationClicked(double selectedRangeKm, int selectedGateNum);

  //////////////
  // Qt slots //
  //////////////

 public slots:

  /**
   * @brief Reset the view to unzoomed.
   */

  void unzoom();

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
  void setLegendsEnabled(bool state);

  // show context menu in response to right click
  
  void showContextMenu(const QPoint &pos);

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Parent widget.
   */
  
  QWidget *_parent;
  const SpriteMgr &_manager;

  /**
   * @brief TDRP params.
   */

  const Params &_params;

  /**
   * @brief The brush for the background.
   */

  QBrush _backgroundBrush;

  // plot panel layouts

  int _titleMargin;

  int _nAscopes;
  int _ascopeStartIx;
  int _ascopeWidth;
  int _ascopeHeight;
  int _ascopeGrossWidth;

  int _nWaterfalls;
  int _waterfallStartIx;
  int _waterfallWidth;
  int _waterfallHeight;
  int _waterfallGrossWidth;

  int _nIqRows;
  int _nIqCols;
  int _nIqPlots;
  int _iqStartIx;
  int _iqPlotWidth;
  int _iqPlotHeight;
  int _iqGrossWidth;
  int _iqGrossHeight;
  
  // ascopes
  
  vector<AscopePlot *> _ascopes;
  bool _ascopesConfigured;

  // waterfall plots
  
  vector<WaterfallPlot *> _waterfalls;
  bool _waterfallsConfigured;

  // IQ plots
  
  vector<IqPlot *> _iqPlots;
  bool _iqPlotsConfigured;

  // Grid overlays

  bool _xGridEnabled;
  bool _yGridEnabled;

  // legends

  bool _legendsEnabled;

  // labels
  
  ScaledLabel _scaledLabel;

  // currently selected range

  double _selectedRangeKm;
  int _selectedGateNum;
  
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

  panel_type_t _mousePressPanelType;
  int _mousePressPanelId;

  panel_type_t _mouseReleasePanelType;
  int _mouseReleasePanelId;

  panel_type_t _contextMenuPanelType;
  int _contextMenuPanelId;
  
  /**
   * @brief Location world of the latest click point.
   */
  
  double _worldPressX, _worldPressY;
  double _worldReleaseX, _worldReleaseY;

  /**
   * @brief Rubber band for zooming.
   */

  QRubberBand *_rubberBand;

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
  
  // time since last rendered
  
  RadxTime _timeLastRendered;

  // beam data

  Beam *_beam;
  int _nSamplesPlot;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Refresh the images.  Note that this is an expensive method and
   *        should only be called where needed.
   */

  void _refresh();

  /**
   * @brief Render the axes, grids, labels and other overlays
   *
   * @param[in] painter    Painter to use for rendering.
   */
  
  void _drawOverlays(QPainter &painter);
  
  /**
   * @brief Initialize the full window transform to use for the widget.
   *
   * @param[in] window    The full window to use for the widget.
   */

  void _setTransform(const QTransform &transform);
  
  /////////////////////////////////
  // Overridden QtWidget methods //
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

  // draw the main title
  
  void _drawMainTitle(QPainter &painter);

  // ascope panels

  void _createAscope(int id);
  void _configureAscope(int id);

  // waterfall panels

  void _createWaterfall(int id);
  void _configureWaterfall(int id);

  // iqplots

  void _createIqPlot(int id);
  void _configureIqPlot(int id);

  // determine the panel type selected by a click

  void _identSelectedPanel(int xx, int yy,
                           panel_type_t &panelType,
                           int &panelId);

  // compute selected gate num from selected range

  void _computeSelectedGateNum();

  // create the context menus
  
  void _createAscopeContextMenu(const QPoint &pos);
  void _createWaterfallContextMenu(const QPoint &pos);
  void _createIqPlotContextMenu(const QPoint &pos);

};

#endif
