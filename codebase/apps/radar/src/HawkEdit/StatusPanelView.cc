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
// StatusPanelView.cc
//
// Status Panel View
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2023
//
///////////////////////////////////////////////////////////////
//
// StatusPanelView manages the display of metadata for the 
// selected (file, sweep, ray, range) = (time, elevation, azimuth, range)
//
///////////////////////////////////////////////////////////////

#include "StatusPanelView.hh"
#include "DisplayField.hh"
#include "FieldListView.hh"
#include "PpiWidget.hh"
#include "RhiWidget.hh"
#include "RhiWindow.hh"
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"
#include "BoundaryPointEditor.hh"

#include <string>
#include <cmath>
#include <iostream>
#include <algorithm>    // std::find
#include <Ncxx/H5x.hh>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDir>
#include <QFrame>
#include <QFont>
#include <QLabel>
#include <QToolTip>
#include <QMenu>
#include <QMenuBar>
#include <QGroupBox>
#include <QMessageBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStringListModel>
#include <QRadioButton>
#include <QStatusBar>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QErrorMessage>
#include <QFileDialog>
#include <QSlider>
#include <QGraphicsScene>
#include <QGraphicsAnchorLayout>
#include <QGraphicsProxyWidget>

#include <cstdlib>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>
#include <Radx/RadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>

#include <toolsa/toolsa_macros.h>
#include <toolsa/Path.hh>

#include "CloseEventFilter.hh"
using namespace std;
using namespace H5x;


StatusPanel* StatusPanel::m_pInstance = NULL;

StatusPanel* StatusPanel::Instance()
{
   return m_pInstance;
}

// Constructor

/*
StatusPanel::StatusPanel(const Params &params,
                           Reader *reader,
                           const vector<DisplayField *> &fields,
                           bool haveFilteredFields) :
        DisplayManager(params, reader, fields, haveFilteredFields), _sweepManager(params), _rhiWindowDisplayed(false)
{
*/

StatusPanel::StatusPanel(DisplayFieldController *displayFieldController,
                           bool haveFilteredFields, bool interactive) :
// DisplayManager(params, reader, displayFieldController, haveFilteredFields), 
        //_sweepManager(params),
        _rhiWindowDisplayed(false),
        QMainWindow(NULL),
        //_params->params),
        //_reader(reader),
        _initialRay(true),
        _displayFieldController(displayFieldController),
        _haveFilteredFields(haveFilteredFields)
{

	m_pInstance = this;

  // initialize

  // from DisplayManager ...
  //_beamTimerId = 0;
  // _frozen = false;
  //  _displayFieldController->setSelectedField(0);
  _prevFieldNum = -1;

  _radarLat = -9999.0;
  _radarLon = -9999.0;
  _radarAltKm = -9999.0;

  _altitudeInFeet = false;
  // end from DisplayManager    

  _firstTime = true;

  // setWindowIcon(QIcon("HawkEyePolarIcon.icns"));
  
  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  //_ppiRays = NULL;
  _rhiMode = false;

  _nGates = 1000;
  _maxRangeKm = 1.0;
  
  //_archiveRetrievalPending = false;
  
  _ppiFrame = NULL;
  _ppi = NULL;

  _rhiWindow = NULL;
  _rhi = NULL;

  _operationMode = INDIVIDUAL;

  //_sweepVBoxLayout = NULL;
  _sweepPanel = NULL;

  //_archiveStartTimeEdit = NULL;
  //_archiveEndTimeEdit = NULL;

  //_selectedTimeLabel = NULL;
  
  //_back1 = NULL;
  //_fwd1 = NULL;
  //_backPeriod = NULL;
  //_fwdPeriod = NULL;

  _timeNavController = NULL;
  _timeNavView = NULL;
  _timeControlPlaced = false;
  //_timeLayout = NULL;
  //_timeSlider = NULL;

  ParamFile *_params = ParamFile::Instance();

  //_setArchiveMode(_params->begin_in_archive_mode);

  if (_timeNavController != NULL) {
    //_timeNavController->setArchiveStartTime(_params->archive_start_time);
    //_timeNavController->setArchiveEndTime(_archiveStartTime + _params->archive_time_span_secs);
    //_timeNavController->setArchiveScanIndex(0);
  }

  _imagesArchiveStartTime.set(_params->images_archive_start_time);
  _imagesArchiveEndTime.set(_params->images_archive_end_time);
  _imagesScanIntervalSecs = _params->images_scan_interval_secs;

  // set up ray locators

// move to RayLocModel
  //_ppiRays = new RayLoc[RayLoc::RAY_LOC_N];
  //_ppiRayLoc = _ppiRays + RayLoc::RAY_LOC_OFFSET;
  _rayLocationController = new RayLocationController();

  sheetView = NULL;
  scriptEditorView = NULL;
  boundaryPointEditorControl = NULL;
  boundaryPointEditorView = NULL;
  boundaryView = NULL;

  // set up windows

  _setupWindows();

  // install event filter to catch when the StatusPanel is closed
  CloseEventFilter *closeFilter = new CloseEventFilter(this);
  installEventFilter(closeFilter);

  setAttribute(Qt::WA_DeleteOnClose);

  _batchEditing = false;

  // set initial field to 0

  //_changeField(0, false);

  connect(this, SIGNAL(readDataFileSignal(vector<string> *)), this, SLOT(inbetweenReadDataFile(vector<string> *)));  
  //connect(this, SIGNAL(fieldSelected(string)), _displayFieldController, SLOT(fieldSelected(string))); 
}

// destructor

StatusPanel::~StatusPanel()
{

  cerr << "StatusPanel destructor called " << endl;
  if (_ppi) {
    delete _ppi;
  }

  if (_rhi) {
    delete _rhi;
  }

  //if (_ppiRays) {
  //  delete[] _ppiRays;
  //}
  // TODO: delete all controllers
  if (_timeNavController) {
    //_timeNavController->~TimeNavController();
    delete _timeNavController;
  }
  if (_undoRedoController) {
    delete _undoRedoController;
  }


/* moved to close()
  if (_ppi) {
    delete _ppi;
  }

  if (_rhi) {
    delete _rhi;
  }

  //if (_ppiRays) {
  //  delete[] _ppiRays;
  //}
  // TODO: delete all controllers
  if (_timeNavController) {
    _timeNavController->~TimeNavController();
    delete _timeNavController;
  }
  if (_timeNavView) delete _timeNavView;
*/
}




//////////////////////////////////////////////////
// Set radar name in title bar

void StatusPanel::_setTitleBar(const string &radarName)
{
  string windowTitle = "HAWK_EDIT -- " + radarName;
  if (_operationMode == BATCH) {
    windowTitle.append(" Batch Mode");
  } else {
    windowTitle.append(" Individual Scan Mode");
  }
  setWindowTitle(tr(windowTitle.c_str()));
}
  
//////////////////////////////////////////////////
// set up windows and widgets 
// make signal and slot connections between controllers
  
void StatusPanel::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  QHBoxLayout *mainLayout = new QHBoxLayout;
  _main->setLayout(mainLayout);
  mainLayout->setSpacing(5);
  mainLayout->setContentsMargins(3,3,3,3);
  setCentralWidget(_main);

  // ppi window

  _ppiFrame = new QFrame(_main);
  _ppiFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _ppiFrame->setLineWidth(3);
  _ppiFrame->setMidLineWidth(3);  
  _ppiFrame->setFrameStyle(QFrame::Box);
  // configure the PPI

 // _ppi = new PpiWidget(_ppiFrame, this, _platform, _displayFieldController, _haveFilteredFields);

  _ppi = new PolarWidget(_ppiFrame, this, _platform, 
    _displayFieldController, _haveFilteredFields,
    _rayLocationController);
  _ppiFrame->setMinimumSize(400,400);
  _ppi->setMinimumSize(400,400);


  connect(this, SIGNAL(frameResized(const int, const int)),
	  _ppi, SLOT(resize(const int, const int)));
  
  // Create the RHI window

  //_rhiWindow = new RhiWindow(this, _params-> _platform,
  //                           _displayFieldController, _haveFilteredFields);
  //_rhiWindow->setRadarName(_params->radar_name);

  // set pointer to the rhiWidget

  //_rhi = _rhiWindow->getWidget();
  
  // connect slots for location

  connect(_ppi, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_ppiLocationClicked(double, double, const RadxRay*)));
  //connect(_rhi, SIGNAL(locationClicked(double, double, const RadxRay*)),
  //        this, SLOT(_rhiLocationClicked(double, double, const RadxRay*)));

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);
  // customContextMenuRequested(e->pos());
  connect(_ppi, SIGNAL(customContextMenuRequested(const QPoint &)),
	  this, SLOT(ShowContextMenu(const QPoint &)));

  // create status panel

  _createStatusPanel();

  // create fields panel
  
  //_createFieldPanel();
  _fieldPanel = new DisplayFieldView(); // _displayFieldController);
  //_displayFieldController->setView(_fieldPanel);
  _fieldPanel->createFieldPanel(_main);
 //TODO: can only connect QObjects with signals and slots...
  //connect(_fieldPanel, SIGNAL(selectedFieldChanged(QString)),
  //  this, SLOT(changeToField(QString)));

  connect(_fieldPanel, SIGNAL(selectedFieldChanged(QString)),
          this, SLOT(selectedFieldChanged(QString)));

  connect(_fieldPanel, SIGNAL(ShowParameterColorDialog(QString)),
    this, SLOT(ShowParameterColorDialog(QString)));  // call DisplayFieldController::deleteFieldFromVolume(DisplayField *field)
 // connect(_fieldPanel, SIGNAL(setFieldToMissing(QString)),
 //   this, SLOT(setFieldToMissing(QString)));


  //connect(this, SIGNAL(addField(QString)), 
  //  _displayFieldController, SLOT(addField(QString)));
  //connect(this, SIGNAL(addField(QString)), 
  //  _ppi, SLOT(addField(QString)));

  // add widgets

  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_fieldPanel); // <=== here 
  mainLayout->addWidget(_ppiFrame);
  _ppiFrame->show();
  _ppi->show();

  // sweep panel

   //_createSweepPanel();
  _sweepPanel = new SweepView(_main);
  _sweepController = new SweepController();
  _sweepController->setView(_sweepPanel);
   mainLayout->addWidget(_sweepPanel);

  connect(_sweepPanel, SIGNAL(selectedSweepChanged(int)),
          this, SLOT(selectedSweepChanged(int)));
  // sweepController does NOT have slots; does NOT derive from QOBJECT
  //connect(this, SIGNAL(newSweepData(int)), _sweepController, SLOT(setSelectedNumber(int sweepNumber)));

  connect(this, SIGNAL(newDataFile()), this, SLOT(dataFileChanged()));
  //connect(this, SIGNAL(sweepSelected()), _sweepController, SLOT(sweepSelected()));

  // fill out menu bar

  _createActions();
  _createMenus();

  // title bar

  _params = ParamFile::Instance();

  _setTitleBar(_params->radar_name);
  setMinimumSize(400, 300);
  resize(1100,635); // _params->main_window_width, _params->main_window_height);
  
  // set location on screen

  QPoint pos;
  pos.setX(_params->main_window_start_x);
  pos.setY(_params->main_window_start_y);
  move(pos);
  
  // set up field status dialog

  _createClickReportDialog();

  //_createBoundaryEditorDialog();

 
   //_setSweepPanelVisibility();

  // time panel

  _createTimeControl();
  _showTimeControl();

  _createUndoRedoStack();
}

