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
#ifndef VertManager_HH
#define VertManager_HH

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

#include "GlobalData.hh"
#include "Params.hh"
#include "VertView.hh"

class GuiManager;
// class DisplayField;

class DLL_EXPORT VertManager : public QMainWindow
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
  
  explicit VertManager(GuiManager *manager);

  /**
   * @brief Destructor
   */

  virtual ~VertManager();
  
  // get the VertView widget

  VertView *getView() { return _vertView; }

  // Update the section identity shown with the separate vertical window.

  void setSectionInfo(const string &fieldName,
                      const DateTime &validTime,
                      double routeLengthKm);

  // enable the zoom button - called by VertView
  
  void enableZoomButton() const;
  void setMovieInteractionActive(bool active);

signals:

  void singleSegmentVsectionChanged(bool enabled);

private slots:
  
  // override
  
  virtual void _unzoom();
  void _setMinimumHeight();
  void _setMaximumHeight();
  
protected:

  void resizeEvent(QResizeEvent *event) override;

  ///////////////////////
  // Protected members //
  ///////////////////////

  // the polar manager that created this window

  GuiManager *_guiManager;

  Params &_params;
  GlobalData &_gd;

  string _fieldName;
  DateTime _validTime;
  double _routeLengthKm;
  double _minHeightKm;
  double _maxHeightKm;
  
  /**
   * @brief The main frame for the window.
   */

  QFrame *_main;
  
  /**
   * @brief The parent frame of the VERT widget.
   */

  QFrame *_vertTopFrame;
  VertView *_vertView;

  /**
   * @brief The View menu.
   */

  QMenu *_overlaysMenu;
  QMenu *_configurationMenu;

  ///////////////////////
  // Protected actions //
  ///////////////////////

  /**
   * @brief Action for unzooming the window.
   */

  QAction *_unzoomAct;

  /**
   * @brief Actions for independently controlling height and distance grids.
   */

  QAction *_heightGridAct;
  QAction *_distanceGridAct;

  // Checked for single-segment selection; unchecked for multi-segment routes.

  QAction *_singleSegmentVsectionAct;

  // Actions edit the unzoomed height range initialized by the TDRP
  // parameters. The current values are included in their menu labels.

  QAction *_minimumHeightAct;
  QAction *_maximumHeightAct;
  QAction *_plotSelectedHorizHeightAct;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // override event handling

  void keyPressEvent(QKeyEvent* event) override;
  void closeEvent(QCloseEvent *event) override;
  
  /**
   * @brief Create the actions for the menu bar menus on this window.
   *
   * @param[in] vert    A pointer to the VERT widget in this window.
   */

  void _createActions(VertView *vert);

  /**
   * @brief Create the menus for the menu bar on this window.
   */
  
  void _createMenus();
  void _updateHeightActionText();
  
};

#endif
