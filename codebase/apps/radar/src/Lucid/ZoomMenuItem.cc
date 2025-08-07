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
///////////////////////////////////////////////////////////////
// ZoomMenuItem.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2024
//
///////////////////////////////////////////////////////////////
//
// Wraps a zoom object, and provides the toggled() method
// for responding to menu selection.
//
///////////////////////////////////////////////////////////////

#include "ZoomMenuItem.hh"
#include "GuiManager.hh"
#include "GlobalData.hh"

// Constructor

ZoomMenuItem::ZoomMenuItem(GuiManager *manager,
                           Params &params) :
        _manager(manager),
        _params(params),
        _gd(GlobalData::Instance()),
        _zoomParams(NULL),
        _zoomIndex(-1),
        _act(NULL)
        
{
  // _manager = NULL;
}

// destructor

ZoomMenuItem::~ZoomMenuItem()

{

}

///////////////////////////////////////////////
// Connect to zoom menu button

void ZoomMenuItem::toggled(bool checked)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> ZoomMenuItem toggled, is_on? " << checked << endl;
    cerr << "  zoomIndex: " << _zoomIndex << endl;
    cerr << "  label: " << _zoomParams->label << endl;
    cerr << "  min_x: " << _zoomParams->min_x << endl;
    cerr << "  min_y: " << _zoomParams->min_y << endl;
    cerr << "  max_x: " << _zoomParams->max_x << endl;
    cerr << "  max_y: " << _zoomParams->max_y << endl;
  }

  if (checked) {
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "==>> Changing to zoom level: " << _zoomParams->label << endl;
    }
    _gd.h_win.zoom_level = _zoomIndex;
    if (_manager != NULL) {
      _manager->setZoomIndex(_zoomIndex); 
    }
  }

}

