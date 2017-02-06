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

RhiWindow::RhiWindow(QFrame *rhiParentFrame,
                     PolarManager *manager,
                     const Params &params,
                     const vector<DisplayField *> &fields):
        QMainWindow(rhiParentFrame),
        _rhiParentFrame(rhiParentFrame),
        _manager(manager),
        _params(params),
        _fields(fields)
        
{
  // Create the main frame which contains everything in this window

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // Create the parent frame for the RHI rendering

  _rhiTopFrame = new QFrame(_main);
  _rhiTopFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // create RHI widget

  _rhiWidget = new RhiWidget(_rhiParentFrame, *manager, _params, _fields.size());
  _rhiWidget->setGrids(true);
  _rhiWidget->setRings(false);
  _rhiWidget->setAzLines(false);
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


/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * _createActions()
 */

void RhiWindow::_createActions(RhiWidget *rhi)
{
  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(true);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  rhi, SLOT(setRings(bool)));

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(false);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  rhi, SLOT(setGrids(bool)));

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(true);
  connect(_azLinesAct, SIGNAL(triggered(bool)),
	  rhi, SLOT(setAzLines(bool)));

  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  connect(_unzoomAct, SIGNAL(triggered()), rhi, SLOT(unzoomView()));
}


/*************************************************************************
 * _createMenus()
 */

void RhiWindow::_createMenus()
{
  _viewMenu = menuBar()->addMenu(tr("&View"));
  _viewMenu->addAction(_ringsAct);
  _viewMenu->addAction(_gridsAct);
  _viewMenu->addAction(_azLinesAct);
  _viewMenu->addSeparator();

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
//  QHBoxLayout *layout = new QHBoxLayout(_statusPanel);
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
//  layout->addWidget(elev_label, 0, Qt::AlignLeft);
  layout->addWidget(elev_label, 0, 0, Qt::AlignLeft);
  
  _elevValue = new QLabel(_statusPanel);
  _elevValue->setText("9999.99");
  _elevValue->setFont(font);
  _elevValue->setMinimumWidth(_elevValue->width());
//  layout->addWidget(_elevValue, 0, Qt::AlignRight);
  layout->addWidget(_elevValue, 0, 1, Qt::AlignRight);
  
  // Azimuth

  QLabel *az_label = new QLabel(_statusPanel);
  az_label->setText("Azimuth ");
  az_label->setFont(font);
//  layout->addWidget(az_label, 0, Qt::AlignLeft);
  layout->addWidget(az_label, 0, 2, Qt::AlignLeft);
  
  _azValue = new QLabel(_statusPanel);
  _azValue->setText("9999.99");
  _azValue->setFont(font);
  _azValue->setMinimumWidth(_azValue->width());
//  layout->addWidget(_azValue, 0, Qt::AlignRight);
  layout->addWidget(_azValue, 0, 3, Qt::AlignRight);
  
//  layout->addStretch(100);
  layout->setColumnStretch(4, 100);
  
  _statusPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

}

