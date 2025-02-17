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

#include "VlevelSelector.hh"
#include "GuiManager.hh"

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QRectF>
#include <QPaintEvent>
#include <iostream>
#include <cmath>
#include "cidd.h"
using namespace std;

VlevelSelector::VlevelSelector(int width,
                               const ColorMap *cmap,
                               VlevelManager &vlevelManager,
                               GuiManager *guiManager) :
        QWidget(guiManager),
        _colorMap(cmap),
        _vlevelManager(vlevelManager),
        _guiManager(guiManager)
{
  
  setMinimumSize(width, 100);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  
  _world.setLeftMargin(_params.vlevel_selector_left_margin);
  _world.setRightMargin(_params.vlevel_selector_right_margin);
  _world.setTopMargin(_params.vlevel_selector_top_margin);
  _world.setBottomMargin(_params.vlevel_selector_bottom_margin);

  _world.setAxisTextMargin(_params.vlevel_selector_axis_text_margin);
  _world.setYAxisTickLen(_params.vlevel_selector_axis_tick_len);
  _world.setYNTicksIdeal(_params.vlevel_selector_n_ticks_ideal);

  _world.setTitleFontSize(_params.vlevel_selector_title_font_size);
  _world.setAxisLabelFontSize(_params.vlevel_selector_labels_font_size);
  _world.setTickValuesFontSize(_params.vlevel_selector_labels_font_size);

  _world.setTitleColor(_params.vlevel_selector_title_color);
  _world.setAxisLineColor(_params.vlevel_selector_axis_color);
  _world.setAxisTextColor(_params.vlevel_selector_axis_color);

  _world.setLegendFontSize(_params.vlevel_selector_title_font_size);

  _world.setYAxisLabelsInside(false);

  _mouseMoveInProgress = false;
  
  update();
  
}

/******************************************************************/
VlevelSelector::~VlevelSelector()
{
}

/******************************************************************/
void VlevelSelector::setColorMap(const ColorMap *map)
{
  _colorMap = map;
  update();
}

