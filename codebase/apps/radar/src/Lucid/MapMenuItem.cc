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
// MapMenuItem.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2024
//
///////////////////////////////////////////////////////////////
//
// Wraps a map object, and provides the toggled() method
// for responding to menu selection.
//
///////////////////////////////////////////////////////////////

#include "MapMenuItem.hh"
#include "GuiManager.hh"
#include "GlobalData.hh"

// Constructor

MapMenuItem::MapMenuItem(GuiManager *manager,
                         Params &params) :
        _manager(manager),
        _params(params),
        _gd(GlobalData::Instance()),
        _mapParams(NULL),
        _overlay(NULL),
        _mapIndex(-1),
        _act(NULL)
        
{
  
  
}

// destructor

MapMenuItem::~MapMenuItem()

{

}

///////////////////////////////////////////////
// Connect to map menu button

void MapMenuItem::toggled(bool checked)
{
  // set activity depending on checked
  
  if (_mapIndex < (int) _gd.overlays.size()) {
    _gd.overlays[_mapIndex]->active = checked;
    _mapParams->on_at_startup = (tdrp_bool_t) checked;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> MapMenuItem toggled, is_on? " << checked << endl;
    cerr << "  mapIndex: " << _mapIndex << endl;
    cerr << "  map_code: " << _mapParams->map_code << endl;
    cerr << "  control_label: " << _mapParams->control_label << endl;
    cerr << "  map_file_name: " << _mapParams->map_file_name << endl;
    cerr << "  active: " << _gd.overlays[_mapIndex]->active << endl;
  }

  if (_manager != NULL) {
    _manager->setOverlaysHaveChanged(true);
  }

}

