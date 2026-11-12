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

#include "HorizView.hh"
#include "VertView.hh"
#include "GuiManager.hh"
#include "ProductMgr.hh"
#include "SymprodRenderContext.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>
#include <QKeyEvent>
#include <QRect>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

namespace {

////////////////////////////////////////////////////////
// normalize physical buttons and single-button alternatives

Qt::MouseButton effectiveButton(const QMouseEvent *event)
{
  const Qt::MouseButton button = event->button();
  const Qt::KeyboardModifiers modifiers = event->modifiers();

  if (button == Qt::LeftButton) {
    if (modifiers & Qt::AltModifier) {
      return Qt::MiddleButton;
    }
#ifdef Q_OS_MACOS
    // Qt normally translates a macOS Control-click to a right-click.  The
    // MetaModifier check covers platforms/input devices which leave it as a
    // modified left-click (Qt maps the Mac Control key to MetaModifier).
    if (modifiers & Qt::MetaModifier) {
      return Qt::RightButton;
    }
#endif
  }

  return button;
}

} // namespace

HorizView::HorizView(QWidget* parent,
                     GuiManager &manager) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _selectedField(0),
        _rubberBand(nullptr)
        
{

  // initialize

  setAttribute(Qt::WA_TranslucentBackground);
  setAutoFillBackground(false);
  
  // Allow the widget to get focus
  
  setFocusPolicy(Qt::StrongFocus);

  // Allow the size_t type to be passed to slots

  qRegisterMetaType<size_t>("size_t");
  qRegisterMetaType<VsectionRoute>("VsectionRoute");

  // init other values

  _worldPressX = 0.0;
  _worldPressY = 0.0;
  _worldReleaseX = 0.0;
  _worldReleaseY = 0.0;

  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;
  _activeMouseButton = Qt::NoButton;
  _panPreviewActive = false;
  _panPreviewX = 0;
  _panPreviewY = 0;
  _vsectionRouteEditing = false;
  _vsectionPreviewX = 0;
  _vsectionPreviewY = 0;

  _zoomCornerX = 0;
  _zoomCornerY = 0;

  _pointClicked = false;
  _haveClickLocation = false;
  _worldClickX = 0.0;
  _worldClickY = 0.0;
  _worldClickLat = 0.0;
  _worldClickLon = 0.0;
  
  _colorScaleWidth = _params.horiz_color_scale_width;

  setGrids(_params.horiz_grids_on_at_startup);
  setRingsFixed(_params.plot_range_rings_fixed);
  setRingsDataDriven(_params.plot_range_rings_from_data);

  _renderFrameIndex = 0;
  _playbackFrameIndex = -1;
  _renderFramePage = 0;

  _renderInvalidImages = false;
  _invalidImagesFrameIndex = 0;
  _vert = NULL;

  _gridsReady = false;
  _mapsReady = false;
  _zoomChanged = true;
  _zoomRequestPending = true;
  _sizeChanged = true;

  _showValue = false;
  _mouseValueVisible = false;
  _mouseValueX = 0;
  _mouseValueY = 0;
  _updateMouseTracking();

  // initialize the horiz projection
  
  _initProjection();
  
  // initialize zooms

  _initZooms();

  // timer ??
  // fires every 50ms. used for boundary editor to
  // (1) detect shift key down (changes cursor)
  // (2) get notified if user zooms in or out so the boundary can be rescaled
  // startTimer(5000);

}

/*************************************************************************
 * Destructor
 */

HorizView::~HorizView()
{

  if (!_rubberBand) {
    delete _rubberBand;
  }

}

/*************************************************************************
 * clear()
 */

void HorizView::clear()
{
  _renderMaps();
}

////////////////////////////////////////////////////////////////////////
// Used to notify BoundaryPointEditor if the user has zoomed in/out
// or is pressing the Shift key
// Todo: investigate implementing a listener pattern instead

void HorizView::timerEvent(QTimerEvent *event)
{

  bool doUpdate = false;
  this->setCursor(Qt::ArrowCursor);

  cerr << "111111111111111111111111111 HorizView::timerEvent" << endl;
  if (doUpdate) {  //only update if something has changed
    cerr << "3333333333333333333333333333 HorizView::timerEvent" << endl;
    update();
  }
}

////////////////////////////////////////////////////////////////////////
// save field data to queue

void HorizView::_saveFieldDataToQueue(int fieldNum)
{
  
  MdvReader *mr = _gd.mread[fieldNum];
  string fieldName = mr->h_fhdr.field_name;
  
  // remove any existing entry with the same name
  
  for (auto ii = _fieldDataQueue.begin(); ii != _fieldDataQueue.end(); ii++) {
    if (ii->fieldName == fieldName) {
      _fieldDataQueue.erase(ii);
      break;
    }
  } // ii

  // check for queue size

  while ((int) _fieldDataQueue.size() >= _params.field_data_queue_max_size) {
    _fieldDataQueue.pop_back();
  }
  
  // push field data onto front of queue

  _fieldDataQueue.push_front(MdvReader::FieldCopy(fieldName,
                                                  mr->h_mhdr,
                                                  mr->h_fhdr,
                                                  mr->h_vhdr,
                                                  mr->proj,
                                                  mr->h_fl32_data));

  // A field selection replaces or augments the queue used by the click
  // dialog.  Refresh the existing report in place at the last click location.
  _updateClickReport();

  // Refresh an existing hover report when a newly displayed field replaces
  // or augments the saved data beneath a stationary cursor.
  if (_showValue && _mouseValueVisible) {
    _updateMouseValue(_mouseValueX, _mouseValueY);
  }

}

void HorizView::clearFieldDataQueue()
{
  _fieldDataQueue.clear();
  _mouseValueVisible = false;
  _mouseValueText.clear();
  update();
}

/*************************************************************************
 * print current time - for debugging
 */

void HorizView::_printNow(int ndecimals,
                          ostream &out)
{

  // Get current system time
  auto now = std::chrono::system_clock::now();
  
  // Convert to time_t for calendar time (date + hour:min:sec)
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  
  // Extract fractional seconds (microseconds)
  auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
  auto fraction = now_us.time_since_epoch().count() % 1000000; // microseconds part
  
  // Format and print time
  out << std::put_time(std::localtime(&now_time_t), "%H:%M:%S");
  out << "." << std::setw(ndecimals) << std::setfill('0') << fraction << std::endl;
  
}

/*************************************************************************
 * paintEvent()
 */

void HorizView::paintEvent(QPaintEvent *event)
{

  if (_playbackFrameIndex >= 0 &&
      _playbackFrameIndex < static_cast<int>(_movieFrames.size()) &&
      !_movieFrames[_playbackFrameIndex].isNull()) {
    QPainter painter(this);
    painter.drawImage(0, 0, _movieFrames[_playbackFrameIndex]);
    return;
  }

  int fieldNum = _gd.h_win.page;

  // render data grids to grid image in WorldPlot

  if (_sizeChanged || _zoomChanged) {
    _renderGrids();
    _renderMaps();
  }
  
  // render invalid images
  
  // if (_renderInvalidImages) {
  //   _doRenderInvalidImages(_invalidImagesFrameIndex, _vert);
  //   _renderInvalidImages = false;
  // }
  
  // Copy the cached data layers into the widget.  During a pan gesture these
  // images are translated only for visual feedback; world limits and data
  // requests remain unchanged until the button is released.
  
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  painter.save();
  if (_panPreviewActive) {
    const QRect plotRect(_zoom->getXMinPixel(), _zoom->getYMaxPixel(),
                         _zoom->getPlotWidth(), _zoom->getPlotHeight());
    painter.setClipRect(plotRect);
    painter.fillRect(plotRect,
                     QColor(string(_params.missing_data_color) == "transparent" ?
                            _params.background_color : _params.missing_data_color));
    painter.translate(_panPreviewX - _mousePressX,
                      _panPreviewY - _mousePressY);
  }

  if (_gridsReady && _mapsReady) {
    painter.drawImage(0, 0, *_zoom->getGridImage());
    painter.drawImage(0, 0, *_zoom->getMapsImage());
  }

  // render the range rings
  
  if (_params.plot_range_rings_fixed || _params.plot_range_rings_from_data) {
    MdvReader *mr = _gd.mread[fieldNum];
    _zoom->drawRangeRingsHoriz(fieldNum, mr);
    painter.drawImage(0, 0, *_zoom->getRingsImage());
  }

  if (_gd.prod_mgr && _gd.prod.products_on) {
    painter.drawImage(0, 0, *_zoom->getProductsImage());
    _drawPickedProduct(painter);
  }
  painter.restore();
  
  // set axis areas to background color
  
  _zoom->fillMargins(painter, _params.background_color);
  
  // draw axes
  
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  string projUnits("km");
  if (_proj.getProjType() == Mdvx::PROJ_LATLON) {
    projUnits = "deg";
  }
  _zoom->setYAxisLabelsInside(_params.horiz_tick_values_inside);
  _zoom->setXAxisLabelsInside(_params.horiz_tick_values_inside);
  _zoom->setAxisLineColor(_params.horiz_axes_color);
  _zoom->setAxisTextColor(_params.horiz_axes_color);
  _zoom->setGridColor(_params.horiz_grid_color);
  _zoom->drawAxisLeft(painter, projUnits, true, true, true,
                      _params.horiz_grids_on_at_startup);
  _zoom->drawAxisTop(painter, projUnits, true, true, false, false);
  _zoom->drawAxisRight(painter, projUnits, true, true, false, false);
  _zoom->drawAxisBottom(painter, projUnits, true, true, true,
                        _params.horiz_grids_on_at_startup);
  
  // draw the color scale

  const ColorMap &colorMap(_gd.mread[fieldNum]->colorMap);
  _zoom->drawColorScale(colorMap, painter, _params.horiz_axis_label_font_size);

  // title
  
  painter.setPen(_params.foreground_color);
  _zoom->drawTitleTopCenter(painter, _params.horiz_frame_label);

  // Keep the selected route as a widget overlay rather than baking it into a
  // cached data layer.  It therefore remains visible while fields rerender and
  // can be transformed again whenever the view changes.

  _drawVsectionRoute(painter);

  // Draw cursor values directly on the widget so they remain transient rather
  // than becoming part of the cached data or map images.

  if (_showValue && _mouseValueVisible) {
    _drawMouseValue(painter);
  }
  
  // click point cross hairs
  
  if (_pointClicked) {

    painter.save();

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    QPen pen(painter.pen());
    pen.setColor(_params.click_cross_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.click_cross_line_width);
    painter.setPen(pen);

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);
    
    painter.restore();

    char titleStr[1024];
    if (_proj.getProjType() == Mdvx::PROJ_LATLON) {
      snprintf(titleStr, 1023, "Click lat,lon: (%.3f, %.3f)",
               _worldClickLat, _worldClickLon);
    } else {
      snprintf(titleStr, 1023, "Click lat,lon:(%.3f, %.3f) x,y:(%.3f, %.3f)",
               _worldClickLat, _worldClickLon, _worldClickX, _worldClickY);
    }
    _manager.setTitleBarStr(titleStr);
             
  }

  _sizeChanged = false;
  _zoomChanged = false;

}