//////////////////////////////
// add/remove  sweep panel (archive mode only)

//void StatusPanel::_setSweepPanelVisibility()
//{
//  if (_sweepPanel != NULL) {
//    if (_archiveMode) {
//      _sweepPanel->setVisible(true);
//    } else {
//     _sweepPanel->setVisible(false);
//    }
//  }
//}

//////////////////////////////
// create actions for menus

void StatusPanel::_createActions()
{

  _params = ParamFile::Instance();

  // freeze display
  //_freezeAct = new QAction(tr("Freeze"), this);
  //_freezeAct->setShortcut(tr("Esc"));
  //_freezeAct->setStatusTip(tr("Freeze display"));
  //connect(_freezeAct, SIGNAL(triggered()), this, SLOT(_freeze()));
  
  // show user click in dialog
  _showClickAct = new QAction(tr("Show Click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, SIGNAL(triggered()), this, SLOT(_showClick()));

  // show boundary editor dialog
  _showBoundaryEditorAct = new QAction(tr("Boundary Editor"), this);
  _showBoundaryEditorAct->setStatusTip(tr("Show boundary editor dialog"));
  connect(_showBoundaryEditorAct, SIGNAL(triggered()), this, SLOT(showBoundaryEditor()));
  
  // set time controller settings
  //_timeControllerAct = new QAction(tr("Time-Config"), this);
  //_timeControllerAct->setStatusTip(tr("Show time control window"));
  //connect(_timeControllerAct, SIGNAL(triggered()), this,
  //        SLOT(_showTimeControl()));

  // show time control window
  _showTimeControlAct = new QAction(tr("Show/hide time navigation"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, SIGNAL(triggered()), this,
          SLOT(_showTimeControl()));

  /* realtime mode
  _realtimeAct = new QAction(tr("Set realtime mode"), this);
  _realtimeAct->setStatusTip(tr("Turn realtime mode on/off"));
  _realtimeAct->setCheckable(true);
  _realtimeAct->setChecked(!_params->begin_in_archive_mode);
  connect(_realtimeAct, SIGNAL(triggered(bool)),
	  this, SLOT(_setRealtime(bool)));
  */

  // unzoom display
  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  _unzoomAct->setEnabled(false);
  connect(_unzoomAct, SIGNAL(triggered()), this, SLOT(_unzoom()));

  // refresh display
  _refreshAct = new QAction(tr("Refresh"), this);
  _refreshAct->setStatusTip(tr("Refresh plot"));
  connect(_refreshAct, SIGNAL(triggered()), this, SLOT(_refresh()));

  // clear display
  _clearAct = new QAction(tr("Clear"), this);
  _clearAct->setStatusTip(tr("Clear data"));
  connect(_clearAct, SIGNAL(triggered()), _ppi, SLOT(clear()));
  connect(_clearAct, SIGNAL(triggered()), _rhi, SLOT(clear()));

  // exit app
  _exitAct = new QAction(tr("Q&uit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  // QWidget::close()
  connect(_exitAct, &QAction::triggered, this, &QMainWindow::close); // this, SLOT(close()));


  // file chooser for Open
  _openFileAct = new QAction(tr("O&pen"), this);
  _openFileAct->setShortcut(tr("Ctrl+F"));
  _openFileAct->setStatusTip(tr("Open File"));
  connect(_openFileAct, SIGNAL(triggered()), this, SLOT(_openFile()));

  // file chooser for Save
  _saveFileAct = new QAction(tr("S&ave"), this);
  //_saveFileAct->setShortcut(tr("Ctrl+S"));
  _saveFileAct->setStatusTip(tr("Save File"));
  connect(_saveFileAct, SIGNAL(triggered()), this, SLOT(_saveFile()));

  // show range rings

  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(_params->ppi_range_rings_on_at_startup);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setRings(bool)));

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params->ppi_grids_on_at_startup);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setGrids(bool)));

  // show azimuth lines

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params->ppi_azimuth_lines_on_at_startup);
  connect(_azLinesAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setAngleLines(bool)));

  // show RHI window

  _showRhiAct = new QAction(tr("Show RHI Window"), this);
  _showRhiAct->setStatusTip(tr("Show the RHI Window"));
  connect(_showRhiAct, SIGNAL(triggered()), _rhiWindow, SLOT(show()));

  // howto and about
  
  _howtoAct = new QAction(tr("&Howto"), this);
  _howtoAct->setStatusTip(tr("Show the application's Howto box"));
  connect(_howtoAct, SIGNAL(triggered()), this, SLOT(_howto()));

  _aboutAct = new QAction(tr("&About"), this);
  _aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAct, SIGNAL(triggered()), this, SLOT(_about()));

  _aboutQtAct = new QAction(tr("About &Qt"), this);
  _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // save image
  
  _saveImageAct = new QAction(tr("Save-Image"), this);
  _saveImageAct->setStatusTip(tr("Save image to png file"));
  connect(_saveImageAct, SIGNAL(triggered()), this, SLOT(_saveImageToFile()));

  // Solo Editing
  
  _examineAct = new QAction(tr("Examine SpreadSheet"), this);
  _examineAct->setStatusTip(tr("Solo Examine using a spreadsheet"));
  connect(_examineAct, SIGNAL(triggered()), this, SLOT(_examineSpreadSheetSetup()));

  _editAct = new QAction(tr("Edit Script"), this);
  _editAct->setStatusTip(tr("Solo Edit using (Java)Script"));
  connect(_editAct, SIGNAL(triggered()), this, SLOT(_scriptEditorSetup()));  

  undoAct = new QAction(tr("&Undo"), this);
  undoAct->setStatusTip(tr("undo edits"));
  connect(undoAct, &QAction::triggered, this, &StatusPanel::undoScriptEdits); 

  redoAct = new QAction(tr("&Redo"), this);
  redoAct->setStatusTip(tr("redo edits"));
  connect(redoAct, &QAction::triggered, this, &StatusPanel::redoScriptEdits);
 
  selectBatchModeAct = new QAction(tr("&Batch Mode"), this);
  selectBatchModeAct->setStatusTip(tr("edit all files"));
  connect(selectBatchModeAct, &QAction::triggered, this, &StatusPanel::selectBatchMode);

  selectIndividualModeAct = new QAction(tr("&Individual Scan Mode"), this);
  selectIndividualModeAct->setStatusTip(tr("edit individual scan files"));
  connect(selectIndividualModeAct, &QAction::triggered, this, &StatusPanel::selectIndividualMode);


  //connect(_applyCfacToggle, &QCheckBox::stateChanged, this, &StatusPanel::_applyCfac);

}

////////////////
// create menus

void StatusPanel::_createMenus()
{

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addSeparator();
  _fileMenu->addAction(_openFileAct);
  _fileMenu->addAction(_saveFileAct);
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  _timeMenu = menuBar()->addMenu(tr("&Time"));
  _timeMenu->addAction(_showTimeControlAct);
  _timeMenu->addSeparator();
  //_timeMenu->addAction(_realtimeAct);

  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _overlaysMenu->addAction(_ringsAct);
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_azLinesAct);
  _overlaysMenu->addSeparator();
  _overlaysMenu->addAction(_showRhiAct);

  //menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showClickAct);

  _boundaryMenu = menuBar()->addMenu(tr("&Boundary"));
  _boundaryMenu->addAction(_showBoundaryEditorAct);
  
  menuBar()->addAction(_unzoomAct);
  menuBar()->addAction(_clearAct);

  _editMenu = menuBar()->addMenu(tr("&Edit"));
  _editMenu->addSeparator();
  _editMenu->addAction(_examineAct);
  _editMenu->addAction(_editAct);
  // _editMenu->addSeparator();
  _editMenu->addAction(undoAct);
  _editMenu->addAction(redoAct);

  _modeMenu = menuBar()->addMenu(tr("&Mode"));  
  _modeMenu->addAction(selectBatchModeAct);
  _modeMenu->addAction(selectIndividualModeAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}


///////////////////////////////////////////////////////////////
/* change sweep

void StatusPanel::_changeSweep(bool value) {

  LOG(DEBUG) << "From StatusPanel: the sweep was changed ";

  if (!value) {
    return;
  }

  for (size_t ii = 0; ii < _sweepRButtons->size(); ii++) {
    if (_sweepRButtons->at(ii)->isChecked()) {
      LOG(DEBUG) << "sweepRButton " << ii << " is checked; moving to sweep index " << ii;
      _sweepManager.setGuiIndex(ii);
      _ppi->setStartOfSweep(true);
      //_rhi->setStartOfSweep(true);
      _moveUpDown();
      return;
    }
  } // ii

}
*/

///////////////////////////////////////////////////////////////
// change sweep
// only set the sweepIndex in one place;
// here, just move the radio button up or down one step
// when the radio button is changed, a signal is emitted and
// the slot that receives the signal will increase the sweepIndex
// value = +1 move forward
// value = -1 move backward in sweeps

void StatusPanel::_changeSweepRadioButton(int increment)

{
  LOG(DEBUG) << "-->> changing sweep index by increment: " << increment;
  
  if (increment != 0) {
    _sweepController->changeSelectedIndex(increment);
   // _sweepRButtons->at(_sweepManager.getGuiIndex())->setChecked(true);
  }

}

void StatusPanel::deleteFieldFromVolume(QString fieldName) {
  _displayFieldController->deleteFieldFromVolume(fieldName.toStdString());
}

void StatusPanel::setFieldToMissing(QString fieldName) {
  _displayFieldController->setFieldToMissing(fieldName.toStdString());
}

