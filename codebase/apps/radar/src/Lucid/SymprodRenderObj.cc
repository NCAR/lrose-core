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
//////////////////////////////////////////////////////////
// SymprodRenderObj.cc
//
// Symprod rendering objects
//
// These classes provide handling routines for the various
// types of symprod objects used for rendering.
//
// Jan 2000
//////////////////////////////////////////////////////////

#include "SymprodRenderObj.hh"
#include "WorldPlot.hh"
#include "Product.hh"
#include <Spdb/Symprod.hh>
#include <Spdb/Product_defines.hh>
#include <dataport/bigend.h>
#include <vector>
#include <toolsa/str.h>
#include <toolsa/pjg.h>     // Map projection geometry
#include <QFontMetrics>
#include <QPainterPath>
#include <QStringList>

namespace {

QColor symprodColor(const char *name, const QColor &fallback = Qt::white)
{
  if (name == nullptr || name[0] == '\0') {
    return fallback;
  }
  const QColor color(name);
  return color.isValid() ? color : fallback;
}

Qt::PenStyle penStyle(int lineType)
{
  switch (lineType) {
    case Symprod::LINETYPE_DASH: return Qt::DashLine;
    case Symprod::LINETYPE_DOT_DASH: return Qt::DashDotLine;
    default: return Qt::SolidLine;
  }
}

Qt::PenCapStyle penCapStyle(int capStyle)
{
  switch (capStyle) {
    case Symprod::CAPSTYLE_PROJECTING: return Qt::SquareCap;
    case Symprod::CAPSTYLE_ROUND: return Qt::RoundCap;
    default: return Qt::FlatCap;
  }
}

Qt::PenJoinStyle penJoinStyle(int joinStyle)
{
  switch (joinStyle) {
    case Symprod::JOINSTYLE_MITER: return Qt::MiterJoin;
    case Symprod::JOINSTYLE_ROUND: return Qt::RoundJoin;
    default: return Qt::BevelJoin;
  }
}

QPen symprodPen(const char *color, int width, int lineType,
                int capStyle, int joinStyle)
{
  QPen pen(symprodColor(color));
  pen.setWidth(qMax(1, width));
  pen.setStyle(penStyle(lineType));
  pen.setCapStyle(penCapStyle(capStyle));
  pen.setJoinStyle(penJoinStyle(joinStyle));
  return pen;
}

QBrush symprodBrush(const char *color, int fillType)
{
  QColor fillColor = symprodColor(color);
  if (fillType == Symprod::FILL_NONE) {
    return Qt::NoBrush;
  }
  if (fillType >= Symprod::FILL_ALPHA10 &&
      fillType <= Symprod::FILL_ALPHA90) {
    const int percent = (fillType - Symprod::FILL_ALPHA10 + 1) * 10;
    fillColor.setAlphaF(percent / 100.0);
    return QBrush(fillColor);
  }
  if (fillType >= Symprod::FILL_STIPPLE10 &&
      fillType <= Symprod::FILL_STIPPLE90) {
    static const Qt::BrushStyle styles[] = {
      Qt::Dense7Pattern, Qt::Dense6Pattern, Qt::Dense5Pattern,
      Qt::Dense4Pattern, Qt::Dense4Pattern, Qt::Dense3Pattern,
      Qt::Dense2Pattern, Qt::Dense1Pattern, Qt::Dense1Pattern
    };
    return QBrush(fillColor, styles[fillType - Symprod::FILL_STIPPLE10]);
  }
  return QBrush(fillColor);
}

bool objectVisible(const Symprod::obj_hdr_t &header,
                   const SymprodRenderContext &context)
{
  if (!(header.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return true;
  }
  if (!context.showHidden || context.showingHidden) {
    return false;
  }
  context.showingHidden = true;
  return true;
}

QPointF projectPoint(double lat, double lon,
                     const SymprodRenderContext &context)
{
  double x = 0.0;
  double y = 0.0;
  context.plot.getProjection().latlon2xy(lat, lon, x, y);
  return context.plot.getPixelPointF(x, y);
}

QPointF projectOffsetKm(double lat, double lon, double dx, double dy,
                        const SymprodRenderContext &context)
{
  double offsetLat = lat;
  double offsetLon = lon;
  PJGLatLonPlusDxDy(lat, lon, dx, dy, &offsetLat, &offsetLon);
  return projectPoint(offsetLat, offsetLon, context);
}

void drawPath(QPainter &painter, const QPainterPath &path,
              const QPen &pen, const QBrush &brush)
{
  painter.save();
  painter.setPen(pen);
  painter.setBrush(brush);
  painter.drawPath(path);
  painter.restore();
}

} // namespace

//////////////////////////////////////////////////////////////////////////////
// Abstract base class

SymprodRenderObj::SymprodRenderObj(SymprodRender *c) :
        _params(Params::Instance()),
        _gd(GlobalData::Instance())
{

  container = c;

}

SymprodRenderObj::~SymprodRenderObj()
{
}

void SymprodRenderObj::drawQt(QPainter &painter,
                              const SymprodRenderContext &context) const
{
  Q_UNUSED(painter);
  Q_UNUSED(context);
}

double SymprodRenderObj::dist(double lat, double lon)
{
  fprintf(stderr,"SymprodRenderObj::Should not Get Here - Dist\n");
  return 999999.0;
}

void SymprodRenderObj::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{ 
  fprintf(stderr,"SymprodRenderObj::Should not Get Here - Fill\n");
}

//////////////////////////////////////////////////////////////////////////////
// Text objects

SymprodRenderText::SymprodRenderText(const void *prod_buffer,
				     const int obj_offset,
				     Product &prod, SymprodRender *c)
        : SymprodRenderObj(c),
          SymprodText(prod_buffer, obj_offset),
          prod(prod)
  
{
  objtype = Symprod::OBJ_TEXT;
  
}

void SymprodRenderText::drawQt(QPainter &painter,
                               const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context)) {
    return;
  }
  if (!(_hdr.detail_level & Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS) &&
      context.iconScale < prod._prodInfo.text_off_threshold) {
    return;
  }

  QPointF anchor = projectPoint(_props.origin.lat, _props.origin.lon, context);
  // Symprod pixel offsets use the plot convention that positive Y is up,
  // while Qt device coordinates increase downward.
  anchor += QPointF(_props.offset.x, -_props.offset.y);

  painter.save();
  QFont font = painter.font();
  if (_props.fontname[0] != '\0') {
    font.setFamily(_props.fontname);
  }
  if (_props.font_size > 0) {
    font.setPixelSize(_props.font_size);
  }
  switch (_props.font_style) {
    case Symprod::TEXT_BOLD: font.setBold(true); break;
    case Symprod::TEXT_ITALICS: font.setItalic(true); break;
    case Symprod::TEXT_UNDERLINE: font.setUnderline(true); break;
    case Symprod::TEXT_STRIKETHROUGH: font.setStrikeOut(true); break;
    default: break;
  }
  painter.setFont(font);
  painter.setPen(symprodColor(_hdr.color));

  const QString text = QString::fromStdString(_text);
  QRectF bounds = QFontMetricsF(font).boundingRect(QRectF(),
                                                Qt::TextDontClip, text);
  qreal left = anchor.x();
  qreal top = anchor.y();
  if (_props.horiz_alignment == Symprod::HORIZ_ALIGN_CENTER) {
    left -= bounds.width() / 2.0;
  } else if (_props.horiz_alignment == Symprod::HORIZ_ALIGN_RIGHT) {
    left -= bounds.width();
  }
  if (_props.vert_alignment == Symprod::VERT_ALIGN_CENTER) {
    top -= bounds.height() / 2.0;
  } else if (_props.vert_alignment == Symprod::VERT_ALIGN_BOTTOM) {
    top -= bounds.height();
  }
  bounds.moveTopLeft(QPointF(left, top));

  if (_hdr.background_color[0] != '\0') {
    painter.fillRect(bounds.adjusted(-1.0, -1.0, 1.0, 1.0),
                     symprodColor(_hdr.background_color, Qt::transparent));
  }
  painter.drawText(bounds, Qt::TextDontClip, text);
  painter.restore();
}