////////////////////////
// set show value flag

void HorizView::setShowValue(bool state)

{
  _showValue = state;
  _updateMouseTracking();

  // Disabling the feature must also erase the last transient report.
  if (!_showValue) {
    _mouseValueVisible = false;
    _mouseValueText.clear();
  }
  update();
}

////////////////////////////////////////////////////////
// set vertical-section route selection mode

void HorizView::setSingleSegmentVsection(bool enabled)
{
  _params.single_segment_vsection = enabled ? pTRUE : pFALSE;

  // A partial route was created under the previous interaction contract.
  // Discard it rather than interpreting its existing points in the new mode.
  if (_vsectionRouteEditing) {
    _cancelVsectionRoute();
  }
}

////////////////////////////////////////////////////////
// update the value report at the current cursor position

void HorizView::_updateMouseValue(int mouseX, int mouseY)
{
  _mouseValueVisible = false;
  _mouseValueText.clear();

  if (!_showValue || _fieldDataQueue.empty()) {
    return;
  }

  const int plotLeft = std::min(_zoom->getXMinPixel(),
                                _zoom->getXMaxPixel());
  const int plotRight = std::max(_zoom->getXMinPixel(),
                                 _zoom->getXMaxPixel());
  const int plotTop = std::min(_zoom->getYMinPixel(),
                               _zoom->getYMaxPixel());
  const int plotBottom = std::max(_zoom->getYMinPixel(),
                                  _zoom->getYMaxPixel());
  if (mouseX < plotLeft || mouseX > plotRight ||
      mouseY < plotTop || mouseY > plotBottom) {
    return;
  }

  const double worldX = _zoom->getXWorld(mouseX);
  const double worldY = _zoom->getYWorld(mouseY);
  double lat, lon;
  _proj.xy2latlon(worldX, worldY, lat, lon);

  const vector<ClickReportField> fields = _getFieldValues(lat, lon);
  for (const ClickReportField &field : fields) {
    char line[1024];
    snprintf(line, sizeof(line), "%s: %g", field.name.c_str(), field.value);
    if (!_mouseValueText.empty()) {
      _mouseValueText += "\n";
    }
    _mouseValueText += line;
  }

  if (!_mouseValueText.empty()) {
    _mouseValueX = mouseX;
    _mouseValueY = mouseY;
    _mouseValueVisible = true;
  }
}

////////////////////////////////////////////////////////
// sample the saved fields at a geographic location

vector<ClickReportField> HorizView::_getFieldValues(double lat,
                                                    double lon) const
{
  vector<ClickReportField> values;

  // The queue retains recently displayed fields, potentially with different
  // projections.  Locate the cursor independently in each saved grid.
  for (const MdvReader::FieldCopy &field : _fieldDataQueue) {
    int gridX, gridY;
    field.proj.latlon2xyIndex(lat, lon, gridX, gridY);
    const Mdvx::field_header_t &fhdr = field.fhdr;
    if (gridX < 0 || gridX >= fhdr.nx ||
        gridY < 0 || gridY >= fhdr.ny) {
      continue;
    }

    const int64_t dataIndex = (int64_t) gridY * fhdr.nx + gridX;
    if (dataIndex < 0 || dataIndex >= (int64_t) field.fl32_data.size()) {
      continue;
    }

    double value = field.fl32_data[dataIndex];
    if (!std::isfinite(value) ||
        value == fhdr.missing_data_value ||
        value == fhdr.bad_data_value) {
      continue;
    }
    if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      value = exp(value);
    }
    if (!std::isfinite(value)) {
      continue;
    }
    if (fabs(value) < 0.0001) {
      value = 0.0;
    }

    ClickReportField reportField;
    reportField.name = field.fieldName;
    reportField.units = fhdr.units;
    reportField.value = value;
    values.push_back(reportField);
  }

  return values;
}

////////////////////////////////////////////////////////
// refresh the report at the latest click location

void HorizView::_updateClickReport()
{
  if (!_haveClickLocation || _fieldDataQueue.empty()) {
    return;
  }

  const MdvReader::FieldCopy &primaryField = _fieldDataQueue.front();
  ClickReport report;
  report.dataTime = primaryField.mhdr.time_centroid;
  report.latitude = _worldClickLat;
  report.longitude = _worldClickLon;
  report.x = _worldClickX;
  report.y = _worldClickY;
  report.xUnits =
    (_proj.getProjType() == Mdvx::PROJ_LATLON) ? "deg" : "km";
  report.yUnits = report.xUnits;
  if (primaryField.fhdr.nz > 0) {
    report.height = primaryField.vhdr.level[0];
    report.heightUnits =
      Mdvx::vertTypeZUnits(primaryField.fhdr.vlevel_type);
  }
  report.fields = _getFieldValues(_worldClickLat, _worldClickLon);
  _manager.updateClickReport(report);
}

////////////////////////////////////////////////////////
// draw the value report beside the cursor

void HorizView::_drawMouseValue(QPainter &painter)
{
  const int padding = 3;
  const int cursorOffset = 8;
  const int plotLeft = std::min(_zoom->getXMinPixel(),
                                _zoom->getXMaxPixel());
  const int plotRight = std::max(_zoom->getXMinPixel(),
                                 _zoom->getXMaxPixel());
  const int plotTop = std::min(_zoom->getYMinPixel(),
                               _zoom->getYMaxPixel());
  const int plotBottom = std::max(_zoom->getYMinPixel(),
                                  _zoom->getYMaxPixel());

  const QString label = QString::fromStdString(_mouseValueText);
  const int textFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip;
  QRect textBounds = painter.fontMetrics().boundingRect
    (QRect(0, 0, plotRight - plotLeft + 1, plotBottom - plotTop + 1),
     textFlags, label);
  const int boxWidth = textBounds.width() + 2 * padding;
  const int boxHeight = textBounds.height() + 2 * padding;

  int boxX = _mouseValueX + cursorOffset;
  if (boxX + boxWidth > plotRight) {
    boxX = _mouseValueX - cursorOffset - boxWidth;
  }
  int boxY = _mouseValueY - cursorOffset - boxHeight;
  if (boxY < plotTop) {
    boxY = _mouseValueY + cursorOffset;
  }
  boxX = std::max(plotLeft,
                  std::min(boxX, std::max(plotLeft,
                                          plotRight - boxWidth + 1)));
  boxY = std::max(plotTop,
                  std::min(boxY, std::max(plotTop,
                                          plotBottom - boxHeight + 1)));

  QRect box(boxX, boxY, boxWidth, boxHeight);
  QColor background(_params.background_color);
  background.setAlpha(210);
  painter.fillRect(box, background);
  painter.setPen(_params.foreground_color);
  painter.drawText(box.adjusted(padding, padding, -padding, -padding),
                   textFlags, label);
}

////////////////////////////////////////////////////////
// keep mouse tracking enabled for every live cursor interaction

void HorizView::_updateMouseTracking()
{
  setMouseTracking(_showValue || _vsectionRouteEditing ||
                   !_params.disable_pick_mode);
}

////////////////////////////////////////////////////////
// select the displayed symbolic product nearest the cursor