///////////////////////////////////////
// set input file list for archive mode

void StatusPanel::setArchiveFileList(const vector<string> &list,
                                      bool fromCommandLine // = true 
)
// TODO: don't think fromCommandLine is used???
{

  if (list.size() <= 0) {
    errorMessage("Error", "Empty list of archive files");
    return;
  } 

  //_archiveFileList = list;
  //_setArchiveRetrievalPending();

  if (_timeNavController) {
    // move up two levels in the directory to find the top level
    // maybe like this ...
    //  .../toplevel/yyyymmdd/chosenfile
    QString fullUrl(QString(list.at(0).c_str()));
    QDir theDir(fullUrl);
    if (theDir.cd("../../")) {
      QString thePath = theDir.absolutePath();
      QFileInfo fileInfo(fullUrl);
      QString theFile = fileInfo.fileName();
      _timeNavController->fetchArchiveFiles(thePath.toStdString(),
        theFile.toStdString(), fullUrl.toStdString());
      //_timeNavController->fetchArchiveFiles
    }
  }
}
  
///////////////////////////////////////////////
// get archive file list by searching for files
// returns 0 on success, -1 on failure
/*
int StatusPanel::loadArchiveFileList()

{
  
  RadxTimeList timeList;
  timeList.setDir(_params->archive_data_url);
  timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  timeList.compile();
  _urlOK = true;

  if (timeList.getPathList().size() < 1) {
    cerr << "ERROR - StatusPanel::loadArchiveFileList()" << endl;
    cerr << "  Cannot load file list for url: " 
         << _params->archive_data_url << endl;
    cerr << "  Start time: " << _archiveStartTime.getStr() << endl;
    cerr << "  End time: " << _archiveEndTime.getStr() << endl;
    _urlOK = false;
    return -1;

  }

  setArchiveFileList(timeList.getPathList(), false);
  
  return 0;
}
*/


///////////////////////////////////////
// handle data in archive mode

//void StatusPanel::_handleArchiveData()
//{
/*
    LOG(DEBUG) << "enter";

  //_ppi->setArchiveMode(true);
  //_ppi->setStartOfSweep(true);

  //_rhi->setArchiveMode(true);
  //_rhi->setStartOfSweep(true);

  // set cursor to wait cursor

  this->setCursor(Qt::WaitCursor);
  //_timeNavController->setCursor(Qt::WaitCursor);
  
  // get data
  try {
    _getArchiveData();
    _setupRayLocation();
  } catch (FileIException &ex) {
    this->setCursor(Qt::ArrowCursor);
    //_timeNavController->setCursor(Qt::ArrowCursor);
    return;
  }
  
  //if (_vol.checkIsRhi()) {
  //  _rhiMode = true;
  //} else {
    _rhiMode = false;
  //}

  // plot the data
  
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
  //_timeNavController->setCursor(Qt::ArrowCursor);

  //_activateArchiveRendering();

  if (_firstTime) {
    _firstTime = false;
  }
  
}
*/

/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure
/*
vector<string> *StatusPanel::getFieldsArchiveData(string fileName)
{

  LOG(DEBUG) << "enter";

  DataModel *DataModel::Instance();
  vector<string> *currentVersionFieldNames = getPossibleFieldNames(currentVersionPath);

  // set up file object for reading
  
  RadxFile file;
  RadxVol vol;

  vol.clear();
  //_setupVolRead(file);
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params->debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }
  file.setReadMetadataOnly(true);
  
  //if (_archiveScanIndex >= 0 &&
  //    _archiveScanIndex < (int) _archiveFileList.size()) {
    
    string inputPath = fileName; // _archiveFileList[_archiveScanIndex];
    
  
      LOG(DEBUG) << "  reading data file path: " << inputPath;
      //cerr << "  archive file index: " << _archiveScanIndex << endl;
    
    
    if (file.readFromPath(inputPath, vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "StatusPanel::_getFieldsArchiveData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + inputPath + "\n";
      cerr << errMsg;
      if (!_params->images_auto_create)  {
        QErrorMessage errorDialog;
        errorDialog.setMinimumSize(400, 250);
        errorDialog.showMessage(errMsg.c_str());
        errorDialog.exec();
      }

    } 
    vol.loadFieldsFromRays();
    const vector<RadxField *> fields = vol.getFields();
    vector<string> *allFieldNames = new vector<string>;
    allFieldNames->reserve(fields.size());
    for (vector<RadxField *>::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      RadxField *field = *iter;
      cout << field->getName() << endl;
      allFieldNames->push_back(field->getName());
    }

    LOG(DEBUG) << "exit";
    return allFieldNames;
  //}
}
*/

vector<string> *StatusPanel::userSelectFieldsForReading(string fileName) {

  DataModel *dataModel = DataModel::Instance();
  vector<string> *availableFields = dataModel->getPossibleFieldNames(fileName);
  //vector<string> *availableFields = getFieldsArchiveData(fileName);
  /*
    QStringListModel model; //  = new QStringListModel();
    QStringList list;
    list << "a" << "b" << "c";
    model.setStringList(list);
    QListView theList;
    theList.setModel(&model);
    theList.show();
  */

  FieldListView *listview = new FieldListView(); // this);
  listview->setList(availableFields);

  QGroupBox *viewBox = new QGroupBox(tr("Select fields to import"));
  QDialogButtonBox *buttonBox = new QDialogButtonBox;
  QPushButton *saveButton = buttonBox->addButton(QDialogButtonBox::Apply);
  QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);


  connect(saveButton, SIGNAL(clicked(bool)), listview, SLOT(fieldsSelected(bool)));
  connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancelFieldListDialog(bool)));
  connect(listview, SIGNAL(sendSelectedFieldsForImport(vector<string> *)),
    this, SLOT(fieldsSelected(vector<string> *)));
        //lve->show();
  QVBoxLayout* viewLayout = new QVBoxLayout;
  viewLayout->addWidget(listview);
  viewBox->setLayout(viewLayout);
  viewBox->setMinimumHeight(200);

  QHBoxLayout* horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(buttonBox);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(viewBox);
  mainLayout->addLayout(horizontalLayout);

  fieldListDialog = new QDialog(this);
  fieldListDialog->setModal(true);
  fieldListDialog->setLayout(mainLayout);

  fieldListDialog->exec();  // this halts the app until fields are selected

  return availableFields;

}


// TODO: I don't think this is used
void StatusPanel::getFileAndFields() {
          QString inputPath = "/"; // QDir::currentPath();
        // get the path of the current file, if available 
        //if (_archiveFileList.size() > 0) {
        //  QDir temp(_archiveFileList[0].c_str());
        //  inputPath = temp.absolutePath();
        //} 

        QString filename =  QFileDialog::getOpenFileName(
                NULL,
                "Open Document",
                inputPath, "All files (*);; Cfradial (*.nc)");  //QDir::currentPath(),
        if (filename.isNull() || filename.isEmpty()) {
          return;
        }
        vector<string> fileList;
        fileList.push_back(filename.toStdString());

        // override archive data url from input file
        //string url = _getArchiveUrl(fileList[0]);
        //TDRP_str_replace(&_params->archive_data_url, url.c_str());

        // choose which fields to import

        vector<string> *allFieldNames = userSelectFieldsForReading(fileList[0]);
        for (size_t ii = 0; ii < allFieldNames->size(); ii++) {
          cerr << "ii, allFieldNames[ii]: "
               << ii << ", " << allFieldNames + ii << endl;
        }

        //setArchiveFileList(fileList, false);

}

string StatusPanel::_getSelectedFile() {
      // need to get the current version of the selected file (by index)
      int selectedArchiveFileIndex = 
        _timeNavController->getSelectedArchiveFileIndex();
      string inputPath = 
        _undoRedoController->getCurrentVersion(selectedArchiveFileIndex);
      if (inputPath.empty()) {
        inputPath = _timeNavController->getSelectedArchiveFile();
      } else {
        // add the file name to the version path
        inputPath.append("/");
        string fileName = _timeNavController->getSelectedArchiveFileName();
        inputPath.append(fileName);
      }
      return inputPath;
}

string StatusPanel::_getFileNewVersion(int archiveFileIndex) {
  string nextVersionPath = 
    _undoRedoController->getNewVersion(archiveFileIndex);
  // add the file name to the version path
  nextVersionPath.append("/");
  // get the fileName from the full path
  string fileName = _timeNavController->getArchiveFileName(archiveFileIndex);
  nextVersionPath.append(fileName);

  return nextVersionPath;

}


int StatusPanel::_getArchiveDataPlainVanilla(string &inputPath) {
  cerr << "_getArchiveDataPlainVanilla enter" <<  inputPath << endl;
  LOG(DEBUG) << "enter";
  bool debug_verbose = false;
  bool debug_extra = false;
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    debug_verbose = true;
  }
  if (_params->debug >= Params::DEBUG_EXTRA) {
    debug_verbose = true;
    debug_extra = true;
  }
  DataModel *dataModel = DataModel::Instance();    
  try {
    vector<string> fieldNames = _displayFieldController->getFieldNames();
    int sweep_number = _sweepController->getSelectedNumber();
    dataModel->readData(inputPath, fieldNames, sweep_number,
      debug_verbose, debug_extra);
  } catch (const string &errMsg) {
    // for fatal errors
    if (!_params->images_auto_create)  {
      QErrorMessage errorDialog;
      errorDialog.setMinimumSize(400, 250);
      errorDialog.showMessage(errMsg.c_str());
      errorDialog.exec();
    }
    return -1;
  } catch (std::invalid_argument &ex) {
    // for non-fatal and warning messages
    errorMessage("Warning", ex.what());
  }

  cerr << "_getArchiveDataPlainVanilla exit" << endl;
  LOG(DEBUG) << "exit";
  return 0;
}


/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure
// precondition: assumes setArchiveFileList is called before

int StatusPanel::_getArchiveData()
{
  string inputPath = _getSelectedFile();
  return _getArchiveData(inputPath);
}

int StatusPanel::_getArchiveData(string &inputPath)
{

  int result = _getArchiveDataPlainVanilla(inputPath);
  if (result == 0) {
    _setupRayLocation();
    //emit newDataFile();
    //dataFileChanged();

    // reconcile sweep info; if the sweep angles are the same, then no need for change
    // _sweepController->updateSweepRadioButtons();

    DataModel *dataModel = DataModel::Instance();  

    // set plot times
    
    _plotStartTime = dataModel->getStartTimeSecs();
    _plotEndTime = dataModel->getEndTimeSecs();

    char text[128];
    snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
             _plotStartTime.getYear(),
             _plotStartTime.getMonth(),
             _plotStartTime.getDay(),
             _plotStartTime.getHour(),
             _plotStartTime.getMin(),
             _plotStartTime.getSec());

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << dataModel->getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << dataModel->getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";
  
  
    _platform = dataModel->getPlatform();
    LOG(DEBUG) << "exit";
    return 0;
  } else {
    LOG(DEBUG) << "exit";
    return -1;
  }

}