/******************************************************************/
void VlevelSelector::paintEvent(QPaintEvent* e)
{

  if (_vlevelManager.getNLevels() == 1) {
    _world.setYNTicksIdeal(3);
  } else {
    _world.setYNTicksIdeal(_params.vlevel_selector_n_ticks_ideal);
  }

  // update world coords

  double vlevelMin = _vlevelManager.getLevelMin();
  double vlevelMax = _vlevelManager.getLevelMax();
  double vlevelRange = vlevelMax - vlevelMin;
  double worldYMin = vlevelMin;
  double worldYMax = vlevelMax;
  if (vlevelMin == vlevelMax) {
    worldYMin -= 1.0;
    worldYMax += 1.0;
  } else {
    worldYMin -= vlevelRange / 40.0;
    worldYMax += vlevelRange / 40.0;
  }

  _world.setWorldLimitsX(0.0, 1.0);
  _world.setWorldLimitsY(worldYMin, worldYMax);
  _world.setWindowGeom(width(), height(), 0, 0);

  // fill with background
  
  QPainter painter;
  painter.begin(this);
  _world.fillCanvas(painter, _params.vlevel_selector_background_color);

  // draw available levels

  for (size_t ii = 0; ii < _vlevelManager.getNLevels(); ii++) {
    double vlevel = _vlevelManager.getLevel(ii);
    QPen pen(_params.vlevel_selector_data_values_color);
    pen.setWidth(3);
    painter.setPen(pen);
    _world.drawLine(painter, 0.3, vlevel, 0.55, vlevel);
  }
  
  // draw selected vlevel
  
  double vlevel = _vlevelManager.getLevel();
  QBrush brush(_params.vlevel_selector_marker_color);

  double ptrHalfHt = 10.0 / _world.getYPixelsPerWorld();
  QVector<QPointF> poly;
  poly.push_back(QPointF(0.6, vlevel));
  poly.push_back(QPointF(1.0, vlevel + ptrHalfHt));
  poly.push_back(QPointF(1.0, vlevel - ptrHalfHt));
  _world.fillPolygon(painter, brush, poly);
  
  // draw Y axis
  
  _world.drawAxisLeft(painter, "", true, true, true, false);
  
  // draw titles

  QPen pen(_params.vlevel_selector_title_color);
  painter.setPen(pen);
  
  vector<string> titles;
  bool writeValue = false;
  switch (_vlevelManager.getMdvxVlevelType()) {
    case Mdvx::VERT_TYPE_Z:
      titles.push_back("Height");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_SIGMA_Z:
      titles.push_back("Sigma Level");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_PRESSURE:
      titles.push_back("Pressure");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_SIGMA_P:
      titles.push_back("SigmaP");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_THETA:
      titles.push_back("Theta");
      writeValue = true;
    case Mdvx::VERT_TYPE_ELEV:
    case Mdvx::VERT_VARIABLE_ELEV:
      titles.push_back("Elevation");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_AZ:
      titles.push_back("Azimuth");
      writeValue = true;
      break;
    case Mdvx::VERT_FLIGHT_LEVEL:
      titles.push_back("Flight Level");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_ETA:
      titles.push_back("ETA Level");
      writeValue = true;
      break;
    case Mdvx::VERT_TYPE_SURFACE:
      titles.push_back("Surface");
      break;
    case Mdvx::VERT_TYPE_COMPOSITE:
      titles.push_back("Composite");
      break;
    case Mdvx::VERT_SATELLITE_IMAGE:
      titles.push_back("Satellite");
      break;
    default:
      titles.push_back("Vert Level");
      break;
  }

  if (writeValue) {
    char text[1024];
    snprintf(text, 1024, "%g", _vlevelManager.getLevel());
    titles.push_back(text);
    if (_vlevelManager.getUnits().size() > 0) {
      titles.push_back(_vlevelManager.getUnits());
    }
  }

  _world.drawTitlesTopCenter(painter, titles);

  // label mouse move vlevel

  if (_mouseMoveInProgress) {
    _world.drawTextScreenCoords(painter, _mouseMoveValStr,
                                _mouseMoveX, _mouseMoveY,
                                Qt::AlignLeft | Qt::AlignVCenter);
  }

  painter.end();

  return;

#ifdef JUNK
  const std::vector<ColorMap::CmapEntry> &cmap = _colorMap->getEntries();
  
  int nBlocks = cmap.size() + 2;

  int h = height();
  int w = width();
  double deltaY = (double)(height())/nBlocks;
  int iDeltaY = (int) deltaY;

  // paint swatches
  
  QPainter p;
  p.begin(this);
  p.setPen(Qt::SolidLine);
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    QColor color(entry.red, entry.green, entry.blue);
    p.setBrush(color);
    double topY = h-(ii+2)*deltaY;
    QRectF r(0, topY, w, deltaY);
    // fill the swatch with the color
    p.fillRect(r, color);
  }
  p.end();

  // compute range

  double minDelta = 1.0e99;
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    double delta = fabs(entry.maxVal - entry.minVal);
    if (delta < minDelta) minDelta = delta;
  }
  int ndecimals = 0;
  char format = 'f';
  if (minDelta <= 0.025) {
    ndecimals = 3;
    format = 'g';
  } else if (minDelta <= 0.05) {
    ndecimals = 3;
  } else if (minDelta <= 0.25) {
    ndecimals = 2;
  } else if (minDelta <= 25) {
    ndecimals = 1;
  }

  // scale the font

  p.begin(this);

  QFont defaultFont = p.font();
  if (defaultFont.pointSize() > deltaY / 3) {
    int pointSize = deltaY / 3;
    if (pointSize < 7) {
      pointSize = 7;
    }
    QFont scaledFont(defaultFont);
    scaledFont.setPointSizeF(pointSize);
    p.setFont(scaledFont);
  }
  
  if (_annotation) {
    // add labels

    p.setBrush(Qt::black);
    p.setBackgroundMode(Qt::OpaqueMode);
    for (size_t ii = 0; ii < cmap.size(); ii++) {
      const ColorMap::CmapEntry &entry = cmap[ii];
      QString label = QString("%1").arg(entry.minVal,0,format,ndecimals);
      double yy = h-(ii+1)*deltaY - deltaY / 2.0;
      p.drawText(0, (int)yy, w, iDeltaY, 
                 Qt::AlignCenter | Qt::AlignHCenter, 
                 label);
    }
    const ColorMap::CmapEntry &entry = cmap[cmap.size()-1];
    QString label = QString("%1").arg(entry.maxVal,0,format,ndecimals);
    double yy = deltaY * 0.5;
    p.drawText(0, (int)yy, w, iDeltaY, 
               Qt::AlignVCenter | Qt::AlignHCenter, 
               label);

    // add Units header

    QString units(_colorMap->getUnits().c_str());
    p.drawText(0, 0, w, iDeltaY, 
               Qt::AlignVCenter | Qt::AlignHCenter, units);
  
  }

  p.end();

