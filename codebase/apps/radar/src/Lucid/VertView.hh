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
#ifndef VertView_HH
#define VertView_HH

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

#include <Radx/RadxPlatform.hh>
#include <Radx/RadxVol.hh>
#include <radar/BeamHeight.hh>
#include <toolsa/DateTime.hh>

#include "ScaledLabel.hh"
#include "WorldPlot.hh"

class GuiManager;
class VertManager;

// Widget for vertical display.

class DLL_EXPORT VertView : public QWidget
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

  VertView(QWidget* parent, 
           const GuiManager &manager,
           const VertManager &vertWindow);

  /**
   * @brief Destructor.
   */

  virtual ~VertView();
  
  /**
   * @brief Configure the CartWidget for world coords
   */
  
  virtual void configureWorldCoords(int zoomLevel = 0);

  // are we in archive mode? and if so are we at the start of a sweep?

  void setArchiveMode(bool state);
  
  /**********************************************
   * turn on archive-style rendering - all fields
   */

  void activateArchiveRendering();

  /**********************************************************************
   * turn on reatlime-style rendering - non-selected fields in background
   */

  void activateRealtimeRendering();

  /**
   * @brief Select the field to display.
   *
   * @param[in] index   Index of the field to display, zero based.
   */

  void selectVar(const size_t index);

  // get plot times

  const DateTime &getPlotStartTime() { return _plotStartTime; }
  const DateTime &getPlotEndTime() { return _plotEndTime; }

  // adjust pixel scale to suit window size etc.

  virtual void updatePixelScales();

  // render frame based on movie index
  
  int renderVMovieFrame(int index, QPainter &painter);

  /**
   * @brief Specify the background color.
   *
   * @param[in] color     The background color.
   *
   * @notes This method is not currently called anywhere.
   */

  void backgroundColor(const QColor &color);

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
		      
  /**
   * @brief Get the aspect ratio of the display.
   *
   * @return Returns the aspect ratio of the display.
   */

  // get zooms
  
  const WorldPlot getZoomWorld() const { return _zoomWorld; }
  const vector<WorldPlot> getSavedZooms() const { return _savedZooms; }

  //  virtual void ShowContextMenu(const QPoint &pos);
  virtual void ShowContextMenu(const QPoint &pos, RadxVol *vol);
  void setFont();
  virtual void informationMessage();

 signals:

  ////////////////
  // Qt signals //
  ////////////////

  void locationClicked(double xkm, double ykm, const RadxRay *closestRay);

  /**
   * @brief Signal emitted when we have processed several beams.  This signal
   *        tells the RHI window that it can do a resize to fix the widget
   *        sizing problem that we have on startup.
   */

  void severalBeamsProcessed();
  
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

  // paint event

  void paintEvent(QPaintEvent *event);

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
   * @brief Resize the window.
   *
   */

  void resize(const int width, const int height);

  /**
   * @brief Set grids visibility.
   *
   * @param[in] enabled   True to show them, false otherwise.
   */

  void setGrids(const bool enabled);

  // context menu
  
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
  const GuiManager &_manager;
  const Params &_params;

  // QMainWindow
  
  const VertManager &_vertWindow;

  // are we in archive mode? and if so are we at the start of a sweep?

  bool _isArchiveMode;
  bool _isStartOfSweep;

  // angles and times in archive mode

  DateTime _plotStartTime;
  DateTime _plotEndTime;
  double _meanAz;
  double _sumAz;
  double _nRays;

  /**
   * @brief The maximum range of the beams, in km.  It affects the
   *        labelling of the range rings
   */

  double _maxHeightKm;
  double _xGridSpacing;
  double _yGridSpacing;

  /**
   * @brief Pointers to all of the active beams are saved here.
   */

  BeamHeight _beamHt;

  // computing angle limits of rays

  DateTime _prevTime;
  double _prevAz;
  double _prevElev;
  double _startElev;
  double _endElev;

  /**
   * @brief The index of the field selected for display.
   */

  size_t _selectedField;

  /**
   * @brief The brush for the background.
   */

  QBrush _backgroundBrush;

  /**
   * @brief True if the grids display is enabled.
   */
  
  bool _gridsEnabled;

  /**
   * @brief This will create labels wiith nicely scaled values and
   *        approriate units.
   */

  ScaledLabel _scaledLabel;

  /**
   * @brief Rubber band for zooming.
   */

  QRubberBand *_rubberBand;

  /**
   * @brief The rubber band origin.
   */

  QPoint _rubberBandOrigin;

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
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  // get ray closest to click point

  virtual const RadxRay *_getClosestRay(double x_km, double y_km);

  /**
   * @brief Render the rings and grid. The current value of _ringsGridColor
   *        will be used for the color.
   *
   * @param[in] painter    Painter to use for rendering.
   */

  virtual void _drawOverlays(QPainter &painter);

  /**
   * @brief Initialize the full window transform to use for the widget.
   *
   * @param[in] window    The full window to use for the widget.
   */

  void _setTransform(const QTransform &transform);

  /**
   * @brief Determine a ring spacing which will give even distances, and
   *        fit a reasonable number of rings in the display.
   *
   * @return Returns the ring spacing in kilometers.
   */

  virtual void _setGridSpacing();
  double _getSpacing(double range);

  // Compute the limits of the ray angles
  
  void _computeAngleLimits(const RadxRay *ray);
  
  // store ray location
  
  void _storeRayLoc(const RadxRay *ray);

  // clear overlap with existing rays

  void _clearRayOverlap(const int startIndex,
                        const int endIndex);

  // overide refresh images

  virtual void _refreshImages();

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
   * @brief Handle a resize event. A timer is used to prevent refreshes until
   *        the resize is finished.
   *
   * @brief event   The resize event.
   */

  virtual void resizeEvent(QResizeEvent * event);

  // reset the world coords

  void _resetWorld(int width, int height);

  // rendering

  void _performRendering();

};

#endif
