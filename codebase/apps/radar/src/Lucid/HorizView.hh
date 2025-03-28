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
#ifndef HorizView_HH
#define HorizView_HH

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
#include <QPixmap>

#include <Radx/RadxPlatform.hh>
#include <Radx/RadxVol.hh>

#include <Mdv/MdvxProj.hh>

#include "GlobalData.hh"
#include "Params.hh"

#include "ScaledLabel.hh"
#include "WorldPlot.hh"

class GuiManager;
class VertView;
class QLabel;

// custom rubber band for transparent rectangle

class CustomRubberBand : public QRubberBand {
public:
  CustomRubberBand(Shape shape, QWidget *parent = nullptr)
          : QRubberBand(shape, parent) {}
  
protected:
  void paintEvent(QPaintEvent *event) override {
    QRubberBand::paintEvent(event);
#ifdef NOTNOW
    QPainter painter(this);
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    // painter.setRenderHint(QPainter::Antialiasing);
    // Draw border
    // painter.setPen(QColor(0xff, 0xff, 0xff));
    // Transparent background
    painter.setBrush(Qt::NoBrush);
    // Adjust to stay within bounds    
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
#endif
  }
};

// Widget representing a horizontal view

class DLL_EXPORT HorizView : public QWidget
{
  // must include this if you use Qt signals/slots
  Q_OBJECT

 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent   Parent widget.
   * @param[in] params   TDRP parameters.
   */
  
  HorizView(QWidget* parent,
            GuiManager &manager);

  /**
   * @brief Destructor.
   */

  virtual ~HorizView();

  /**
   * @brief Configure the CartWidget for world coords
   */

  virtual void configureWorldCoords(int zoomLevel = 0);

  /**********************************************
   * turn on archive-style rendering - all fields
   */

  void activateArchiveRendering();

  /**********************************************************************
   * turn on reatlime-style rendering - non-selected fields in background
   */

  void activateRealtimeRendering();

  /**
   * @brief Specify the background color.
   *
   * @param[in] color     The background color.
   *
   * @notes This method is not currently called anywhere.
   */

  void backgroundColor(const QColor &color);

  /**
   * @brief Specify the grid and rings color.
   *
   * @params[in] color   The grid/rings color.
   *
   * @notes This method is not currently called anywhere.
   */

  void gridRingsColor(const QColor &color);

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
		      
  // get zooms
  
  const WorldPlot &getZoomWorld() const { return _zoomWorld; }
  const vector<WorldPlot> &getSavedZooms() const { return _savedZooms; }
  void clearSavedZooms() { _savedZooms.clear(); }
  
  // are we in archive mode? and if so are we at the start of a sweep?

  void setArchiveMode(bool state);

  /**
   * react to click point from remote display - Sprite
   * redraw the click point cursor
   */

  void setClickPoint(double azimuthDeg,
                     double elevationDeg,
                     double rangeKm);

  // get plot times

  const DateTime &getPlotStartTime() { return _plotStartTime; }
  const DateTime &getPlotEndTime() { return _plotEndTime; }

  void ShowContextMenu(const QPoint &pos /*, RadxVol *vol */);

  QLabel *_openingFileInfoLabel;
  void showOpeningFileMsg(bool isVisible);

  // adjust pixel scale to suit window size etc.

  virtual void updatePixelScales();

  // override QWidget methods
  
  /**
   * @brief The method that is called when a repaint event is triggered.
   *
   * @param[in] event   The repaint event.
   */

  void paintEvent(QPaintEvent *event);

  // set flags to control rendering

  void setFrameForRendering(int index, int page);
  
  // set flags to render invalid images
  
  void setRenderInvalidImages(int index, VertView *vert);

  // virtual void ShowContextMenu(const QPoint &pos, RadxVol *vol);

  void setFont();
  virtual void informationMessage();

  ////////////////
  // Qt signals //
  ////////////////

 signals:
 
  void locationClicked(double xkm, double ykm, const RadxRay *closestRay);

  //////////////
  // Qt slots //
  //////////////

 public slots:
   
  /**
   * @brief Slot called when a beam has finished rendering.
   *
   * @params[in] field_num   The index of the field that was rendered.  This
   *                         is used to check if this was the selected field.
   */

  void displayImage(const size_t field_num);

  /**
   * @brief go back to prev zoom
   */

  void zoomBackView();

  /**
   * @brief unzoom all the way
   */

  void zoomOutView();

  /**
   * @brief Resize the window.
   *
   */

  void resize(const int width, const int height);

  /**
   * @brief Set ring visibility.
   *
   * @param[in] enabled    True to show them, false otherwise.
   */

  void setRings(const bool enabled);

  /**
   * @brief Set grids visibility.
   *
   * @param[in] enabled   True to show them, false otherwise.
   */

  void setGrids(const bool enabled);

  /**
   * @brief Set azimuth lines visibility.
   *
   * @param[in] enabled    True to show them, false otherwise.
   */

  void setAngleLines(const bool enabled);

  /**
   * @brief Clear the data in the view.
   */

  void clear();

  virtual void contextMenuCancel();
  virtual void contextMenuParameterColors();
  virtual void contextMenuView();
  virtual void contextMenuEditor();
  virtual void contextMenuExamine(); // const QPoint &pos);
  virtual void contextMenuDataWidget();
  
 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The sine of 45 degrees.  Used for positioning the labels on the
   *        45 degree lines.
   */

