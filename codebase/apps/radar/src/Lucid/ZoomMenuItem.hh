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
/////////////////////////////////////////////////////////////
// ProdMenuItem.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2024
//
///////////////////////////////////////////////////////////////
//
// Action for each entry in the zoom menu.
//
///////////////////////////////////////////////////////////////

#ifndef ZoomMenuItem_HH
#define ZoomMenuItem_HH

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

#include <string>
#include <vector>
#include <QObject>
#include <QAction>
#include "GlobalData.hh"
#include "Params.hh"

using namespace std;

class Overlay_t;
class GuiManager;

class DLL_EXPORT ZoomMenuItem : public QObject {
  
  Q_OBJECT

 public:
  
  // constructor
  
  ZoomMenuItem(GuiManager *manager,
               Params &params);
  
  // destructor
  
  virtual ~ZoomMenuItem();
  
  // set
  
  void setZoomParams(Params::zoom_level_t *val) { _zoomParams = val; }
  void setZoomIndex(int val) { _zoomIndex = val; }
  void setAction(QAction *val) { _act = val; }

  // get
  
  const Params::zoom_level_t *getZoomParams() const { return _zoomParams; }
  int getZoomIndex() const { return _zoomIndex; }
  QAction *getAction() { return _act; }

 protected:
  
  GuiManager *_manager;
  Params &_params;
  GlobalData &_gd;
  Params::zoom_level_t *_zoomParams;
  int _zoomIndex;
  QAction *_act;               

 public slots:

  void toggled(bool checked); // connected to menu button

};

#endif