void HorizView::_updateProductPick(int mouseX, int mouseY)
{
  if (_params.disable_pick_mode || !_gd.prod_mgr ||
      !_gd.prod.products_on || _renderFrameIndex < 0 ||
      _renderFrameIndex >= _gd.movie.num_frames) {
    if (_gd.prod_mgr) {
      _gd.prod_mgr->clearPickedObject();
    }
    return;
  }

  const int left = std::min(_zoom->getXMinPixel(), _zoom->getXMaxPixel());
  const int right = std::max(_zoom->getXMinPixel(), _zoom->getXMaxPixel());
  const int top = std::min(_zoom->getYMinPixel(), _zoom->getYMaxPixel());
  const int bottom = std::max(_zoom->getYMinPixel(), _zoom->getYMaxPixel());
  if (mouseX < left || mouseX > right || mouseY < top || mouseY > bottom) {
    _gd.prod_mgr->clearPickedObject();
    return;
  }

  const double worldX = _zoom->getXWorld(mouseX);
  const double worldY = _zoom->getYWorld(mouseY);
  double lat = 0.0;
  double lon = 0.0;
  _proj.xy2latlon(worldX, worldY, lat, lon);

  time_t dataTime = 0;
  const int fieldNum = _gd.h_win.page;
  if (fieldNum >= 0 && fieldNum < _gd.num_datafields &&
      _gd.mread[fieldNum]) {
    dataTime = _gd.mread[fieldNum]->h_date.utime();
  }
  const movie_frame_t &frame = _gd.movie.frame[_renderFrameIndex];
  double iconScale = 1.0;
  if (_params.scale_constant > 0.0) {
    iconScale = log10(_params.scale_constant /
                      _gd.h_win.km_across_screen) + 1.0;
    if (iconScale < 0.05) {
      iconScale = 0.0;
    }
  }
  SymprodRenderContext context(*_zoom, _gd.epoch_start, _gd.epoch_end,
                               frame.time_start, frame.time_end, dataTime,
                               _gd.h_win.km_across_screen, iconScale);
  _gd.prod_mgr->pickClosestObject(lat, lon, context,
                                  _gd.h_win.km_across_screen / 20.0);
}

////////////////////////////////////////////////////////
// draw hidden details for the product selected by cursor picking

void HorizView::_drawPickedProduct(QPainter &painter)
{
  if (_params.disable_pick_mode || !_gd.prod_mgr ||
      _renderFrameIndex < 0 ||
      _renderFrameIndex >= _gd.movie.num_frames) {
    return;
  }

  time_t dataTime = 0;
  const int fieldNum = _gd.h_win.page;
  if (fieldNum >= 0 && fieldNum < _gd.num_datafields &&
      _gd.mread[fieldNum]) {
    dataTime = _gd.mread[fieldNum]->h_date.utime();
  }
  const movie_frame_t &frame = _gd.movie.frame[_renderFrameIndex];
  double iconScale = 1.0;
  if (_params.scale_constant > 0.0) {
    iconScale = log10(_params.scale_constant /
                      _gd.h_win.km_across_screen) + 1.0;
    if (iconScale < 0.05) {
      iconScale = 0.0;
    }
  }
  SymprodRenderContext context(*_zoom, _gd.epoch_start, _gd.epoch_end,
                               frame.time_start, frame.time_end, dataTime,
                               _gd.h_win.km_across_screen, iconScale);
  painter.save();
  painter.setClipRect(QRect(std::min(_zoom->getXMinPixel(),
                                     _zoom->getXMaxPixel()),
                            std::min(_zoom->getYMinPixel(),
                                     _zoom->getYMaxPixel()),
                            abs(_zoom->getXMaxPixel() -
                                _zoom->getXMinPixel()) + 1,
                            abs(_zoom->getYMaxPixel() -
                                _zoom->getYMinPixel()) + 1));
  _gd.prod_mgr->drawPickedQt(painter, context);
  painter.restore();
}

/*************************************************************************
 * react to click point from remote display - Sprite
 * redraw the click point cursor
 */

void HorizView::setClickPoint(double azimuthDeg,
                              double elevationDeg,
                              double rangeKm)
{

  double x_km =
    rangeKm * sin(azimuthDeg * DEG_TO_RAD) * cos(elevationDeg * DEG_TO_RAD);
  double y_km =
    rangeKm * cos(azimuthDeg * DEG_TO_RAD) * cos(elevationDeg * DEG_TO_RAD);

  _mouseReleaseX = _zoom->getIxPixel(x_km);
  _mouseReleaseY = _zoom->getIyPixel(y_km);
  _pointClicked = true;

  update();

}

/*************************************************************************
 * initialize the geographic projection
 */

void HorizView::_initProjection()
{

  if (_params.proj_type == Params::PROJ_LATLON) {
    _proj.initLatlon(_params.proj_origin_lon);
  } else if (_params.proj_type == Params::PROJ_FLAT) {
    _proj.initFlat(_params.proj_origin_lat,
                   _params.proj_origin_lon,
                   _params.proj_rotation);
  } else if (_params.proj_type == Params::PROJ_LAMBERT_CONF) {
    _proj.initLambertConf(_params.proj_origin_lat,
                          _params.proj_origin_lon,
                          _params.proj_lat1,
                          _params.proj_lat2);
  } else if (_params.proj_type == Params::PROJ_POLAR_STEREO) {
    Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
    if (!_params.proj_pole_is_north) {
      poleType = Mdvx::POLE_SOUTH;
    }
    _proj.initPolarStereo(_params.proj_tangent_lon,
                          poleType,
                          _params.proj_central_scale);
  } else if (_params.proj_type == Params::PROJ_OBLIQUE_STEREO) {
    _proj.initObliqueStereo(_params.proj_origin_lat,
                            _params.proj_origin_lon,
                            _params.proj_tangent_lat,
                            _params.proj_tangent_lon,
                            _params.proj_central_scale);
  } else if (_params.proj_type == Params::PROJ_MERCATOR) {
    _proj.initMercator(_params.proj_origin_lat,
                       _params.proj_origin_lon);
  } else if (_params.proj_type == Params::PROJ_TRANS_MERCATOR) {
    _proj.initTransMercator(_params.proj_origin_lat,
                            _params.proj_origin_lon,
                            _params.proj_central_scale);
  } else if (_params.proj_type == Params::PROJ_ALBERS) {
    _proj.initAlbers(_params.proj_origin_lat,
                     _params.proj_origin_lon,
                     _params.proj_lat1,
                     _params.proj_lat2);
  } else if (_params.proj_type == Params::PROJ_LAMBERT_AZIM) {
    _proj.initLambertAzim(_params.proj_origin_lat,
                          _params.proj_origin_lon);
  }

}

/*************************************************************************
 * initialize zooms
 */

void HorizView::_initZooms()
{

  _zoomLevel = 0;
  _markZoomChanged();
  
  // create defined zooms
  
  for(int izoom = 0; izoom < _params.zoom_levels_n; izoom++) {

    const Params::zoom_level_t &zoomLevel = _params._zoom_levels[izoom];
    
    double minx = zoomLevel.min_x;
    double miny = zoomLevel.min_y;
    double maxx = zoomLevel.max_x;
    double maxy = zoomLevel.max_y;
    
    Params::zoom_units_t units = zoomLevel.units;
    
    if (_proj.getProjType() == Mdvx::PROJ_LATLON &&
        units == Params::ZOOM_LIMITS_IN_KM) {
      
      // convert km limits to lat/lon
      
      double minKmX = minx;
      double maxKmX = maxx;
      double minKmY = miny;
      double maxKmY = maxy;
      
      _proj.xy2latlon(minKmX, minKmY, miny, minx);
      _proj.xy2latlon(maxKmX, maxKmY, maxy, maxx);
      
    } else if (_proj.getProjType() != Mdvx::PROJ_LATLON &&
               units == Params::ZOOM_LIMITS_IN_DEG) {
      
      // convert lat/lon limits to km
      
      double minLon = minx;
      double maxLon = maxx;
      double minLat = miny;
      double maxLat = maxy;
      
      _proj.latlon2xy(minLat, minLon, minx, miny);
      _proj.latlon2xy(maxLat, maxLon, maxx, maxy);

    }
    
    WorldPlot world;
    _initWorld(world, zoomLevel.label);
    world.setWorldLimits(minx, miny, maxx, maxy);
    _definedZooms.push_back(world);
    
    if (strcmp(zoomLevel.label, _params.start_zoom_label) == 0) {
      _zoomLevel = izoom;
    }
    
  } // izoom
  
  _zoom = &_definedZooms[_zoomLevel];
  _updateGlobalCurrentZoom();
  
}

////////////////////////////////////////////
// copy current zoom limits to globals

void HorizView::_updateGlobalCurrentZoom()
{
  _gd.h_win.cmin_y = _zoom->getYMinWorld();
  _gd.h_win.cmax_y = _zoom->getYMaxWorld();
  _gd.h_win.cmin_x = _zoom->getXMinWorld();
  _gd.h_win.cmax_x = _zoom->getXMaxWorld();
}

/*************************************************************************
 * initialize WorldPlot()
 */

void HorizView::_initWorld(WorldPlot &world,
                           const string &name)

{

  world.setName(name);
  world.setWindowGeom(width(), height(), 0, 0);
  
  world.setLeftMargin(_params.horiz_left_margin);
  world.setRightMargin(_params.horiz_right_margin);
  world.setTopMargin(_params.horiz_top_margin);
  world.setBottomMargin(_params.horiz_bot_margin);
  world.setTitleTextMargin(_params.horiz_title_text_margin);
  world.setLegendTextMargin(_params.horiz_legend_text_margin);
  world.setAxisTextMargin(_params.horiz_axis_text_margin);
  world.setColorScaleWidth(_params.horiz_color_scale_width);

  world.setXAxisTickLen(_params.horiz_axis_tick_len);
  world.setXNTicksIdeal(_params.horiz_n_ticks_ideal);
  world.setYAxisTickLen(_params.horiz_axis_tick_len);
  world.setYNTicksIdeal(_params.horiz_n_ticks_ideal);

  world.setTitleFontSize(_params.horiz_title_font_size);
  world.setAxisLabelFontSize(_params.horiz_axis_label_font_size);
  world.setTickValuesFontSize(_params.horiz_tick_values_font_size);
  world.setLegendFontSize(_params.horiz_legend_font_size);

  world.setTitleColor(_params.horiz_title_color);
  world.setAxisLineColor(_params.horiz_axes_color);
  world.setAxisTextColor(_params.horiz_axes_color);
  world.setGridColor(_params.horiz_grid_color);

  world.setProjection(_proj);
  
}

