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
#ifndef VLEVEL_SELECTOR_HH
#define VLEVEL_SELECTOR_HH

#include <QWidget>
#include <QLayout>
#include <QVBoxLayout>
#include <vector>
#include <string>
#include <QImage>
#include <QPixmap>
#include "WorldPlot.hh"
#include "VlevelManager.hh"

class GuiManager;

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

/// A widget that displays a representation of ColorMap.
/// Swatches are displayed in a vertical column, with text labels
/// indicating the value of the swatch.
class DLL_EXPORT VlevelSelector: public QWidget
{

  Q_OBJECT

public:

  VlevelSelector(int width,
                 VlevelManager &vlevelManager,
                 GuiManager* parent = 0);

  virtual ~VlevelSelector(void);
  
  /// Capture a key press event
  virtual void keyPressEvent(QKeyEvent* e) override;

 signals:

  void released();
  
 protected:

  /// Capture a mouse release and emit a released() signal.
  virtual void mouseReleaseEvent(QMouseEvent* e) override;
  
  /// Capture a mouse move event.
  virtual void mouseMoveEvent(QMouseEvent* e) override;

  /// The paint event is where we will draw the color bar.
  virtual void paintEvent(QPaintEvent* e) override;

 private:
  
  WorldPlot _world;
  VlevelManager &_vlevelManager;
  GuiManager *_guiManager;
  const Params &_params;

  bool _mouseMoveInProgress;
  int _mouseMoveX, _mouseMoveY;
  double _mouseMoveVlevel;
  string _mouseMoveVlevelStr;

};

#endif