double SymprodRenderText::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;
  PJGLatLon2RTheta(_props.origin.lat, _props.origin.lon,
                   lat, lon, &d, &theta);

  return d;
}

void SymprodRenderText::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  label =   _text;
}

//////////////////////////////////////////////////////////////////////////////
// Polyline objects

SymprodRenderPolyline::
  SymprodRenderPolyline(const void *prod_buffer,
                        const int obj_offset,
                        Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodPolyline(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_POLYLINE;
}

void SymprodRenderPolyline::drawQt(QPainter &painter,
                                   const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context) || _props.num_points < 2 || !_points) {
    return;
  }

  const QPen pen = symprodPen(_hdr.color, _props.linewidth,
                             _props.linetype, _props.capstyle,
                             _props.joinstyle);
  const QBrush brush = _props.close_flag ?
    symprodBrush(_hdr.color, _props.fill) : QBrush(Qt::NoBrush);

  // Pen-up markers delimit independent paths. Longitude jumps are also split
  // so a dateline crossing cannot draw an erroneous line across the display.
  QPainterPath path;
  bool havePoint = false;
  double previousLon = 0.0;
  for (int ii = 0; ii < _props.num_points; ++ii) {
    const Symprod::wpt_t &point = _points[ii];
    const bool penUp = point.lat == Symprod::WPT_PENUP ||
                       point.lon == Symprod::WPT_PENUP;
    if (penUp || (havePoint && fabs(point.lon - previousLon) > 180.0)) {
      if (_props.close_flag && havePoint) {
        path.closeSubpath();
      }
      havePoint = false;
      continue;
    }

    const QPointF pixel = projectPoint(point.lat, point.lon, context);
    if (!havePoint) {
      path.moveTo(pixel);
      havePoint = true;
    } else {
      path.lineTo(pixel);
    }
    previousLon = point.lon;
  }
  if (_props.close_flag && havePoint) {
    path.closeSubpath();
  }
  drawPath(painter, path, pen, brush);
}

