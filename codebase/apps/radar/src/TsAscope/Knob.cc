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
#include "Knob.hh"
#include <qlabel.h>
#include <qwt/qwt_knob.h>

Knob::Knob( QWidget* parent):
        QWidget(parent)
{
  setupUi(this);
  connect(_knob, SIGNAL(valueChanged(double)), this, SLOT(valueChangedSlot(double)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
Knob::~Knob()
{
  // no need to delete child widgets, Qt does it all for us
}

void
  Knob::setTitle(std::string title)
{
  _label->setText(title.c_str());
}

void Knob::setRange(double min, double max)
{
#if (QWT_VERSION < 0x060100)
  _knob->setRange(min, max);
#else
  _knob->setScale(min, max);
#endif
}

void Knob::setValue(double val)
{
  _knob->setValue(val);
}

void Knob::valueChangedSlot(double v)
{
  emit valueChanged(v);
}

void Knob::setScaleMaxMajor(int ticks)
{
  _knob->setScaleMaxMajor(ticks);
}

void Knob::setScaleMaxMinor(int ticks)
{
  _knob->setScaleMaxMinor(ticks);
}

void Knob::getRange(double& min, double& max)
{
#if (QWT_VERSION < 0x060100)
  min = _knob->minValue();
  max = _knob->maxValue();
#else
  min = _knob->lowerBound();
  max = _knob->upperBound();
#endif
}

double Knob::value()
{
  return _knob->value();
}
