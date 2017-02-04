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
#ifndef BscanWidget_HH
#define BscanWidget_HH

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
#include "BscanBeam.hh"
#include "FieldRenderer.hh"
#include "ScaledLabel.hh"
#include "WorldPlot.hh"

class BscanManager;

/// Base class for widgets displaying BSCAN data.
///
/// A beam is the basic building block for the scan. It has
/// a starting time, ending time, a fixed number of range 
/// gates, and a fixed number of fields that can be rendered 
/// on the beam.
///
/// The scan is given color maps for each of the fields. 
/// It is then simply called with the data for all fields
/// for a given beam. As each beam is received, the current field
/// and any fields in the beam that are being background rendered
/// are rendered into QImages.  The QImage for the current field is
/// then popped up on the display.  When the selected field is changed,
/// the image is cleared, and either the QImage for the new field is
/// popped up on the display or the new field is rendered. This
/// allows for quick switches between fields being rendered in the
/// background (which are the most recently viewed fields), without
/// the overhead of rendering all fields in the background.
///
/// Zooming is accomplished using the QtTransform.

class DLL_EXPORT BscanWidget : public QWidget
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
   * @param[in] parent         Parent widget.
   * @param[in] params         TDRP parameters.
   */

  BscanWidget(QWidget* parent,
              const BscanManager &manager,
              const Params &params,
              size_t n_fields);
  
  /**
   * @brief Destructor.
   */

  virtual ~BscanWidget();

  // configure the axes
  
  void configureAxes(Params::range_axis_mode_t range_axis_mode,
                     double min_range,
                     double max_range,
                     double min_altitude,
                     double max_altitude,
                     double time_span_secs,
                     bool archive_mode);

  /**
   * @brief Select the field to display.
   *
   * @param[in] index   Index of the field to display, zero based.
   */

  void selectVar(size_t index);

  /**
   * @brief Clear the specified field.
   *
   * @param[in] index    Index of the field to be cleared, zero based.
   *
   * @notes This method is not currently called anywhere.
   */

  void clearVar(size_t index);
  
  /*************************************************************************
   * archive mode - turn on background rendering for all fields
   */
  
  void activateArchiveRendering();

  /*************************************************************************
   * reatime mode - turn on background rendering for non-selected fields
   * for limited time
   */
  
  void activateRealtimeRendering();

  /**
   * @brief Add a new beam to the display. Data for all fields and all
   *        gates are provided, as well as color maps for all fields.
   *        addBeam() will map the field values to  the correct color, and
   *        render the beam for each field in the appropriate pixamp. The
   *        existing wedge for this beam will be discarded.
   *
   */

  void addBeam(const RadxRay *ray,
               double instHtKm, // height of instrument in km
               const RadxTime &plot_start_time,
               const RadxTime &beam_start_time,
               const RadxTime &beam_end_time,
               const std::vector< std::vector< double > > &beam_data,
               const std::vector< DisplayField* > &fields);

  /**
   * @brief Specify the background color.
   *
   * @param[in] color     The background color.
   *
   * @notes This method is not currently called anywhere.
   */

  void setBackgroundColor(const QColor &color);

  /**
   * @brief Get the current number of beams. This is interesting to monitor
   *        when BscanWidget is operating in the dynamically allocated beam mode.
   *
   * @return Returns the current number of beams.
   *
   * @notes This method is not currently called anywhere.
   */

  size_t getNumBeams() const;

  // get the vertical scale type
  
  Params::range_axis_mode_t getRangeAxisMode() const { return _rangeAxisMode; }

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

  void locationClicked(double xkm, double ykm, const RadxRay *closestRay);

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

  void setRangeGridEnabled(bool state);
  void setTimeGridEnabled(bool state);
  void setInstHtLineEnabled(bool state);
  void setLatlonLegendEnabled(bool state);
  void setSpeedTrackLegendEnabled(bool state);
  void setDistScaleEnabled(bool state);
  void setAltitudeInFeet(bool state);
  void setRangeInFeet(bool state);

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Parent widget.
   */

  QWidget *_parent;
  const BscanManager &_manager;

  /**
   * @brief TDRP params.
   */

  const Params &_params;

  // number of fields

  size_t _nFields;

  /**
   * @brief Pointers to all of the active beams are saved here.
   */

  std::vector<BscanBeam*> _beams;
  
  /**
   * @brief The renderer for each field.
   */

  vector<FieldRenderer*> _fieldRenderers;
  
  /**
   * @brief The index of the field selected for display.
   */

  size_t _selectedField;

  /**
   * @brief The brush for the background.
   */

  QBrush _backgroundBrush;

  /**
   * Overlays
   */

  bool _rangeGridEnabled;
  bool _timeGridEnabled;
  bool _instHtLineEnabled;
  bool _latlonLegendEnabled;
  bool _speedTrackLegendEnabled;
  bool _distScaleEnabled;

  /**
   * @brief This will create labels wiith nicely scaled values and
   *        approriate units.
   */

  ScaledLabel _scaledLabel;

  // start time of plot

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  double _timeSpanSecs;
  bool _archiveMode;

  /**
   * @brief The maximum range of the beams, in km.  It affects the
   *        labelling of the range rings
   */

  double _minRange;
  double _maxRange;
  double _minAltitude;
  double _maxAltitude;
  bool _altitudeInFeet;
  bool _rangeInFeet;

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
  
  /**
   * @brief The width of the color scale
   */

  int _colorScaleWidth;
  
  // vertical scale state
  
  Params::range_axis_mode_t _rangeAxisMode;
  
  // time since last rendered
  
  RadxTime _timeLastRendered;

  // distance, speed and track data

  class DistLoc {
  public:
    int beamNum;
    RadxTime time;
    double lat;
    double lon;
    double speedMps;
    double dirnDeg;
    double distKm;
    double sumDistKm;
    DistLoc(int beamNum_, RadxTime time_, double lat_, double lon_) :
            beamNum(beamNum_), time(time_), lat(lat_), lon(lon_),
            speedMps(0), dirnDeg(0), distKm(0), sumDistKm(0) {}
  };
  
  vector<DistLoc> _distLocs;
  double _startLat, _startLon;
  double _meanSpeedMps, _meanDirnDeg;
  double _sumDistKm;

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
   * @brief Initialize the full window transform to use for the widget.
   *
   * @param[in] window    The full window to use for the widget.
   */

  void _setTransform(const QTransform &transform);
  
  // update the renderers

  void _updateRenderers();

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

  // call the renderers for each field

  void _performRendering();

  // Compute the distance details, starting lat/lon and mean track/speed

  void _computeSummaryDistanceSpeedTrack();

  // add distance scale to time axis

  void _plotDistanceOnTimeAxis(QPainter &painter);

  // get time for a specified distance value

  RadxTime _getTimeForDistanceVal(double distKm);

};

#endif
