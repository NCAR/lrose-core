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
#include "ColorBar.hh"

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QRectF>
#include <QPaintEvent>
#include <iostream>
using namespace std;

ColorBar::ColorBar(int width, const ColorMap *cmap,
                   QWidget* parent) :
        QWidget(parent),
        _colorMap(cmap)
{
  setMinimumSize(width, 100);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  update();
}

/******************************************************************/
ColorBar::~ColorBar()
{
}

/******************************************************************/
void ColorBar::setColorMap(const ColorMap *map) {
  _colorMap = map;
  update();
}

/******************************************************************/
void
  ColorBar::paintEvent(QPaintEvent* e) {

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

  // add labels
  
  p.begin(this);
  p.setBrush(Qt::black);
  p.setBackgroundMode(Qt::OpaqueMode);
  for (size_t ii = 0; ii < cmap.size(); ii++) {
    const ColorMap::CmapEntry &entry = cmap[ii];
    QString label = QString("%1").arg(entry.minVal,0,'f',1);
    double yy = h-(ii+1)*deltaY - deltaY / 2.0;
    p.drawText(0, (int)yy, w, iDeltaY, 
               Qt::AlignCenter | Qt::AlignHCenter, 
               label);
  }
  const ColorMap::CmapEntry &entry = cmap[cmap.size()-1];
  QString label = QString("%1").arg(entry.maxVal,0,'f',1);
  double yy = deltaY * 0.5;
  p.drawText(0, (int)yy, w, iDeltaY, 
             Qt::AlignVCenter | Qt::AlignHCenter, 
             label);

  // add Units header

  QString units(_colorMap->getUnits().c_str());
  p.drawText(0, 0, w, iDeltaY, 
             Qt::AlignVCenter | Qt::AlignHCenter, units);
  
  p.end();

}

/******************************************************************/
void
  ColorBar::mouseReleaseEvent(QMouseEvent *e) {
  emit released();
}

/******************************************************************/
QImage* ColorBar::getImage()
{	
  QPixmap pixmap = QPixmap::grabWidget(this);
  QImage* image = new QImage(pixmap.toImage());
  return image;
}

/******************************************************************/
QPixmap* ColorBar::getPixmap()
{	
  QPixmap* pixmap = new QPixmap(QPixmap::grabWidget(this));
  return pixmap;
}
