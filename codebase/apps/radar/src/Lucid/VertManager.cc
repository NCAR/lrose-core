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
#include <iostream>

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QVBoxLayout>

#include "GuiManager.hh"
#include "VertManager.hh"
#include "GlobalData.hh"

using namespace std;

/*************************************************************************
 * Constructor
 */

VertManager::VertManager(GuiManager *manager):
        QMainWindow(manager),
        _guiManager(manager),
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _minHeightKm(_params.vsection_min_ht_km),
        _maxHeightKm(_params.vsection_max_ht_km)
        
{
  // Create the main frame which contains everything in this window

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // Create the parent frame for the VERT rendering

  _vertTopFrame = new QFrame(_main);
  _vertTopFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  // create VERT view
  
  _vertView = new VertView(_vertTopFrame, *manager, *this);

  // Let Qt propagate the frame's final geometry to the rendering widget.
  // The inherited HawkEye code used a one-time resize workaround, which could
  // leave this child at its small construction size when the window opened.

  QVBoxLayout *vertLayout = new QVBoxLayout(_vertTopFrame);
  vertLayout->setContentsMargins(0, 0, 0, 0);
  vertLayout->addWidget(_vertView);
  
  // Create the main window layout.  We need a layout so the main window can
  // contain multiple views.

  QVBoxLayout *main_layout = new QVBoxLayout(_main);
  main_layout->setContentsMargins(0,0,0,0);
  main_layout->addWidget(_vertTopFrame, 100);
  
  // Create the actions and the menus

  _createActions(_vertView);
  _createMenus();

  // Set the window attributes.

  setWindowTitle(tr("VERT"));
  setMinimumSize(_params.vsection_window_min_width,
                 _params.vsection_window_min_height);

  // Keep the vertical window's startup geometry independent of the main
  // window. The color scale has its own width parameter, while the configured
  // vertical plot height is used directly as the initial window height.

  resize(_params.vsection_plot_width + _params.vsection_color_scale_width,
         _params.vsection_plot_height);

}


/*************************************************************************
 * Destructor
 */

VertManager::~VertManager()
{

}


////////////////////////////////////////////////////////////////
void VertManager::keyPressEvent(QKeyEvent * e)
{
  // pass event up to GuiManager
  _guiManager->keyPressEvent(e);
}

void VertManager::resizeEvent(QResizeEvent *event)
{
  QMainWindow::resizeEvent(event);
  if (_guiManager->isMovieActive()) {
    _guiManager->stopMovieForDisplayChange();
  }
}

////////////////////////////////////////////////////////////////
void VertManager::closeEvent(QCloseEvent * e)
{
  // The open vertical window is part of every frame's readiness contract once
  // construction starts. Keep it present until construction completes;
  // during playback it is safe to hide because its cached index continues to
  // advance with the horizontal view.
  if (_guiManager->isMovieBuilding()) {
    _guiManager->showMovieInteractionBlocked();
    e->ignore();
    return;
  }
  // pass event up to GuiManager
  _guiManager->vertWindowClosed(e);
}

// Protected methods

/*************************************************************************
 * _createActions()
 */

void VertManager::_createActions(VertView *vert)
{

  // show height grid
  
  _heightGridAct = new QAction(tr("Height Grid"), this);
  _heightGridAct->setStatusTip(tr("Turn horizontal height grid lines on/off"));
  _heightGridAct->setCheckable(true);
  _heightGridAct->setChecked(_params.vsection_ht_grid_on_at_startup);
  connect(_heightGridAct, &QAction::toggled, vert, &VertView::setHeightGrid);

  // show distance grid
  
  _distanceGridAct = new QAction(tr("Distance Grid"), this);
  _distanceGridAct->setStatusTip(tr("Turn vertical distance grid lines on/off"));
  _distanceGridAct->setCheckable(true);
  _distanceGridAct->setChecked(_params.vsection_distance_grid_on_at_startup);
  connect(_distanceGridAct, &QAction::toggled, vert, &VertView::setDistanceGrid);

  // unzoom
  
  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  _unzoomAct->setEnabled(false);
  connect(_unzoomAct, SIGNAL(triggered()), this, SLOT(_unzoom()));

  // set single segment on/off
  
  _singleSegmentVsectionAct =
    new QAction(tr("Single-segment selection"), this);
  _singleSegmentVsectionAct->setStatusTip(
    tr("Complete a vertical section after one right-button drag"));
  _singleSegmentVsectionAct->setCheckable(true);
  _singleSegmentVsectionAct->setChecked(_params.single_segment_vsection);
  connect(_singleSegmentVsectionAct, &QAction::toggled,
          this, [this](bool enabled) {
            // Keep the runtime parameter authoritative so the selection code
            // and help text immediately reflect the menu state.
            _params.single_segment_vsection = enabled ? pTRUE : pFALSE;
            emit singleSegmentVsectionChanged(enabled);
          });

  // Height-range actions use dialogs rather than embedded menu widgets so a
  // partially entered value cannot update the plot or violate min < max.

  _minimumHeightAct = new QAction(this);
  _minimumHeightAct->setStatusTip(
    tr("Set the minimum height of the vertical-section plot"));
  connect(_minimumHeightAct, &QAction::triggered,
          this, &VertManager::_setMinimumHeight);

  _maximumHeightAct = new QAction(this);
  _maximumHeightAct->setStatusTip(
    tr("Set the maximum height of the vertical-section plot"));
  connect(_maximumHeightAct, &QAction::triggered,
          this, &VertManager::_setMaximumHeight);

  _updateHeightActionText();

  // Show the plan-view level as a reference line across the section.

  _plotSelectedHorizHeightAct =
    new QAction(tr("Show selected horizontal height"), this);
  _plotSelectedHorizHeightAct->setStatusTip(
    tr("Show the main horizontal view's selected height on the section"));
  _plotSelectedHorizHeightAct->setCheckable(true);
  _plotSelectedHorizHeightAct->setChecked(
    _params.vsection_plot_selected_horiz_height);
  connect(_plotSelectedHorizHeightAct, &QAction::toggled,
          this, [this, vert](bool enabled) {
            _params.vsection_plot_selected_horiz_height =
              enabled ? pTRUE : pFALSE;
            vert->setPlotSelectedHorizHeight(enabled);
          });

  // Terrain is retained independently of the gridded field section, allowing
  // this inexpensive overlay to be hidden or restored without another field
  // request.
  _terrainAct = new QAction(tr("Show terrain"), this);
  _terrainAct->setStatusTip(
    tr("Show the terrain surface and earth mask on the vertical section"));
  _terrainAct->setCheckable(true);
  _terrainAct->setChecked(_gd.layers.earth.terrain_active != 0);
  _terrainAct->setEnabled(_gd.layers.earth.terr != nullptr);
  connect(_terrainAct, &QAction::toggled,
          _guiManager, &GuiManager::setVerticalTerrainEnabled);

}