double SymprodRenderPolyline::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  if (_props.num_points < 1 || !_points) {
    return d;
  }

  // Use a point close to the center of the polyline
  int index =  _props.num_points / 2;

  // Make sure the point is not a pen up.
  while (_points[index].lon == Symprod::WPT_PENUP && index > 0) {
    index--;
  }

  PJGLatLon2RTheta(_points[index].lat,_points[index].lon,lat,lon,&d,&theta);


  return d;
}

void SymprodRenderPolyline::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  for (int i = 0; i < _props.num_points; i++)  {
    w.lon = _points[i].lon;
    w.lat = _points[i].lat;
    wpt.push_back(w);
  }
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}

//////////////////////////////////////////////////////////////////////////////
// Iconline objects

SymprodRenderIconline::
  SymprodRenderIconline(const void *prod_buffer,
                        const int obj_offset,
                        Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodIconline(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_ICONLINE;
}

void SymprodRenderIconline::drawQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context) || _props.num_points < 2 || !_points) {
    return;
  }

  const double scale =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) ?
    1.0 : context.iconScale;
  const QPointF origin = projectPoint(_props.origin.lat, _props.origin.lon,
                                      context);
  QPainterPath path;
  bool havePoint = false;
  for (int ii = 0; ii < _props.num_points; ++ii) {
    if (_points[ii].x == Symprod::PPT_PENUP ||
        _points[ii].y == Symprod::PPT_PENUP) {
      if (_props.close_flag && havePoint) {
        path.closeSubpath();
      }
      havePoint = false;
      continue;
    }
    const QPointF point(origin.x() + scale * _points[ii].x,
                        origin.y() - scale * _points[ii].y);
    if (havePoint) {
      path.lineTo(point);
    } else {
      path.moveTo(point);
      havePoint = true;
    }
  }
  if (_props.close_flag && havePoint) {
    path.closeSubpath();
  }

  const QPen pen = symprodPen(_hdr.color, _props.linewidth,
                             _props.linetype, _props.capstyle,
                             _props.joinstyle);
  const QBrush brush = _props.close_flag ?
    symprodBrush(_hdr.color, _props.fill) : QBrush(Qt::NoBrush);
  drawPath(painter, path, pen, brush);
}