#endif
  
}

/******************************************************************/
void VlevelSelector::mouseReleaseEvent(QMouseEvent *e)
{
  
#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  double yVal = _world.getYWorld(pos.y());
  _vlevelManager.setLevel(yVal);
  _guiManager->setVlevelHasChanged(true);

  cerr << "YYYYYYYYYYYYYYYYYYY yVal: " << yVal << endl;

  _mouseMoveInProgress = false;
  
  emit released();

}

/******************************************************************/
void VlevelSelector::mouseMoveEvent(QMouseEvent *e)
{
  
#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  int xx = _params.vlevel_selector_left_margin / 2;
  int yy = pos.y();
  double yVal = _world.getYWorld(pos.y());
  
  char text[1024];
  snprintf(text, 1024, "%g", yVal);

  cerr << "YYYYYYYYYYYYYYYYYYY yy, yVal: " << yy << ", " << yVal << endl;

  _mouseMoveX = xx;
  _mouseMoveY = yy;
  _mouseMoveValStr = text;
  _mouseMoveInProgress = true;

  update();
  
}

////////////////////////////////////////////////////////////////
void VlevelSelector::keyPressEvent(QKeyEvent * e)
{

  // get key pressed
  
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "VlevelSelector, clicked char: "
         << keychar << ":" << (int) keychar << endl;
    cerr << "         key: " << hex << key << dec << endl;
  }
  
  if (key == Qt::Key_Up) {
    
    if (_vlevelManager.getGuiIndex() > 0) {
      if (_params.debug) {
        cerr << "VlevelSelector clicked up arrow, go up a vlevel" << endl;
      }
      // _changeVlevelRadioButton(-1);
    }

  } else if (key == Qt::Key_Down) {

    if (_vlevelManager.getGuiIndex() < (int) _vlevelManager.getNLevels() - 1) {
      if (_params.debug) {
        cerr << "VlevelSelector clicked down arrow, go down a vlevel" << endl;
      }
      // _changeVlevelRadioButton(+1);
    }
    
  }

}

/******************************************************************/
QImage* VlevelSelector::getImage()
{	
  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;
}

/******************************************************************/
QPixmap* VlevelSelector::getPixmap()
{	
  QPixmap* pixmap = new QPixmap(grab());
  return pixmap;
}

/******************************************************************/
QPixmap* VlevelSelector::getPixmap(int someWidth, int someHeight)
{	
  // we want the middle third 
  int leftThird = width()/3;
  QRect rect(QPoint(leftThird, 0), QSize(width()/3, height()));
  QPixmap *pixmap = new QPixmap(grab(rect)); // new QPixmap(QPixmap::grab());
  return pixmap;
}
