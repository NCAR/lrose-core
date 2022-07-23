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
#include <assert.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "Beam.hh"
#include <qtplot/ColorMap.hh>
#include "AllocCheck.hh"

using namespace std;

/////////////////////
// polar constructor

Beam::Beam(const Params &params,
           const RadxRay *ray,
	   int n_fields) :
        _params(params),
        _ray(ray),
        _nGates(ray->getNGates()),
        _nFields(n_fields)
{

  // Set the "being rendered" flags.  We always render the new beams, so start
  // out with all of the flags being set to true.

  for (int i = 0; i < n_fields; ++i) {
    _beingRendered.push_back(true);
  }
  
  _beingRenderedAnywhere = true;
  
  // Allocate space for the brushes. There is a brush for each gate of each
  // field.  The colors for the brushes will be filled in later in the
  // fillColors() method.
  
  _brushes.resize(n_fields);
  for (int field = 0; field < n_fields; ++field) {
    _brushes[field].resize(_nGates);
  }

  // initialize client counting for this object

  _nClients = 0;
  pthread_mutex_init(&_nClientsMutex, NULL);

  // increment client count on the ray

  _ray->addClient();

}

////////////////////////////////////////////////////////////////

Beam::~Beam()
{

  _brushes.clear();

  // decrement client count on the ray
  // and delete if no other clients

  if (_ray->removeClient() == 0) {
    delete _ray;
    AllocCheck::inst().addFree();
  }

  // clear client reference counting mutex

  pthread_mutex_destroy(&_nClientsMutex);

}

////////////////////////////////////////////////////////////////
void Beam::resetFieldBrush(size_t field, const QBrush *brush)
{

  assert(field < _nFields);
  
  for (size_t gate = 0; gate < _nGates; ++gate) {
    _brushes[field][gate] = brush;
  }

}

////////////////////////////////////////////////////////////////

void Beam::fillColors(const std::vector<std::vector<double> >& beam_data,
		      const std::vector<DisplayField*>& fields,
		      const QBrush *background_brush)

{

  for (size_t field = 0; field < _nFields; ++field) {

    const ColorMap &map = fields[field]->getColorMap();
    const double *field_data = &(beam_data[field][0]);
    
    for (size_t igate = 0; igate < _nGates; ++igate) {
      
      double data = field_data[igate];
      
      if (data < -9990) {
	_brushes[field][igate] = background_brush;
      } else {
	_brushes[field][igate] = map.dataBrush(data);
      }

    } // igate

  }

}

/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class optionally uses the notion of clients to decide when it
// should be deleted.
// If removeClient() returns 0, the object should be deleted.
// These functions are protected by a mutex for multi-threaded ops

int Beam::addClient() const
  
{
  pthread_mutex_lock(&_nClientsMutex);
  _nClients++;
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

int Beam::removeClient() const

{
  pthread_mutex_lock(&_nClientsMutex);
  if (_nClients > 0) {
    _nClients--;
  }
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

int Beam::removeAllClients() const

{
  pthread_mutex_lock(&_nClientsMutex);
  _nClients = 0;
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

void Beam::deleteIfUnused(const Beam *beam)
  
{
  if (beam->removeClient() == 0) {
    delete beam;
  }
}