  static const double SIN_45;
  
  /**
   * @brief The sine of 30 degrees.  Used for positioning the azimuth lines on
   *        the 30 degree lines.
   */

  static const double SIN_30;
  
  /**
   * @brief The cosine of 30 degrees.  Used for positioning the azimuth lines on
   *        the 30 degree lines.
   */

  static const double COS_30;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Parent widget.
   */

  QWidget *_parent;
  GuiManager &_manager;

  Params &_params;
  GlobalData &_gd;

  // pixmap for rendering
  
  QPixmap _pixmap;

  // flags for controlling rendering

  bool _renderFrame;
  int _renderFrameIndex;
  int _renderFramePage;
  
  bool _renderInvalidImages;
  int _invalidImagesFrameIndex;
  VertView *_vert;
  
  /**
   * @brief The index of the field selected for display.
   */

  size_t _selectedField;

  /**
   * @brief The brush for the background.
   */

  QBrush _backgroundBrush;

  /**
   * @brief The color for the grid and rings.
   */

  QColor _gridRingsColor;

  /**
   * @brief True if the ring display is enabled.
   */

  bool _ringsEnabled;

  /**
   * @brief True if the grids display is enabled.
   */
  
  bool _gridsEnabled;

  /**
   * @brief True if the angle lines enabled.
   */
  bool _angleLinesEnabled;


  /**
   * @brief This will create labels wiith nicely scaled values and
   *        approriate units.
   */

  ScaledLabel _scaledLabel;

  /**
   * @brief The maximum range of the beams, in km.  It affects the
   *        labelling of the range rings
   */

  double _maxRangeKm;

  // archive mode

  bool _archiveMode;

  /**
   * @brief Last X,Y location of the mouse during mouse move events; used for
   *        panning.
   */

  bool _pointClicked;
  int _mousePressX, _mousePressY;
  int _mouseReleaseX, _mouseReleaseY;
  int _zoomCornerX, _zoomCornerY;
  
  /**
   * @brief Location world of the latest click point.
   */
  
  double _worldPressX, _worldPressY;
  double _worldReleaseX, _worldReleaseY;

  /**
   * @brief Rubber band for zooming.
   */

  CustomRubberBand *_rubberBand;

  /**
   * @brief The current ring spacing in km.  This value is changed when we
   *        zoom.
   */

  double _ringSpacing;
  
  /**
   * @brief The width of the color scale
   */

  int _colorScaleWidth;
  
  /**
   * @brief The full window rendering dimensions.  These are different for
   *        PPI windows and RHI windows.
   */

  QTransform _fullTransform;
  WorldPlot _fullWorld;
  
  /**
   * @brief The window to use for rendering.  This is where the zoom is
   *        implemented.
   */

  bool _isZoomed;
  QTransform _zoomTransform;
  WorldPlot _zoomWorld;
  vector<WorldPlot> _savedZooms;
  
  // are we in archive mode? and if so are we at the start of a sweep?

  bool _isArchiveMode;
  bool _isStartOfSweep;

  // angles and times in archive mode

  DateTime _plotStartTime;
  DateTime _plotEndTime;
  // double _meanElev;
  // double _sumElev;
  // double _nRays;

  // projection

  MdvxProj _proj;

  ///////////////////////
  // Protected methods //
  ///////////////////////

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
   * @brief Handle a resize event. A timer is used to prevent refreshes until
   *        the resize is finished.
   *
   * @brief event   The resize event.
   */

  virtual void resizeEvent(QResizeEvent * event);

  // used to detect shift key pressed for boundary editor (switches cursor)
  virtual void timerEvent(QTimerEvent * event);

  // handle a zoom change

  void _handleZoom();
  
  // reset the world coords
  
  void _resetWorld(int width, int height);
  
  // rendering
  
  void _performRendering();
  
  // get ray closest to click point
  virtual const RadxRay *_getClosestRay(double x_km, double y_km);

  /**
   * @brief Initialize the full window transform to use for the widget.
   *
   * @param[in] window    The full window to use for the widget.
   */

  void _setTransform(const QTransform &transform);

  /**
   * @brief Render the rings and grid. The current value of _ringsGridColor
   *        will be used for the color.
   *
   * @param[in] painter    Painter to use for rendering.
   */

  void _renderGrids();

  void _drawOverlays(QPainter &painter);

  void _drawMaps(QPainter &painter);
  void _drawRingsAndAzLines(QPainter &painter);
  
  /**
   * @brief Determine a ring spacing which will give even distances, and
   *        fit a reasonable number of rings in the display.
   *
   * @return Returns the ring spacing in kilometers.
   */

  virtual void _setGridSpacing();

  // draw text in screen coords

  void _drawScreenText(QPainter &painter, const string &text,
                       int text_x, int text_y,
                       int flags);
    
  /**
   * @brief Refresh the images.  Note that this is an expensive method and
   *        should only be called where needed.
   */

  virtual void _refreshImages();

  // initialize the geographic projection

  void _initProjection();
  
  int _controlRendering(int page,
                        time_t start_time, time_t end_time);
  
  int _renderGrid(int page,
                  MdvReader *mr,
                  time_t start_time, time_t end_time,
                  bool is_overlay_field);
  
  void _doRenderInvalidImages(int index, VertView *vert);
  
};


#endif