double SymprodRenderIconline::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}


void SymprodRenderIconline::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  w.lon = _props.origin.lon;
  w.lat = _props.origin.lat;
  wpt.push_back(w);
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}

//////////////////////////////////////////////////////////////////////////////
// StrokedIcon objects

SymprodRenderStrokedIcon::
  SymprodRenderStrokedIcon(const void *prod_buffer,
                           const int obj_offset,
                           Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodStrokedIcon(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_STROKED_ICON;
}

#define ICON_BUF_SIZE 32

void SymprodRenderStrokedIcon::drawQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context) || _props.num_icons < 1 ||
      _props.num_icon_pts < 1 || !_iconOrigins || !_iconPts) {
    return;
  }

  const double scale =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) ?
    1.0 : context.iconScale;
  const QPen pen = symprodPen(_hdr.color, _props.linewidth,
                             Symprod::LINETYPE_SOLID,
                             Symprod::CAPSTYLE_BUTT,
                             Symprod::JOINSTYLE_ROUND);

  painter.save();
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  for (int iicon = 0; iicon < _props.num_icons; ++iicon) {
    const QPointF origin = projectPoint(_iconOrigins[iicon].lat,
                                        _iconOrigins[iicon].lon, context);
    QPainterPath path;
    bool havePoint = false;
    for (int ipt = 0; ipt < _props.num_icon_pts; ++ipt) {
      if (_iconPts[ipt].x == Symprod::PPT_PENUP ||
          _iconPts[ipt].y == Symprod::PPT_PENUP) {
        havePoint = false;
        continue;
      }
      const QPointF point(origin.x() + scale * _iconPts[ipt].x,
                          origin.y() - scale * _iconPts[ipt].y);
      if (havePoint) {
        path.lineTo(point);
      } else {
        path.moveTo(point);
        havePoint = true;
      }
    }
    painter.drawPath(path);
  }
  painter.restore();
}

double SymprodRenderStrokedIcon::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  if( _iconOrigins != NULL) PJGLatLon2RTheta(_iconOrigins[0].lat, _iconOrigins[0].lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderStrokedIcon::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  for (int ipt = 0; ipt < _props.num_icon_pts; ipt++) {
    w.lon = _iconOrigins[ipt].lon;
    w.lat = _iconOrigins[ipt].lat;
    wpt.push_back(w);
  }
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}

//////////////////////////////////////////////////////////////////////////////
// NamedIcon objects

SymprodRenderNamedIcon::
  SymprodRenderNamedIcon(const void *prod_buffer,
                         const int obj_offset,
                         Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodNamedIcon(prod_buffer, obj_offset),
            prod(prod),
            _iconDef(nullptr),
            _iconLookupComplete(false)
  
{
  objtype = Symprod::OBJ_NAMED_ICON;
  
}

void SymprodRenderNamedIcon::drawQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context) || _props.num_icons < 1 ||
      !_iconOrigins) {
    return;
  }

  // Named Symprod icons do not carry their geometry. Reuse icon definitions
  // already loaded from the RAP map files, which are the application's only
  // configured source of named icon geometry.
  if (!_iconLookupComplete) {
    for (const MapOverlay_t *overlay : _gd.overlays) {
      if (!overlay) {
        continue;
      }
      for (int ii = 0; ii < overlay->num_icondefs; ++ii) {
        const Geo_feat_icondef_t *candidate = overlay->geo_icondef[ii];
        if (candidate &&
            strncmp(candidate->name, _props.name,
                    SYMPROD_ICON_NAME_LEN) == 0) {
          _iconDef = candidate;
          break;
        }
      }
      if (_iconDef) {
        break;
      }
    }
    _iconLookupComplete = true;
  }

  const double scale =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) ?
    1.0 : context.iconScale;
  if (scale <= 0.0) {
    return;
  }
  painter.save();
  painter.setPen(QPen(symprodColor(_hdr.color), 1));
  painter.setBrush(Qt::NoBrush);
  for (int ii = 0; ii < _props.num_icons; ++ii) {
    const QPointF origin = projectPoint(_iconOrigins[ii].lat,
                                        _iconOrigins[ii].lon, context);
    if (_iconDef && _iconDef->num_points > 1) {
      QPainterPath path;
      bool havePoint = false;
      for (int jj = 0; jj < _iconDef->num_points; ++jj) {
        if (_iconDef->x[jj] == 32767 || _iconDef->y[jj] == 32767) {
          havePoint = false;
          continue;
        }
        const QPointF point(origin.x() + scale * _iconDef->x[jj],
                            origin.y() + scale * _iconDef->y[jj]);
        if (havePoint) {
          path.lineTo(point);
        } else {
          path.moveTo(point);
          havePoint = true;
        }
      }
      painter.drawPath(path);
    } else {
      // Keep unsupported names visible and diagnosable without inventing a
      // platform-dependent icon search path.
      const double radius = qMax(2.0, 3.0 * scale);
      painter.drawLine(origin + QPointF(-radius, -radius),
                       origin + QPointF(radius, radius));
      painter.drawLine(origin + QPointF(-radius, radius),
                       origin + QPointF(radius, -radius));
      painter.drawText(origin + QPointF(radius + 2.0, 0.0),
                       QString::fromLatin1(
                         _props.name,
                         strnlen(_props.name, SYMPROD_ICON_NAME_LEN)));
    }
  }
  painter.restore();
}