//////////////////////////////////////////
// set flags to control frame rendering

void HorizView::triggerGridRendering(int page, int index)

{

  _renderFramePage = page;
  _renderFrameIndex = index;

  // save field data to queue

  _saveFieldDataToQueue(page);

  // render data grids to grid image in WorldPlot
  
  _renderGrids();
  
  // render invalid images
  
  if (_renderInvalidImages) {
    _doRenderInvalidImages(_invalidImagesFrameIndex, _vert);
    _renderInvalidImages = false;
  }
  
  // A rendered field and its live color scale form one visual update. Force a
  // complete paint now so the inexpensive scale redraw cannot lag the cached
  // grid image.
  repaint();
  
}

/////////////////////////////////////////////////////////////
// rerender retained data without changing its request state

void HorizView::rerenderGrid()
{
  const int fieldNum = _gd.h_win.page;
  if (_renderFrameIndex >= 0 &&
      _renderFrameIndex < _gd.movie.num_frames &&
      fieldNum >= 0 && fieldNum < _gd.num_datafields &&
      _gd.mread[fieldNum] != nullptr &&
      _gd.mread[fieldNum]->isValidH()) {
    _renderGrids();
  }

  // A horizontal read temporarily invalidates the reader while its buffers
  // are replaced. Avoid regenerating the cached grid in that interval, but
  // still repaint live annotations such as an interactively edited color bar.
  repaint();
}

/////////////////////////////////////////////////////////////
// rerender symbolic products from the retained SPDB data

void HorizView::rerenderProducts()
{
  _renderProducts();
  repaint();
}

/////////////////////////////////////////////////////////////
// manage immutable composites used by movie playback

void HorizView::clearMovieFrames(int frameCount)
{
  _playbackFrameIndex = -1;
  _movieFrames.assign(std::max(0, frameCount), QImage());
  update();
}

bool HorizView::hasMovieFrame(int frameIndex) const
{
  return frameIndex >= 0 &&
    frameIndex < static_cast<int>(_movieFrames.size()) &&
    !_movieFrames[frameIndex].isNull();
}

QImage HorizView::movieFrameImage(int frameIndex) const
{
  if (!hasMovieFrame(frameIndex)) {
    return QImage();
  }
  // QImage copies are implicitly shared, so the exporter gets stable
  // value semantics without duplicating the complete movie in memory.
  return _movieFrames[frameIndex];
}

bool HorizView::captureMovieFrame(int frameIndex)
{
  if (frameIndex < 0 || frameIndex >= static_cast<int>(_movieFrames.size())) {
    return false;
  }
  _playbackFrameIndex = -1;
  repaint();
  _movieFrames[frameIndex] = grab().toImage();
  return !_movieFrames[frameIndex].isNull();
}

bool HorizView::showMovieFrame(int frameIndex)
{
  if (!hasMovieFrame(frameIndex)) {
    return false;
  }
  _playbackFrameIndex = frameIndex;
  repaint();
  return true;
}

void HorizView::stopMoviePlayback()
{
  _playbackFrameIndex = -1;
  repaint();
}

void HorizView::setMovieInteractionActive(bool active)
{
  if (!active) {
    return;
  }

  // Cached playback cannot provide frame-specific data for transient canvas
  // tools. Clear any gesture or sample inherited from live mode so it cannot
  // reappear with stale coordinates when playback stops.
  _activeMouseButton = Qt::NoButton;
  _panPreviewActive = false;
  _mouseValueVisible = false;
  _mouseValueText.clear();
  unsetCursor();
  if (_rubberBand) {
    _rubberBand->hide();
  }
  if (_vsectionRouteEditing) {
    _cancelVsectionRoute();
  }
  if (_gd.prod_mgr) {
    _gd.prod_mgr->clearPickedObject();
  }
  update();
}
  
//////////////////////////////////////////
// set flags to check for invalid images

void HorizView::setRenderInvalidImages(int index, VertView *vert)

{
  _renderInvalidImages = true;
  _invalidImagesFrameIndex = index;
  _vert = vert;
  update(); // call paint event
}
  
/*************************************************************************
 * _renderGrids()
 */

void HorizView::_renderGrids()
{
  
  if(_gd.debug2) {
    fprintf(stderr,
            "Rendering Horizontal movie_frame %d - field: %d\n",
            _renderFrameIndex, _renderFramePage);
  }

  _controlRenderGrid(_renderFramePage,
                     _gd.movie.frame[_renderFrameIndex].time_start,
                     _gd.movie.frame[_renderFrameIndex].time_end);
  

  _gridsReady = true;

  _renderProducts();
  
}

/////////////////////////////////////////////////////////////
// render symbolic products into their independent overlay

void HorizView::_renderProducts()
{
  if (!_zoom || !_gd.prod_mgr || _renderFrameIndex < 0 ||
      _renderFrameIndex >= _gd.movie.num_frames) {
    return;
  }

  time_t dataTime = 0;
  const int fieldNum = _gd.h_win.page;
  if (fieldNum >= 0 && fieldNum < _gd.num_datafields &&
      _gd.mread[fieldNum] != nullptr) {
    dataTime = _gd.mread[fieldNum]->h_date.utime();
  }
  const movie_frame_t &frame = _gd.movie.frame[_renderFrameIndex];
  _zoom->renderSymprods(*_gd.prod_mgr,
                        _gd.epoch_start, _gd.epoch_end,
                        frame.time_start, frame.time_end,
                        dataTime, _gd.h_win.km_across_screen);
}

/************************************************************************
 * RENDER_INVALID_IMAGES: Check for images in which the data 
 * are no longer valid. Look for the "best" invalid  image to 
 *  render.
 *
 */

void HorizView::_doRenderInvalidImages(int index, VertView *vert)
{

#ifdef JUNK
  
  int i;
  int h_image,v_image;
  int stat;
  int none_found = 1;
  QPaintDevice *pdev;
  
  h_image = _gd.h_win.page + 1;
  v_image = _gd.v_win.page + 1;
  if(!_params.run_once_and_exit)  PMU_auto_register("Checking Images (OK)");
  
  /* look through the rest of the images  */
  for (i=0; i < _gd.num_datafields-1; i++) {    
    
    /*
     * Render horizontal image, if necessary.
     */
    
    if (h_image >= _gd.num_datafields) {
      h_image = 0;
    }
    
    if (_gd.mread[h_image]->currently_displayed && _gd.mread[h_image]->auto_render) {
      
      if (_gd.h_win.redraw_flag[h_image] || (_gd.mread[h_image]->h_data_valid == 0)) {
        none_found = 0;
        stat = gather_hwin_data(h_image,
                                _gd.movie.frame[index].time_start,
                                _gd.movie.frame[index].time_end);
        if (stat == CIDD_SUCCESS) {
          if(_gd.mread[h_image]->auto_render) {
            pdev = _gd.h_win.page_pdev[h_image];
          } else {
            pdev = _gd.h_win.tmp_pdev;
          }
          QPainter painter(this);
          _controlRenderGrid(painter,
                             h_image,
                             _gd.movie.frame[index].time_start,
                             _gd.movie.frame[index].time_end);
          
          save_h_movie_frame(index,pdev,h_image);
          
          _gd.h_win.redraw_flag[h_image] = 0;
        } else {
          return;
        }
        if (h_image == _gd.h_win.prev_page && _gd.h_win.redraw_flag[h_image] == 0) {
          _gd.h_copy_flag = 1;
        }
      } // if (_gd.h_win.redraw_flag[h_image] ...
    } // if (_gd.mread[h_image]->currently_displayed ...
    h_image++;

    /*
     * Render vertical image, if necessary.
     */

    if (v_image >= _gd.num_datafields) v_image = 0;

    if (_gd.mread[v_image]->currently_displayed && _gd.mread[v_image]->auto_render) {
      if ((_gd.v_win.active) && (_gd.v_win.redraw_flag[v_image] || (_gd.mread[v_image]->v_data_valid == 0))) {
        stat = gather_vwin_data(v_image, _gd.movie.frame[index].time_start,
                                _gd.movie.frame[index].time_end);
        if (stat == CIDD_SUCCESS) {
          if(_gd.mread[v_image]->auto_render) {
            pdev = _gd.v_win.page_pdev[v_image];
          } else {
            pdev = _gd.v_win.tmp_pdev;
          }
          QPainter painter(this);
#ifdef NOTYET
          vert->renderVertDisplay(painter, v_image, _gd.movie.frame[index].time_start,
                                  _gd.movie.frame[index].time_end);
#endif
          _gd.v_win.redraw_flag[v_image] = 0;
        } else {
          return;
        }
        if (v_image == _gd.v_win.prev_page && _gd.v_win.redraw_flag[v_image] == 0) _gd.v_copy_flag = 1;
      }
    }
        
    v_image++;
  }

  // At this point all background images have been rendered. and nothing else is
  //  happening

  // In html mode, cycle through all zooms and heights
  if(none_found && _params.html_mode && _gd.io_info.outstanding_request == 0) {
    
    /* If more zoom levels to render */
    if(_gd.h_win.zoom_level < (_gd.h_win.num_zoom_levels -  NUM_CUSTOM_ZOOMS - 2)) {

      /* Set zoom to next level */
      _gd.h_win.zoom_level++;
      // set_domain_proc(_gd.zoom_pu->domain_st,_gd.h_win.zoom_level,NULL);

      // If more heights to render
    } else if (_gd.cur_render_height < _gd.num_render_heights -1) {

      // Set height to next level
      _gd.cur_render_height++;
      if(_gd.debug) fprintf(stderr,"HTML_MODE: Height now: %g\n",_gd.h_win.cur_ht);
      _gd.h_win.cur_ht = _gd.height_array[_gd.cur_render_height];

      // Reset Zoom back to first  level
      _gd.h_win.zoom_level = 0;
      // set_domain_proc(_gd.zoom_pu->domain_st,_gd.h_win.zoom_level,NULL);

      // Make sure new data gets loaded
      reset_data_valid_flags(1,0);
      reset_terrain_valid_flags(1,0);
               
      // No more heights and no more zooms to render
    } else if(_params.run_once_and_exit) {
      if(!_gd.quiet_mode)  fprintf(stderr,"Exiting\n");
      // xv_destroy(_gd.h_win_horiz_bw->horiz_bw);
      exit(-1);
    }
  }
#endif
  return;
}

