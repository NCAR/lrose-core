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
#include "FieldRenderer.hh"
using namespace std;


/*************************************************************************
 * Constructor
 */

FieldRenderer::FieldRenderer(const Params &params,
                             const size_t field_index,
                             const DisplayField &field) :
        _params(params),
        _fieldIndex(field_index),
        _field(field),
        _image(NULL),
        _backgroundRender(false),
        _backgroundRenderTimer(NULL),
        _useHeight(false),
        _drawInstHt(false)
{

  // set up background rendering timer
  
  _backgroundRenderTimer = new QTimer(this);
  _backgroundRenderTimer->setSingleShot(true);
  _backgroundRenderTimer->setInterval
    ((int)(_params.background_render_mins * 60000.0));
  
  connect(_backgroundRenderTimer, SIGNAL(timeout()),
	  this, SLOT(setBackgroundRenderOff()));

}

/*************************************************************************
 * Destructor
 */

FieldRenderer::~FieldRenderer()
{
  delete _image;
}

/////////////////////////////////////
// create image into which we render

void FieldRenderer::createImage(int width, int height)

{
  delete _image;
  _image = new QImage(width, height, QImage::Format_RGB32);
}

//////////////////////////////////////////////////
// Add to the beams to be rendered.

void FieldRenderer::addBeam(Beam *beam)
{

  TaThread::LockForScope locker;

  _beams.push_back(beam);
  beam->addClient();

}

////////////////////////////////////////////////////////////////////
// Perform the housekeeping needed when this field is newly selected.

void FieldRenderer::selectField() 

{
  activateBackgroundRendering();
}
  
////////////////////////////////////////////////////////////////////
// Perform the housekeeping needed when this field is newly unselected.

void FieldRenderer::unselectField() 

{

  // Turn on background rendering
  
  _backgroundRender = true;
  
  // Start the timer for turning off the background rendering after the
  // specified period of time.
  
  _backgroundRenderTimer->start();
  
}
  
////////////////////////////////////////////////////////////////////
// Activate background rendering - turn on until time resets

void FieldRenderer::activateBackgroundRendering() 

{

  // Turn on background rendering
  
  _backgroundRender = true;
  
  // Start the timer for turning off the background rendering after the
  // specified period of time.
  
  _backgroundRenderTimer->start();
  
}
  
////////////////////////////////////////////////////////////////////
// Set background rendering on - until set off

void FieldRenderer::setBackgroundRenderingOn() 

{

  // Turn on background rendering
  
  _backgroundRender = true;
  
}
  
////////////////////////////////////////////////////////////////
// Thread run method
// Actually performs the rendering

void FieldRenderer::run()
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Start of rendering for field: " 
         << _field.getLabel() << endl;
  }

  if (_beams.size() == 0) {
    return;
  }
  if (_image == NULL) {
    return;
  }
  
  TaThread::LockForScope locker;

  vector< Beam* >::iterator beam;
  for (beam = _beams.begin(); beam != _beams.end(); ++beam)
  {
    if (*beam == NULL) {
      continue;
    }
    // The following print can be problematic for threading
    // if (_params.debug >= Params::DEBUG_EXTRA) {
    //   cerr << "Rendering beam field:" 
    //        << _field.getLabel() << endl;
    //   (*beam)->print(cerr);
    // }
    (*beam)->paint(_image, _transform, _fieldIndex, _useHeight, _drawInstHt);
    (*beam)->setBeingRendered(_fieldIndex, false);
  }
  
  for (beam = _beams.begin(); beam != _beams.end(); ++beam)
  {
    Beam::deleteIfUnused(*beam);
  }
  _beams.clear();
  
}