double SymprodRenderNamedIcon::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  if (_props.num_icons > 0 && _iconOrigins) {
    PJGLatLon2RTheta(_iconOrigins[0].lat, _iconOrigins[0].lon,
                     lat, lon, &d, &theta);
  }

  return d;
}

void SymprodRenderNamedIcon::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  if (!_iconOrigins) {
    return;
  }
  for (int ii = 0; ii < _props.num_icons; ++ii) {
    world_pt_t point;
    point.lat = _iconOrigins[ii].lat;
    point.lon = _iconOrigins[ii].lon;
    wpt.push_back(point);
  }
  world_pt_t separator;
  separator.lat = -9999.9;
  separator.lon = -9999.9;
  wpt.push_back(separator);
  label.assign(_props.name,
               strnlen(_props.name, SYMPROD_ICON_NAME_LEN));
}

//////////////////////////////////////////////////////////////////////////////
// BitmapIcon objects

SymprodRenderBitmapIcon::
  SymprodRenderBitmapIcon( const void *prod_buffer,
                           const int obj_offset,
                           Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodBitmapIcon(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_BITMAP_ICON;
  
}

void SymprodRenderBitmapIcon::drawQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context) || _props.num_icons < 1 ||
      !_iconOrigins || !_bitmap || _props.bitmap_x_dim < 1 ||
      _props.bitmap_y_dim < 1) {
    return;
  }

  // Cache the immutable bitmap as an alpha image. Scaling remains a painter
  // operation so zoom changes do not require rebuilding the icon pixels.
  if (_bitmapImage.isNull()) {
    _bitmapImage = QImage(_props.bitmap_x_dim, _props.bitmap_y_dim,
                          QImage::Format_ARGB32_Premultiplied);
    _bitmapImage.fill(Qt::transparent);
    const QRgb pixel = symprodColor(_hdr.color).rgba();
    for (int yy = 0; yy < _props.bitmap_y_dim; ++yy) {
      QRgb *line = reinterpret_cast<QRgb *>(_bitmapImage.scanLine(yy));
      for (int xx = 0; xx < _props.bitmap_x_dim; ++xx) {
        if (_bitmap[yy * _props.bitmap_x_dim + xx]) {
          line[xx] = pixel;
        }
      }
    }
  }

  const double scale =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) ?
    1.0 : context.iconScale;
  if (scale <= 0.0) {
    return;
  }
  const QSizeF size(_props.bitmap_x_dim * scale,
                    _props.bitmap_y_dim * scale);
  painter.save();
  painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
  for (int ii = 0; ii < _props.num_icons; ++ii) {
    const QPointF origin = projectPoint(_iconOrigins[ii].lat,
                                        _iconOrigins[ii].lon, context);
    const QRectF target(origin.x() - size.width() / 2.0,
                        origin.y() - size.height() / 2.0,
                        size.width(), size.height());
    painter.drawImage(target, _bitmapImage);
  }
  painter.restore();
}