//////////////////////////////////////////////////
// enable the zoom button - called by VertView

void VertManager::enableZoomButton() const
{
  if (_guiManager->isMovieActive()) {
    return;
  }
  _unzoomAct->setEnabled(true);
}

void VertManager::setMovieInteractionActive(bool active)
{
  // Cached vertical frames share one transform and overlay configuration.
  // Defer changes to those controls until live rendering resumes.
  _overlaysMenu->setEnabled(!active);
  _configurationMenu->setEnabled(!active);
  _unzoomAct->setEnabled(!active && !_vertView->getSavedZooms().empty());
  _vertView->setMovieInteractionActive(active);
}

////////////////////////////////
// unzoom display

void VertManager::_unzoom()
{
  _vertView->unzoomView();
  _unzoomAct->setEnabled(false);
}

/////////////////////////////////////////////////////////////
// set the minimum height of the unzoomed plot

void VertManager::_setMinimumHeight()
{
  bool accepted = false;
  const double value = QInputDialog::getDouble(
    this, tr("Minimum vertical-section height"), tr("Height (km MSL):"),
    _minHeightKm, -100000.0, _maxHeightKm - 0.001, 3, &accepted,
    Qt::WindowFlags(), 0.1);
  if (!accepted) {
    return;
  }

  _minHeightKm = value;
  _params.vsection_min_ht_km = value;
  _vertView->setHeightRange(_minHeightKm, _maxHeightKm);
  _unzoomAct->setEnabled(false);
  _updateHeightActionText();
}

/////////////////////////////////////////////////////////////
// set the maximum height of the unzoomed plot

void VertManager::_setMaximumHeight()
{
  bool accepted = false;
  const double value = QInputDialog::getDouble(
    this, tr("Maximum vertical-section height"), tr("Height (km MSL):"),
    _maxHeightKm, _minHeightKm + 0.001, 100000.0, 3, &accepted,
    Qt::WindowFlags(), 0.1);
  if (!accepted) {
    return;
  }

  _maxHeightKm = value;
  _params.vsection_max_ht_km = value;
  _vertView->setHeightRange(_minHeightKm, _maxHeightKm);
  _unzoomAct->setEnabled(false);
  _updateHeightActionText();
}

/////////////////////////////////////////////////////////////
// show the active height limits in the configuration menu

void VertManager::_updateHeightActionText()
{
  _minimumHeightAct->setText(
    tr("Minimum height: %1 km...").arg(_minHeightKm, 0, 'g', 6));
  _maximumHeightAct->setText(
    tr("Maximum height: %1 km...").arg(_maxHeightKm, 0, 'g', 6));
}

/*************************************************************************
 * _createMenus()
 */

void VertManager::_createMenus()
{

  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));

  _overlaysMenu->addAction(_heightGridAct);
  _overlaysMenu->addAction(_distanceGridAct);
  _overlaysMenu->addSeparator();

  _configurationMenu = menuBar()->addMenu(tr("&Configuration"));
  _configurationMenu->addAction(_singleSegmentVsectionAct);
  _configurationMenu->addSeparator();
  _configurationMenu->addAction(_minimumHeightAct);
  _configurationMenu->addAction(_maximumHeightAct);
  _configurationMenu->addSeparator();
  _configurationMenu->addAction(_plotSelectedHorizHeightAct);
  _configurationMenu->addAction(_terrainAct);

  menuBar()->addAction(_unzoomAct);
}

/////////////////////////////////////////////////////////////
// update the displayed vertical-section identity

void VertManager::setSectionInfo(const string &fieldName,
                                 const DateTime &validTime,
                                 double routeLengthKm)
{
  _fieldName = fieldName;
  _validTime = validTime;
  _routeLengthKm = routeLengthKm;
  _vertView->setRouteLengthKm(routeLengthKm);
}

