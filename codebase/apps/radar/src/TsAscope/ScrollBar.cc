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
#include <qstyle.h>
#if QT_VERSION >= 0x040000
#include <qstyleoption.h>
#endif
#include "ScrollBar.hh"

ScrollBar::ScrollBar(QWidget * parent):
        QScrollBar(parent)
{
  init();
}

ScrollBar::ScrollBar(Qt::Orientation o, 
                     QWidget *parent):
        QScrollBar(o, parent)
{
  init();
}

ScrollBar::ScrollBar(double minBase, double maxBase, 
                     Qt::Orientation o, QWidget *parent):
        QScrollBar(o, parent)
{
  init();
  setBase(minBase, maxBase);
  moveSlider(minBase, maxBase);
}

void ScrollBar::init()
{
  d_inverted = orientation() == Qt::Vertical;
  d_baseTicks = 1000000;
  d_minBase = 0.0;
  d_maxBase = 1.0;
  moveSlider(d_minBase, d_maxBase);

  connect(this, SIGNAL(sliderMoved(int)), SLOT(catchSliderMoved(int)));
  connect(this, SIGNAL(valueChanged(int)), SLOT(catchValueChanged(int)));
}

void ScrollBar::setInverted(bool inverted)
{
  if ( d_inverted != inverted )
  {
    d_inverted = inverted;
    moveSlider(minSliderValue(), maxSliderValue());
  }
}

bool ScrollBar::isInverted() const
{
  return d_inverted;
}

void ScrollBar::setBase(double min, double max)
{
  if ( min != d_minBase || max != d_maxBase )
  {
    d_minBase = min;
    d_maxBase = max;

    moveSlider(minSliderValue(), maxSliderValue());
  }
}

void ScrollBar::moveSlider(double min, double max)
{
  const int sliderTicks = qRound((max - min) / 
                                 (d_maxBase - d_minBase) * d_baseTicks);

  // setRange initiates a valueChanged of the scrollbars
  // in some situations. So we block
  // and unblock the signals.

  blockSignals(true);

  setRange(sliderTicks / 2, d_baseTicks - sliderTicks / 2);
  int steps = sliderTicks / 200;
  if ( steps <= 0 )
    steps = 1;

#if QT_VERSION < 0x040000
  setSteps(steps, sliderTicks);
#else
  setSingleStep(steps);
  setPageStep(sliderTicks);
#endif

  int tick = mapToTick(min + (max - min) / 2);
  if ( isInverted() )
    tick = d_baseTicks - tick;

#if QT_VERSION < 0x040000
  directSetValue(tick);
  rangeChange();
#else
  setSliderPosition(tick);
#endif
  blockSignals(false);
}

double ScrollBar::minBaseValue() const
{
  return d_minBase;
}

double ScrollBar::maxBaseValue() const
{
  return d_maxBase;
}

void ScrollBar::sliderRange(int value, double &min, double &max) const
{
  if ( isInverted() )
    value = d_baseTicks - value;

  const int visibleTicks = pageStep();

  min = mapFromTick(value - visibleTicks / 2);
  max = mapFromTick(value + visibleTicks / 2);
}

double ScrollBar::minSliderValue() const
{
  double min, dummy;
  sliderRange(value(), min, dummy);

  return min;
}

double ScrollBar::maxSliderValue() const
{
  double max, dummy;
  sliderRange(value(), dummy, max);

  return max;
}

int ScrollBar::mapToTick(double v) const
{   
  return (int) ( ( v - d_minBase) / (d_maxBase - d_minBase ) * d_baseTicks );
}

double ScrollBar::mapFromTick(int tick) const
{   
  return d_minBase + ( d_maxBase - d_minBase ) * tick / d_baseTicks;
}

void ScrollBar::catchValueChanged(int value)
{
  double min, max;
  sliderRange(value, min, max);
  emit valueChanged(orientation(), min, max);
}

void ScrollBar::catchSliderMoved(int value)
{
  double min, max;
  sliderRange(value, min, max);
  emit sliderMoved(orientation(), min, max);
}

int ScrollBar::extent() const
{
#if QT_VERSION < 0x040000
  return style().pixelMetric(QStyle::PM_ScrollBarExtent, this);
#else
  QStyleOptionSlider opt;
  //opt.init(this);
  opt.subControls = QStyle::SC_None;
  opt.activeSubControls = QStyle::SC_None;
  opt.orientation = orientation();
  opt.minimum = minimum();
  opt.maximum = maximum();
  opt.sliderPosition = sliderPosition();
  opt.sliderValue = value();
  opt.singleStep = singleStep();
  opt.pageStep = pageStep();
  opt.upsideDown = invertedAppearance();
  if (orientation() == Qt::Horizontal)
    opt.state |= QStyle::State_Horizontal;
  return style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt, this);
#endif
}