/**********************************************************************
 * RENDER_HORIZ_DISPLAY: Render a complete horizontal plane of data a
 *        and its associated overlays and labels  labels. 
 */

int HorizView::_controlRenderGrid(int page,
                                  time_t start_time,
                                  time_t end_time)
{

  if(_params.images_creation_mode != Params::CREATE_IMAGES_THEN_EXIT) {
    PMU_auto_register("Rendering (OK)");
  }

  if(_gd.debug2) fprintf(stderr,"Rendering Plan View Image, page :%d\n",page);

  // compute distance across the image for setting font sizes, etc.
  switch(_gd.display_projection) {
    default:
    case Mdvx::PROJ_FLAT :
    case Mdvx::PROJ_LAMBERT_CONF :
      /* compute km across the image */
      _gd.h_win.km_across_screen = (_gd.h_win.cmax_x - _gd.h_win.cmin_x);
      break;

    case Mdvx::PROJ_LATLON :
      _gd.h_win.km_across_screen = (_gd.h_win.cmax_x - _gd.h_win.cmin_x) * KM_PER_DEG_AT_EQ;
      break;
  }

  MdvReader *mr = _gd.mread[page];
 
  // Clear time lists
  // if(_gd.time_plot) _gd.time_plot->clear_grid_tlist();
  // if(_gd.time_plot) _gd.time_plot->clear_prod_tlist();

  // if(_params.show_data_messages) {
  //   // gui_label_h_frame("Rendering",-1);
  // }

  // RENDER the LAND_USE field first
  if(_gd.layers.earth.landuse_active && _gd.layers.earth.land_use != NULL) {
    _renderFieldGrid(page, _gd.layers.earth.land_use, start_time, end_time,1);
    // render_grid(xid,_gd.layers.earth.land_use,start_time,end_time,1);
  }

  if(!_params.draw_main_on_top) {
    _renderFieldGrid(page, mr, start_time, end_time, 0);
    if(_gd.layers.earth.terrain_active &&
       (mr->ds_fhdr.nz > 1) &&
       ((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0) &&
       (mr->composite_mode == FALSE) &&
       mr->ds_fhdr.vlevel_type != Mdvx::VERT_TYPE_ELEV) {
#ifdef HAVE_XID
      render_h_terrain(xid, page);
#endif
    }
  }
     
  /* Render each of the gridded_overlay fields */
  for(int i=0; i < Constants::NUM_GRID_LAYERS; i++) {
    if(_gd.layers.overlay_field_on[i] && _gd.mread[_gd.layers.overlay_field[i]] != NULL) {
      _renderFieldGrid(page, _gd.mread[_gd.layers.overlay_field[i]],start_time,end_time,1);
      // render_grid(xid,_gd.mread[_gd.layers.overlay_field[i]],start_time,end_time,1);
    }
  }
  
  if(_params.draw_main_on_top) {
    _renderFieldGrid(page, mr, start_time, end_time, 0);
    if(_gd.layers.earth.terrain_active && 
       ((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0) &&
       (mr->composite_mode == FALSE) && (mr->ds_fhdr.nz > 1)) {

#ifdef HAVE_XID
      render_h_terrain(xid, page);
#endif
    }
  }

  /* render contours if selected */
  for(int i= 0; i < Constants::NUM_CONT_LAYERS; i++) {
    if(_gd.layers.cont[i].active) {
      if (_gd.layers.use_alt_contours) {
#ifdef HAVE_XID
        RenderLineContours(xid, &(_gd.layers.cont[i]));
#endif
      } else {
#ifdef HAVE_XID
        render_line_contours(xid, &(_gd.layers.cont[i]));
#endif
      }
    }
  }

  /* render Winds if selected */
  if(_gd.layers.wind_vectors) {
    switch(_gd.layers.wind_mode) {
      default:
      case WIND_MODE_ON:  /* winds get rendered in each frame */
#ifdef HAVE_XID
        render_wind_vectors(xid,start_time,end_time);
#endif
        break;
        
      case WIND_MODE_LAST: /* Winds get rendered in last farame only */
        if(_gd.movie.cur_frame == _gd.movie.end_frame)
#ifdef HAVE_XID
          render_wind_vectors(xid,start_time,end_time);
#endif
        break;

      case WIND_MODE_STILL: /* Winds get rendered in the last frame only
                             * if the movie loop is off
                             */
        if(!_gd.movie.movie_on && _gd.movie.cur_frame == _gd.movie.end_frame)
#ifdef HAVE_XID
          render_wind_vectors(xid,start_time,end_time);
#endif
        break;
    }
  }


#ifdef HAVE_XID
  render_top_layers(xid);  // Range rings X section reference etc.
#endif

#ifdef HAVE_XID
  render_horiz_margins(xid,page,start_time,end_time);
#endif

  // update_frame_time_msg(_gd.movie.cur_frame);

  return Constants::CIDD_SUCCESS;    /* avaliable data has been rendered */
}

/**********************************************************************
 * RENDER_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int HorizView::_renderFieldGrid(int page,
                                MdvReader *mr,
                                time_t start_time,
                                time_t end_time,
                                bool is_overlay_field)
  
{
  
  if (mr == NULL) {
    return Constants::CIDD_SUCCESS;
  }

  // Render with appropriate rendering routine
  
  Mdvx::Mdvx::projection_type_t projType = mr->proj.getProjType();
  const PjgMath &dataMath = mr->proj.getPjgMath();
  const PjgMath &displayMath = _proj.getPjgMath();
  
  if (mr->render_method == NONE) {
    _zoom->prepareGridImageForUnfilledField(is_overlay_field);
  } else if (projType == Mdvx::PROJ_POLAR_RADAR) {
    _zoom->renderGridRadarPolar(page, mr,
                                start_time, end_time,
                                is_overlay_field);
  } else if (mr->render_method == FILLED_CONTOURS ||
             (mr->render_method == DYNAMIC_CONTOURS &&
              (int64_t) mr->h_fhdr.nx * mr->h_fhdr.ny <
                _params.dynamic_contour_threshold)) {
    // Polar data retains its gate polygons, while Cartesian grids share one
    // contour path which handles any projection distortion internally.
    _zoom->renderFilledContours(page, mr,
                                start_time, end_time,
                                is_overlay_field);
  } else if (dataMath == displayMath) {
    _zoom->renderGridRect(page, mr,
                          start_time, end_time,
                          is_overlay_field);
  } else {
    _zoom->renderGridDistorted(page, mr,
                               start_time, end_time,
                               is_overlay_field);
  }

  // Line contours are an overlay choice independent of how the underlying
  // colors were filled. Polar radar gates retain their native geometry and do
  // not currently have a Cartesian contour topology suitable for this path.
  if (mr->overlay_line_contours && projType != Mdvx::PROJ_POLAR_RADAR) {
    _zoom->renderLineContours(mr);
  }

#ifdef NOTYET
  
  int out_of_date = 0;
  int stretch_secs = (int) (60.0 * mr->time_allowance);
  char message[1024];    /* Error message area */
  
  if(_params.check_data_times) {
    if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
    if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
  }

  // Add the list of times to the time plot
  // if(mr->time_list.num_entries > 0) {
  //   if(_gd.time_plot) _gd.time_plot->add_grid_tlist(mr->legend_name,mr->time_list.tim,
  //                                                 mr->time_list.num_entries,
  //                                                 mr->h_date.unix_time);
  // }

  /* For the Main Field - Clear the screen and  Print a special message 
   * and draw the wall clock time if the data is not availible 
   */
  if(!is_overlay_field) {


    /* If no data in current response or data is way out of date */
    if( mr->h_data == NULL || out_of_date ) {

      if(strncasecmp(mr->button_name,"None",4) &&
         strncasecmp(mr->button_name,"Empty",5)) {
        /* display "Data Not Available" message */
        if(out_of_date) {
          snprintf(message, 1023, "%s - Data too Old", _params.no_data_message);
        } else {
          STRcopy(message, _params.no_data_message, 1023);
        }

        int xmid,ymid;
        Font font = choose_font(message, _gd.h_win.img_dim.width, _gd.h_win.img_dim.height, &xmid, &ymid);
        XSetFont(_gd.dpy,_gd.legends.foreground_color->gc,font);
        XDrawImageString(_gd.dpy,xid,_gd.legends.foreground_color->gc,
                         _gd.h_win.margin.left + (_gd.h_win.img_dim.width /2) + xmid  ,
                         _gd.h_win.margin.top + (_gd.h_win.img_dim.height /4) + ymid ,
                         message,strlen(message));

        if(_gd.debug2) {
          fprintf(stderr, "No data from service: %s\n",
                  _gd.io_info.mr->url );
        }

	if(_params.show_clock) {
          /* draw a clock */
          int ht = (int) (_gd.h_win.can_dim.height * 0.05);
          int startx = _gd.h_win.can_dim.width - _gd.h_win.margin.right - ht - 5;
          int starty = _gd.h_win.margin.top + ht + 5;
          XUDRdraw_clock(_gd.dpy,xid,_gd.legends.foreground_color->gc,
                         startx,starty,ht, (start_time + (end_time - start_time) /2),1);
	}
      }  // if mr->button_name != "None"

      return CIDD_FAILURE;
    }
  }
     
  set_busy_state(1);

#endif

#ifdef NOTNOW
  switch(mr->h_fhdr.proj_type) {
    default: // Projections which need only matching types and origins.
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == _proj.getProjType() &&
         (fabs(_gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(_gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoom->renderGridRect(page, mr,
                              start_time, end_time,
                              is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else { // Must use polygon rendering
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoom->renderGridDistorted(page, mr,
                                   start_time, end_time,
                                   is_overlay_field);
      }
      break;

    case  Mdvx::PROJ_FLAT: // Needs to test Param 1
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == _proj.getProjType() &&
         (fabs(_proj_param[0] - mr->h_fhdr.proj_param[0]) < 0.001) &&
         (fabs(_gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(_gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoom->renderGridRect(page, mr,
                              start_time, end_time,
                              is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else { // Must use polygon rendering
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoom->renderGridDistorted(page, mr,
                                   start_time, end_time,
                                   is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() done\n");
      }
      break;
      
    case  Mdvx::PROJ_LAMBERT_CONF: // Needs to test param 1 & 2
    case  Mdvx::PROJ_POLAR_STEREO: 
    case  Mdvx::PROJ_OBLIQUE_STEREO:
    case  Mdvx::PROJ_MERCATOR:
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == _proj.getProjType() &&
         (fabs(_proj_param[0] - mr->h_fhdr.proj_param[0]) < 0.001) &&
         (fabs(_proj_param[1] - mr->h_fhdr.proj_param[1]) < 0.001) &&
         (fabs(_gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(_gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoom->renderGridRect(page, mr,
                              start_time, end_time,
                              is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else { // Must use polygon rendering
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoom->renderGridDistorted(page, mr,
                                   start_time, end_time,
                                   is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() done\n");
      }
      break;
      
    case  Mdvx::PROJ_LATLON:
      if(mr->h_fhdr.proj_type == _proj.getProjType()) {
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoom->renderGridRect(page, mr,
                              start_time, end_time,
                              is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else {
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoom->renderGridDistorted(page, mr,
                                   start_time, end_time,
                                   is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() done\n");
      }
      break;
      
    case  Mdvx::PROJ_POLAR_RADAR:
      if(_gd.debug2) fprintf(stderr,"render_polar_grid() selected\n");
#ifdef NOTYET
      render_polar_grid(page, mr,start_time, end_time, 
                        is_overlay_field);
#endif
      if(_gd.debug2) fprintf(stderr,"render_polar_grid() done\n");
      break;
      
  }

  set_busy_state(0);

#endif

  // plot legends

  if (mr != nullptr) {
    char line[2048];
    snprintf(line, 2048,
             "%s", mr->fieldLabelHoriz().c_str());
    vector<string> legendText;
    legendText.push_back(line);
    _zoom->drawLegends(legendText, _params.horiz_main_legend_pos);
  }
  
  return Constants::CIDD_SUCCESS;
}

/*************************************************************************
 * zoomBack the view
 */

void HorizView::zoomBackView()
{
  if (_manager.isMovieActive()) {
    _manager.showMovieInteractionBlocked();
    return;
  }
  if (_customZooms.size() <= 1) {
    // go back to defined zooms
    _customZooms.clear();
    _zoom = &_definedZooms[_zoomLevel];
  } else {
    // go back to previous custom zoom
    _customZooms.pop_back();
    _zoom = &_customZooms[_customZooms.size()-1];
  }
  _updateGlobalCurrentZoom();
  _markZoomChanged();
  update();
}

/*************************************************************************
 * zoom all the way out
 */

void HorizView::zoomOutView()
{
  if (_manager.isMovieActive()) {
    _manager.showMovieInteractionBlocked();
    return;
  }
  _customZooms.clear();
  _zoom = &_definedZooms[_zoomLevel];
  _updateGlobalCurrentZoom();
  _markZoomChanged();
  update();
}

//////////////////////////////////////////////////
// set zoom index in _definedZooms
// called from GuiManager in response to zoom menu selection

void HorizView::setZoomIndex(int zoomIndex)
{
  if (_manager.isMovieActive()) {
    _manager.showMovieInteractionBlocked();
    return;
  }
  if (zoomIndex > (int) _definedZooms.size() - 1) {
    zoomIndex = (int) _definedZooms.size() - 1;
  }
  _zoomLevel = zoomIndex;
  _zoom = &_definedZooms[_zoomLevel];
  _updateGlobalCurrentZoom();
  _markZoomChanged();
  update();
}

/////////////////////////////////////////////////////////
// check for zoom change

bool HorizView::checkForZoomChange()
{
  if (_zoomRequestPending) {
    _zoomRequestPending = false;
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////////////////////////////
// invalidate live layers after the display transform changes

void HorizView::_markZoomChanged()
{
  // Request and paint state are intentionally separate. The GUI timer may
  // consume the request before Qt delivers a paint event; retaining the paint
  // flag prevents a newly copied WorldPlot from exposing empty image buffers.
  _zoomRequestPending = true;
  _zoomChanged = true;
  _gridsReady = false;
  _mapsReady = false;
}

/*************************************************************************
 * setRings()
 */

void HorizView::setRingsFixed(const bool enabled)
{
  _params.plot_range_rings_fixed = (enabled?pTRUE:pFALSE);
  _manager.setOverlaysHaveChanged(true);
}

void HorizView::setRingsDataDriven(const bool enabled)
{
  _params.plot_range_rings_from_data = (enabled?pTRUE:pFALSE);
  _manager.setOverlaysHaveChanged(true);
}


/*************************************************************************
 * setGrids()
 */

void HorizView::setGrids(const bool enabled)
{
  _params.horiz_grids_on_at_startup = (enabled?pTRUE:pFALSE);
  _manager.setOverlaysHaveChanged(true);
}


/*************************************************************************
 * getImage()
 */

QImage* HorizView::getImage()
{
  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;
}


/*************************************************************************
 * getPixmap()
 */

QPixmap* HorizView::getPixmap()
{
  QPixmap* pixmap = new QPixmap(grab());
  return pixmap;
}


/*************************************************************************
 * Slots
 *************************************************************************/

/*************************************************************************
 * mousePressEvent()
 */

void HorizView::mousePressEvent(QMouseEvent *e)
{

  if (_manager.isMovieActive()) {
    _activeMouseButton = Qt::NoButton;
    _manager.showMovieInteractionBlocked();
    e->accept();
    return;
  }

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  _activeMouseButton = effectiveButton(e);

  _mousePressX = pos.x();
  _mousePressY = pos.y();
  _worldPressX = _zoom->getXWorld(_mousePressX);
  _worldPressY = _zoom->getYWorld(_mousePressY);

  if (_activeMouseButton == Qt::RightButton) {
    if (e->modifiers() & Qt::ShiftModifier) {
      // Ordinary right-button gestures belong to vertical-section selection.
      // Retain access to the existing context menu through an explicit
      // modifier so the two interactions cannot become ambiguous.
      _activeMouseButton = Qt::NoButton;
      emit customContextMenuRequested(pos.toPoint());
      return;
    }

    setFocus(Qt::MouseFocusReason);
    if (!_vsectionRouteEditing) {
      _vsectionRoute.clear();
      _vsectionRouteEditing =
        _appendVsectionRoutePoint(pos.x(), pos.y());
      _updateMouseTracking();
    }
    _vsectionPreviewX = pos.x();
    _vsectionPreviewY = pos.y();
    update();
  } else if (_activeMouseButton == Qt::LeftButton) {
    if (!_rubberBand) {
      _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    }
    _rubberBand->setGeometry(pos.x(), pos.y(), 0, 0);
    _rubberBand->show();
  } else if (_activeMouseButton == Qt::MiddleButton) {
    _panPreviewActive = true;
    _panPreviewX = _mousePressX;
    _panPreviewY = _mousePressY;
    _mouseValueVisible = false;
    setCursor(Qt::ClosedHandCursor);
  }
}


/*************************************************************************
 * mouseMoveEvent(), mouse button is down and mouse is moving
 */

void HorizView::mouseMoveEvent(QMouseEvent * e)
{

  if (_manager.isMovieActive()) {
    return;
  }

  // Zooming with the mouse

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  int xx = pos.x();
  int yy = pos.y();
  int deltaX = xx - _mousePressX;
  int deltaY = yy - _mousePressY;

  if (_vsectionRouteEditing) {
    _vsectionPreviewX = xx;
    _vsectionPreviewY = yy;
    update();
  }

  if (_showValue && _activeMouseButton != Qt::MiddleButton) {
    _updateMouseValue(xx, yy);
    update();
  }

  if (_activeMouseButton == Qt::NoButton && !_params.disable_pick_mode) {
    _updateProductPick(xx, yy);
    update();
  }

  // Mouse tracking also delivers moves with no gesture in progress.  Right
  // gestures edit the section route and must not create a zoom band.

  if (_activeMouseButton == Qt::NoButton ||
      _activeMouseButton == Qt::RightButton) {
    return;
  }

  if (_activeMouseButton == Qt::MiddleButton) {
    _panPreviewX = xx;
    _panPreviewY = yy;
    update();
    return;
  }

  // Make the rubberband aspect ratio match that
  // of the window

  double aspectRatio =
    (double) _zoom->getPlotHeight() / (double) _zoom->getPlotWidth();
  
  double dx = std::max(fabs((double)deltaX),
                       fabs((double)deltaY) / aspectRatio);
  double dy = dx * aspectRatio;
  
  // double dx = fabs(deltaY / aspectRatio);
  // double dy = fabs(dx * aspectRatio);
  
  // Preserve the signs

  if (deltaX < 0) dx = -dx;
  if (deltaY < 0) dy = -dy;

  int moveX = (int) floor(dx + 0.5);
  int moveY = (int) floor(dy + 0.5);

  QRect newRect = QRect(_mousePressX, _mousePressY, moveX, moveY);

  _zoomCornerX = _mousePressX + moveX;
  _zoomCornerY = _mousePressY + moveY;

  newRect = newRect.normalized();

  if (_rubberBand) {
    _rubberBand->setGeometry(newRect);
  }

}

////////////////////////////////////////////////////////
// clear the cursor value when leaving the canvas

void HorizView::leaveEvent(QEvent *event)
{
  _mouseValueVisible = false;
  _mouseValueText.clear();
  if (_gd.prod_mgr) {
    _gd.prod_mgr->clearPickedObject();
  }
  update();
  QWidget::leaveEvent(event);
}

/*************************************************************************
 * mouseReleaseEvent()
 */
void HorizView::mouseReleaseEvent(QMouseEvent *e)
{

  if (_manager.isMovieActive()) {
    _activeMouseButton = Qt::NoButton;
    _panPreviewActive = false;
    unsetCursor();
    if (_rubberBand) {
      _rubberBand->hide();
    }
    e->accept();
    return;
  }

  _pointClicked = false;

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  // Use the button normalized at press time.  In particular, an Option-drag
  // remains a middle-button gesture even if Option is released first.

  const Qt::MouseButton gestureButton = _activeMouseButton;
  _activeMouseButton = Qt::NoButton;

  if (gestureButton == Qt::MiddleButton) {
    _panPreviewActive = false;
    unsetCursor();
    _panView(pos.x(), pos.y());
    if (_showValue) {
      _updateMouseValue(pos.x(), pos.y());
    }
    update();
    return;
  }
  if (gestureButton == Qt::RightButton) {
    if (_vsectionRouteEditing) {
      _vsectionPreviewX = pos.x();
      _vsectionPreviewY = pos.y();
      const bool endpointAdded =
        _appendVsectionRoutePoint(pos.x(), pos.y());

      // Single-segment mode treats one drag as the complete interaction. If
      // the endpoint is invalid, discard the starting point so a later drag
      // cannot unintentionally continue it as a multi-segment route.

      if (_params.single_segment_vsection) {
        if (endpointAdded) {
          _completeVsectionRoute();
        } else {
          _cancelVsectionRoute();
        }
      } else {
        update();
      }
    }
    return;
  }
  if (gestureButton == Qt::NoButton) {
    if (_rubberBand) {
      _rubberBand->hide();
    }
    return;
  }

  _mouseReleaseX = pos.x();
  _mouseReleaseY = pos.y();

  double distX = _mouseReleaseX - _mousePressX;
  double distY = _mouseReleaseY - _mousePressY;
  double distMoved = sqrt(distX * distX + distY * distY);
  
  if (distMoved <= 20) {
    
    // mouse moved less than 20 pixels, so we interpret that as a click
    // get click location in world coords
    
    _worldReleaseX = _zoom->getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoom->getYWorld(_mouseReleaseY);

    _pointClicked = true;
    _worldClickX = _worldReleaseX;
    _worldClickY = _worldReleaseY;
    _proj.xy2latlon(_worldClickX, _worldClickY, _worldClickLat, _worldClickLon);
    _haveClickLocation = true;
    _manager.setOverlaysHaveChanged(true);

    // Build the report from the primary field's actual returned metadata and
    // sample every cached field at the same geographic point.
    _updateClickReport();
    _manager.showClickDialog();
    
    // Emit a signal to indicate that the click location has changed

    emit locationClicked(_worldClickX, _worldClickY);
    
    update();

  } else {

    // mouse moved more than 20 pixels, so a zoom occurred
    // save current zoom
    
    char zoomName[1024];
    snprintf(zoomName, 1024, "saved-zoom-%d", (int) _customZooms.size());
    _customZooms.push_back(*_zoom);
    _customZooms[_customZooms.size()-1].setName(zoomName);
    _zoom = &_customZooms[_customZooms.size()-1];
    _updateGlobalCurrentZoom();
    _markZoomChanged();
    
    // handle the zoom action
    
    _worldPressX = _zoom->getXWorld(_mousePressX);
    _worldPressY = _zoom->getYWorld(_mousePressY);
    _worldReleaseX = _zoom->getXWorld(_zoomCornerX);
    _worldReleaseY = _zoom->getYWorld(_zoomCornerY);
    
    // update world coords transform
    
    _zoom->setWorldLimits(_worldPressX, _worldPressY,
                          _worldReleaseX, _worldReleaseY);
    
    _zoom->updatePixelScales();
    
    // enable unzooms
    
    _manager.enableUnzoomButtons(true);
    
    // render
    
    update();
  
  }
    
  // hide the rubber band

  if (_rubberBand) {
    _rubberBand->hide();
  }
  
}

/*
 * mouseDoubleClickEvent()
 */

void HorizView::mouseDoubleClickEvent(QMouseEvent *e)
{
  if (_manager.isMovieActive()) {
    _manager.showMovieInteractionBlocked();
    e->accept();
    return;
  }
  if (effectiveButton(e) != Qt::RightButton ||
      (e->modifiers() & Qt::ShiftModifier)) {
    QWidget::mouseDoubleClickEvent(e);
    return;
  }

#if QT_VERSION >= 0x060000
  const QPointF pos(e->position());
#else
  const QPointF pos(e->pos());
#endif

  if (!_params.single_segment_vsection && _vsectionRouteEditing) {
    _appendVsectionRoutePoint(pos.x(), pos.y());
    _completeVsectionRoute();
  }
  _activeMouseButton = Qt::NoButton;
  e->accept();
}

/*
 * keyPressEvent()
 */

void HorizView::keyPressEvent(QKeyEvent *e)
{
  if (_vsectionRouteEditing && e->key() == Qt::Key_Escape) {
    _cancelVsectionRoute();
    e->accept();
    return;
  }

  if (_vsectionRouteEditing && e->key() == Qt::Key_Backspace) {
    _vsectionRoute.removeLast();
    if (_vsectionRoute.empty()) {
      _cancelVsectionRoute();
    } else {
      _vsectionPreviewX = _zoom->getIxPixel(_vsectionRoute.back().x);
      _vsectionPreviewY = _zoom->getIyPixel(_vsectionRoute.back().y);
      update();
    }
    e->accept();
    return;
  }

  QWidget::keyPressEvent(e);
}

////////////////////////////////////////////////////////
// append a route point if it is inside the plot and distinct

bool HorizView::_appendVsectionRoutePoint(int mouseX, int mouseY)
{
  const int plotLeft = std::min(_zoom->getXMinPixel(),
                                _zoom->getXMaxPixel());
  const int plotRight = std::max(_zoom->getXMinPixel(),
                                 _zoom->getXMaxPixel());
  const int plotTop = std::min(_zoom->getYMinPixel(),
                               _zoom->getYMaxPixel());
  const int plotBottom = std::max(_zoom->getYMinPixel(),
                                  _zoom->getYMaxPixel());
  if (mouseX < plotLeft || mouseX > plotRight ||
      mouseY < plotTop || mouseY > plotBottom) {
    return false;
  }

  // A double-click and a stationary press/release can deliver the same point
  // more than once.  Apply the tolerance in screen space so it has consistent
  // interaction behavior at every zoom level.

  if (!_vsectionRoute.empty()) {
    const int lastX = _zoom->getIxPixel(_vsectionRoute.back().x);
    const int lastY = _zoom->getIyPixel(_vsectionRoute.back().y);
    if (std::hypot(mouseX - lastX, mouseY - lastY) <= 5.0) {
      return false;
    }
  }

  const double x = _zoom->getXWorld(mouseX);
  const double y = _zoom->getYWorld(mouseY);
  double lat = 0.0;
  double lon = 0.0;
  _proj.xy2latlon(x, y, lat, lon);
  _vsectionRoute.append(x, y, lat, lon);
  return true;
}

////////////////////////////////////////////////////////
// cancel the route currently being constructed

void HorizView::_cancelVsectionRoute()
{
  _vsectionRoute.clear();
  _vsectionRouteEditing = false;
  _activeMouseButton = Qt::NoButton;
  _updateMouseTracking();
  update();
}

////////////////////////////////////////////////////////
// publish a completed route

void HorizView::_completeVsectionRoute()
{
  if (_vsectionRoute.size() < 2) {
    return;
  }

  _vsectionRouteEditing = false;
  _updateMouseTracking();
  emit verticalRouteCompleted(_vsectionRoute);
  update();
}

////////////////////////////////////////////////////////
// draw the committed route and its live extension

void HorizView::_drawVsectionRoute(QPainter &painter)
{
  if (_vsectionRoute.empty()) {
    return;
  }

  painter.save();
  const QRect plotRect(std::min(_zoom->getXMinPixel(), _zoom->getXMaxPixel()),
                       std::min(_zoom->getYMinPixel(), _zoom->getYMaxPixel()),
                       std::abs(_zoom->getPlotWidth()),
                       std::abs(_zoom->getPlotHeight()));
  painter.setClipRect(plotRect);
  if (_panPreviewActive) {
    painter.translate(_panPreviewX - _mousePressX,
                      _panPreviewY - _mousePressY);
  }

  QPen routePen(QColor(_params.route_path_color));
  routePen.setWidth(2);
  routePen.setCapStyle(Qt::RoundCap);
  routePen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(routePen);

  QPainterPath path;
  const std::vector<VsectionRoute::Point> &points = _vsectionRoute.points();
  path.moveTo(_zoom->getPixelPointF(points[0].x, points[0].y));
  for (size_t ii = 1; ii < points.size(); ++ii) {
    path.lineTo(_zoom->getPixelPointF(points[ii].x, points[ii].y));
  }
  painter.drawPath(path);

  painter.setBrush(QColor(_params.route_path_color));
  for (const VsectionRoute::Point &point : points) {
    const QPointF pixel = _zoom->getPixelPointF(point.x, point.y);
    painter.drawEllipse(pixel, 3.0, 3.0);
  }

  if (_vsectionRouteEditing) {
    routePen.setStyle(Qt::DashLine);
    painter.setPen(routePen);
    const QPointF last = _zoom->getPixelPointF(points.back().x,
                                                points.back().y);
    painter.drawLine(last, QPointF(_vsectionPreviewX, _vsectionPreviewY));
  }

  painter.restore();
}

////////////////////////////////////////////////////////
// pan the current view using a middle-button drag

void HorizView::_panView(int mouseReleaseX, int mouseReleaseY)
{
  if (_manager.isMovieActive()) {
    _manager.showMovieInteractionBlocked();
    return;
  }
  const int deltaPixelsX = mouseReleaseX - _mousePressX;
  const int deltaPixelsY = mouseReleaseY - _mousePressY;
  if (abs(deltaPixelsX) <= 5 && abs(deltaPixelsY) <= 5) {
    return;
  }

  const double worldReleaseX = _zoom->getXWorld(mouseReleaseX);
  const double worldReleaseY = _zoom->getYWorld(mouseReleaseY);
  const double shiftX = _worldPressX - worldReleaseX;
  const double shiftY = _worldPressY - worldReleaseY;

  double minX = _zoom->getXMinWorld() + shiftX;
  double maxX = _zoom->getXMaxWorld() + shiftX;
  double minY = _zoom->getYMinWorld() + shiftY;
  double maxY = _zoom->getYMaxWorld() + shiftY;

  // Preserve the current span at domain edges, matching CIDD's pan behavior.
  // A view as large as the permitted domain cannot be panned on that axis.

  auto constrainToDomain = [](double domainMin, double domainMax,
                              double &viewMin, double &viewMax) {
    const double viewSpan = viewMax - viewMin;
    const double domainSpan = domainMax - domainMin;
    if (viewSpan >= domainSpan) {
      viewMin = domainMin;
      viewMax = domainMax;
    } else if (viewMax > domainMax) {
      viewMin -= viewMax - domainMax;
      viewMax = domainMax;
    } else if (viewMin < domainMin) {
      viewMax += domainMin - viewMin;
      viewMin = domainMin;
    }
  };

  constrainToDomain(_gd.h_win.min_x, _gd.h_win.max_x, minX, maxX);
  constrainToDomain(_gd.h_win.min_y, _gd.h_win.max_y, minY, maxY);

  if (minX == _zoom->getXMinWorld() && maxX == _zoom->getXMaxWorld() &&
      minY == _zoom->getYMinWorld() && maxY == _zoom->getYMaxWorld()) {
    return;
  }

  // Store pans in the custom-view history so Zoom Back reverses them in the
  // same predictable sequence as rubber-band zooms.

  char panName[1024];
  snprintf(panName, 1024, "saved-pan-%d", (int) _customZooms.size());
  WorldPlot pannedView(*_zoom);
  pannedView.setName(panName);
  pannedView.setWorldLimits(minX, minY, maxX, maxY);
  _customZooms.push_back(pannedView);
  _zoom = &_customZooms.back();

  _updateGlobalCurrentZoom();
  _markZoomChanged();
  _manager.enableUnzoomButtons(true);
  update();
}

/*************************************************************************
 * resizeEvent()
 */

void HorizView::resizeEvent(QResizeEvent * e)
{

  for (size_t ii = 0; ii < _definedZooms.size(); ii++) {
    _definedZooms[ii].resize(width(), height());
    if ((int) ii < _params.zoom_levels_n) {
      const Params::zoom_level_t &zoomLevel = _params._zoom_levels[ii];
      _definedZooms[ii].setWorldLimits(zoomLevel.min_x, zoomLevel.min_y,
                                       zoomLevel.max_x, zoomLevel.max_y);
    }
  }
  for (size_t ii = 0; ii < _customZooms.size(); ii++) {
    _customZooms[ii].resize(width(), height());
  }
  _pixmap = _pixmap.scaled(width(), height());
  _sizeChanged = true;
  
}


/*************************************************************************
 * resize()
 */

void HorizView::resize(const int width, const int height)
{

  setGeometry(0, 0, width, height);
  _sizeChanged = true;
  
}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * render the maps
 */

void HorizView::_renderMaps()
{

  _zoom->drawMaps();
  _mapsReady = true;
  
}

void HorizView::informationMessage()
{
  
  // QMessageBox::StandardButton reply;
  // QLabel *informationLabel;
  
  // reply = QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  //  if (reply == QMessageBox::Ok)
  //  informationLabel->setText("OK");
  //else
  //  informationLabel->setText("Escape");

}

/////////////////////////////////////////////////////////////////////////////////////
// slots for context editing; create and show the associated modeless dialog and return                                   

void HorizView::contextMenuCancel()
{
  // informationMessage();
  // notImplemented();                                                                                                     
}

void HorizView::contextMenuParameterColors()
{

#ifdef NOTNOW
  
  LOG(DEBUG_VERBOSE) << "enter";

  //DisplayField selectedField;                                                                             

  // const DisplayField &field = _manager.getSelectedField();
  // const ColorMap &colorMapForSelectedField = field.getColorMap();
  ParameterColorView *parameterColorView = new ParameterColorView(this);
  vector<DisplayField *> displayFields = _manager.getDisplayFields(); // TODO: I guess, implement this as a signal and a slot? // getDisplayFields();
  DisplayField selectedField = _manager.getSelectedField();
  string emphasis_color = "white";
  string annotation_color = "white";
  DisplayFieldModel *displayFieldModel = 
    new DisplayFieldModel(displayFields, selectedField.getName(),
			  _params.grid_and_range_ring_color,
			  emphasis_color,
			  annotation_color,
			  _params.background_color);
  FieldColorController *fieldColorController = new FieldColorController(parameterColorView, displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  //  connect(parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
  	  &_manager, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!
  //  GuiManager::colorMapRedefineReceived(string, ColorMap)
  //connect(fieldColorController, SIGNAL(colorMapRedefined(string)),
  //	  this, SLOT(changeToDisplayField(string))); // THIS IS NOT CALLED!!

  /* TODO: combine with replot
     connect(fieldColorController, SIGNAL(backgroundColorSet(QColor)),
     this, SLOT(backgroundColor(QColor)));
  */

  fieldColorController->startUp(); 

  //connect(parameterColorView, SIGNAL(needFieldNames()), this, SLOT(getFieldNames()));
  //connect(this, SIGNAL(fieldNamesSupplied(vector<string>)), 
  //  parameterColorView, SLOT(fieldNamesSupplied(vector<string>));
  // TODO: move this call to the controller?                                                                
  // parameterColorView.exec();

  //  if(parameterColorController.Changes()) {
  // TODO: what are changes?  new displayField(s)?                                                        
  //}
 
  LOG(DEBUG_VERBOSE) << "exit ";

#endif
  
  informationMessage();
  
}

void HorizView::contextMenuView()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

void HorizView::contextMenuExamine()         
{
  informationMessage();                                                                                                 

}

void HorizView::contextMenuDataWidget()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

void HorizView::contextMenuEditor()
{
  LOG(DEBUG_VERBOSE) << "enter";

  // get click location in world coords
  // by using the location stored in class variables
#ifdef JUNK
  double x_km = _worldPressX;
  double y_km = _worldPressY;

  // get ray closest to click point
  const RadxRay *closestRay = _getClosestRay(x_km, y_km);
  // TODO: make sure the point is in the valid area
  if (closestRay == NULL) {
    // report error
    QMessageBox::information(this, QString::fromStdString(""), QString::fromStdString("No ray found at location clicked"));
    // TODO: move to this ...  errorMessage("", "No ray found at location clicked");
  } else {
    // ExamineEdit(closestRay);
  }
  LOG(DEBUG_VERBOSE) << "exit";
#endif
  
}

void HorizView::ShowContextMenu(const QPoint &pos/* , RadxVol *vol */)
{

  if (_manager.isMovieActive()) {
    _manager.showMovieInteractionBlocked();
    return;
  }

  // _vol = vol;

  QMenu contextMenu("Context menu", this);
  
  QAction action1("Cancel", this);
  connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
  contextMenu.addAction(&action1);

  QAction action3("Parameters + Colors", this);
  connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
  contextMenu.addAction(&action3);

  QAction action4("View", this);
  connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuView()));
  contextMenu.addAction(&action4);

  QAction action5("Editor", this);
  connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuEditor()));
  contextMenu.addAction(&action5);
  
  QAction action6("Examine", this);
  connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuExamine()));
  contextMenu.addAction(&action6);

  /*
    QAction action7("Data Widget", this);
    connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuDataWidget()));
    contextMenu.addAction(&action7);
  */

  contextMenu.exec(this->mapToGlobal(pos));
}

