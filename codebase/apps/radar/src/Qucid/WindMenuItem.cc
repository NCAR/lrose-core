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
// WindMenuItem.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2024
//
///////////////////////////////////////////////////////////////
//
// Wraps a wind object, and provides the toggled() method
// for responding to menu selection.
//
///////////////////////////////////////////////////////////////

#include "WindMenuItem.hh"
#include "cidd.h"

// Constructor

WindMenuItem::WindMenuItem(QObject *parent) :
        _parent(parent),
        _windParams(NULL),
        _windData(NULL),
        _windIndex(-1),
        _act(NULL)
        
{
  
  
}

// destructor

WindMenuItem::~WindMenuItem()

{

}

///////////////////////////////////////////////
// Connect to wind menu button

void WindMenuItem::toggled(bool checked)
{

  // set activity depending on checked
  
  if (_windIndex < (int) gd.layers.wind.size()) {
    if (checked) {
      gd.layers.wind[_windIndex].active = 1;
    } else {
      gd.layers.wind[_windIndex].active = 0;
    }
    _windParams->on_at_startup = (tdrp_bool_t) checked;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> WindMenuItem toggled, is_on? " << checked << endl;
    cerr << "  windIndex: " << _windIndex << endl;
    cerr << "  button_label: " << _windParams->button_label << endl;
    cerr << "  url: " << _windParams->url << endl;
    cerr << "  active: " << gd.layers.wind[_windIndex].active << endl;
  }

}