/*
double StatusPanel::getSelectedSweepAngle() {
  double angle = _sweepController->getSelectedAngle();
  return angle;
}
*/

size_t StatusPanel::getSelectedFieldIndex() {
  size_t selectedFieldIndex = 0;
  try {
    selectedFieldIndex = _displayFieldController->getSelectedFieldNum();
  } catch (std::range_error &ex) {
      LOG(ERROR) << ex.what();
      QMessageBox::warning(NULL, "Error selectingField (_getArchiveData):", ex.what());
      return 0;
  }
  return selectedFieldIndex;
}




/////////////////////////////
// apply new, edited  data in archive mode
// the volume has been updated 



void StatusPanel::_applyDataEdits()
{
  LOG(DEBUG) << "enter";
  _setupRayLocation();
  _plotArchiveData();
  LOG(DEBUG) << "exit";
}


/////////////////////////////////////
// activate archive rendering

void StatusPanel::_addNewFields(QStringList  newFieldNames)
{
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "all fields in _vol ... ";
  //vector<RadxField *> allFields = _vol.getFields();
  //vector<RadxField *>::iterator it;
  //for (it = allFields.begin(); it != allFields.end(); it++) {
  //  RadxField *radxField = *it;
  //  LOG(DEBUG) << radxField->getName();
  //}


  // TODO: 
  // inheret the color map, units, etc. from the similar field
  // Do this in the ScriptEditor, when the new field is added to RadxVol

  // make new DisplayFields for PpiWidget
  // -----

  vector<DisplayField *> newFields;

  LOG(DEBUG) << "newFieldNames ...";
  //  newFieldNames.split(',');
  for (int i=0; i < newFieldNames.size(); ++i) {
    string name = newFieldNames.at(i).toLocal8Bit().constData();

    LOG(DEBUG) << name;
 
    ColorMap map(-20.0, 20.0, "default");

     // new DisplayField(pfld.label, pfld.raw_name, pfld.units,
      //		       pfld.shortcut, map, ifield, false);
    int buttonRow = _displayFieldController->getNFields() + 1;
    DisplayField *field =
      new DisplayField(name, name, "m/s",
		       "-1", map, buttonRow, false);
    //if (noColorMap)
    field->setNoColorMap();

    newFields.push_back(field);
    _displayFieldController->addField(field);

    // filtered field                                                                                    
    
    /*
      if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
      new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut,
      map, ifield, true);
      newFields.push_back(filt);
      }
    */
    // -----------
  }   
  
  if (_ppi) {
    _ppi->addNewFields(newFields);
  }
  
  //if (_rhi) {
  //  _rhi->addNewFields(newFields);
  //}
  LOG(DEBUG) << "exit";
}


void StatusPanel::_addNewFields(vector<string> *newFieldNames)
{
  LOG(DEBUG) << "enter";
  //LOG(DEBUG) << "all fields in _vol ... ";
  //vector<RadxField *> allFields = _vol.getFields();
  //vector<RadxField *>::iterator it;
  //for (it = allFields.begin(); it != allFields.end(); it++) {
  //  RadxField *radxField = *it;
  //  LOG(DEBUG) << radxField->getName();
  //}


  // TODO: 
  // inheret the color map, units, etc. from the similar field
  // Do this in the ScriptEditor, when the new field is added to RadxVol

  // make new DisplayFields for PpiWidget
  // -----

  vector<DisplayField *> newFields;

  LOG(DEBUG) << "newFieldNames ...";
  //  newFieldNames.split(',');
  for (int i=0; i < newFieldNames->size(); ++i) {
    string name = newFieldNames->at(i); // .toLocal8Bit().constData();

    LOG(DEBUG) << name;
 
    ColorMap map(-20.0, 20.0, "default");

     // new DisplayField(pfld.label, pfld.raw_name, pfld.units,
      //           pfld.shortcut, map, ifield, false);
    int buttonRow = _displayFieldController->getNFields() + 1;
    DisplayField *field =
      new DisplayField(name, name, "m/s",
           "-1", map, buttonRow, false);
    //if (noColorMap)
    field->setNoColorMap();

    newFields.push_back(field);
    _displayFieldController->addField(field);

    // filtered field                                                                                    
    
    /*
      if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
      new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut,
      map, ifield, true);
      newFields.push_back(filt);
      }
    */
    // -----------
  }   
  
  if (_ppi) {
    _ppi->addNewFields(newFields);
  }
  
  //if (_rhi) {
  //  _rhi->addNewFields(newFields);
  //}
  LOG(DEBUG) << "exit";
}

/////////////////////////////
// add results of scripts to display; new, edited  data in archive mode
// the volume has been updated 

// The new field names have been added to the RadxVol,
// pull them out and create DisplayFields for them???
// 
void StatusPanel::_volumeDataChanged(QStringList newFieldNames)
{
  LOG(DEBUG) << "enter";
  // the in memory RadxVol has changed; 
  // new fields have been added or deleted

  /* NOT THIS ...
  //  load the sweep manager
    _sweepManager.set(_vol);
  */

  //TODO: need to get a list of new Fields, somehow ...
  // could be as a list of DisplayFields? 

  //vector<DisplayField *> newFields;
  

  _addNewFields(newFieldNames);
  //if (newFieldNames.size() > 0)
  //   _updateFieldPanel(newFieldNames[0].toStdString());
  _fieldPanel->update();

  // _applyDataEdits();
  //_activateArchiveRendering();
  // _plotArchiveData();
  // TODO: create this ... from plotArchiveData()
  //_updateArchiveData(newFieldNames); 
  //_activateArchiveRendering();

  LOG(DEBUG) << "exit"; 
}


void StatusPanel::selectedFieldChanged(QString newFieldName) {
  selectedFieldChanged(newFieldName.toStdString());
}

void StatusPanel::selectedFieldChanged(string fieldName) {
  _displayFieldController->setSelectedField(fieldName);
  refreshBoundaries();
  _plotArchiveData();
}


void StatusPanel::selectedSweepChanged(int sweepNumber) {
  LOG(DEBUG) << "enter"; 
  //string fieldName = newFieldName.toStdString();
  //_displayFieldController->setSelectedField(fieldName);
  if (_sweepController->getSelectedNumber() != sweepNumber) {
    _sweepController->setSelectedNumber(sweepNumber);
    _readDataFile();
    // signal polar display to update; which causes rayLocations to update
    selectedFieldChanged(_displayFieldController->getSelectedFieldName());
    emit newSweepData(sweepNumber);
    //processEvents();
    //_setupRayLocation();  // this is done by _getArchiveData
    //_plotArchiveData();
    //refreshBoundaries();
  } else {
    if (sheetView != NULL) {
      spreadSheetControl->displaySweepData(sweepNumber);
    }
  }
  LOG(DEBUG) << "exit";
}


void StatusPanel::dataFileChanged() {

 // _sweepController->clearSweepRadioButtons();
  //_sweepController->updateSweepRadioButtons();

  if (sheetView != NULL) {
    sheetView->applyEdits();
  }
  //_plotArchiveData();

}

////////////////////////////////////////////
// refresh

void StatusPanel::_refresh()
{
}

///////////////////////////////////////////////////////////
// respond to change field request from field button group

//void StatusPanel::_changeField(string fieldName, bool guiMode)
void StatusPanel::changeToField(QString newFieldName)
{  
  LOG(DEBUG) << "enter";

  // convert fieldId to field name (from the tool tip) because not all fields are displayed,
  // so we cannot rely on the id/index.
  //QString fieldNameQt = _fieldButtons.at(fieldId)->toolTip();
  //string fieldName = fieldNameQt.toStdString();


  //size_t newSelectionNum = _displayFieldController->getFieldIndex(fieldName);
/*
  // removing this: if we click the already-selected field, go back to previous field
  // 
  try {
    size_t ;
      LOG(DEBUG) << "fieldName is " << fieldName;

    if (guiMode) {
      if (fieldNum == newSelectionNum && _prevFieldNum >= 0) {
        QRadioButton *button =
          (QRadioButton *) _fieldGroup->button(_prevFieldNum);
        button->click();
        return;
      }
    }
*/   
  
  //_prevFieldNum = fieldNum;
  //fieldNum = newSelectionNum;
  string fieldName = newFieldName.toStdString();
  //_displayFieldController->setSelectedField(fieldName);
  // I don't think this does anything ... _ppi->selectVar(fieldName);
  _plotArchiveData();  // either one of these, not both?
  //_rhi->selectVar(fieldNum);  TODO: reinstate this 
// TODO: update the statusPanel
//  _statusPanel->???

  //_selectedName = _selectedField->getName();
  //_selectedLabel = _selectedField->getLabel();
  //_selectedUnits = _selectedField->getUnits();
  
  //_selectedLabelWidget->setText(_selectedLabel.c_str());
  //char text[128];
  //if (_selectedField->getSelectValue() > -9990) {
  //  sprintf(text, "%g %s", 
  //          _selectedField->getSelectValue(),
  //          _selectedField->getUnits().c_str());
  //} else {
  //  text[0] = '\0';
  //}
  //_valueLabel->setText(text);
  //} catch (std::range_error &ex) {
  //    LOG(ERROR) << ex.what();
  //    QMessageBox::warning(NULL, "Error changing field (_changeField):", ex.what());
  //}
  LOG(DEBUG) << "exit";
}





///////////////////////////////////////////////////
// respond to a change in click location on the PPI

void StatusPanel::_ppiLocationClicked(double xkm, double ykm, 
                                       const RadxRay *closestRay)