double SymprodRenderBitmapIcon::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  if (_props.num_icons > 0 && _iconOrigins) {
    PJGLatLon2RTheta(_iconOrigins[0].lat, _iconOrigins[0].lon,
                     lat, lon, &d, &theta);
  }

  return d;
}

void SymprodRenderBitmapIcon::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  for (int ipt = 0; ipt < _props.num_icons; ipt++) {
    w.lon = _iconOrigins[ipt].lon;
    w.lat = _iconOrigins[ipt].lat;
    wpt.push_back(w);
  }
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}


//////////////////////////////////////////////////////////////////////////////
// Arc objects

SymprodRenderArc::
  SymprodRenderArc(const void *prod_buffer,
                   const int obj_offset,
                   Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodArc(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_ARC;
}

void SymprodRenderArc::drawQt(QPainter &painter,
                              const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context)) {
    return;
  }

  const QPen pen = symprodPen(_hdr.color, _props.linewidth,
                             _props.linetype, _props.capstyle,
                             _props.joinstyle);
  const QBrush brush = symprodBrush(_hdr.color, _props.fill);
  const int nsegments = qMax(4, _props.nsegments);
  const double span = _props.angle2 - _props.angle1;
  const double iconScale =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) ?
    1.0 : context.iconScale;
  const QPointF center = projectPoint(_props.origin.lat, _props.origin.lon,
                                      context);

  QPainterPath path;
  if (_props.fill != Symprod::FILL_NONE && fabs(span) < 359.999) {
    path.moveTo(center);
  }
  for (int ii = 0; ii <= nsegments; ++ii) {
    const double angle = (_props.angle1 + span * ii / nsegments) *
                         M_PI / 180.0;
    const double rotation = _props.axis_rotation * M_PI / 180.0;
    const double ellipseX = _props.radius_x * cos(angle);
    const double ellipseY = _props.radius_y * sin(angle);
    const double dx = ellipseX * cos(rotation) - ellipseY * sin(rotation);
    const double dy = ellipseX * sin(rotation) + ellipseY * cos(rotation);
    QPointF point;
    if (_props.radii_in_pixels) {
      point = QPointF(center.x() + iconScale * dx,
                      center.y() - iconScale * dy);
    } else {
      point = projectOffsetKm(_props.origin.lat, _props.origin.lon,
                              dx, dy, context);
    }
    if (ii == 0 && _props.fill == Symprod::FILL_NONE) {
      path.moveTo(point);
    } else {
      path.lineTo(point);
    }
  }
  if (_props.fill != Symprod::FILL_NONE) {
    path.closeSubpath();
  }
  drawPath(painter, path, pen, brush);
}


double SymprodRenderArc::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderArc::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  w.lon = _props.origin.lon;
  w.lat = _props.origin.lat;
  wpt.push_back(w);
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}


//////////////////////////////////////////////////////////////////////////////
// Rectangle objects

SymprodRenderRectangle::
  SymprodRenderRectangle(const void *prod_buffer,
                         const int obj_offset,
                         Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodRectangle(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_RECTANGLE;
}

void SymprodRenderRectangle::drawQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context)) {
    return;
  }

  // Rectangle dimensions are in km from the lower-left origin. Projecting
  // every corner preserves their geographic placement in non-Cartesian views.
  QPainterPath path;
  path.moveTo(projectOffsetKm(_props.origin.lat, _props.origin.lon,
                              0.0, 0.0, context));
  path.lineTo(projectOffsetKm(_props.origin.lat, _props.origin.lon,
                              _props.width, 0.0, context));
  path.lineTo(projectOffsetKm(_props.origin.lat, _props.origin.lon,
                              _props.width, _props.height, context));
  path.lineTo(projectOffsetKm(_props.origin.lat, _props.origin.lon,
                              0.0, _props.height, context));
  path.closeSubpath();

  const QPen pen = symprodPen(_hdr.color, _props.linewidth,
                             _props.linetype, _props.capstyle,
                             _props.joinstyle);
  drawPath(painter, path, pen, symprodBrush(_hdr.color, _props.fill));
}

