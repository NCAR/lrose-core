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
#ifndef RhiWindow_HH
#define RhiWindow_HH

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

#include <QAction>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <vector>

#include "Params.hh"
#include "RhiWidget.hh"

class PolarManager;
class DisplayField;

class DLL_EXPORT RhiWindow : public QMainWindow
{

  // must include this if you use Qt signals/slots
  Q_OBJECT

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent   The parent widget.
   */

  RhiWindow(PolarManager *manager,
            const Params &params,
            const RadxPlatform &platform,
            const vector<DisplayField *> &fields,
            bool haveFilteredFields);
            

  /**
   * @brief Destructor
   */

  virtual ~RhiWindow();

  // get the RHI widget

  RhiWidget *getWidget() { return _rhiWidget; }

  /**
   * @brief Set the azimuth value displayed in the window.
   */

  void setAzimuth(const double azimuth);

  /**
   * @brief Set the elevation value displayed in the window.
   */

  void setElevation(const double elevation);

  /**
   * @brief Set the radar name.  The name is included as part of the window
   *        title.
   */

  void setRadarName(const string &radar_name);

  // enable the zoom button - called by RhiWidget
  
  void enableZoomButton() const;

public slots:

  //////////////
  // Qt slots //
  //////////////

  /**
   * @brief Method called by the main program on the first timer event to
   *        ask the window to resize.
   */

  void resize();

private slots:
  
  // override
  
  virtual void _unzoom();
  
signals:

  ////////////////
  // Qt signals //
  ////////////////

  /**
   * @brief Signal emitted when the window is resized.
   *
   * @param[in] width    The new width of the window.
   * @param[in] height   The new height of the window.
   */

  void windowResized(const int width, const int height);
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The main frame for the window.
   */

  QFrame *_main;
  
  /**
   * @brief The parent frame of the RHI widget.
   */

  QFrame *_rhiTopFrame;
  RhiWidget *_rhiWidget;

  // the polar manager that created this window

  PolarManager *_manager;

  // params

  const Params &_params;
  
  // instrument platform details  - platform exists in DisplayManager
  
  const RadxPlatform &_platform;
  
  // data fields
            
  const vector<DisplayField *> &_fields;
  bool _haveFilteredFields;

  /**
   * @brief The View menu.
   */

  QMenu *_overlaysMenu;

  /**
   * @brief The status panel frame.
   */

  QFrame *_statusPanel;
  
  /**
   * @brief The elevation value label.
   */

  QLabel *_elevValue;
  
  /**
   * @brief The azimuth value label.
   */

  QLabel *_azValue;
  

  ///////////////////////
  // Protected actions //
  ///////////////////////

  /**
   * @brief Action for unzooming the window.
   */

  QAction *_unzoomAct;

  /**
   * @brief Action for turning range rings on and off.
   */

  QAction *_ringsAct;

  /**
   * @brief Action for turning azimuth lines on and off.
   */

  QAction *_azLinesAct;

  /**
   * @brief Action for turning grid lines on and off.
   */

  QAction *_gridsAct;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  // override event handling

  virtual void keyPressEvent(QKeyEvent* event);
  virtual void resizeEvent(QResizeEvent *event);

  /**
   * @brief Create the actions for the menu bar menus on this window.
   *
   * @param[in] rhi    A pointer to the RHI widget in this window.
   */

  void _createActions(RhiWidget *rhi);

  /**
   * @brief Create the menus for the menu bar on this window.
   */

  void _createMenus();
  
  /**
   * @brief Create the status panel.
   *
   * @param[in] label_font_size  The label font size from the parameter file.
   */

  void _createStatusPanel(const int label_font_size);
  
};

#endif
