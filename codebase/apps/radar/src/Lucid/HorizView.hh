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
#include "XyBox.hh"

class GuiManager;
class VertView;
class QLabel;

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
		      
  // zooms
  
  const WorldPlot &getZoomWorld() const { return _zoomWorld; }
  const vector<WorldPlot> &getCustomZooms() const { return _customZooms; }
  void clearCustomZooms() { _customZooms.clear(); }
  void setXyZoom(double minY, double maxY, double minX, double maxX);
  bool checkForZoomChange();
  
  /**
   * react to click point from remote display - Sprite
   * redraw the click point cursor
   */

  void setClickPoint(double azimuthDeg,
                     double elevationDeg,
                     double rangeKm);

  void ShowContextMenu(const QPoint &pos /*, RadxVol *vol */);

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

  void triggerGridRendering(int index, int page);
  
  // set flags to render invalid images
  
  void setRenderInvalidImages(int index, VertView *vert);

  // virtual void ShowContextMenu(const QPoint &pos, RadxVol *vol);

  void setFont();
  virtual void informationMessage();

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
   * @brief Set grids visibility.
   *
   * @param[in] enabled   True to show them, false otherwise.
   */

  void setGrids(const bool enabled);

  /**
   * @brief Set fixed ring visibility.
   *
   * @param[in] enabled    True to show them, false otherwise.
   */

  void setRingsFixed(const bool enabled);

  /**
   * @brief Set data-driven ring visibility.
   *
   * @param[in] enabled    True to show them, false otherwise.
   */

  void setRingsDataDriven(const bool enabled);
  
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
  
  // color scale

  int _colorScaleWidth;
  
  // flags for controlling rendering

  int _renderFrameIndex;
  int _renderFramePage;
  
  bool _renderInvalidImages;
  int _invalidImagesFrameIndex;
  VertView *_vert;
  
  bool _zoomChanged;
  bool _sizeChanged;
  bool _gridsReady;
  bool _mapsReady;
  
  // The index of the field selected for display.
  
  size_t _selectedField;

  // Class to create labels wiith nicely scaled values and approriate units.
  
  ScaledLabel _scaledLabel;

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
  double _worldClickX, _worldClickY;
  double _worldClickLat, _worldClickLon;

  /**
   * @brief Rubber band for zooming.
   */

  QRubberBand *_rubberBand;
  
  // zooms

  XyBox _zoomXy;
  XyBox _prevZoomXy;
  
  WorldPlot _fullWorld;
  WorldPlot _zoomWorld;
  vector<WorldPlot> _definedZooms;
  vector<WorldPlot> _customZooms;
  bool _isZoomed;
  int _zoomLevel;
  
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

  // handle a zoom change from mouse rectangle

  void _handleMouseZoom();
  
  // reset the world coords
  
  void _resetWorld(int width, int height);
  
  // render the grid data

  void _renderGrids();
  
  // render the maps
  
  void _renderMaps();
  
  // initialize horiz projection

  void _initProjection();

  // initialize zooms

  void _initZooms();
  
  // init world plot
  
  void _initWorld(WorldPlot &world, const string &name);

  // grid rendering
  
  int _controlRenderGrid(int page,
                         time_t start_time, time_t end_time);
  
  int _renderGrid(int page,
                  MdvReader *mr,
                  time_t start_time, time_t end_time,
                  bool is_overlay_field);
  
  void _doRenderInvalidImages(int index, VertView *vert);

  // debug prints
  
  void _printNow(int ndecimals, ostream &out);

};


#endif
