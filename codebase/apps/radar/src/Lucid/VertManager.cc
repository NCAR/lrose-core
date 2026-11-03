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
        _gd(GlobalData::Instance())
        
{
  // Create the main frame which contains everything in this window

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // Create the parent frame for the VERT rendering

  _vertTopFrame = new QFrame(_main);
  _vertTopFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  // create VERT view
  
  _vertView = new VertView(_vertTopFrame, *manager, *this);
  _vertView->setGrids(_params.vert_grids_on_at_startup);

  // Let Qt propagate the frame's final geometry to the rendering widget.
  // The inherited HawkEye code used a one-time resize workaround, which could
  // leave this child at its small construction size when the window opened.

  QVBoxLayout *vertLayout = new QVBoxLayout(_vertTopFrame);
  vertLayout->setContentsMargins(0, 0, 0, 0);
  vertLayout->addWidget(_vertView);
  
  // Create the status panel
  
  _createStatusPanel(_params.label_font_size);
  
  // Create the main window layout.  We need a layout so the main window can
  // contain multiple views.

  QVBoxLayout *main_layout = new QVBoxLayout(_main);
  main_layout->setContentsMargins(0,0,0,0);
  main_layout->addWidget(_vertTopFrame, 100);
  main_layout->addWidget(_statusPanel, 0);
  
  // Create the actions and the menus

  _createActions(_vertView);
  _createMenus();

  // Set the window attributes.

  setWindowTitle(tr("VERT"));
  setMinimumSize(200, 200);
  resize(_params.vert_plot_width + _params.vert_color_scale_width,
         _params.vert_plot_height);

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

////////////////////////////////////////////////////////////////
void VertManager::closeEvent(QCloseEvent * e)
{
  // pass event up to GuiManager
  _guiManager->vertWindowClosed(e);
}

// Protected methods

/*************************************************************************
 * _createActions()
 */

void VertManager::_createActions(VertView *vert)
{
  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(_params.plot_range_rings_from_data);
  // connect(_ringsAct, SIGNAL(triggered(bool)),
  //         vert, SLOT(setRings(bool)));

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.vert_grids_on_at_startup);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  vert, SLOT(setGrids(bool)));

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params.plot_range_rings_from_data);
  // connect(_azLinesAct, SIGNAL(triggered(bool)),
  //         vert, SLOT(setAngleLines(bool)));

  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  _unzoomAct->setEnabled(false);
  connect(_unzoomAct, SIGNAL(triggered()), this, SLOT(_unzoom()));

}

//////////////////////////////////////////////////
// enable the zoom button - called by VertView

void VertManager::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

////////////////////////////////
// unzoom display

void VertManager::_unzoom()
{
  _vertView->unzoomView();
  _unzoomAct->setEnabled(false);
}

/*************************************************************************
 * _createMenus()
 */

void VertManager::_createMenus()
{
  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addSeparator();

  menuBar()->addAction(_unzoomAct);
}


/*************************************************************************
 * _createStatusPanel()
 */

void VertManager::_createStatusPanel(const int label_font_size)
{
  // Create the status panel
  
  _statusPanel = new QFrame(_main);
  _statusPanel->setFrameShape(QFrame::Box);
  // QHBoxLayout *layout = new QHBoxLayout(_statusPanel);
  QGridLayout *layout = new QGridLayout(_statusPanel);
  layout->setSpacing(0);

  // Create the desired font and alignment
  
  QLabel dummy;
  QFont font = dummy.font();
  int fsize = label_font_size + 2;
  font.setPixelSize(fsize);

  // The HawkEye window reported radar angles here.  For an MDV cross section,
  // field identity and route length are the persistent context the user needs.

  QLabel *elev_label = new QLabel(_statusPanel);
  elev_label->setText("Field ");
  elev_label->setFont(font);
  // layout->addWidget(elev_label, 0, Qt::AlignLeft);
  layout->addWidget(elev_label, 0, 0, Qt::AlignLeft);
  
  _elevValue = new QLabel(_statusPanel);
  _elevValue->setText("-----");
  _elevValue->setFont(font);
  _elevValue->setMinimumWidth(_elevValue->width());
  // layout->addWidget(_elevValue, 0, Qt::AlignRight);
  layout->addWidget(_elevValue, 0, 1, Qt::AlignRight);
  
  QLabel *az_label = new QLabel(_statusPanel);
  az_label->setText("Vsection length ");
  az_label->setFont(font);
  // layout->addWidget(az_label, 0, Qt::AlignLeft);
  layout->addWidget(az_label, 0, 2, Qt::AlignLeft);
  
  _azValue = new QLabel(_statusPanel);
  _azValue->setText("-----");
  _azValue->setFont(font);
  _azValue->setMinimumWidth(_azValue->width());
  // layout->addWidget(_azValue, 0, Qt::AlignRight);
  layout->addWidget(_azValue, 0, 3, Qt::AlignRight);
  
  //layout->addStretch(100);
  layout->setColumnStretch(4, 100);
  
  _statusPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

}

/**
 * @brief Set the azimuth value displayed in the window.
 */

void VertManager::setAzimuth(const double azimuth)
{
  if (_guiManager->getArchiveMode()) {
    _azValue->setText("-----");
    _azValue->setEnabled(false);
  } else {
    char text[1024];
    snprintf(text, 1024, "%6.2f", azimuth);
    _azValue->setText(text);
    _azValue->setEnabled(true);
  }
}

/**
 * @brief Set the elevation value displayed in the window.
 */

void VertManager::setElevation(const double elevation)
{
  if (_guiManager->getArchiveMode()) {
    _elevValue->setText("-----");
    _elevValue->setEnabled(false);
  } else {
    char text[1024];
    snprintf(text, 1024, "%6.2f", elevation);
    _elevValue->setText(text);
    _elevValue->setEnabled(true);
  }
}


/**
 * @brief Set the radar name.  The name is included as part of the window
 *        title.
 */

void VertManager::setRadarName(const string &radar_name)
{
  string window_title = "VERT -- " + radar_name;
  setWindowTitle(tr(window_title.c_str()));
}

/////////////////////////////////////////////////////////////
// update the displayed vertical-section identity

void VertManager::setSectionInfo(const string &fieldName,
                                 const DateTime &validTime,
                                 double routeLengthKm)
{
  _elevValue->setText(QString::fromStdString(fieldName));
  _azValue->setText(QString("%1 km").arg(routeLengthKm, 0, 'f', 1));

  const string title = "Vertical section -- " + fieldName + " -- " +
                       validTime.asString(3);
  setWindowTitle(QString::fromStdString(title));
}