double SymprodRenderRectangle::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderRectangle::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  w.lon = _props.origin.lon;
  w.lat = _props.origin.lat;
  wpt.push_back(w);
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}



//////////////////////////////////////////////////////////////////////////////
// Chunk objects

SymprodRenderChunk::
  SymprodRenderChunk(const void *prod_buffer,
                     const int obj_offset,
                     Product &prod, SymprodRender *c)
          : SymprodRenderObj(c),
            SymprodChunk(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_CHUNK;
}

void SymprodRenderChunk::drawQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!objectVisible(_hdr, context) || !_data || _props.nbytes_chunk <= 0) {
    return;
  }

  if (_props.chunk_type == SPDB_STATION_REPORT_ID) {
    // Some report variants are shorter than station_report_t. Zero padding
    // lets the established decoder select the variant without reading beyond
    // the Symprod chunk.
    station_report_t station;
    memset(&station, 0, sizeof(station));
    memcpy(&station, _data,
           qMin(static_cast<int>(sizeof(station)), _props.nbytes_chunk));
    station_report_from_be(&station);
    _drawStationQt(station, painter, context);
    return;
  }

  if (_props.chunk_type == SPDB_STATION_REPORT_ARRAY_ID &&
      _props.nbytes_chunk >=
      static_cast<int>(sizeof(station_report_array_header_t))) {
    station_report_array_header_t header;
    memcpy(&header, _data, sizeof(header));
    const size_t headerSize = sizeof(station_report_array_header_t);
    const size_t available = _props.nbytes_chunk - headerSize;
    const size_t maxReports = available / sizeof(station_report_t);
    ui32 numReports = BE_to_ui32(header.num_reports);
    if (numReports > maxReports && header.num_reports <= maxReports) {
      // station_report_from_be() accepts native-order data too. Match that
      // tolerance for arrays written by older local producers.
      numReports = header.num_reports;
    }
    if (numReports > maxReports) {
      return;
    }

    const char *reportData = static_cast<const char *>(_data) + headerSize;
    for (ui32 ii = 0; ii < numReports; ++ii) {
      station_report_t station;
      memcpy(&station, reportData + ii * sizeof(station), sizeof(station));
      station_report_from_be(&station);
      _drawStationQt(station, painter, context);
    }
  }
}

