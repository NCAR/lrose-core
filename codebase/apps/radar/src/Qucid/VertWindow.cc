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

#include "PolarManager.hh"
#include "RhiWindow.hh"

using namespace std;

/*************************************************************************
 * Constructor
 */

RhiWindow::RhiWindow(PolarManager *manager,
                     const Params &params,
                     const RadxPlatform &platform,
                     const vector<DisplayField *> &fields,
                     bool haveFilteredFields):
        QMainWindow(manager),
        _manager(manager),
        _params(params),
        _platform(platform),
        _fields(fields),
        _haveFilteredFields(haveFilteredFields)
        
{
  // Create the main frame which contains everything in this window

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // Create the parent frame for the RHI rendering

  _rhiTopFrame = new QFrame(_main);
  _rhiTopFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // create RHI widget
  
  _rhiWidget = new RhiWidget(_main, *manager, *this,
                             _params, _platform, _fields, _haveFilteredFields);
  _rhiWidget->setGrids(_params.rhi_grids_on_at_startup);
  _rhiWidget->setRings(_params.rhi_range_rings_on_at_startup);
  _rhiWidget->setAngleLines(_params.rhi_elevation_lines_on_at_startup);
  _rhiWidget->setParent(_rhiTopFrame);
  
  // Connect the window resize signal to the RHI widget resize() method.

  connect(this, SIGNAL(windowResized(const int, const int)),
	  _rhiWidget, SLOT(resize(const int, const int)));
  
  // Connect the first beam recieved widget from the RhiWidget object to the
  // resize() method so that we resize the window after processing several
  // RHI beams.  This is done to get around a window resizing problem at
  // startup.

  connect(_rhiWidget, SIGNAL(severalBeamsProcessed()), this, SLOT(resize()));
  
  // Create the status panel
  
  _createStatusPanel(params.label_font_size);
  
  // Create the main window layout.  We need a layout so the main window can
  // contain multiple widgets.

  QVBoxLayout *main_layout = new QVBoxLayout(_main);
  main_layout->setMargin(0);
  main_layout->addWidget(_rhiTopFrame, 100);
  main_layout->addWidget(_statusPanel, 0);
  
  // Create the actions and the menus

  _createActions(_rhiWidget);
  _createMenus();

  // Set the window attributes.

  setWindowTitle(tr("RHI"));
  setMinimumSize(200, 200);
  setGeometry(params.rhi_window_start_x, params.rhi_window_start_y,
	      params.rhi_window_width, params.rhi_window_height);
}


/*************************************************************************
 * Destructor
 */

RhiWindow::~RhiWindow()
{

}


/*************************************************************************
 * resize()
 */

void RhiWindow::resize()
{
  emit windowResized(_rhiTopFrame->width(), _rhiTopFrame->height());
}


/*************************************************************************
 * resizeEvent()
 */

void RhiWindow::resizeEvent(QResizeEvent *event)
{
  resize();
}


////////////////////////////////////////////////////////////////
void RhiWindow::keyPressEvent(QKeyEvent * e)
{
  // pass event up to PolarManager
  _manager->keyPressEvent(e);
}

// Protected methods

/*************************************************************************
 * _createActions()
 */

void RhiWindow::_createActions(RhiWidget *rhi)
{
  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(_params.rhi_range_rings_on_at_startup);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  rhi, SLOT(setRings(bool)));

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.rhi_grids_on_at_startup);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  rhi, SLOT(setGrids(bool)));

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params.rhi_elevation_lines_on_at_startup);
  connect(_azLinesAct, SIGNAL(triggered(bool)),
	  rhi, SLOT(setAngleLines(bool)));

  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  _unzoomAct->setEnabled(false);
  connect(_unzoomAct, SIGNAL(triggered()), this, SLOT(_unzoom()));

}

//////////////////////////////////////////////////
// enable the zoom button - called by RhiWidget

void RhiWindow::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

////////////////////////////////
// unzoom display

void RhiWindow::_unzoom()
{
  _rhiWidget->unzoomView();
  _unzoomAct->setEnabled(false);
}

/*************************************************************************
 * _createMenus()
 */

void RhiWindow::_createMenus()
{
  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _overlaysMenu->addAction(_ringsAct);
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_azLinesAct);
  _overlaysMenu->addSeparator();

  menuBar()->addAction(_unzoomAct);
}


/*************************************************************************
 * _createStatusPanel()
 */

void RhiWindow::_createStatusPanel(const int label_font_size)
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

  // Elevation

  QLabel *elev_label = new QLabel(_statusPanel);
  elev_label->setText("Elevation ");
  elev_label->setFont(font);
  // layout->addWidget(elev_label, 0, Qt::AlignLeft);
  layout->addWidget(elev_label, 0, 0, Qt::AlignLeft);
  
  _elevValue = new QLabel(_statusPanel);
  _elevValue->setText("9999.99");
  _elevValue->setFont(font);
  _elevValue->setMinimumWidth(_elevValue->width());
  // layout->addWidget(_elevValue, 0, Qt::AlignRight);
  layout->addWidget(_elevValue, 0, 1, Qt::AlignRight);
  
  // Azimuth

  QLabel *az_label = new QLabel(_statusPanel);
  az_label->setText("Azimuth ");
  az_label->setFont(font);
  // layout->addWidget(az_label, 0, Qt::AlignLeft);
  layout->addWidget(az_label, 0, 2, Qt::AlignLeft);
  
  _azValue = new QLabel(_statusPanel);
  _azValue->setText("9999.99");
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

void RhiWindow::setAzimuth(const double azimuth)
{
  if (_manager->checkArchiveMode()) {
    _azValue->setText("-----");
    _azValue->setEnabled(false);
  } else {
    char text[1024];
    sprintf(text, "%6.2f", azimuth);
    _azValue->setText(text);
    _azValue->setEnabled(true);
  }
}

/**
 * @brief Set the elevation value displayed in the window.
 */

void RhiWindow::setElevation(const double elevation)
{
  if (_manager->checkArchiveMode()) {
    _elevValue->setText("-----");
    _elevValue->setEnabled(false);
  } else {
    char text[1024];
    sprintf(text, "%6.2f", elevation);
    _elevValue->setText(text);
    _elevValue->setEnabled(true);
  }
}


/**
 * @brief Set the radar name.  The name is included as part of the window
 *        title.
 */

void RhiWindow::setRadarName(const string &radar_name)
{
  string window_title = "RHI -- " + radar_name;
  setWindowTitle(tr(window_title.c_str()));
}