{

  // find the relevant ray
  // ignore closest ray sent in
  
  double azDeg = 0.0;
  if (xkm != 0 || ykm != 0) {
    azDeg = atan2(xkm, ykm) * RAD_TO_DEG;
    if (azDeg < 0) {
      azDeg += 360.0;
    }
  }
  LOG(DEBUG) << "    azDeg = " << azDeg;
  
  double range = 0.0;
  if (xkm != 0 || ykm != 0) {
    range = sqrt(xkm * xkm + ykm * ykm);
  }
  LOG(DEBUG) << "    range = " << range;
  
  //int rayIndex = (int) (azDeg * RayLoc::RAY_LOC_RES);

  //  LOG(DEBUG) << "    rayIndex = " << rayIndex;
  
  // first right of refusal is the boundary editor, if a polygon is not complete
  // TODO:  moved to BPE
    //BoundaryPointEditor *editor = boundaryPointEditorControl;
    //int _worldReleaseX = mouseReleaseX;
    //int _worldReleaseY = mouseReleaseY;
  bool isUseful = false;
  if (boundaryPointEditorControl != NULL) {
    isUseful = boundaryPointEditorControl->evaluatePoint((int) xkm, (int) ykm);
  }

  /*
      if (editor->getCurrentTool() == BoundaryToolType::polygon) {
        if (!editor->isAClosedPolygon()) {
          editor->addPoint(_worldReleaseX, _worldReleaseY);
        } else { //polygon finished, user may want to insert/delete a point
          editor->checkToAddOrDelPoint(_worldReleaseX,
                                       _worldReleaseY);
        }
      } else if (editor->getCurrentTool() == BoundaryToolType::circle) {
        if (editor->isAClosedPolygon()) {
          editor->checkToAddOrDelPoint(_worldReleaseX,
                                       _worldReleaseY);
        } else {
          editor->makeCircle(_worldReleaseX,
                             _worldReleaseY,
                             editor->getCircleRadius());
        }
      }  
  */

  // second choice is the spreadsheet editor
  if (!isUseful) {
    // update the spreadsheet if it is active
    if (sheetView != NULL) {
      _examineSpreadSheetSetup(azDeg, range);
    }
  }

  // last chance is the lefthand status panel
  const RadxRay *ray = _rayLocationController->getClosestRay(azDeg);
  //const RadxRay *ray = _ppiRayLoc[rayIndex].ray;
  if (ray == NULL) {
    // no ray data yet

      LOG(DEBUG) << "    No ray data yet...";
      errorMessage("Error", "No ray data found at " + to_string(azDeg));
      //LOG(DEBUG) << "      active = " << _ppiRayLoc[rayIndex].active;
      // cerr << "      master = " << _ppiRayLoc[rayIndex].master << endl;
      //LOG(DEBUG) << "      startIndex = " << _ppiRayLoc[rayIndex].startIndex;
      //LOG(DEBUG) << "      endIndex = " << _ppiRayLoc[rayIndex].endIndex;
    
    return;
  }

  // notify the status display area to update
  _locationClicked(xkm, ykm, ray);  

}


////////////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void StatusPanel::_locationClicked(double xkm, double ykm,
                                    const RadxRay *ray)
{

  LOG(DEBUG) << "*** Entering StatusPanel::_locationClicked()";
  
  double range = sqrt(xkm * xkm + ykm * ykm);
  int gate = (int) 
    ((range - ray->getStartRangeKm()) / ray->getGateSpacingKm() + 0.5);

  if (gate < 0 || gate >= (int) ray->getNGates())
  {
    //user clicked outside of ray
    return;
  }

  if (_params->debug) {
    LOG(DEBUG) << "Clicked on location: xkm, ykm: " << xkm << ", " << ykm;
    LOG(DEBUG) << "  range, gate: " << range << ", " << gate;
    LOG(DEBUG) << "  az, el from ray: "
         << ray->getAzimuthDeg() << ", "
         << ray->getElevationDeg();
  }

  DateTime rayTime(ray->getTimeSecs());
  char text[256];
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateClicked->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) (ray->getNanoSecs() / 1000000)));
  _timeClicked->setText(text);
  
  _setText(text, "%6.2f", ray->getElevationDeg());
  _elevClicked->setText(text);
  
  _setText(text, "%6.2f", ray->getAzimuthDeg());
  _azClicked->setText(text);
  
  _setText(text, "%d", gate);
  _gateNumClicked->setText(text);
  
  _setText(text, "%6.2f", range);
  _rangeClicked->setText(text);

  _displayFieldController->setForLocationClicked(-9999.0, "----");  
  
  vector<RadxField *> flds = ray->getFields();
  for (size_t ifield = 0; ifield < flds.size(); ifield++) {
    const RadxField *field = flds[ifield];

    const string fieldName = field->getName();
    if (fieldName.size() == 0) {
      continue;
    }
    Radx::fl32 *data = (Radx::fl32 *) field->getData();
    double val = data[gate];
    const string fieldUnits = field->getUnits();

    LOG(DEBUG) << "Field name, selected name: "
	   << fieldName << ", "
	   << _selectedName;
    
    if (fieldName == _selectedName) {
      char text[128];
      if (fabs(val) < 10000) {
        sprintf(text, "%g %s", val, fieldUnits.c_str());
      } else {
        sprintf(text, "%g %s", -9999.0, fieldUnits.c_str());
      }
      _valueLabel->setText(text);
    }

    LOG(DEBUG) << "Field name, units, val: "
	   << field->getName() << ", "
	   << field->getUnits() << ", "
	   << val;
    
    char text[128];
    if (fabs(val) > 10000) {
      sprintf(text, "----");
    } else if (fabs(val) > 10) {
      sprintf(text, "%.2f", val);
    } else {
      sprintf(text, "%.3f", val);
    }

    _displayFieldController->setForLocationClicked(fieldName, val, text);

  } // ifield

  // update the status panel
  
  _updateStatusPanel(ray);
    
}

void StatusPanel::_createUndoRedoStack() {
  _undoRedoController = new UndoRedoController();
}

//////////////////////////////////////////////
// create the time navigation controller and view

void StatusPanel::_createTimeControl()
{
  _timeNavView = new TimeNavView(this);
  _timeNavController = new TimeNavController(_timeNavView);
  
  // connect slots for time slider

  connect(_timeNavView, &TimeNavView::newStartEndTime,
          this, &StatusPanel::startEndTimeChanged);
  
  connect(_timeNavView, &TimeNavView::resetStartEndTime,
          this, &StatusPanel::resetStartEndTime);
  
  connect(_timeNavView, &TimeNavView::newTimeIndexSelected,
          this, &StatusPanel::newTimeSelected);

//  connect(_timeNavView, SIGNAL(sliderPressed()),
//          this, SLOT(_timeSliderPressed()));

}






void StatusPanel::_notifyDataModelNewFieldsSelected() {
  // data model does not keep track of which fields are selected;
  // The DisplayField MVC keeps track of the selected fields.
  DataModel *dataModel = DataModel::Instance();
  dataModel->clearVolume();
  dataModel->moveToLookAhead();

}


void StatusPanel::fieldsSelected(vector<string> *selectedFields) {

// TODO:
//  delete availableFields;
  LOG(DEBUG) << "enter";

  if (selectedFields->size() > 0) {
    //QStringList qselectedFields;
    LOG(DEBUG) << selectedFields->size() << " selected";
    for (vector<string>::iterator it=selectedFields->begin(); it != selectedFields->end(); ++it) {
      LOG(DEBUG) << *it;
      //qselectedFields.push_back(QString::fromStdString(*it));
      //_displayFieldController->addField(*it);
      //emit addField(*it);
    }
    _notifyDataModelNewFieldsSelected();
    _addNewFields(selectedFields);
    // reconcile sweep info; if the sweep angles are the same, then no need for change
    //string inputPath = _getSelectedFile();
    //_sweepController->updateSweepRadioButtons(inputPath);  
    //  If volume is empty, then just read meta data and select first sweep number
    // give the selected fields to the volume read ...
    _sweepController->updateSweepRadioButtons();
    // trigger a read of ray data
    emit readDataFileSignal(selectedFields);
    // FIX HERE !!! these should happen AFTER the data file is successfully read

    //selectedSweepChanged(_sweepController->getSelectedNumber());
    //_updateDisplayFields(selectedFields);
    //selectedFieldChanged(_displayFieldController->getSelectedFieldName());
  }  
  // close the modal dialog box for field selection
  closeFieldListDialog(true);
  //fieldListDialog->close();
  LOG(DEBUG) << "exit";
}