void SymprodRenderChunk::_drawStationQt(
  const station_report_t &station, QPainter &painter,
  const SymprodRenderContext &context) const
{
  if (!std::isfinite(station.lat) || !std::isfinite(station.lon) ||
      station.lat < -90.0 || station.lat > 90.0 ||
      station.lon < -720.0 || station.lon > 720.0) {
    return;
  }

  const double scale =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) ?
    1.0 : context.iconScale;
  if (scale <= 0.0) {
    return;
  }
  const QPointF origin = projectPoint(station.lat, station.lon, context);
  const QColor foreground = symprodColor(_hdr.color);
  const QColor background = symprodColor(_hdr.background_color,
                                          Qt::transparent);

  painter.save();
  painter.setPen(foreground);
  QFont font = painter.font();
  font.setPixelSize(qMax(8, static_cast<int>(10.0 * qMax(0.75, scale))));
  painter.setFont(font);
  const QFontMetrics metrics(font);
  const int gap = qMax(3, metrics.horizontalAdvance(QLatin1Char(' ')) / 2);

  enum class StationTextRow { Upper, Middle, Lower, Dewpoint };
  auto drawStationText = [&](const QString &text, bool left,
                             StationTextRow row) {
    if (text.isEmpty()) {
      return;
    }
    const QRect bounds = metrics.boundingRect(text);
    const double x = left ? origin.x() - gap - bounds.width() :
                            origin.x() + gap;
    double baseline;
    switch (row) {
      case StationTextRow::Upper:
        baseline = origin.y() - gap - metrics.descent();
        break;
      case StationTextRow::Middle:
        baseline = origin.y() + metrics.ascent() - metrics.descent();
        break;
      case StationTextRow::Lower:
        baseline = origin.y() + gap + metrics.ascent();
        break;
      case StationTextRow::Dewpoint:
        // CIDD gives the lower-left dewpoint an additional ascent of
        // clearance; the lower-right gust uses only the normal lower row.
        baseline = origin.y() + gap + 2.0 * metrics.ascent();
        break;
    }
    if (background.alpha() > 0) {
      painter.fillRect(QRectF(x - 1.0, baseline - metrics.ascent() - 1.0,
                              bounds.width() + 2.0,
                              metrics.height() + 2.0), background);
    }
    painter.drawText(QPointF(x, baseline), text);
  };

  const bool drawText =
    (_hdr.detail_level & Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS) ||
    scale >= prod._prodInfo.text_off_threshold;
  if (drawText) {
    if (station.temp != STATION_NAN) {
      drawStationText(QString::number(station.temp, 'f', 0), true,
                      StationTextRow::Upper);
    }
    if (station.dew_point != STATION_NAN) {
      drawStationText(QString::number(station.dew_point, 'f', 0), true,
                      StationTextRow::Dewpoint);
    }

    QString weather;
    if (station.msg_id == METAR_REPORT) {
      weather = QString::fromLatin1(
                  station.shared.metar.weather_str,
                  strnlen(station.shared.metar.weather_str,
                          METAR_WX_STR_LEN))
                  .split(QLatin1Char(' '), Qt::SkipEmptyParts).value(0);
    } else if (station.weather_type != WX_NAN) {
      weather = QString::fromLatin1(
        weather_type2string_trunc(station.weather_type, 1)).trimmed();
    }
    drawStationText(weather, true, StationTextRow::Middle);

    if (context.kmAcrossScreen < 1000.0 && station.station_label[0] != '\0') {
      QString label = QString::fromLatin1(
                        station.station_label,
                        strnlen(station.station_label,
                                ST_LABEL_SIZE)).trimmed();
      if (label.startsWith(QLatin1Char('K'))) {
        label.remove(0, 1);
      }
      drawStationText(label, false, StationTextRow::Upper);
    }
    if (station.windgust != STATION_NAN && station.windgust > 2.0) {
      drawStationText(QString("G%1").arg(station.windgust * NMH_PER_MS,
                                          0, 'f', 0), false,
                      StationTextRow::Lower);
    }
  }

  if (station.winddir != STATION_NAN && station.windspd != STATION_NAN &&
      station.windspd >= 0.0) {
    const double shaftLength = qMax(4.0,
      _params.wind_barb_shaft_len * qMax(0.25, scale));
    const double direction = station.winddir * M_PI / 180.0;
    const QPointF unit(sin(direction), -cos(direction));
    const QPointF side(-unit.y(), unit.x());
    const QPointF tip = origin + shaftLength * unit;
    painter.setPen(QPen(foreground, 1));
    painter.drawLine(origin, tip);

    // Wind speed is encoded with the conventional 50/10/5 knot flags.
    // Draw from the outer end inward so the station-model layout remains
    // stable as flags are added or removed.
    int knots = qMax(0, static_cast<int>(floor(station.windspd *
                                               NMH_PER_MS / 5.0 + 0.5)) * 5);
    QPointF flagAt = tip;
    const double spacing = qMax(2.0, 3.0 * qMax(0.5, scale));
    const double flagLength = qMax(4.0, shaftLength * 0.35);
    while (knots >= 50) {
      QPolygonF pennant;
      pennant << flagAt << flagAt - spacing * unit
              << flagAt - spacing * unit + flagLength * side;
      painter.setBrush(foreground);
      painter.drawPolygon(pennant);
      painter.setBrush(Qt::NoBrush);
      flagAt -= spacing * unit;
      knots -= 50;
    }
    while (knots >= 10) {
      painter.drawLine(flagAt, flagAt + flagLength * side);
      flagAt -= spacing * unit;
      knots -= 10;
    }
    if (knots >= 5) {
      painter.drawLine(flagAt, flagAt + 0.5 * flagLength * side);
    }
  }
  painter.restore();
}

double SymprodRenderChunk::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.

  // Not Implemented.
  //PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}


void SymprodRenderChunk::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  // Do nothing. - Not implemented.
}