// no prompting, and use a progress bar ...
void StatusPanel::_goHere(int nFiles, string saveDirName) {

    QProgressDialog progress("Saving files...", "Cancel", 0, nFiles, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoReset(false); 
    progress.setAutoClose(false);
    progress.setValue(1);
    progress.show();

    //progress.exec();
    
    // for each file in timeNav ...
    int i=0;
    bool cancel = false;

    while (i<nFiles && !cancel) {
        
        if (progress.wasCanceled()) {
          cancel = true;
          break;          
        }

        try {
          // the active one in memory (i.e. the selected file)
          // may have unsaved changes, so treat it differently.
          if (i == _timeNavController->getSelectedArchiveFileIndex()) {
            DataModel *dataModel = DataModel::Instance();
            string currentFile = _timeNavController->getSelectedArchiveFile();
            string saveFile = _combinePathFile(saveDirName, _fileName(QString(currentFile.c_str())));
            dataModel->writeWithMergeData(saveFile, currentFile);
          }  else {

            string versionName = _undoRedoController->getCurrentVersion(i);
            string realName = _timeNavController->getArchiveFilePath(i);
            // versionName is something like v1, or v2, etc.
            // realName is something like cfrad_yyyymmdd...
            string justTheFileName = _fileName(QString(realName.c_str()));
            string sourcePathFile;
            if (versionName.empty()) {
              // this is the original version; and the name is NOT v1, v2, etc.
              //versionName = _timeNavController->getArchiveFilePath(i);
              sourcePathFile = realName;
              // TODO: just copy the file to the destination
            } else {
              sourcePathFile = _combinePathFile(versionName, justTheFileName);
            }
            const char *source_path = sourcePathFile.c_str();
            string savePathFile = _combinePathFile(saveDirName, justTheFileName);
            const char *dest_path = savePathFile.c_str();

            cout << "source_path = " << source_path << endl;
            cout << "dest_path = " << dest_path << endl;

            // use toolsa utility
            // Returns -1 on error, 0 otherwise
            // filecopy_by_name doesnn't work;
            // also, need to merge the files with the original file,
            // since we have saved a delta ...
            string originalPath = _timeNavController->getArchiveFilePath(i);

            DataModel *dataModel = DataModel::Instance();
            int return_val = dataModel->mergeDataFiles(dest_path, source_path, originalPath);
            if (return_val != 0) {
              stringstream ss;
              ss << "could not save file: " << dest_path;
              errorMessage("Error", ss.str());
            }

          }
        } catch (FileIException &ex) {
          this->setCursor(Qt::ArrowCursor);
          return;
        }
        i+= 1;
        progress.setValue(i);
        stringstream m;
        m << i+1 << " of " << nFiles;
        QString QStr = QString::fromStdString(m.str());
        progress.setLabelText(QStr);
        QCoreApplication::processEvents();
    } // end while loop each file in timeNav and !cancel
    if (cancel) {
      progress.setLabelText("cancelling ...");
      QCoreApplication::processEvents();
    }
}


//////////////////////////////////////////////
// create the status panel

void StatusPanel::_createStatusPanel()
{
 
  Qt::Alignment alignLeft(Qt::AlignLeft);
  Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignTop(Qt::AlignTop);

  // status panel - rows of label value pairs
  
  _statusPanel = new QGroupBox(_main);
  _statusLayout = new QGridLayout(_statusPanel);
  _statusLayout->setVerticalSpacing(5);

  int row = 0;
  
  ParamFile *_params = ParamFile::Instance();
  // fonts
  
  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font6 = dummy.font();
  int fsize = _params->label_font_size;
  int fsize2 = _params->label_font_size; //  + 2;
  int fsize6 = _params->label_font_size; //  + 6;
  font.setPixelSize(fsize);
  font2.setPixelSize(fsize2);
  font6.setPixelSize(fsize6);

  // radar and site name
  
  _radarName = new QLabel(_statusPanel);
  string rname(_params->radar_name);
  if (_params->display_site_name) {
    rname += ":";
    rname += _params->site_name;
  }
  _radarName->setText(rname.c_str());
  _radarName->setFont(font6);
  _statusLayout->addWidget(_radarName, row, 0, 1, 4, alignCenter);
  row++;

  // date and time

  _dateVal = new QLabel("9999/99/99", _statusPanel);
  _dateVal->setFont(font2);
  _statusLayout->addWidget(_dateVal, row, 0, 1, 2, alignCenter);
  row++;

  _timeVal = new QLabel("99:99:99.999", _statusPanel);
  _timeVal->setFont(font2);
  _statusLayout->addWidget(_timeVal, row, 0, 1, 2, alignCenter);
  row++;


  // other labels.  Note that we set the minimum size of the column
  // containing the right hand labels in timerEvent() to prevent the
  // wiggling we were seeing in certain circumstances.  For this to work,
  // the default values for these fields must represent the maximum digits
  // posible for each field.

  _elevVal = _createStatusVal("Elev", "-99.99", row++, fsize2);
  _azVal = _createStatusVal("Az", "-999.99", row++, fsize2);

  if (_params->show_status_in_gui.fixed_angle) {
    _fixedAngVal = _createStatusVal("Fixed ang", "-99.99", row++, fsize2);
  } else {
    _fixedAngVal = NULL;
  }
  
  if (_params->show_status_in_gui.volume_number) {
    _volNumVal = _createStatusVal("Volume", "0", row++, fsize);
  } else {
    _volNumVal = NULL;
  }
  
  if (_params->show_status_in_gui.sweep_number) {
    _sweepNumVal = _createStatusVal("Sweep", "0", row++, fsize);
  } else {
    _sweepNumVal = NULL;
  }

  if (_params->show_status_in_gui.n_samples) {
    _nSamplesVal = _createStatusVal("N samp", "0", row++, fsize);
  } else {
    _nSamplesVal = NULL;
  }

  if (_params->show_status_in_gui.n_gates) {
    _nGatesVal = _createStatusVal("N gates", "0", row++, fsize);
  } else {
    _nGatesVal = NULL;
  }

  if (_params->show_status_in_gui.gate_length) {
    _gateSpacingVal = _createStatusVal("Gate len", "0", row++, fsize);
  } else {
    _gateSpacingVal = NULL;
  }
  
  if (_params->show_status_in_gui.pulse_width) {
    _pulseWidthVal = _createStatusVal("Pulse width", "-9999", row++, fsize);
  } else {
    _pulseWidthVal = NULL;
  }

  if (_params->show_status_in_gui.prf_mode) {
    _prfModeVal = _createStatusVal("PRF mode", "Fixed", row++, fsize);
  } else {
    _prfModeVal = NULL;
  }

  if (_params->show_status_in_gui.prf) {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
  } else {
    _prfVal = NULL;
  }

  if (_params->show_status_in_gui.nyquist) {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
  } else {
    _nyquistVal = NULL;
  }

  if (_params->show_status_in_gui.max_range) {
    _maxRangeVal = _createStatusVal("Max range", "-9999", row++, fsize);
  } else {
    _maxRangeVal = NULL;
  }

  if (_params->show_status_in_gui.unambiguous_range) {
    _unambigRangeVal = _createStatusVal("U-A range", "-9999", row++, fsize);
  } else {
    _unambigRangeVal = NULL;
  }

  if (_params->show_status_in_gui.measured_power_h) {
    _powerHVal = _createStatusVal("Power H", "-9999", row++, fsize);
  } else {
    _powerHVal = NULL;
  }

  if (_params->show_status_in_gui.measured_power_v) {
    _powerVVal = _createStatusVal("Power V", "-9999", row++, fsize);
  } else {
    _powerVVal = NULL;
  }

  if (_params->show_status_in_gui.scan_name) {
    _scanNameVal = _createStatusVal("Scan name", "unknown", row++, fsize);
  } else {
    _scanNameVal = NULL;
  }

  if (_params->show_status_in_gui.scan_mode) {
    _sweepModeVal = _createStatusVal("Scan mode", "SUR", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_params->show_status_in_gui.polarization_mode) {
    _polModeVal = _createStatusVal("Pol mode", "Single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_params->show_status_in_gui.latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_params->show_status_in_gui.longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_params->show_status_in_gui.altitude) {
    if (_altitudeInFeet) {
      _altVal = _createStatusVal("Alt(kft)", "-999.999",
                                 row++, fsize, &_altLabel);
    } else {
      _altVal = _createStatusVal("Alt(km)", "-999.999",
                                 row++, fsize, &_altLabel);
    }
  } else {
    _altVal = NULL;
  }

  if (_params->show_status_in_gui.altitude_rate) {
    if (_altitudeInFeet) {
      _altRateVal = _createStatusVal("AltRate(ft/s)", "-999.999",
                                     row++, fsize, &_altRateLabel);
    } else {
      _altRateVal = _createStatusVal("AltRate(m/s)", "-999.999",
                                     row++, fsize, &_altRateLabel);
    }
  } else {
    _altRateVal = NULL;
  }

  if (_params->show_status_in_gui.speed) {
    _speedVal = _createStatusVal("Speed(m/s)", "-999.99", row++, fsize);
  } else {
    _speedVal = NULL;
  }

  if (_params->show_status_in_gui.heading) {
    _headingVal = _createStatusVal("Heading(deg)", "-999.99", row++, fsize);
  } else {
    _headingVal = NULL;
  }

  if (_params->show_status_in_gui.track) {
    _trackVal = _createStatusVal("Track(deg)", "-999.99", row++, fsize);
  } else {
    _trackVal = NULL;
  }

  if (_params->show_status_in_gui.sun_elevation) {
    _sunElVal = _createStatusVal("Sun el (deg)", "-999.999", row++, fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_params->show_status_in_gui.sun_azimuth) {
    _sunAzVal = _createStatusVal("Sun az (deg)", "-999.999", row++, fsize);
  } else {
    _sunAzVal = NULL;
  }

  _georefsApplied = _createStatusVal("Georefs applied?", "T/F", row++, fsize,
    &_georefsAppliedLabel);
  _geoRefRotationVal = _createStatusVal("Georef Rot (deg)", "0.0", row++, fsize,
    &_geoRefRotationLabel);
  _geoRefRollVal = _createStatusVal("Georef Roll (deg)", "0.0", row++, fsize,
    &_geoRefRollLabel);
  _geoRefTiltVal = _createStatusVal("Georef Tilt (deg)", "0.0", row++, fsize,
    &_geoRefTiltLabel);
  _geoRefTrackRelRotationVal = _createStatusVal("Track Rel Rot (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelRotationLabel);
  _geoRefTrackRelTiltVal = _createStatusVal("Track Rel  Tilt (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelTiltLabel);
  _geoRefTrackRelAzVal = _createStatusVal("Track Rel  Az (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelAzLabel);
  _geoRefTrackRelElVal = _createStatusVal("Track Rel  El (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelElLabel);
  _cfacRotationVal = _createStatusVal("Cfac Rot (deg)", "0.0", row++, fsize,
    &_cfacRotationLabel);
  _cfacRollVal = _createStatusVal("Cfac Roll (deg)", "", row++, fsize,
    &_cfacRollLabel);
  _cfacTiltVal = _createStatusVal("Cfac Tilt (deg)", "", row++, fsize,
    &_cfacTiltLabel);
                            
  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

  hideCfacs(); 

}



///////////////////////////////////////////////////////
// create the click report dialog
//
// This shows the field values at the latest click point

void StatusPanel::_createClickReportDialog()
{
  
  _clickReportDialog = new QDialog(this);
  _clickReportDialog->setMinimumSize(100, 100);
  _clickReportDialog->setWindowTitle("Field values");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  _clickReportDialogLayout = new QGridLayout(_clickReportDialog);
  _clickReportDialogLayout->setVerticalSpacing(5);

  int row = 0;
  QLabel *mainHeader = new QLabel("CLICK POINT DATA", _clickReportDialog);
  _clickReportDialogLayout->addWidget(mainHeader, row, 0, 1, 3, alignCenter);
  row++;

  // _clickReportDialogLayout->addWidget(left, row, 0, alignRight);

  _dateClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Date", "9999/99/99", row++);
  _timeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Time", "99:99:99.999", row++);
  _elevClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Elevation", "-9999", row++);
  _azClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                            "Azimuth", "-9999", row++);
  _gateNumClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                                 "Gate num", "-9999", row++);
  _rangeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                               "Range", "-9999", row++);
  _altitudeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                                  "Altitude", "-9999", row++);

  QLabel *valHeader = new QLabel("FIELD VALUES", _clickReportDialog);
  _clickReportDialogLayout->addWidget(valHeader, row, 0, 1, 3, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _clickReportDialog);
  _clickReportDialogLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *rawHeader = new QLabel("Raw", _clickReportDialog);
  _clickReportDialogLayout->addWidget(rawHeader, row, 1, alignCenter);
  if (_haveFilteredFields) {
    QLabel *filtHeader = new QLabel("Filt", _clickReportDialog);
    _clickReportDialogLayout->addWidget(filtHeader, row, 2, alignCenter);
  }
  row++;

  // add fields, one row at a time
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present
  size_t nFields = _displayFieldController->getNFields();
  for (size_t ifield = 0; ifield < nFields; ifield++) {

    // get raw field - always present
    
    DisplayField *rawField = _displayFieldController->getField(ifield); // _fields[ifield];
    int buttonRow = rawField->getButtonRow();
    
    // get filt field - may not be present
    DisplayField *filtField = _displayFieldController->getFiltered(ifield, buttonRow);

    QLabel *label = new QLabel(_clickReportDialog);
    label->setText(rawField->getLabel().c_str());
    _clickReportDialogLayout->addWidget(label, row, 0, alignCenter);
    
    rawField->createDialog(_clickReportDialog, "-------------");
    _clickReportDialogLayout->addWidget(rawField->getDialog(), row, 1, alignCenter);

    if (filtField) {
      filtField->createDialog(_clickReportDialog, "-------------");
      _clickReportDialogLayout->addWidget(filtField->getDialog(), row, 2, alignCenter);
    }
      
    if (filtField != NULL) {
      ifield++;
    }

    row++;
  }
}

//////////////////////////////////////////////
// make a new label with right justification

QLabel *StatusPanel::_newLabelRight(const string &text)
{
  QLabel *label = new QLabel;
  label->setText("-----");
  label->setAlignment(Qt::AlignRight);
  return label;
}

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *StatusPanel::_createStatusVal(const string &leftLabel,
                                         const string &rightLabel,
                                         int row, 
                                         int fontSize,
                                         QLabel **label)
  
{

  QLabel *left = new QLabel(_statusPanel);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _statusLayout->addWidget(left, row, 0, alignRight);
  if (label != NULL) {
    *label = left;
  }

  QLabel *right = new QLabel(_statusPanel);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  _statusLayout->addWidget(right, row, 1, alignRight);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  _valsRight.push_back(right);

  return right;
}

//////////////////////////////////////////////////
// create a label row in a dialog

QLabel *StatusPanel::_addLabelRow(QWidget *parent,
                                     QGridLayout *layout,
                                     const string &leftLabel,
                                     const string &rightLabel,
                                     int row, 
                                     int fontSize)
  
{

  QLabel *left = new QLabel(parent);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  layout->addWidget(left, row, 0, alignRight);
  
  QLabel *right = new QLabel(parent);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  layout->addWidget(right, row, 1, 1, 2, alignCenter);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  return right;
}




//////////////////////////////////////////////
// update the status panel

void StatusPanel::_updateStatusPanel(const RadxRay *ray)
{

  // set time etc

  char text[1024];
  
  QString prev_radar_name = _radarName->text();
  
  string rname(_platform.getInstrumentName());
  if (_params->override_radar_name) rname = _params->radar_name;
  if (_params->display_site_name) {
    rname += ":";
    if (_params->override_site_name) {
      rname += _params->site_name;
    } else {
      rname += _platform.getSiteName();
    }
  }
  _radarName->setText(rname.c_str());

  if (prev_radar_name != _radarName->text()) {
    _setTitleBar(rname);
  }
  
  DateTime rayTime(ray->getTimeSecs());
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateVal->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) ray->getNanoSecs() / 1000000));
  _timeVal->setText(text);
  
  if (_volNumVal) {
    _setText(text, "%d", ray->getVolumeNumber());
    _volNumVal->setText(text);
  }
  
  if (_sweepNumVal) {
    _setText(text, "%d", ray->getSweepNumber());
    _sweepNumVal->setText(text);
  }
  
  if (_fixedAngVal) {  
    _setText(text, "%6.2f", ray->getFixedAngleDeg());
    _fixedAngVal->setText(text);
  }
  
  if (_elevVal) {
    if (fabs(ray->getElevationDeg()) < 1000) {
      _setText(text, "%6.2f", ray->getElevationDeg());
      _elevVal->setText(text);
    }
  }

  if (_azVal) {
    if (fabs(ray->getAzimuthDeg()) < 1000) {
      _setText(text, "%6.2f", ray->getAzimuthDeg());
      _azVal->setText(text);
    }
  }
  
  if (_nSamplesVal) {
    _setText(text, "%d", (int) ray->getNSamples());
    _nSamplesVal->setText(text);
  }
  
  if (_nGatesVal) {
    _setText(text, "%d", (int) ray->getNGates());
    _nGatesVal->setText(text);
  }
  
  if (_gateSpacingVal) {
    _setText(text, "%.4f", ray->getGateSpacingKm());
    _gateSpacingVal->setText(text);
  }
  
  if (_pulseWidthVal) {
    _setText(text, "%.2f", ray->getPulseWidthUsec());
    _pulseWidthVal->setText(text);
  }

  if (_prfVal) {
    if (ray->getPrtMode() == Radx::PRT_MODE_FIXED) {
      if (ray->getPrtSec() <= 0) {
        _setText(text, "%d", -9999);
      } else {
        _setText(text, "%d", (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10);
      }
    } else {
      double prtSec = ray->getPrtSec();
      if (prtSec <= 0) {
        _setText(text, "%d", -9999);
      } else {
        int iprt = (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10;
        double prtRatio = ray->getPrtRatio();
        if (prtRatio > 0.6 && prtRatio < 0.7) {
          _setText(text, "%d(2/3)", iprt);
        } else if (prtRatio < 0.775) {
          _setText(text, "%d(3/4)", iprt);
        } else if (prtRatio < 0.825) {
          _setText(text, "%d(4/5)", iprt);
        } else {
          _setText(text, "%d", iprt);
        }
      }
    }
    _prfVal->setText(text);
  }

  if (_nyquistVal) {
    if (fabs(ray->getNyquistMps()) < 1000) {
      _setText(text, "%.1f", ray->getNyquistMps());
      _nyquistVal->setText(text);
    }
  }

  if (_maxRangeVal) {
    double maxRangeData = ray->getStartRangeKm() +
      ray->getNGates() * ray->getGateSpacingKm();
    _setText(text, "%.1f", maxRangeData);
    _maxRangeVal->setText(text);
  }

  if (_unambigRangeVal) {
    if (fabs(ray->getUnambigRangeKm()) < 100000) {
      _setText(text, "%.1f", ray->getUnambigRangeKm());
      _unambigRangeVal->setText(text);
    }
  }
  
  if (_powerHVal) {
    if (ray->getMeasXmitPowerDbmH() > -9990) {
      _setText(text, "%.1f", ray->getMeasXmitPowerDbmH());
      _powerHVal->setText(text);
    }
  }
   
  if (_powerVVal) {
    if (ray->getMeasXmitPowerDbmV() > -9990) {
      _setText(text, "%.1f", ray->getMeasXmitPowerDbmV());
      _powerVVal->setText(text);
    }
  }

  if (_scanNameVal) {
    string scanName = ray->getScanName();
    size_t len = scanName.size();
    if (len > 8) {
      scanName = scanName.substr(0, 8);
    }
    _scanNameVal->setText(scanName.c_str());
  }

  if (_sweepModeVal) {
    switch (ray->getSweepMode()) {
      case Radx::SWEEP_MODE_SECTOR: {
        _sweepModeVal->setText("sector"); break;
      }
      case Radx::SWEEP_MODE_COPLANE: {
        _sweepModeVal->setText("coplane"); break;
      }
      case Radx::SWEEP_MODE_RHI: {
        _sweepModeVal->setText("RHI"); break;
      }
      case Radx::SWEEP_MODE_VERTICAL_POINTING: {
        _sweepModeVal->setText("vert"); break;
      }
      case Radx::SWEEP_MODE_IDLE: {
        _sweepModeVal->setText("idle"); break;
      }
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE: {
        _sweepModeVal->setText("SUR"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN: {
        _sweepModeVal->setText("sunscan"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN_RHI: {
        _sweepModeVal->setText("sun_rhi"); break;
      }
      case Radx::SWEEP_MODE_POINTING: {
        _sweepModeVal->setText("point"); break;
      }
      case Radx::SWEEP_MODE_CALIBRATION: {
        _sweepModeVal->setText("cal"); break;
      }
      default: {
        _sweepModeVal->setText("unknown");
      }
    }
  }

  if (_polModeVal) {
    _polModeVal->setText(Radx::polarizationModeToStr
                         (ray->getPolarizationMode()).c_str());
  }
   
  if (_prfModeVal) {
    _prfModeVal->setText(Radx::prtModeToStr
                         (ray->getPrtMode()).c_str());
  }

  if (fabs(_radarLat - _platform.getLatitudeDeg()) > 0.0001 ||
      fabs(_radarLon - _platform.getLongitudeDeg()) > 0.0001 ||
      fabs(_radarAltKm - _platform.getAltitudeKm()) > 0.0001) {
    _radarLat = _platform.getLatitudeDeg();
    _radarLon = _platform.getLongitudeDeg();
    _radarAltKm = _platform.getAltitudeKm();
    _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm * 1000.0);
  }

  if (ray->getGeoreference() != NULL) {

    if (_latVal) {
      _radarLat = ray->getGeoreference()->getLatitude();
      _setText(text, "%.3f", _radarLat);
      _latVal->setText(text);
    }
     
    if (_lonVal) {
      _radarLon = ray->getGeoreference()->getLongitude();
      _setText(text, "%.3f", _radarLon);
      _lonVal->setText(text);
    }

    if (_altVal) {
      _radarAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      if (_altitudeInFeet) {
        _setText(text, "%.3f", _radarAltKm / 0.3048);
      } else {
        _setText(text, "%.3f", _radarAltKm);
      }
      _altVal->setText(text);
    }

    // compute altitude rate every 2 secs
    
    if (_prevAltKm > -9990) {
      double deltaTime = ray->getRadxTime() - _prevAltTime;
      if (deltaTime > 2.0) {
        double altKm = ray->getGeoreference()->getAltitudeKmMsl();
        double deltaAltKm = altKm - _prevAltKm;
        _altRateMps = (deltaAltKm / deltaTime) * 1000.0;
        _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
        _prevAltTime = ray->getRadxTime();
      }
    } else {
      _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      _prevAltTime = ray->getRadxTime();
    }
    
    if (_altitudeInFeet) {
      _setText(text, "%.1f", _altRateMps / 0.3048);
    } else {
      _setText(text, "%.1f", _altRateMps);
    }
    if (_altRateVal) {
      _altRateVal->setText(text);
    }

    if (_speedVal) {
      double ewVel = ray->getGeoreference()->getEwVelocity();
      double nsVel = ray->getGeoreference()->getNsVelocity();
      double speed = sqrt(ewVel * ewVel + nsVel * nsVel);
      _setText(text, "%.2f", speed);
      _speedVal->setText(text);
    }
     
    if (_headingVal) {
      double heading = ray->getGeoreference()->getHeading();
      if (heading >= 0 && heading <= 360.0) {
        _setText(text, "%.2f", heading);
        _headingVal->setText(text);
      }
    }
     
    if (_trackVal) {
      double track = ray->getGeoreference()->getTrack();
      if (track >= 0 && track <= 360.0) {
        _setText(text, "%.2f", track);
        _trackVal->setText(text);
      }
    }
     
  } else {
    
    _setText(text, "%.3f", _platform.getLatitudeDeg());
    if (_latVal) {
      _latVal->setText(text);
    }
    
    _setText(text, "%.3f", _platform.getLongitudeDeg());
    if (_lonVal) {
      _lonVal->setText(text);
    }
    
    if (_altitudeInFeet) {
      _setText(text, "%.3f", _platform.getAltitudeKm() / 0.3048);
    } else {
      _setText(text, "%.3f", _platform.getAltitudeKm());
    }
    if (_altVal) {
      _altVal->setText(text);
    }
    if (_altRateVal) {
      _altRateVal->setText("0.0");
    }

  }
  
  double sunEl, sunAz;
  _sunPosn.computePosn(ray->getTimeDouble(), sunEl, sunAz);
  _setText(text, "%.3f", sunEl);
  if (_sunElVal) {
    _sunElVal->setText(text);
  }
  _setText(text, "%.3f", sunAz);
  if (_sunAzVal) {
    _sunAzVal->setText(text);
  }

  // if airborne data ...
  if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    
    _georefsApplied->show();
    _geoRefRotationVal->show();
    _geoRefRollVal->show();
    _geoRefTiltVal->show();
    _geoRefTrackRelRotationVal->show(); 
    _geoRefTrackRelTiltVal->show(); 
    _geoRefTrackRelAzVal->show(); 
    _geoRefTrackRelElVal->show(); 
    _cfacRotationVal->show();
    _cfacRollVal->show();
    _cfacTiltVal->show();     
    //_applyCfacToggle->show();

    _georefsAppliedLabel->show(); 
    _geoRefRotationLabel->show(); 
    _geoRefRollLabel->show(); 
    _geoRefTiltLabel->show();
    _geoRefTrackRelRotationLabel->show(); 
    _geoRefTrackRelTiltLabel->show(); 
    _geoRefTrackRelAzLabel->show(); 
    _geoRefTrackRelElLabel->show(); 
    _cfacRotationLabel->show(); 
    _cfacRollLabel->show(); 
    _cfacTiltLabel->show();     

    const RadxGeoref *georef = ray->getGeoreference();
    if (georef != NULL) {

      if (ray->getGeorefApplied()) {
        _georefsApplied->setText("true");       
      } else {
        _georefsApplied->setText("false");          
      }

      _setText(text, "%.3f", georef->getRotation());  
      _geoRefRotationVal->setText(text); 
      _setText(text, "%.3f", georef->getRoll());  
      _geoRefRollVal->setText(text); 
      _setText(text, "%.3f", georef->getTilt());  
      _geoRefTiltVal->setText(text);

      _setText(text, "%.3f", georef->getTrackRelRot());  
      _geoRefTrackRelRotationVal->setText(text); 
      _setText(text, "%.3f", georef->getTrackRelTilt());  
      _geoRefTrackRelTiltVal->setText(text);
      _setText(text, "%.3f", georef->getTrackRelAz());  
      _geoRefTrackRelAzVal->setText(text); 
      _setText(text, "%.3f", georef->getTrackRelEl());  
      _geoRefTrackRelElVal->setText(text); 

      double rollCorr = 0.0;
      double rotCorr = 0.0;
      double tiltCorr = 0.0;
      DataModel *dataModel = DataModel::Instance();
      dataModel->getCfactors(&rollCorr, &rotCorr, &tiltCorr);
      _setText(text, "%.3f", rollCorr);
      _cfacRollVal->setText(text);
      _setText(text, "%.3f", rotCorr);
      _cfacRotationVal->setText(text);   
      _setText(text, "%.3f", tiltCorr);    
      _cfacTiltVal->setText(text);

    }
  } else {
    hideCfacs();
  }

}

void StatusPanel::hideCfacs() {
  _georefsApplied->hide(); 
  _geoRefRotationVal->hide(); 
  _geoRefRollVal->hide(); 
  _geoRefTiltVal->hide();   
  _geoRefTrackRelRotationVal->hide(); 
  _geoRefTrackRelAzVal->hide(); 
  _geoRefTrackRelTiltVal->hide();  
  _geoRefTrackRelElVal->hide();  
  _cfacRotationVal->hide(); 
  _cfacRollVal->hide(); 
  _cfacTiltVal->hide(); 
  _georefsAppliedLabel->hide(); 
  _geoRefRotationLabel->hide(); 
  _geoRefRollLabel->hide(); 
  _geoRefTiltLabel->hide(); 
  _geoRefTrackRelRotationLabel->hide(); 
  _geoRefTrackRelAzLabel->hide(); 
  _geoRefTrackRelTiltLabel->hide();   
  _geoRefTrackRelElLabel->hide(); 
  _cfacRotationLabel->hide(); 
  _cfacRollLabel->hide(); 
  _cfacTiltLabel->hide(); 

  //_applyCfacToggle->hide();  
}

///////////////////////////////////////////
// set text for GUI panels

void StatusPanel::_setText(char *text,
                              const char *format,
                              int val)
{
  if (abs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999);
  }
}

void StatusPanel::_setText(char *text,
                              const char *format,
                              double val)
{
  if (fabs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999.0);
  }
}

////////////////////////////////////////////////////////////////

double StatusPanel::_getInstHtKm(const RadxRay *ray)

{
  double instHtKm = _platform.getAltitudeKm();
  if (ray->getGeoreference() != NULL) {
    instHtKm = ray->getGeoreference()->getAltitudeKmMsl();
  }
  return instHtKm;
}

/////////////////////////////////////////////////////////////////////  
// slots

/////////////////////////////
// show data at click point
void StatusPanel::_showClick()
{
  if (_clickReportDialog) {
    if (_clickReportDialog->isVisible()) {
      _clickReportDialog->setVisible(false);
    } else {
      if (_clickReportDialog->x() == 0 &&
          _clickReportDialog->y() == 0) {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y());
        _clickReportDialog->move(pos);
      }
      _clickReportDialog->setVisible(true);
      _clickReportDialog->raise();
    }
  }
}


//TODO: change model for displayFieldController
//_displayFieldController->setModel(new DisplayFieldModel(...))
  
// reset or sync the displayFields with those in the list
// used for a read of new data file
// TODO: shouldn't this go to DisplayFieldController?? just send the color map directory?
int StatusPanel::_updateDisplayFields(vector<string> *fieldNames) {

  _displayFieldController->reconcileFields(fieldNames, _fieldPanel);

  // TODO: not sure where this needs to happen ...
  // check for color map location
  
  string colorMapDir = _params->color_scale_dir;
  Path mapDir(_params->color_scale_dir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params->color_scale_dir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - DisplayManager" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params->color_scale_dir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      return -1;
    }
    if (_params->debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

/*
  //vector<DisplayField *> displayFields;

  // There is the fieldPanel, which is a view of the DisplayFields, for selection
  // then, there is the displayFieldController which manages the fields and all
  // their attributes.  
  // sometimes, we need to add a field, remove a field, and then sync the fields as in reset.

  //for (int ifield = 0; ifield < _params->fields_n; ifield++) {
  int ifield = (int) _displayFieldController->getNFields() + 1;
  for (vector<string>::iterator it = fieldNames->begin(); it != fieldNames->end(); ++it) {
    string fieldName = *it;

//HERE TODO:
//distingquish between add and update on fieldName;
//then set last field or first field as selected? or do something to render image

    if (!_displayFieldController->contains(fieldName)) {

      ColorMap map;
      map = ColorMap(0.0, 1.0);
      bool noColorMap = true; 
      // unfiltered field
      string shortcut = to_string(ifield);
      DisplayField *field =
        new DisplayField(fieldName, fieldName, "m/s", 
                         shortcut, map, ifield, false);
      if (noColorMap)
        field->setNoColorMap();

      //displayFields.push_back(field);
      _displayFieldController->addField(field);
      //_updateFieldPanel(fieldName);
      // TODO: causes a EXC_BAD_ACCESS if outside the loop
      // somehow this is called when setting up the menus???
      //_fieldPanel->updateFieldPanel(fieldName, fieldName, fieldName);
      ifield += 1;
    }
    _fieldPanel->updateFieldPanel(fieldName, fieldName, fieldName);


  } // ifield
*/

  if (fieldNames->size() < 1) {
    cerr << "ERROR - StatusPanel::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }

  _displayFieldController->setSelectedField(0);


  return 0;

}




void StatusPanel::closeEvent(QEvent *event)
{
    if (_unSavedEdits) {
        string msg = "Unsaved changes to the data. \n";
        msg.append("Use File->Save before closing to avoid this message. \n");
        msg.append("Do you want to save these changes?");

        QMessageBox::StandardButton reply =
            QMessageBox::warning(this, "QMessageBox::warning()",
                          QString::fromStdString(msg),
                          QMessageBox::Save | QMessageBox::Discard);
  
        //  QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
        if (reply == QMessageBox::Save) {
            LOG(DEBUG) << "Save";
            _saveFile();
            _unSavedEdits = false;
        }

    }  
    event->accept();
    //emit close();

}


/* use the default QWidget::close() slot
   which first sends the widget a QCloseEvent. 
   If the widget has the Qt::WA_DeleteOnClose flag, 
   the widget is also deleted. 
   A close event is delivered to the widget no matter
  if the widget is visible or not.

void StatusPanel::close(QEvent *event) {

cerr << "StatusPanel::close() called" << endl;
  event->accept();

  QMainWindow::closeEvent(event);
  if (_ppi) {
    delete _ppi;
  }

  if (_rhi) {
    delete _rhi;
  }

  //if (_ppiRays) {
  //  delete[] _ppiRays;
  //}
  // TODO: delete all controllers
  if (_timeNavController) {
    //_timeNavController->~TimeNavController();
    delete _timeNavController;
  }
  // if (_timeNavView) delete _timeNavView;

}
*/


