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
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// GuiManager manages cartesian rendering - plan view
//
///////////////////////////////////////////////////////////////

#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
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
#include <QProcess>
#include <QListWidget>
#include <QListWidgetItem>
#include <QRadioButton>
#include <QStatusBar>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QTimeZone>
#include <QDir>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QErrorMessage>
#include <QFileDialog>
#include <QSlider>
#include <QGraphicsScene>
#include <QGraphicsAnchorLayout>
#include <QGraphicsProxyWidget>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QWidgetAction>
#include <QTimer>
#include <QDesktopServices>
#include <QTableWidget>
#include <QHeaderView>
#include <QImageWriter>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringList>
#include <QXmlStreamWriter>

#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaStr.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>
#include <Radx/RadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <toolsa/DateTime.hh>
#include <Radx/RadxPath.hh>
#include <Ncxx/H5x.hh>

#include "GuiManager.hh"
#include "AutomaticImageGenerator.hh"
#include "ProductMgr.hh"
#include "FieldTableItem.hh"
#include "HorizView.hh"
#include "MapMenuItem.hh"
#include "MdvReader.hh"
#include "ProdMenuItem.hh"
#include "TimeControl.hh"
#include "VlevelSelector.hh"
#include "VertView.hh"
#include "VertManager.hh"
#include "WindMenuItem.hh"
#include "WindManager.hh"
#include "ZoomMenuItem.hh"

// GlobalData.hh must be included AFTER qt includes because of #define None in X11/X.h

#include "GlobalData.hh"

using namespace std;
using namespace H5x;

namespace {

// Place numeric controls directly in the context menu so related contour and
// color-scale values can be inspected and edited together.

QDoubleSpinBox *addFieldNumberEditor(QMenu *menu, const QString &label,
                                     double value,
                                     int decimals = 6,
                                     double minimum = -1.0e12,
                                     double maximum = 1.0e12)
{
  QWidget *editor = new QWidget(menu);
  QHBoxLayout *layout = new QHBoxLayout(editor);
  layout->setContentsMargins(8, 2, 8, 2);

  QLabel *nameLabel = new QLabel(label, editor);
  nameLabel->setMinimumWidth(65);
  layout->addWidget(nameLabel);

  QDoubleSpinBox *spinBox = new QDoubleSpinBox(editor);
  spinBox->setRange(minimum, maximum);
  spinBox->setDecimals(decimals);
  spinBox->setValue(value);
  spinBox->setKeyboardTracking(false);
  layout->addWidget(spinBox);

  QWidgetAction *action = new QWidgetAction(menu);
  action->setDefaultWidget(editor);
  menu->addAction(action);
  return spinBox;
}

QString safeFileNameComponent(QString value)
{
  value = value.trimmed();
  value.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
  value.remove(QRegularExpression("^_+|_+$"));
  return value.isEmpty() ? QString("field") : value;
}

} // namespace

GuiManager* GuiManager::m_pInstance = NULL;

GuiManager* GuiManager::Instance()
{
  return m_pInstance;
}

// Constructor

GuiManager::GuiManager() :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _vertWindowDisplayed(false)
{

  _timerEventCount = 0;
  _archiveMode = true;
  
  m_pInstance = this;

  // initialize

  _main = NULL;
  _resizeTimer = NULL;
  _resized = false;
  
  _horizFrame = NULL;
  _horiz = NULL;

  _clickReportDialog = NULL;
  _clickReportDialogLayout = NULL;
  _clickDate = NULL;
  _clickTime = NULL;
  _clickLatitude = NULL;
  _clickLongitude = NULL;
  _clickHeightName = NULL;
  _clickHeight = NULL;
  _clickXName = NULL;
  _clickX = NULL;
  _clickYName = NULL;
  _clickY = NULL;
  _clickFieldsTable = NULL;
  _clickReportPlaced = false;

  _vertWindow = NULL;
  _vert = NULL;

  _fieldMenu = NULL;
  _fieldTable = NULL;
  _fieldMenuPlaced = false;
  _fieldMenuPanel = NULL;
  _fieldMenuLayout = NULL;

  _fieldNum = 0;
  _prevFieldNum = -1;
  _fieldHasChanged = true;
  _horizontalRequestHasChanged = false;
  
  _fieldTableCol = 0;
  _fieldTableRow = 0;
  
  _prevFieldTableCol = -1;
  _prevFieldTableRow = -1;

  _vlevelManager.requestLevel(_params.start_ht_km);
  _vlevelHasChanged = true;
  
  _overlaysHaveChanged = true;
  _movieBuildFrame = -1;
  _movieGridReady = false;
  _movieIncludesVertical = false;
  _movieVerticalReady = false;
  _movieVerticalRequestGeneration = -1;

  _automaticImagesStarted = false;
  _automaticImagesActive = false;
  _automaticImageGenerator = new AutomaticImageGenerator;
  _automaticBatchFailureBaseline = 0;
  _automaticBatchAttempted = 0;
  _automaticBatchSaved = 0;
  _automaticBatchTimedOut = 0;
  _automaticBatchProductOmissions = 0;
  _automaticTotalFailures = 0;
  _automaticTotalAttempted = 0;
  _automaticTotalSaved = 0;
  _automaticTotalTimedOut = 0;
  _automaticTotalProductOmissions = 0;
  _automaticCompletedBatches = 0;
  _automaticDisplayStateSaved = false;
  _automaticSavedField = -1;
  _automaticSavedZoom = 0;
  _automaticSavedHeight = 0.0;
  _automaticSavedFrameCount = 1;
  _automaticSavedFrameIntervalSecs = 1.0;
  _automaticSavedFrameIndex = 0;
  _automaticSavedRealtime = false;
  _automaticImageTimeoutTimer = new QTimer(this);
  _automaticImageTimeoutTimer->setSingleShot(true);
  connect(_automaticImageTimeoutTimer, &QTimer::timeout,
          this, &GuiManager::_automaticImageTimedOut);
  _automaticImageScheduleTimer = new QTimer(this);
  _automaticImageScheduleTimer->setSingleShot(true);
  connect(_automaticImageScheduleTimer, &QTimer::timeout,
          this, &GuiManager::_automaticImageScheduleDue);
  
  _timeControl = NULL;
  _timeControlPlaced = false;

  _imagesStartTime.set(_params.images_archive_start_time);
  _imagesEndTime.set(_params.images_archive_end_time);
  _imagesScanIntervalSecs = _params.images_schedule_interval_secs;

  // set up ray locators

  // _rayLoc.resize(RayLoc::RAY_LOC_N);

  // set up windows

  _setupWindows();

  // init for timer
  
  redraw_interv = _params.redraw_interval;
  update_interv = _params.update_interval;
  update_due = 0;
  last_frame_tm = {0,0};
  last_dcheck_tm = {0,0};
  last_tick = 0;
  client_seq_num = 0;

  // startup mode
  
  setArchiveMode(_params.start_mode == Params::MODE_ARCHIVE);
  _archiveStartTime.set(_params.archive_start_time);

  // qt attributes
  
  setAttribute(Qt::WA_TranslucentBackground);
  setAutoFillBackground(false);

}

// destructor

GuiManager::~GuiManager()

{

  delete _automaticImageGenerator;

  if (_gd.prod_mgr) {
    _gd.prod_mgr->setProductsChangedCallback(std::function<void()>());
    _gd.prod_mgr->setStatusCallback(
      std::function<void(const string &, bool)>());
  }

  if (_horiz) {
    delete _horiz;
  }

  if (_vert) {
    delete _vert;
  }

}

//////////////////////////////////////////////////
// Run

int GuiManager::run(QApplication &app)
{

  // make window visible
  
  show();

  // move to starting point
  
  QPoint pos;
  pos.setX(_params.horiz_window_x_pos);
  pos.setY(_params.horiz_window_y_pos);
  move(pos);
  
  // set timer running
  
  _mainTimerId = startTimer(2); // msecs
  
  return app.exec();

}

//////////////////////////////////////////////////
// enable the unzoom buttons

void GuiManager::enableUnzoomButtons(bool val) const
{
  const bool spatialChangesAllowed = !isMovieActive();
  _zoomBackAct->setEnabled(spatialChangesAllowed && val);
  _zoomOutAct->setEnabled(spatialChangesAllowed && val);
  if (!spatialChangesAllowed) {
    for (size_t ii = 0; ii < _zoomMenuItems.size(); ii++) {
      _zoomMenuItems[ii]->getAction()->setEnabled(false);
    }
  } else if (val) {
    // disable zoom menu items
    for (size_t ii = 0; ii < _zoomMenuItems.size(); ii++) {
      _zoomMenuItems[ii]->getAction()->setEnabled(false);
    }
  } else {
    // enable zoom menu items
    for (size_t ii = 0; ii < _zoomMenuItems.size(); ii++) {
      _zoomMenuItems[ii]->getAction()->setEnabled(true);
    }
  }
}

//////////////////////////////////////////////////
// respond to zoom action, set the zoom index

void GuiManager::setZoomIndex(int zoomIndex)
{
  if (isMovieActive()) {
    showMovieInteractionBlocked();
    return;
  }
  _horiz->setZoomIndex(zoomIndex);
}

// void GuiManager::setXyZoom(double minY, double maxY,
//                            double minX, double maxX)
// {
//   _horiz->setXyZoom(minY, maxY, minX, maxX);
// }

/////////////////////////////////////
// set archive mode

void GuiManager::setArchiveMode(bool state)
{

  _archiveMode = state;
  
}

////////////////////////////////////////////////////////
// initialize storage and request state for a new movie build

void GuiManager::prepareMovieBuild(int frameCount)
{
  _movieBuildFrame = -1;
  _movieGridReady = false;
  _movieIncludesVertical = _vertWindow && _vert &&
    _vertWindow->isVisible() && _vsectionRoute.size() >= 2;
  _movieVerticalReady = !_movieIncludesVertical;
  _movieVerticalRequestGeneration = -1;
  _movieFieldLabel = getSelectedFieldLabel();
  if (_horiz) {
    _horiz->clearMovieFrames(frameCount);
  }
  if (_vert) {
    _vert->clearMovieFrames(_movieIncludesVertical ? frameCount : 0);
  }
  // Force frame zero to be rebuilt even when it is already the live frame.
  if (_gd.prod_mgr) {
    _gd.prod_mgr->reset_product_valid_flags();
  }
}

void GuiManager::beginMovieFrameBuild(int frameIndex)
{
  _movieBuildFrame = frameIndex;
  _movieGridReady = false;
  _movieVerticalReady = !_movieIncludesVertical;
  _movieVerticalRequestGeneration = -1;
  _horizontalRequestHasChanged = true;
  if (_horiz) {
    _horiz->stopMoviePlayback();
  }
  if (_movieIncludesVertical && _vert) {
    _vert->stopMoviePlayback();
    _movieVerticalRequestGeneration = _requestVerticalSection();
    if (_movieVerticalRequestGeneration < 0) {
      // Do not deadlock movie construction if the retained route can no
      // longer produce a request. The captured vertical frame will show the
      // current no-data state for this time.
      _movieVerticalReady = true;
    }
  }
  statusBar()->showMessage(
    tr("Building movie frame %1 of %2")
      .arg(frameIndex + 1).arg(_timeControl->getNFramesMovie()));
}

void GuiManager::cancelMovieBuild()
{
  _movieBuildFrame = -1;
  _movieGridReady = false;
  _movieIncludesVertical = false;
  _movieVerticalReady = false;
  _movieVerticalRequestGeneration = -1;
}

bool GuiManager::showMovieFrame(int frameIndex)
{
  if (!_horiz || !_horiz->hasMovieFrame(frameIndex) ||
      (_movieIncludesVertical &&
       (!_vert || !_vert->hasMovieFrame(frameIndex)))) {
    return false;
  }
  if (!_horiz->showMovieFrame(frameIndex)) {
    return false;
  }
  return !_movieIncludesVertical || _vert->showMovieFrame(frameIndex);
}

bool GuiManager::exportMovieLoop(QString &message)
{
  message.clear();
  if (!_timeControl || !_horiz) {
    message = tr("The movie views are not available.");
    return false;
  }

  const int frameCount = _timeControl->getNFramesMovie();
  if (frameCount < 1) {
    message = tr("The movie contains no frames.");
    return false;
  }
  for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
    if (!_horiz->hasMovieFrame(frameIndex) ||
        (_movieIncludesVertical &&
         (!_vert || !_vert->hasMovieFrame(frameIndex)))) {
      message = tr("Movie frame %1 is not ready for output.")
        .arg(frameIndex + 1);
      return false;
    }
  }

  QString outputDirPath = QString::fromUtf8(_params.images_output_dir);
  outputDirPath.replace("$(HOME)", QDir::homePath());
  if (outputDirPath.startsWith("$HOME")) {
    outputDirPath.replace(0, 5, QDir::homePath());
  }
  if (outputDirPath.trimmed().isEmpty()) {
    message = tr("images_output_dir is empty.");
    return false;
  }
  outputDirPath = QDir::cleanPath(outputDirPath);
  QDir outputDir;
  if (!outputDir.mkpath(outputDirPath)) {
    message = tr("Cannot create the output directory:\n%1")
      .arg(outputDirPath);
    return false;
  }
  outputDir.setPath(outputDirPath);

  QString extension = safeFileNameComponent(
    QString::fromUtf8(_params.images_file_name_extension)).toLower();
  if (extension.startsWith('.')) {
    extension.remove(0, 1);
  }
  if (extension.isEmpty() || extension == "field") {
    extension = "png";
  }
  // Playback permits the live field selection to change, so name the files
  // from the display configuration captured when this cache was built.
  const QString fieldPart = safeFileNameComponent(
    QString::fromStdString(_movieFieldLabel));
  const QString startPart = QDateTime::fromSecsSinceEpoch(
          _timeControl->getStartTime().utime(), QTimeZone(QTimeZone::UTC))
    .toString("yyyyMMdd_HHmmss'_UTC'");

  // One shared suffix keeps the horizontal and vertical sequences paired.
  // Checking every target prevents a repeated export from replacing even a
  // partially retained earlier sequence.
  QString seriesSuffix;
  for (int sequence = 1; ; ++sequence) {
    seriesSuffix = sequence == 1 ? QString() :
      QString("_%1").arg(sequence, 2, 10, QChar('0'));
    bool collision = false;
    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
      const QString framePart = QString("_%1.%2")
        .arg(frameIndex + 1, 3, 10, QChar('0')).arg(extension);
      const QString commonPart = fieldPart + "_" + startPart +
        seriesSuffix + framePart;
      collision = QFileInfo::exists(outputDir.filePath(
        "lucid_horiz_" + commonPart)) ||
        (_movieIncludesVertical && QFileInfo::exists(outputDir.filePath(
          "lucid_vert_" + commonPart)));
      if (collision) {
        break;
      }
    }
    if (!collision) {
      break;
    }
  }

  QStringList horizontalFiles;
  QStringList verticalFiles;
  for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
    const QString framePart = QString("_%1.%2")
      .arg(frameIndex + 1, 3, 10, QChar('0')).arg(extension);
    const QString commonPart = fieldPart + "_" + startPart +
      seriesSuffix + framePart;
    const QString horizontalPath = outputDir.filePath(
      "lucid_horiz_" + commonPart);
    if (!_horiz->movieFrameImage(frameIndex).save(horizontalPath)) {
      message = tr("Could not save movie frame:\n%1").arg(horizontalPath);
      return false;
    }
    horizontalFiles.push_back(horizontalPath);

    if (_movieIncludesVertical) {
      const QString verticalPath = outputDir.filePath(
        "lucid_vert_" + commonPart);
      if (!_vert->movieFrameImage(frameIndex).save(verticalPath)) {
        message = tr("Could not save movie frame:\n%1").arg(verticalPath);
        return false;
      }
      verticalFiles.push_back(verticalPath);
    }
  }

  QStringList convertersStarted;
  const QString converter = QString::fromUtf8(
    _params.movie_convert_script).trimmed();
  auto startConverter = [&](const QStringList &files,
                            const QString &viewName) {
    if (converter.isEmpty()) {
      return true;
    }
    QStringList command = QProcess::splitCommand(converter);
    if (command.isEmpty()) {
      return true;
    }
    const QString program = command.takeFirst();
    command.append(files);
    if (!QProcess::startDetached(program, command, outputDirPath)) {
      message = tr("Saved %1 frames in:\n%2\n\n"
                   "Could not start converter: %3")
        .arg(frameCount).arg(outputDirPath, converter);
      return false;
    }
    convertersStarted.push_back(viewName);
    return true;
  };

  if (!startConverter(horizontalFiles, tr("horizontal")) ||
      (_movieIncludesVertical &&
       !startConverter(verticalFiles, tr("vertical")))) {
    return false;
  }

  const int fileCount = horizontalFiles.size() + verticalFiles.size();
  message = tr("Saved %1 image files in:\n%2")
    .arg(fileCount).arg(outputDirPath);
  if (!convertersStarted.isEmpty()) {
    message += tr("\n\nStarted %1 for the %2 sequence(s).")
      .arg(converter, convertersStarted.join(tr(" and ")));
  } else {
    message += tr("\n\nNo animation converter was configured.");
  }
  return true;
}

void GuiManager::stopMoviePlayback()
{
  if (_horiz) {
    _horiz->stopMoviePlayback();
  }
  if (_vert) {
    _vert->stopMoviePlayback();
  }
}

void GuiManager::restoreLiveDisplayAfterMovie()
{
  stopMoviePlayback();
  if (_vertWindow && _vertWindow->isVisible() && _vsectionRoute.size() >= 2) {
    // Supersede any build request so the live section corresponds to the
    // exact frame at which playback was stopped.
    _requestVerticalSection();
  }
}

void GuiManager::stopMovieForDisplayChange()
{
  if (_timeControl && _timeControl->isMovieActive()) {
    _timeControl->stopMovie();
  }
}

bool GuiManager::isMovieBuilding() const
{
  return _timeControl && _timeControl->isMovieBuilding();
}

bool GuiManager::isMoviePlaying() const
{
  return _timeControl && _timeControl->isMoviePlaying();
}

bool GuiManager::isMovieActive() const
{
  return _timeControl && _timeControl->isMovieActive();
}

void GuiManager::movieStateChanged()
{
  const bool building = isMovieBuilding();
  const bool active = isMovieActive();

  // A movie build must capture one stable display configuration. Playback is
  // less restrictive: field and level choices may update the hidden live
  // state and become visible after Stop, but spatial transforms would make
  // the immutable cached frames incompatible with that state.
  if (_fieldTable) {
    _fieldTable->setEnabled(!building);
  }
  if (_vlevelSelector) {
    _vlevelSelector->setEnabled(!building);
  }
  if (_mapsMenu) {
    _mapsMenu->setEnabled(!building);
  }
  if (_windsMenu) {
    _windsMenu->setEnabled(!building);
  }
  if (_productsMenu) {
    _productsMenu->setEnabled(!building);
  }
  if (_overlaysMenu) {
    _overlaysMenu->setEnabled(!building);
  }
  if (_reloadAct) {
    _reloadAct->setEnabled(!building);
  }
  if (_clearAct) {
    _clearAct->setEnabled(!building);
  }
  if (_zoomsMenu) {
    _zoomsMenu->setEnabled(!active);
  }
  if (_showValueAct) {
    _showValueAct->setEnabled(!active);
  }
  if (_showClickAct) {
    _showClickAct->setEnabled(!active);
  }
  if (_horiz) {
    _horiz->setMovieInteractionActive(active);
  }
  if (_vertWindow) {
    _vertWindow->setMovieInteractionActive(active);
  }
  if (_showVertAct) {
    // A cached vertical loop may be hidden and reopened. If no vertical cache
    // was selected at Start, opening a live section during horizontal-only
    // playback would imply synchronization which does not exist.
    _showVertAct->setEnabled(!active || _movieIncludesVertical);
  }

  enableUnzoomButtons(_horiz && !_horiz->getCustomZooms().empty());
}

void GuiManager::showMovieInteractionBlocked()
{
  statusBar()->showMessage(tr("Stop the movie to change the view"), 3000);
}

void GuiManager::_checkMovieFrameReady()
{
  if (_movieBuildFrame < 0 || !_movieGridReady || !_movieVerticalReady ||
      !_horiz || !_timeControl) {
    return;
  }
  const bool productsReady = !_gd.prod.products_on || !_gd.prod_mgr ||
    _gd.prod_mgr->activeProductsReady();
  if (!productsReady) {
    return;
  }
  if (_gd.wind_mgr && !_gd.wind_mgr->activeWindsReady()) {
    return;
  }
  if (_movieIncludesVertical && _gd.wind_mgr &&
      !_gd.wind_mgr->activeVerticalWindsSettled()) {
    return;
  }

  const int completedFrame = _movieBuildFrame;
  _movieBuildFrame = -1;
  _movieGridReady = false;
  _movieVerticalReady = false;
  const bool horizCaptured = _horiz->captureMovieFrame(completedFrame);
  const bool vertCaptured = !_movieIncludesVertical ||
    (_vert && _vert->captureMovieFrame(completedFrame));
  if (horizCaptured && vertCaptured) {
    _timeControl->movieFrameRendered(completedFrame);
  }
}

//////////////////////////////////////////////////
// Set radar name in title bar

void GuiManager::setTitleBarStr(const string &titleStr)
{
  setWindowTitle(tr(titleStr.c_str()));
}
  
//////////////////////////////////////////////////////////////
// respond to timer events
  
void GuiManager::timerEvent(QTimerEvent *event)
{

  // register with procmap

  PMU_auto_register("timerEvent");
  
  // check ID
  
  if (event->timerId() != _mainTimerId) {
    return;
  }
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.
  
  if (_timerEventCount == 0) {
    _handleFirstTimerEvent();
  }    
  _timerEventCount++;

  // get the time control into the right place
  // we need to let the windows fully draw
  
  if (_timerEventCount == 10) {
    _placeTimeControl();
  }

  // Wait until the initial show/resize events have settled. Automatic output
  // captures the configured canvas geometry, so starting during the resize
  // debounce could associate a completed read with an obsolete transform.
  if (_params.create_images_automatically && !_automaticImagesStarted &&
      _timerEventCount > 10 && !_resizeTimer->isActive()) {
    _maybeStartAutomaticImages();
  }

  // read click point info from FMQ
  
  _readClickPoint();
  
  // handle client events
  
  if(_gd.coord_expt->client_event != NO_MESSAGE) {
    _handleClientEvent();
  }

  // is previous read still busy?
  
  MdvReader *mr = _gd.mread[_fieldNum];
  if (!mr->getReadBusyH()) {
    // read the H data if needed
    _checkAndReadH(mr);
  }

  // U and V complete independently. Publish a new wind overlay only after
  // the manager has revalidated the pair for the current display request.
  if (_gd.wind_mgr && _gd.wind_mgr->poll()) {
    if (_horiz) {
      _horiz->rerenderWinds();
    }
    _checkMovieFrameReady();
    _checkAutomaticImageReady();
  }
  if (_gd.wind_mgr && _gd.wind_mgr->pollVertical()) {
    if (_vert) {
      _vert->rerenderWinds();
    }
    _checkMovieFrameReady();
    _checkAutomaticImageReady();
  }
  
}

///////////////////////////////////////////////
// override resize event
// we use a timer to debounce the resize event

void GuiManager::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  // Cached movie composites have the canvas dimensions and projection that
  // were active at build time. A resize necessarily returns to live mode so
  // the display can rebuild all layers for the new geometry.
  if (_timeControl && _timeControl->isMovieActive()) {
    _timeControl->stopMovie();
  }
  // Restart the timer on each resize event
  _resizeTimer->start();
}

void GuiManager::_resizeFinished()
{
  // Called after the resize events have "settled"
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent, width, height: "
         << _horizFrame->width() << ", " << _horizFrame->height() << endl;
  }
  _horiz->resize(_horizFrame->width(), _horizFrame->height());
  _resized = true;
}

////////////////////////////////////////////////////////////////
void GuiManager::keyPressEvent(QKeyEvent * e)
{

  // get key pressed
  
  // Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Clicked char: " << keychar << ":" << (int) keychar << endl;
    cerr << "         key: " << hex << key << dec << endl;
  }

  // Do not let keyboard shortcuts alter the field or level halfway through a
  // build; every captured frame must use the same display configuration.
  if (isMovieBuilding() &&
      (keychar == '.' || key == Qt::Key_Up || key == Qt::Key_Down)) {
    showMovieInteractionBlocked();
    e->accept();
    return;
  }
  
  // for '.', swap with previous field
  
  if (keychar == '.') {
    if (_prevFieldNum >= 0) {
      _fieldTable->setCurrentCell(_prevFieldTableRow, _prevFieldTableCol);
      emit _fieldTable->cellClicked(_prevFieldTableRow, _prevFieldTableCol);
    }
    return;
  }
  
  // trap ESC - reset some items

  if (keychar == 27) {
    // zoom out
    _zoomOut();
  }

  // check for up/down in vlevels
  
  if (key == Qt::Key_Left) {
    
    if (_params.debug) {
      cerr << "Clicked left arrow, go back in time" << endl;
    }
    _timeControl->goBack1();
    
  } else if (key == Qt::Key_Right) {

    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _timeControl->goFwd1();
    
  } else if (key == Qt::Key_Up) {

    // Keep vlevel navigation and its boundary checks in the selector so the
    // behavior is identical whether the selector or main window has focus.
    _vlevelSelector->keyPressEvent(e);
    
  } else if (key == Qt::Key_Down) {

    _vlevelSelector->keyPressEvent(e);
    
  }

}

/////////////////////////////////////
// clear display widgets

void GuiManager::_clear()
{
  if (_horiz) {
    _horiz->clear();
  }
  if (_vert) {
    _vert->clear();
  }
}

//////////////////////////////////////////////////
// set up windows and widgets
  
void GuiManager::_setupWindows()
{

  // set up windows
  
  _main = new QFrame(this);
  QHBoxLayout *mainLayout = new QHBoxLayout;
  _main->setLayout(mainLayout);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(2,2,2,2);
  setCentralWidget(_main);

  // horiz window

  _horizFrame = new QFrame(_main);
  mainLayout->addWidget(_horizFrame);
  _horizFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _horizFrame->resize(_params.horiz_plot_width + _params.horiz_color_scale_width,
                      _params.horiz_plot_height);

  // configure the HORIZ

  _horiz = new HorizView(_horizFrame, *this);
  if (_gd.prod_mgr) {
    // SPDB workers publish data on the GUI thread. Regenerate only the
    // transparent product layer when a product snapshot is replaced.
    _gd.prod_mgr->setProductsChangedCallback([this]() {
      if (_horiz) {
        _horiz->rerenderProducts();
      }
      if (_timeControl) {
        _timeControl->refreshDataAvailability();
      }
      _checkMovieFrameReady();
      _checkAutomaticImageReady();
    });
    _gd.prod_mgr->setStatusCallback(
      [this](const string &message, bool isError) {
        // Successful requests use a brief progress message. Errors remain
        // visible longer while the previous valid product stays displayed.
        statusBar()->showMessage(QString::fromStdString(message),
                                 isError ? 15000 : 3000);
      });
  }

  connect(_horiz, &HorizView::verticalRouteCompleted,
          this, &GuiManager::_verticalRouteCompleted);
  
  // Create the VERT window

  _vertWindow = new VertManager(this);
  connect(_vertWindow, &VertManager::singleSegmentVsectionChanged,
          _horiz, &HorizView::setSingleSegmentVsection);
  
  // set pointer to the vertWidget
  
  _vert = _vertWindow->getView();
  connect(_vert, &VertView::clickReportAvailable,
          this, [this](const ClickReport &report) {
            updateClickReport(report);
            showClickDialog();
          });
  
  // connect slots for location

#ifdef NOTNOW
  connect(_horiz, &HorizView::locationClicked,
          this, &GuiManager::_horizLocationClicked);
  connect(_vert, &VertWidget::locationClicked,
          this, &GuiManager::_vertLocationClicked);
#endif

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);

  // customContextMenuRequested(e->pos());
  connect(_horiz, &HorizView::customContextMenuRequested,
	  this, &GuiManager::ShowContextMenu);

  // vlevel selector

  _vlevelSelector = new VlevelSelector(_params.vlevel_selector_width,
                                       _vlevelManager, this);
  mainLayout->addWidget(_vlevelSelector);
                                       
  // resize timer for debouncing resize events
  
  _resizeTimer = new QTimer(this);
  _resizeTimer->setInterval(250); // 0.25s delay
  _resizeTimer->setSingleShot(true);
  connect(_resizeTimer, &QTimer::timeout, this, &GuiManager::_resizeFinished);

  // field menu

  _createFieldMenu();
  
  // time panel
  
  _timeControl = new TimeControl(this);
  _timeControl->refreshDataAvailability();
  
  // fill out menu bar

  _createActions();
  _createMenus();

  // title bar

  setTitleBarStr(_params.horiz_frame_label);

  setMinimumSize(_params.horiz_window_min_width, _params.horiz_window_min_height);
  // resize(_params.main_window_width, _params.main_window_height);
  // connect(this, &GuiManager::frameResized, _horiz, &HorizView::resize);
  resize(_params.horiz_plot_width + _params.horiz_color_scale_width,
         _params.horiz_plot_height);
  
  // set location on screen

  QPoint pos(_params.horiz_window_x_pos, _params.horiz_window_y_pos);
  move(pos);
  show();

  // Resolve automatic placement only after the main window has its final
  // frame geometry. Each configured coordinate can override its fallback
  // independently.

  int vertX = _params.vsection_window_x_pos;
  int vertY = _params.vsection_window_y_pos;
  if (vertX == -1) {
    // Leave enough space for both window-manager borders to remain visible.
    constexpr int vertWindowGap = 5;
    vertX = frameGeometry().right() + 1 + vertWindowGap;
  }
  if (vertY == -1) {
    vertY = frameGeometry().top();
  }
  _vertWindow->move(vertX, vertY);

  // set up click report dialog
  _createClickReportDialog();

  // createBoundaryEditorDialog();

  // if (_archiveMode) {
  //   _showTimeControl();
  // }
  // _setVlevelPanelVisibility();

  // set up status bar

  statusBar()->setStyleSheet(
          "QStatusBar {"
          "  background-color: palette(alternate-base);"
          "  border-top: 2px solid palette(mid);"
          "  padding: 4px 6px;"
          "}"
                             );
  
}

///////////////////////////////////////////////////////
// create the click report dialog

void GuiManager::_createClickReportDialog()
{
  _clickReportDialog = new QDialog(this);
  _clickReportDialog->setWindowTitle("Click report");
  _clickReportDialog->setMinimumSize(210, 280);

  _clickReportDialogLayout = new QGridLayout(_clickReportDialog);
  _clickReportDialogLayout->setVerticalSpacing(5);

  // Keep the metadata names at their natural width and give remaining space
  // to the values.  Allowing both columns to stretch creates a large empty gap
  // between them and makes the dialog appear wider than its content requires.
  _clickReportDialogLayout->setColumnStretch(0, 0);
  _clickReportDialogLayout->setColumnStretch(1, 1);

  int row = 0;
  QLabel *locationHeader = new QLabel("CLICK POINT DATA", _clickReportDialog);
  _clickReportDialogLayout->addWidget(locationHeader, row++, 0, 1, 2,
                                      Qt::AlignCenter);

  _clickDate = _addClickReportRow("Date", row++);
  _clickTime = _addClickReportRow("Time", row++);
  _clickLatitude = _addClickReportRow("Latitude", row++);
  _clickLongitude = _addClickReportRow("Longitude", row++);
  _clickHeight = _addClickReportRow("Height", row++, &_clickHeightName);
  _clickX = _addClickReportRow("X", row++, &_clickXName);
  _clickY = _addClickReportRow("Y", row++, &_clickYName);

  QLabel *fieldsHeader = new QLabel("FIELD VALUES", _clickReportDialog);
  _clickReportDialogLayout->addWidget(fieldsHeader, row++, 0, 1, 2,
                                      Qt::AlignCenter);

  // Field availability follows the user's display history, so use a table
  // that can be rebuilt for each click instead of fixed rows.
  _clickFieldsTable = new QTableWidget(_clickReportDialog);
  _clickFieldsTable->setColumnCount(3);
  _clickFieldsTable->setHorizontalHeaderLabels
    (QStringList() << "Name" << "Value" << "Units");
  _clickFieldsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  _clickFieldsTable->setSelectionMode(QAbstractItemView::NoSelection);
  _clickFieldsTable->setMinimumWidth(0);
  _clickFieldsTable->setSizeAdjustPolicy
    (QAbstractScrollArea::AdjustToContents);
  _clickFieldsTable->verticalHeader()->setVisible(false);
  _clickFieldsTable->horizontalHeader()->setSectionResizeMode
    (QHeaderView::ResizeToContents);
  _clickFieldsTable->horizontalHeader()->setStretchLastSection(false);
  _clickReportDialogLayout->addWidget(_clickFieldsTable, row, 0, 1, 2);

  // Closing the dialog directly must leave the checkable Show action in the
  // same state as the window it controls.
  connect(_clickReportDialog, &QDialog::finished, this, [this]() {
    _showClickAct->setChecked(false);
  });

  _clickReportDialog->hide();
  _clickReportDialog->resize(230, 340);
}

///////////////////////////////////////////////////////
// add one metadata row to the click report dialog

QLabel *GuiManager::_addClickReportRow(const string &name, int row,
                                       QLabel **nameLabelOut)
{
  QLabel *nameLabel = new QLabel(name.c_str(), _clickReportDialog);
  if (nameLabelOut != nullptr) {
    *nameLabelOut = nameLabel;
  }
  nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _clickReportDialogLayout->addWidget(nameLabel, row, 0, Qt::AlignLeft);

  QLabel *valueLabel = new QLabel("----", _clickReportDialog);
  valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _clickReportDialogLayout->addWidget(valueLabel, row, 1, Qt::AlignLeft);
  return valueLabel;
}

//////////////////////////////
// create actions for menus

void GuiManager::_createActions()
{

  // show field menu
  _showFieldMenuAct = new QAction(tr("Fields"), this);
  _showFieldMenuAct->setStatusTip(tr("Show field menu"));
  connect(_showFieldMenuAct, &QAction::triggered,
          this, &GuiManager::_showFieldMenu);
  
  // show time control window
  _showTimeControlAct = new QAction(tr("Movie"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, &QAction::triggered,
          this, &GuiManager::_showTimeControl);
  
  // reload data
  
  _reloadAct = new QAction(tr("Reload"), this);
  _reloadAct->setStatusTip(tr("Reload data"));
  connect(_reloadAct, &QAction::triggered,
          this, &GuiManager::_reloadData);

  // clear display
  _clearAct = new QAction(tr("Clear"), this);
  _clearAct->setStatusTip(tr("Clear data"));
  connect(_clearAct, &QAction::triggered, _horiz, &HorizView::clear);
  connect(_clearAct, &QAction::triggered, _vert, &VertView::clear);

  // exit app
  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, &QAction::triggered, this, &GuiManager::close);

  // file chooser for Open
  _openFileAct = new QAction(tr("O&pen"), this);
  _openFileAct->setShortcut(tr("Ctrl+F"));
  _openFileAct->setStatusTip(tr("Open File"));
  connect(_openFileAct, &QAction::triggered, this, &GuiManager::_openFile);

  // file chooser for Save
  _saveFileAct = new QAction(tr("S&ave"), this);
  //_saveFileAct->setShortcut(tr("Ctrl+S"));
  _saveFileAct->setStatusTip(tr("Save File"));
  connect(_saveFileAct, &QAction::triggered, this, &GuiManager::_saveFile);

  // unzoom
  
  _zoomOutAct = new QAction(tr("Unzoom"), this);
  _zoomOutAct->setStatusTip(tr("Unzoom all the way"));
  _zoomOutAct->setEnabled(false);
  
  _zoomBackAct = new QAction(tr("Zoom back"), this);
  _zoomBackAct->setStatusTip(tr("Unzoom to previous view"));
  _zoomBackAct->setEnabled(false);

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.horiz_grids_on_at_startup);
  connect(_gridsAct, &QAction::triggered, _horiz, &HorizView::setGrids);

  // show range rings

  _fixedRingsAct = new QAction(tr("Fixed range Rings"), this);
  _fixedRingsAct->setStatusTip(tr("Turn fixed range rings on/off"));
  _fixedRingsAct->setCheckable(true);
  _fixedRingsAct->setChecked(_params.plot_range_rings_fixed);
  connect(_fixedRingsAct, &QAction::triggered, _horiz, &HorizView::setRingsFixed);

  _dataRingsAct = new QAction(tr("Data-driven range Rings"), this);
  _dataRingsAct->setStatusTip(tr("Turn data-driven range rings on/off"));
  _dataRingsAct->setCheckable(true);
  _dataRingsAct->setChecked(_params.plot_range_rings_from_data);
  connect(_dataRingsAct, &QAction::triggered, _horiz, &HorizView::setRingsDataDriven);
  
  // show values in data window

  _showValueAct = new QAction(tr("Show values"), this);
  _showValueAct->setStatusTip(tr("Show values in data window"));
  _showValueAct->setCheckable(true);
  _showValueAct->setChecked(false);
  connect(_showValueAct, &QAction::triggered, this, &GuiManager::_showValue);

  // show user click in dialog

  _showClickAct = new QAction(tr("Show click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  _showClickAct->setCheckable(true);
  _showClickAct->setChecked(false);
  connect(_showClickAct, &QAction::triggered, this, &GuiManager::showClickDialog);

  // show VERT window

  _showVertAct = new QAction(tr("Show VERT Window"), this);
  _showVertAct->setStatusTip(tr("Show the VERT Window"));
  _showVertAct->setCheckable(true);
  _showVertAct->setChecked(false);
  connect(_showVertAct, &QAction::triggered, _vertWindow, &VertView::show);

  // howto and about
  
  _howtoAct = new QAction(tr("Howto"), this);
  _howtoAct->setStatusTip(tr("Show the application's Howto box"));
  connect(_howtoAct, &QAction::triggered, this, &GuiManager::_howto);

  _aboutAct = new QAction(tr("About"), this);
  _aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAct, &QAction::triggered, this, &GuiManager::_about);

  _aboutQtAct = new QAction(tr("About &Qt"), this);
  _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(_aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

  // save image
  
  _saveImageAct = new QAction(tr("Save-Image"), this);
  _saveImageAct->setStatusTip(
    tr("Save the horizontal and visible vertical views to the image output directory"));
  connect(_saveImageAct, &QAction::triggered,
          this, &GuiManager::_saveViewImages);

}

////////////////
// create menus

void GuiManager::_createMenus()
{

  // file menu
  
  _fileMenu = menuBar()->addMenu(tr("File"));
  _fileMenu->addSeparator();
  _fileMenu->addAction(_openFileAct);
  _fileMenu->addAction(_saveFileAct);
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  // field menu
  
  menuBar()->addAction(_showFieldMenuAct);
  
  // add maps menu
  
  _mapsMenu = menuBar()->addMenu(tr("Maps"));
  _populateMapsMenu();

  // add products menu
  
  _productsMenu = menuBar()->addMenu(tr("Products"));
  _populateProductsMenu();

  // add winds menu
  
  _windsMenu = menuBar()->addMenu(tr("Winds"));
  _populateWindsMenu();

  // add zooms menu
  
  _zoomsMenu = menuBar()->addMenu(tr("Zooms"));
  _populateZoomsMenu();
  menuBar()->addAction(_zoomOutAct);
  
  // time selector
  
  menuBar()->addAction(_showTimeControlAct);

  // add overlay menu
  
  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _populateOverlaysMenu();

  // add show menu
  
  _showMenu = menuBar()->addMenu(tr("Show"));
  _populateShowMenu();

  // misc actions
  
  menuBar()->addAction(_reloadAct);

  _helpMenu = menuBar()->addMenu(tr("Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

/////////////////////////////////////////////////////////////
// populate maps menu

void GuiManager::_populateMapsMenu()
{

  // maps enabled

  _mapsEnabled = _params.maps_enabled_at_startup;
  _mapsEnabledAct = new QAction(tr("Maps enabled"), this);
  _mapsEnabledAct->setStatusTip(tr("Enable / disable all maps"));
  _mapsEnabledAct->setCheckable(true);
  _mapsEnabledAct->setChecked(_mapsEnabled);
  connect(_mapsEnabledAct, &QAction::toggled,
	  this, &GuiManager::_setMapsEnabled);

  _mapsMenu->addAction(_mapsEnabledAct);
  _mapsMenu->addSeparator();
  
  // loop through map entries
  
  for (int imap = 0; imap < _params.maps_n; imap++) {

    Params::map_t &mparams = _params._maps[imap];

    // create action for this entry

    MapMenuItem *item = new MapMenuItem(nullptr, this);
    QAction *act = new QAction;
    item->setMapParams(&mparams);
    item->setMapIndex(imap);
    item->setOverlay(_gd.overlays[imap]);
    item->setAction(act);
    act->setText(mparams.map_code);
    act->setStatusTip(tr("Turn map layer on/off"));
    act->setCheckable(true);
    act->setChecked(mparams.on_at_startup);
    connect(act, &QAction::toggled,
            item, &MapMenuItem::toggled);
    
    // add item for map selection
    
    _mapMenuItems.push_back(item);
    
    // add to maps menu

    _mapsMenu->addAction(act);

  } // imap
  
}

////////////////////////////////////////////////////////////
// enable maps

void GuiManager::_setMapsEnabled(bool enable)
{
  _mapsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> maps enabled? " << (_mapsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate products menu

void GuiManager::_populateProductsMenu()
{

  // products enabled

  _productsEnabled = _params.symprods_enabled_at_startup;
  _gd.prod.products_on = _productsEnabled;
  _productsEnabledAct = new QAction(tr("Products enabled"), this);
  _productsEnabledAct->setStatusTip(tr("Enable / disable all products"));
  _productsEnabledAct->setCheckable(true);
  _productsEnabledAct->setChecked(_productsEnabled);
  connect(_productsEnabledAct, &QAction::toggled,
	  this, &GuiManager::_setProductsEnabled);

  _productsMenu->addAction(_productsEnabledAct);
  _productsMenu->addSeparator();
  
  // loop through wind entries
  
  for (int iprod = 0; iprod < _params.symprod_prod_info_n; iprod++) {
    
    Params::symprod_prod_info_t &prodParams = _params._symprod_prod_info[iprod];
    
    // create action for this entry

    ProdMenuItem *item = new ProdMenuItem;
    const Product *product = _gd.prod_mgr->getProduct(iprod);
    QAction *act = new QAction;
    item->setProdParams(&prodParams);
    item->setProduct(product);
    item->setProdIndex(iprod);
    item->setAction(act);
    act->setText(prodParams.menu_label);
    act->setStatusTip(tr("Turn product on/off"));
    act->setCheckable(true);
    act->setChecked(prodParams.on_by_default);
    connect(act, &QAction::toggled,
            item, &ProdMenuItem::toggled);
    connect(act, &QAction::toggled, this, [this](bool) {
      if (_horiz) {
        _horiz->rerenderProducts();
      }
      if (_timeControl) {
        _timeControl->refreshDataAvailability();
      }
    });
    
    // add item for wind selection
    
    _productMenuItems.push_back(item);

    // add to products menu

    _productsMenu->addAction(act);

  } // iprod
  
}

////////////////////////////////////////////////////////////
// enable products

void GuiManager::_setProductsEnabled(bool enable)
{
  _productsEnabled = enable;
  _gd.prod.products_on = enable;
  if (_horiz) {
    _horiz->rerenderProducts();
  }
  if (enable && _gd.prod_mgr) {
    _gd.prod_mgr->refreshActiveProducts();
  }
  if (_timeControl) {
    _timeControl->refreshDataAvailability();
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> products enabled? " << (_productsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate winds menu

void GuiManager::_populateWindsMenu()
{

  // winds enabled

  _windsEnabled = _params.winds_on_at_startup;
  _windsEnabledAct = new QAction(tr("Winds enabled"), this);
  _windsEnabledAct->setStatusTip(tr("Enable / disable all winds"));
  _windsEnabledAct->setCheckable(true);
  _windsEnabledAct->setChecked(_windsEnabled);
  connect(_windsEnabledAct, &QAction::toggled,
	  this, &GuiManager::_setWindsEnabled);

  _windsMenu->addAction(_windsEnabledAct);
  _windsMenu->addSeparator();
  
  // loop through wind entries
  
  for (int iwind = 0; iwind < _params.winds_n; iwind++) {

    Params::wind_t &wparams = _params._winds[iwind];

    // create action for this entry

    QAction *act = new QAction;
    act->setText(wparams.button_label);
    act->setStatusTip(tr("Turn wind layer on/off"));
    act->setCheckable(true);
    act->setChecked(wparams.on_at_startup);
    connect(act, &QAction::toggled, this,
            [this, iwind](bool checked) {
      _params._winds[iwind].on_at_startup =
        static_cast<tdrp_bool_t>(checked);
      if (_gd.wind_mgr) {
        _gd.wind_mgr->setLayerEnabled(iwind, checked);
        _gd.wind_mgr->requestData(_timeControl->getSelectedTime().utime(),
                                  _vlevelManager.getRequestedLevel(),
                                  _gd.h_win.page);
        _requestVerticalWinds();
      }
      if (_horiz) {
        _horiz->rerenderWinds();
      }
      if (_vert) {
        _vert->rerenderWinds();
      }
      _checkAutomaticImageReady();
    });

    // add to winds menu

    _windsMenu->addAction(act);

  } // iwind
  
}

////////////////////////////////////////////////////////////
// enable winds

void GuiManager::_setWindsEnabled(bool enable)
{
  _windsEnabled = enable;
  _params.winds_on_at_startup = static_cast<tdrp_bool_t>(enable);
  if (_gd.wind_mgr) {
    _gd.wind_mgr->setEnabled(enable);
    _gd.wind_mgr->requestData(_timeControl->getSelectedTime().utime(),
                              _vlevelManager.getRequestedLevel(),
                              _gd.h_win.page);
    _requestVerticalWinds();
  }
  if (_horiz) {
    _horiz->rerenderWinds();
  }
  if (_vert) {
    _vert->rerenderWinds();
  }
  _checkAutomaticImageReady();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> winds enabled? " << (_windsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate zooms menu
// this is exclusive - only 1 item selected at a time

void GuiManager::_populateZoomsMenu()
{

  _zoomsActionGroup = new QActionGroup(_zoomsMenu);
  _zoomsActionGroup->setExclusive(true);

  // unzoom all of the way
  
  connect(_zoomOutAct, &QAction::triggered, this, &GuiManager::_zoomOut);
  _zoomsMenu->addAction(_zoomOutAct);

  // unzoom by 1 level
  
  connect(_zoomBackAct, &QAction::triggered, this, &GuiManager::_zoomBack);
  _zoomsMenu->addAction(_zoomBackAct);
  
  // loop through zoom entries
  
  for (int izoom = 0; izoom < _params.zoom_levels_n; izoom++) {
    
    Params::zoom_level_t &zparams = _params._zoom_levels[izoom];
    
    // create item for this entry
    
    ZoomMenuItem *item = new ZoomMenuItem(nullptr, this);
    item->setZoomParams(&zparams);
    item->setZoomIndex(izoom);

    // create action for the item
    
    QAction *act = new QAction;
    item->setAction(act);
    act->setText(zparams.label);
    act->setStatusTip(tr("Select predefined zoom"));
    act->setCheckable(true);
    if (izoom == _gd.h_win.zoom_level) {
      act->setChecked(true);
    }
    
    // add to exclusive group
    
    _zoomsActionGroup->addAction(act);

    // connect
    
    connect(act, &QAction::toggled,
            item, &ZoomMenuItem::toggled);
    
    // add item for zoom selection
    
    _zoomMenuItems.push_back(item);

    // add to zooms menu
    
    _zoomsMenu->addAction(act);
    
  } // izoom
  
}

/////////////////////////////////////////////////////////////
// populate overlays menu

void GuiManager::_populateOverlaysMenu()
{

  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_fixedRingsAct);
  _overlaysMenu->addAction(_dataRingsAct);
  _overlaysMenu->addSeparator();

}

/////////////////////////////////////////////////////////////
// populate show menu

void GuiManager::_populateShowMenu()
{

  _showMenu->addAction(_showValueAct);
  _showMenu->addAction(_showClickAct);
  _showMenu->addAction(_showVertAct);

}

//////////////////////////////////////////////
// get size of table widget

QSize GuiManager::_getTableWidgetSize(QTableWidget *t,
                                      bool includeVertHeader,
                                      bool includeHorizHeader)
{
  int w = 4;
  if (includeVertHeader) {
    w += t->verticalHeader()->width();
  }
  for (int i = 0; i < t->columnCount(); i++) {
    w += t->columnWidth(i);
  }
  int h = 4;
  if (includeHorizHeader) {
    h += t->horizontalHeader()->height();
  }
  for (int i = 0; i < t->rowCount(); i++) {
    h += t->rowHeight(i);
  }
  return QSize(w, h);
}

//////////////////////////////////////////////
// create the field menu

void GuiManager::_createFieldMenu()
{

  // top-level
  
  _fieldMenu = new QDialog(this);
  _fieldMenu->setWindowTitle("Select active field");
  QPoint pos(0,0);
  _fieldMenu->move(pos);
  // _fieldMenu->show();

  // layout
  
  QBoxLayout *fieldMenuLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, _fieldMenu);
  fieldMenuLayout->setSpacing(0);
  
  // panel
  
  _fieldMenuPanel = new QFrame(_fieldMenu);
  fieldMenuLayout->addWidget(_fieldMenuPanel, Qt::AlignCenter);
  _fieldMenuLayout = new QVBoxLayout;
  _fieldMenuPanel->setLayout(_fieldMenuLayout);

  // table
  
  _fieldTable = new QTableWidget(_fieldMenu);
  _fieldMenuLayout->addWidget(_fieldTable);

  int ncols = _params.num_field_menu_cols;
  int nfields = _params.fields_n;
  int nrows = (int) ((double) (nfields - 1) / ncols) + 1;
  
  _fieldTable->setRowCount(nrows);
  _fieldTable->setColumnCount(ncols);
  
  for (int irow = 0; irow < nrows; irow++) {
    // is this row active?
    for (int icol = 0; icol < ncols; icol++) {
      int fieldNum = irow * ncols + icol;
      FieldTableItem *item = new FieldTableItem(this, "");
      item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      if (fieldNum < _params.fields_n) {
        item->setFieldParams(&_params._fields[fieldNum]);
        if (strlen(_params._fields[fieldNum].group_name) > 0) {
          item->setText(_params._fields[fieldNum].button_label);
          item->setFieldIndex(fieldNum);
          if (strlen(_params._fields[fieldNum].button_label) == 0) {
            // gray out
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setBackground(Qt::lightGray);
            item->setForeground(Qt::gray);
          }
        }
      } else {
        item->setFieldIndex(-1);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setBackground(Qt::lightGray);
        item->setForeground(Qt::gray);
      }
      _fieldTable->setItem(irow, icol, item);
    } // icol
  } // irow

  _fieldTable->verticalHeader()->setVisible(false);
  _fieldTable->horizontalHeader()->setVisible(false);

  // set the column widths, for the strings they contain

  _fieldTable->resizeColumnsToContents();
  _fieldTable->resizeRowsToContents();
  _fieldTable->adjustSize();
  _fieldTable->setAlternatingRowColors(true);
  _fieldTable->setSelectionMode(QAbstractItemView::SingleSelection);
  _fieldTable->setContextMenuPolicy(Qt::CustomContextMenu);
  
  // initialize

  _fieldTable->setCurrentCell(0, 0);

  // set the size
  
  _fieldTable->setMaximumSize(_getTableWidgetSize(_fieldTable, false, false));
  _fieldTable->setMinimumSize(_fieldTable->maximumSize());

  connect(_fieldTable, SIGNAL(cellClicked(const int, const int)),
          this, SLOT(_fieldTableCellClicked(const int, const int)));
  connect(_fieldTable, &QWidget::customContextMenuRequested,
          this, &GuiManager::_showFieldContextMenu);
  
}

/////////////////////////////////////////////////////
// show the configuration controls for one field

void GuiManager::_showFieldContextMenu(const QPoint &pos)
{
  FieldTableItem *item =
    dynamic_cast<FieldTableItem *>(_fieldTable->itemAt(pos));
  if (item == nullptr || item->getFieldIndex() < 0 ||
      item->getFieldIndex() >= _params.fields_n ||
      !(item->flags() & Qt::ItemIsEnabled)) {
    return;
  }

  const int fieldNum = item->getFieldIndex();
  Params::field_t *field = item->getFieldParams();
  if (field == nullptr) {
    return;
  }

  QMenu menu(_fieldTable);
  menu.addSection(QString::fromUtf8(field->button_label));

  QMenu *renderMenu = menu.addMenu(tr("Render method"));
  QActionGroup *renderGroup = new QActionGroup(renderMenu);
  renderGroup->setExclusive(true);
  QAction *noneAction = renderMenu->addAction(tr("None"));
  QAction *polygonsAction = renderMenu->addAction(tr("Polygons"));
  QAction *filledContoursAction =
    renderMenu->addAction(tr("Filled contours"));
  for (QAction *action : {noneAction, polygonsAction,
                          filledContoursAction}) {
    action->setCheckable(true);
    renderGroup->addAction(action);
  }
  noneAction->setChecked(field->render_mode == Params::NONE);
  polygonsAction->setChecked(field->render_mode == Params::POLYGONS);
  filledContoursAction->setChecked(field->render_mode ==
                                   Params::FILLED_CONTOURS);

  connect(noneAction, &QAction::triggered, this,
          [this, field, fieldNum]() {
            field->render_mode = Params::NONE;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->render_method = NONE;
            }
            _applyFieldRenderingChange(fieldNum);
          });
  connect(polygonsAction, &QAction::triggered, this,
          [this, field, fieldNum]() {
            field->render_mode = Params::POLYGONS;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->render_method = POLYGONS;
            }
            _applyFieldRenderingChange(fieldNum);
          });
  connect(filledContoursAction, &QAction::triggered, this,
          [this, field, fieldNum]() {
            field->render_mode = Params::FILLED_CONTOURS;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->render_method = FILLED_CONTOURS;
            }
            _applyFieldRenderingChange(fieldNum);
          });

  QAction *lineContoursAction =
    menu.addAction(tr("Overlay line contours"));
  lineContoursAction->setCheckable(true);
  lineContoursAction->setChecked(field->overlay_line_contours);
  connect(lineContoursAction, &QAction::toggled, this,
          [this, field, fieldNum](bool enabled) {
            field->overlay_line_contours = enabled ? pTRUE : pFALSE;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->overlay_line_contours = enabled;
            }
            _applyFieldRenderingChange(fieldNum);
          });

  QMenu *contourMenu = menu.addMenu(tr("Contour details"));
  QDoubleSpinBox *contourLow =
    addFieldNumberEditor(contourMenu, tr("Low"), field->contour_low, 3);
  QDoubleSpinBox *contourHigh =
    addFieldNumberEditor(contourMenu, tr("High"), field->contour_high, 3);
  QDoubleSpinBox *contourInterval =
    addFieldNumberEditor(contourMenu, tr("Interval"),
                         field->contour_interval, 3, 0.0);
  contourMenu->addSeparator();
  QAction *applyContoursAction = contourMenu->addAction(tr("Apply"));
  connect(applyContoursAction, &QAction::triggered, this,
          [this, field, fieldNum, contourLow, contourHigh,
           contourInterval]() {
            const double low = contourLow->value();
            const double high = contourHigh->value();
            const double interval = contourInterval->value();
            if (high < low || interval <= 0.0) {
              QMessageBox::warning(_fieldMenu, tr("Invalid contours"),
                                   tr("High must not be below low, and interval must be positive."));
              return;
            }

            // Params retain the interactive choices for subsequent menus;
            // MdvReader is the rendering-time copy used by both views.
            field->contour_low = low;
            field->contour_high = high;
            field->contour_interval = interval;
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader != nullptr) {
              reader->cont_low = low;
              reader->cont_high = high;
              reader->cont_interv = interval;
            }
            _applyFieldRenderingChange(fieldNum);
          });

  QAction *columnMaxAction = menu.addAction(tr("Column max mode"));
  columnMaxAction->setCheckable(true);
  columnMaxAction->setChecked(field->composite_mode);
  connect(columnMaxAction, &QAction::toggled, this,
          [this, field, fieldNum](bool enabled) {
            field->composite_mode = enabled ? pTRUE : pFALSE;
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader != nullptr) {
              reader->composite_mode = enabled ? TRUE : FALSE;
            }

            // Column-max is an MDV request option rather than a rendering
            // choice.  Queue a new horizontal read for the active field; the
            // normal timer path waits for an existing read and deliberately
            // leaves the vertical-section request and display unchanged.
            if (fieldNum == _fieldNum) {
              _horizontalRequestHasChanged = true;
            }
          });

  double colorScaleMin = field->contour_low;
  double colorScaleMax = field->contour_high;
  if (_gd.mread[fieldNum] != nullptr) {
    const vector<ColorMap::CmapEntry> &entries =
      _gd.mread[fieldNum]->colorMap.getEntries();
    if (!entries.empty()) {
      colorScaleMin = entries.front().minVal;
      colorScaleMax = entries.back().maxVal;
    }
  }
  QMenu *colorScaleMenu = menu.addMenu(tr("Color scale"));
  QDoubleSpinBox *colorScaleMinEditor =
    addFieldNumberEditor(colorScaleMenu, tr("Minimum"), colorScaleMin, 3);
  QDoubleSpinBox *colorScaleMaxEditor =
    addFieldNumberEditor(colorScaleMenu, tr("Maximum"), colorScaleMax, 3);
  colorScaleMenu->addSeparator();
  QAction *applyColorScaleAction = colorScaleMenu->addAction(tr("Apply"));
  connect(applyColorScaleAction, &QAction::triggered, this,
          [this, fieldNum, colorScaleMinEditor, colorScaleMaxEditor]() {
            const double rangeMin = colorScaleMinEditor->value();
            const double rangeMax = colorScaleMaxEditor->value();
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader == nullptr || rangeMax <= rangeMin ||
                !reader->colorMap.rescaleRange(rangeMin, rangeMax)) {
              QMessageBox::warning(_fieldMenu, tr("Invalid color scale"),
                                   tr("Maximum must be greater than minimum, "
                                      "and the existing color scale must have "
                                      "a valid range."));
              return;
            }

            _applyFieldRenderingChange(fieldNum);
          });
  QAction *resetColorScaleAction = colorScaleMenu->addAction(tr("Reset"));
  MdvReader *colorScaleReader = _gd.mread[fieldNum];
  const bool canResetColorScale =
    colorScaleReader != nullptr &&
    colorScaleReader->colorMap.hasOriginalRange();
  resetColorScaleAction->setEnabled(canResetColorScale);
  connect(resetColorScaleAction, &QAction::triggered, this,
          [this, fieldNum]() {
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader == nullptr || !reader->colorMap.resetRange()) {
              QMessageBox::warning(_fieldMenu, tr("Cannot reset color scale"),
                                   tr("The original color-scale range is not "
                                      "available."));
              return;
            }

            _applyFieldRenderingChange(fieldNum);
          });

  menu.exec(_fieldTable->viewport()->mapToGlobal(pos));
}

/////////////////////////////////////////////////////////////
// redraw retained data after an interactive rendering edit

void GuiManager::_applyFieldRenderingChange(int fieldNum)
{
  if (fieldNum < 0 || fieldNum >= _gd.num_datafields ||
      _gd.mread[fieldNum] == nullptr) {
    return;
  }

  // These settings change only presentation. Reuse retained horizontal and
  // vertical data so menu edits do not initiate MDV requests or disturb the
  // current movie time and selected level.
  if (_horiz != nullptr) {
    _horiz->rerenderGrid();
  }
  if (_vert != nullptr) {
    _vert->displayImage(fieldNum);
  }
}

/////////////////////////////////////
// show the field menu

void GuiManager::_showFieldMenu()
{

  if (_fieldMenu) {
    if (_fieldMenu->isVisible()) {
      _fieldMenu->setVisible(false);
    } else {
      _fieldMenu->setVisible(true);
      _fieldMenu->raise();
      _placeFieldMenu();
    }
  }

}

/////////////////////////////////////
// get (x, y) of top right corner
// for placing child windows

QPoint GuiManager::getTopRightPos()
{
  
  int margin = (frameGeometry().width() - geometry().width()) / 2;
  int titleBarHt = frameGeometry().height() - geometry().height() - margin;
  QPoint pos(x() + (frameGeometry().width()), y() + titleBarHt);

  return pos;

}

/////////////////////////////////////
// place the field menu

void GuiManager::_placeFieldMenu()
{

  if (!_fieldMenu) {
    return;
  }
  
  if (_fieldMenuPlaced) {
    return;
  }
  
  _fieldMenu->move(getTopRightPos());

  _fieldMenuPlaced = true;

}

void GuiManager::_fieldTableCellClicked(int row, int col)
{

  if (_fieldTableRow == _fieldTable->currentRow() &&
      _fieldTableCol == _fieldTable->currentColumn()) {
    // no change
    _fieldHasChanged = false;
    return;
  }

  _prevFieldTableCol = _fieldTableCol;
  _prevFieldTableRow = _fieldTableRow;
  
  _fieldTableCol = col;
  _fieldTableRow = row;
  
  const FieldTableItem *item =
    (const FieldTableItem *) _fieldTable->item(row, col);

  _prevFieldNum = _fieldNum;
  _fieldNum = item->getFieldIndex();
  if (_fieldNum < 0 || _fieldNum >= _params.fields_n) {
    _fieldHasChanged = false;
    return;
  }
  
  _gd.prev_field = _gd.h_win.page;
  _setField(_fieldNum);
  _gd.mread[_fieldNum]->h_data_valid = 0;

  if (_params.debug) {
    const Params::field_t *fparams = item->getFieldParams();
    cerr << "Changing to field: " << fparams->button_label << endl;
    cerr << "              url: " << fparams->url << endl;
    cerr << "         fieldNum: " << _fieldNum << endl;
    cerr << "      field_label: " << _gd.mread[_fieldNum]->field_label << endl;
    cerr << "      button_name: " << _gd.mread[_fieldNum]->button_name << endl;
    cerr << "      legend_name: " << _gd.mread[_fieldNum]->legend_name << endl;
  }

  // _gd.redraw_horiz = true;
  _gd.field_has_changed = true;
  _gd.selected_field = _fieldNum;
  _gd.h_win.page = _fieldNum;

  _fieldHasChanged = true;

  // A completed route describes display geometry rather than a particular
  // field.  Reuse it so the vertical view follows the horizontal field
  // selection without requiring the user to draw the section again.

  if (_vsectionRoute.size() >= 2) {
    _requestVerticalSection();
  }
  
}

/*************************************************************************
 * Set proper flags which switching fields 
 */

void GuiManager::_setField(int value)
{

  static int last_page = 0;
  static int last_value = 0;
  static int cur_value = 0;
  
  if(value < 0) {
    int tmp = last_page;
    last_page = _gd.h_win.page;
    _gd.h_win.page = tmp;
    _gd.v_win.page = tmp;
    tmp = cur_value;
    cur_value = last_value;
    value = last_value;
    last_value = tmp;
  } else {
    last_page = _gd.h_win.page;
    last_value = cur_value;
    cur_value = value;
    _gd.h_win.page = _gd.field_index[value];
    _gd.v_win.page = _gd.field_index[value];
  }
  
  for(int i=0; i < _gd.num_datafields; i++) {
    if(_gd.mread[i]->auto_render == 0) _gd.h_win.redraw_flag[i] = 1;
  }
  
  if(_gd.mread[_gd.h_win.page]->auto_render && 
     _gd.h_win.page_pdev[_gd.h_win.page] != 0 &&
     _gd.h_win.redraw_flag[_gd.h_win.page] == 0) {

#ifdef NOTNOW
    save_h_movie_frame(_gd.movie.cur_frame,
                       _gd.h_win.page_pdev[_gd.h_win.page],
                       _gd.h_win.page);
#endif
    
  }
  
  for(int i=0; i < Constants::MAX_FRAMES; i++) {
    _gd.movie.frame[i].redraw_horiz = 1;
  }
  
  if(_gd.movie.movie_on ) {
    // reset_data_valid_flags(1,0);
  }
  
}


//////////////////////////////////////////////////////////////
// check if we need new H data, and read accordingly
// then trigger rendering if appropriate
  
void GuiManager::_checkAndReadH(MdvReader *mr)
{

  // check for state change, except field
  
  bool stateHasChanged = _checkForStateChange();

  // clear field data if non-field state has changed
  
  if (stateHasChanged) {
    _horiz->clearFieldDataQueue();
  }
  
  // check for field change
  
  if (_fieldHasChanged) {
    stateHasChanged = true;
    _fieldHasChanged = false;
    _timeControl->refreshDataAvailability();
  }

  // Request options such as column-max require new horizontal data even
  // though the selected field, level, time, and display geometry are stable.
  if (_horizontalRequestHasChanged) {
    stateHasChanged = true;
    _horizontalRequestHasChanged = false;
  }
  
  // get new data if needed - data is retrieved in a thread
  
  int frameIndex = _gd.movie.cur_frame;
  if (_gd.movie.cur_frame < 0) {
    frameIndex = _gd.movie.num_frames - 1;
  }

  if (stateHasChanged) {
    const int generation = mr->requestHorizPlane(
      _timeControl->getSelectedTime().utime(),
      _vlevelManager.getRequestedLevel(),
      _gd.h_win.page);
    if (_gd.wind_mgr) {
      _gd.wind_mgr->requestData(_timeControl->getSelectedTime().utime(),
                                _vlevelManager.getRequestedLevel(),
                                _gd.h_win.page);
    }
    if (_automaticImagesActive &&
        _automaticImageGenerator->currentJobIsHorizontal() &&
        _automaticImageGenerator->hasCurrentJob() &&
        _fieldNum == _automaticImageGenerator->currentJob().fieldIndex &&
        mr == _gd.mread[_fieldNum]) {
      _automaticImageGenerator->setExpectedGeneration(generation);
    }
    if (_gd.prod_mgr && frameIndex >= 0 &&
        frameIndex < _gd.movie.num_frames) {
      const movie_frame_t &frame = _gd.movie.frame[frameIndex];
      _gd.prod_mgr->requestData(frame.time_start, frame.time_end,
                                _timeControl->getSelectedTime().utime(),
                                _vlevelManager.getRequestedLevel(),
                                *_horiz->getZoomWorld());
    }
  } else if (_gd.prod_mgr &&
             _gd.prod_mgr->num_products_invalid() > 0) {
    _gd.prod_mgr->refreshActiveProducts();
  }
  
  // check for new data
  
  if (mr->isNewH()) {
    if (!mr->isValidH()) {
      cerr << "ERROR - GuiManager::timerEvent" << endl;
      cerr << "  mr->requestHorizPlane" << endl;
      cerr << "  time_start: " << DateTime::strm(_gd.movie.frame[frameIndex].time_start) << endl;
      cerr << "  time_end: " << DateTime::strm(_gd.movie.frame[frameIndex].time_end) << endl;
      cerr << "  page: " << _gd.h_win.page << endl;
    }
    int frameIndex = _gd.movie.cur_frame;
    if (_gd.movie.cur_frame < 0) {
      frameIndex = _gd.movie.num_frames - 1;
    }
    _horiz->triggerGridRendering(_gd.h_win.page, frameIndex);
    if (_automaticImagesActive &&
        _automaticImageGenerator->currentJobIsHorizontal() &&
        _automaticImageGenerator->hasCurrentJob() &&
        _fieldNum == _automaticImageGenerator->currentJob().fieldIndex &&
        mr == _gd.mread[_fieldNum] && mr->isValidH() &&
        _automaticImageGenerator->acceptDataCompletion(
          mr->getHorizCompletedGeneration())) {
      _checkAutomaticImageReady();
    }
    if (_movieBuildFrame == frameIndex) {
      _movieGridReady = true;
      _checkMovieFrameReady();
    }
    if (_gd.h_win.page < _gd.num_datafields) {
      _vlevelManager.set(*_gd.mread[_gd.h_win.page]);
    }
    _vert->setSelectedHorizHeight(_vlevelManager.getSelectedLevel());
    _vlevelSelector->update();
    _timeControl->refreshDataAvailability();
  }

}

///////////////////////////////////////////
// check for state changes

bool GuiManager::_checkForStateChange()

{

  bool stateHasChanged = false;

  // zoom change?
  
  if (_horiz->checkForZoomChange()) {
    stateHasChanged = true;
  }

  // vlevel change?
  
  if (_vlevelHasChanged) {
    _vlevelHasChanged = false;
    _vert->setSelectedHorizHeight(_vlevelManager.getSelectedLevel());
    stateHasChanged = true;
  }

  // time change

  if (_timeControl->timeHasChanged()) {
    stateHasChanged = true;
  }

  // resize?

  if (_resized) {
    _resized = false;
    stateHasChanged = true;
  }

  // overlays?

  if (_overlaysHaveChanged) {
    _overlaysHaveChanged = false;
    stateHasChanged = true;
  }

  return stateHasChanged;

}

////////////////////////////////////////////
// unzoom to previous zoom

void GuiManager::_zoomBack()
{
  if (isMovieActive()) {
    showMovieInteractionBlocked();
    return;
  }
  _horiz->zoomBackView();
  if (_horiz->getCustomZooms().size() == 0) {
    enableUnzoomButtons(false);
  }
}

////////////////////////////////////////////
// unzoom all the way out

void GuiManager::_zoomOut()
{
  if (isMovieActive()) {
    showMovieInteractionBlocked();
    return;
  }
  _horiz->zoomOutView();
  _horiz->clearCustomZooms();
  enableUnzoomButtons(false);
}

/////////////////////////////////////////////////////////
// handle first time event

void GuiManager::_handleFirstTimerEvent()
{
  _horiz->resize(_horizFrame->width(), _horizFrame->height());
}

////////////////////////////////////////////
// reload the data currently represented by the display

void GuiManager::_reloadData()
{
  // Use the normal request path so a reload waits for any active horizontal
  // read and preserves the displayed image until replacement data arrive.
  _horizontalRequestHasChanged = true;
  if (_gd.h_win.page >= 0 && _gd.h_win.page < _gd.num_datafields &&
      _gd.mread[_gd.h_win.page]) {
    _gd.mread[_gd.h_win.page]->invalidateTimeList();
  }
  if (_gd.prod_mgr) {
    _gd.prod_mgr->reset_product_valid_flags();
    _gd.prod_mgr->refreshActiveProducts();
  }
  if (_gd.wind_mgr) {
    _gd.wind_mgr->invalidate();
  }

  // A visible vertical section is part of the current display state.  Reuse
  // its retained route; request generations prevent an older in-flight result
  // from replacing this explicit reload when it eventually completes.
  if (_vertWindow != nullptr && _vertWindow->isVisible() &&
      _vsectionRoute.size() >= 2) {
    _requestVerticalSection();
  }
}

////////////////////////////////////////////////////////
// handle params file

void GuiManager::_openFile() {
}

void GuiManager::_saveFile() {
}


#ifdef NOTNOW

///////////////////////////////////////////////////
// respond to a change in click location on the HORIZ

void GuiManager::_horizLocationClicked(double xkm, double ykm, 
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
  if (_params.debug) {
    cerr << "    azDeg = " << azDeg << endl;
  }
  
  int rayIndex = ((int) (azDeg * RayLoc::RAY_LOC_RES)) % RayLoc::RAY_LOC_N;
  if (_params.debug) {
    cerr << "    rayIndex = " << rayIndex << endl;
  }
  
  const RadxRay *ray = _rayLoc[rayIndex].ray;
  if (ray == NULL) {
    // no ray data yet
    if (_params.debug) {
      cerr << "    No ray data yet..." << endl;
      cerr << "      active = " << _rayLoc[rayIndex].active << endl;
      cerr << "      startIndex = " << _rayLoc[rayIndex].startIndex << endl;
      cerr << "      endIndex = " << _rayLoc[rayIndex].endIndex << endl;
    }
    return;
  }

  _locationClicked(xkm, ykm, ray);

}

///////////////////////////////////////////////////
// respond to a change in click location on the VERT

void GuiManager::_vertLocationClicked(double xkm, double ykm, 
                                      const RadxRay *closestRay)
  
{

  _locationClicked(xkm, ykm, closestRay);

}

////////////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void GuiManager::_locationClicked(double xkm, double ykm,
                                  const RadxRay *ray)
  
{

  if (_params.debug) {
    cerr << "*** Entering GuiManager::_locationClicked()" << endl;
  }
  
  double rangeKm = sqrt(xkm * xkm + ykm * ykm);
  int gateNum = (int) 
    ((rangeKm - ray->getStartRangeKm()) / ray->getGateSpacingKm() + 0.5);

  if (gateNum < 0 || gateNum >= (int) ray->getNGates()) {
    //user clicked outside of ray
    return;
  }

  if (_params.debug) {
    cerr << "Clicked on location: xkm, ykm: " << xkm << ", " << ykm << endl;
    cerr << "  rangeKm, gateNum: " << rangeKm << ", " << gateNum << endl;
    cerr << "  az, el from ray: "
         << ray->getAzimuthDeg() << ", "
         << ray->getElevationDeg() << endl;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ray->print(cerr);
    }
  }

  DateTime rayTime(ray->getTimeSecs());
  char text[256];
  snprintf(text, 256, "%.4d/%.2d/%.2d",
           rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateClicked->setText(text);

  snprintf(text, 256, "%.2d:%.2d:%.2d.%.3d",
           rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
           ((int) (ray->getNanoSecs() / 1000000)));
  _timeClicked->setText(text);
  
  _setText(text, sizeof(text), "%6.2f", ray->getElevationDeg());
  _elevClicked->setText(text);
  
  _setText(text, sizeof(text), "%6.2f", ray->getAzimuthDeg());
  _azClicked->setText(text);
  
  _setText(text, sizeof(text), "%d", gateNum);
  _gateNumClicked->setText(text);
  
  _setText(text, sizeof(text), "%6.2f", rangeKm);
  _rangeClicked->setText(text);

  if (_radarAltKm > -9990) {
    double gateHtKm = _beamHt.computeHtKm(ray->getElevationDeg(), rangeKm);
    _setText(text, sizeof(text), "%6.2f (km)", gateHtKm);
    _altitudeClicked->setText(text);
  }

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setSelectValue(-9999.0);
    _fields[ii]->setDialogText("----");
  }

  vector<RadxField *> rflds = ray->getFields();
  for (size_t ifield = 0; ifield < rflds.size(); ifield++) {
    const RadxField *field = rflds[ifield];
  
    const string fieldName = field->getName();
    if (fieldName.size() == 0) {
      continue;
    }
    Radx::fl32 *data = (Radx::fl32 *) field->getData();
    double val = data[gateNum];
    const string fieldUnits = field->getUnits();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Field name, selected name: "
	   << fieldName << ", "
	   << _selectedName << endl;
    }
    if (fieldName == _selectedName) {
      char text[128];
      if (fabs(val) < 10000) {
        snprintf(text, 128, "%g %s", val, fieldUnits.c_str());
      } else {
        snprintf(text, 128, "%g %s", -9999.0, fieldUnits.c_str());
      }
      _valueLabel->setText(text);
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Field name, units, val: "
	   << field->getName() << ", "
	   << field->getUnits() << ", "
	   << val << endl;
    }
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      if (_fields[ii]->getName() == fieldName) {
	_fields[ii]->setSelectValue(val);
        char text[128];
        if (fabs(val) > 10000) {
          snprintf(text, 128, "----");
        } else if (fabs(val) > 10) {
          snprintf(text, 128, "%.2f", val);
        } else {
          snprintf(text, 128, "%.3f", val);
        }
        _fields[ii]->setDialogText(text);
      }
    }

  } // ifield

  // update the status panel
  
  _updateStatusPanel(ray);

  // write the click location to FMQ

  _writeClickPointXml2Fmq(ray, rangeKm, gateNum);
  
}

#endif
  
/////////////////////////////////////
// show the time controller dialog

void GuiManager::_showTimeControl()
{

  if (_timeControl) {
    if (_timeControl->isVisible()) {
      _timeControl->setVisible(false);
    } else {
      _timeControl->setVisible(true);
      _timeControl->raise();
      _placeTimeControl();
    }
  }

}

/////////////////////////////////////
// place the time controller dialog

void GuiManager::_placeTimeControl()
{

  if (!_timeControl) {
    return;
  }

  if (_timeControl->x() == 0 && _timeControl->y() == 0) {
    // not yet sized and placed
    return;
  }
  
  if (_timeControlPlaced) {
    return;
  }
  
  int margin = (frameGeometry().width() - geometry().width()) / 2;
  int titleBarHt = frameGeometry().height() - geometry().height() - margin;
  
  QPoint pos;
  pos.setX(x() + margin);
  pos.setY(y() + frameGeometry().height() + titleBarHt);
  _timeControl->move(pos);
  _timeControlPlaced = true;

  _timeControl->resize(width(), _timeControl->height());
  
}

/////////////////////////////////////////////////////////////
// initialize unattended catalog generation and its scheduling policy

void GuiManager::_maybeStartAutomaticImages()
{
  if (_automaticImagesStarted) {
    return;
  }
  _automaticImagesStarted = true;

  QString configurationError;
  if (!_validateAutomaticImageConfiguration(configurationError)) {
    _finishAutomaticImageRun(configurationError);
    return;
  }

  if (!_automaticImageGenerator->configure(
        _params, _gd.num_datafields, configurationError)) {
    _finishAutomaticImageRun(configurationError);
    return;
  }

  switch (_params.images_creation_mode) {
    case Params::CREATE_IMAGES_THEN_EXIT:
      _startAutomaticImageBatch(_imagesStartTime);
      break;

    case Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE:
      _nextAutomaticImageTime = _imagesStartTime;
      _automaticImageScheduleDue();
      break;

    case Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE: {
      const time_t now = std::time(nullptr);
      const time_t interval = _params.images_schedule_interval_secs;
      const time_t delay = _params.images_schedule_delay_secs;
      time_t target = (now / interval) * interval;
      if (target + delay < now) {
        target += interval;
      }
      _nextAutomaticImageTime.set(target);
      _scheduleNextRealtimeImageBatch();
      break;
    }

    default:
      _finishAutomaticImageRun(tr("Unknown images_creation_mode"));
      break;
  }
}

/////////////////////////////////////////////////////////////
// reject schedules which cannot make deterministic progress

bool GuiManager::_validateAutomaticImageConfiguration(QString &error) const
{
  error.clear();
  if (_params.images_request_timeout_secs < 1 ||
      _params.images_request_timeout_secs >
        std::numeric_limits<int>::max() / 1000) {
    error = tr("images_request_timeout_secs is outside the supported range");
    return false;
  }

  QString extension = QString::fromUtf8(
    _params.images_file_name_extension).trimmed().toLower();
  while (extension.startsWith('.')) {
    extension.remove(0, 1);
  }
  const QByteArray format = extension.toLatin1();
  if (extension.isEmpty() ||
      !QImageWriter::supportedImageFormats().contains(format)) {
    error = tr("Unsupported images_file_name_extension: %1")
      .arg(extension);
    return false;
  }
  if (_params.make_images_transparent) {
    static const QStringList opaqueFormats = {
      "bmp", "jpg", "jpeg", "jpe", "pbm", "pgm", "ppm"
    };
    if (opaqueFormats.contains(extension)) {
      error = tr("Image format %1 cannot preserve transparency")
        .arg(extension);
      return false;
    }
    if (!QColor(QString::fromUtf8(_params.background_color)).isValid()) {
      error = tr("background_color must be valid when transparency is enabled");
      return false;
    }
  }

  QString outputRoot = QString::fromUtf8(
    _params.images_output_dir).trimmed();
  outputRoot.replace("$(HOME)", QDir::homePath());
  if (outputRoot.startsWith("$HOME")) {
    outputRoot.replace(0, 5, QDir::homePath());
  }
  if (outputRoot.isEmpty()) {
    error = tr("images_output_dir must not be empty");
    return false;
  }
  auto validOutputSubdir = [](const char *value) {
    const QString subdir = QDir::cleanPath(
      QString::fromUtf8(value).trimmed());
    return !QDir::isAbsolutePath(subdir) && subdir != ".." &&
      !subdir.startsWith("../") && !subdir.contains("/../");
  };
  if (!validOutputSubdir(_params.images_horiz_subdir) ||
      !validOutputSubdir(_params.images_vsect_subdir)) {
    error = tr("Image subdirectories must remain below images_output_dir");
    return false;
  }
  const QString delimiter = QString::fromUtf8(
    _params.images_file_name_delimiter);
  if (delimiter.contains('/') || delimiter.contains('\\')) {
    error = tr("images_file_name_delimiter cannot contain a path separator");
    return false;
  }

  if (_params.images_creation_mode == Params::CREATE_IMAGES_THEN_EXIT ||
      _params.images_creation_mode ==
        Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    if (!_imagesStartTime.isValid()) {
      error = tr("images_archive_start_time is invalid");
      return false;
    }
  }
  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    if (!_imagesEndTime.isValid() || _imagesEndTime < _imagesStartTime) {
      error = tr("images_archive_end_time must not precede the start time");
      return false;
    }
    if (_params.images_archive_interval_secs < 1) {
      error = tr("images_archive_interval_secs must be greater than zero");
      return false;
    }
  }
  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
    if (_params.images_schedule_interval_secs < 1) {
      error = tr("images_schedule_interval_secs must be greater than zero");
      return false;
    }
    if (_params.images_schedule_delay_secs < 0) {
      error = tr("images_schedule_delay_secs must not be negative");
      return false;
    }
  }

  const bool repeatedOutput =
    _params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE ||
    (_params.images_creation_mode ==
       Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE &&
     _imagesEndTime > _imagesStartTime);
  if (repeatedOutput &&
      !_params.images_include_time_part_in_file_name) {
    error = tr("Scheduled image generation requires a time in each file name");
    return false;
  }
  const int scheduleInterval =
    _params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE ?
      _params.images_archive_interval_secs :
      _params.images_schedule_interval_secs;
  if (repeatedOutput && scheduleInterval < 60 &&
      !_params.images_include_seconds_in_time_part) {
    error = tr("Sub-minute image schedules require seconds in file names");
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// start the common field/height/zoom/route plan for one target time

void GuiManager::_startAutomaticImageBatch(const DateTime &imageTime)
{
  if (_automaticImagesActive) {
    return;
  }
  if (!imageTime.isValid()) {
    _finishAutomaticImageRun(tr("Automatic image target time is invalid"));
    return;
  }

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
    _saveAutomaticDisplayState();
  }

  _automaticBatchTime = imageTime;
  _automaticBatchFailureBaseline =
    _automaticImageGenerator->failureCount();
  _automaticBatchAttempted = 0;
  _automaticBatchSaved = 0;
  _automaticBatchTimedOut = 0;
  _automaticBatchProductOmissions = 0;
  _automaticBatchElapsedTimer.start();
  _automaticImagesActive = true;
  _timeControl->configureAutomaticImageTime(imageTime);

  if (!_automaticImageGenerator->start()) {
    _automaticImagesActive = false;
    _finishAutomaticImageRun(tr("No automatic image jobs were configured"));
    return;
  }

  cerr << "Automatic image batch started for "
       << imageTime.asString() << ", "
       << _automaticImageGenerator->jobCount() << " image(s)" << endl;
  _startAutomaticImageJob();
}

/////////////////////////////////////////////////////////////
// preserve the live display while a realtime catalog batch takes control

void GuiManager::_saveAutomaticDisplayState()
{
  if (_automaticDisplayStateSaved) {
    return;
  }
  _automaticSavedField = _fieldNum;
  _automaticSavedZoom = _horiz ? _horiz->getZoomIndex() : 0;
  _automaticSavedHeight = _vlevelManager.getRequestedLevel();
  _automaticSavedStartTime = _timeControl->getStartTime();
  _automaticSavedFrameCount = _timeControl->getNFramesMovie();
  _automaticSavedFrameIntervalSecs = _timeControl->getFrameIntervalSecs();
  _automaticSavedFrameIndex = _timeControl->getFrameIndex();
  _automaticSavedRealtime = _timeControl->isRealtime();
  _automaticSavedVsectionRoute = _vsectionRoute;
  _automaticDisplayStateSaved = true;
}

/////////////////////////////////////////////////////////////
// return control to the accepted display between realtime batches

void GuiManager::_restoreAutomaticDisplayState()
{
  if (!_automaticDisplayStateSaved) {
    return;
  }
  _timeControl->restoreAfterAutomaticImages(
    _automaticSavedStartTime, _automaticSavedFrameCount,
    _automaticSavedFrameIntervalSecs, _automaticSavedFrameIndex,
    _automaticSavedRealtime);
  _vlevelManager.requestLevel(_automaticSavedHeight);
  _vlevelHasChanged = true;
  setZoomIndex(_automaticSavedZoom);
  if (_automaticSavedField >= 0 &&
      _automaticSavedField < _gd.num_datafields) {
    _selectAutomaticImageField(_automaticSavedField);
  }
  _vsectionRoute = _automaticSavedVsectionRoute;
  if (_vsectionRoute.size() >= 2 && _vertWindowDisplayed) {
    _requestVerticalSection();
  }
  _automaticDisplayStateSaved = false;
}

/////////////////////////////////////////////////////////////
// wait until target plus delay without allowing overlapping batches

void GuiManager::_scheduleNextRealtimeImageBatch()
{
  const time_t dueTime = _nextAutomaticImageTime.utime() +
    _params.images_schedule_delay_secs;
  const qint64 waitMillis = std::max<qint64>(
    0, static_cast<qint64>(dueTime - std::time(nullptr)) * 1000);
  const int timerMillis = static_cast<int>(std::min<qint64>(
    waitMillis, std::numeric_limits<int>::max()));
  cerr << "Next automatic image target: "
       << _nextAutomaticImageTime.asString()
       << ", generation begins after "
       << DateTime(dueTime).asString() << endl;
  _automaticImageScheduleTimer->start(timerMillis);
}

/////////////////////////////////////////////////////////////
// dispatch a due archive or realtime target on the GUI thread

void GuiManager::_automaticImageScheduleDue()
{
  if (_automaticImagesActive) {
    return;
  }
  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
    const time_t dueTime = _nextAutomaticImageTime.utime() +
      _params.images_schedule_delay_secs;
    if (std::time(nullptr) < dueTime) {
      _scheduleNextRealtimeImageBatch();
      return;
    }
  }
  _startAutomaticImageBatch(_nextAutomaticImageTime);
}

/////////////////////////////////////////////////////////////
// apply the current generator job to the live display

void GuiManager::_startAutomaticImageJob()
{
  if (!_automaticImagesActive ||
      !_automaticImageGenerator->hasCurrentJob()) {
    if (_automaticImagesActive) {
      _completeAutomaticImageBatch();
    }
    return;
  }
  const AutomaticImageGenerator::Job &job =
    _automaticImageGenerator->currentJob();
  ++_automaticBatchAttempted;
  const int fieldIndex = job.fieldIndex;
  if (fieldIndex < 0 || _gd.mread[fieldIndex] == nullptr) {
    _automaticImageGenerator->recordFailure();
    cerr << "ERROR - automatic image: field " << fieldIndex
         << " has no MDV reader" << endl;
    _automaticImageGenerator->advance();
    QTimer::singleShot(0, this, [this]() {
      _startAutomaticImageJob();
    });
    return;
  }

  if (_automaticImageGenerator->currentJobIsVertical()) {
    _startAutomaticVerticalJob();
  } else {
    _startAutomaticHorizontalJob();
  }
}

/////////////////////////////////////////////////////////////
// apply one horizontal height/zoom/field job

void GuiManager::_startAutomaticHorizontalJob()
{
  const AutomaticImageGenerator::Job &job =
    _automaticImageGenerator->currentJob();
  const int fieldIndex = job.fieldIndex;

  // Apply both outer dimensions before selecting the field. Their normal
  // invalidation paths then let the timer issue one request containing the
  // complete height, geometry, and field state for this job.
  _vlevelManager.requestLevel(job.heightKm);
  _vlevelHasChanged = true;
  setZoomIndex(job.zoomIndex);
  _selectAutomaticImageField(fieldIndex);

  const QString fieldName = QString::fromUtf8(
    _params._fields[fieldIndex].field_name);
  statusBar()->showMessage(
    tr("Creating automatic image %1 of %2: %3 at %4 km, zoom %5")
      .arg(_automaticImageGenerator->currentJobNumber())
      .arg(_automaticImageGenerator->jobCount())
      .arg(fieldName).arg(job.heightKm).arg(job.zoomName));
  if (_params.images_debug != Params::IMAGE_DEBUG_OFF) {
    cerr << "Automatic image: requesting field " << fieldName.toStdString()
         << " at " << job.heightKm << " km, zoom "
         << job.zoomName.toStdString() << " ("
         << _automaticImageGenerator->currentJobNumber() << " of "
         << _automaticImageGenerator->jobCount() << ")" << endl;
  }

  _automaticImageTimeoutTimer->start(
    _params.images_request_timeout_secs * 1000);
}

/////////////////////////////////////////////////////////////
// request one configured route for the selected field

void GuiManager::_startAutomaticVerticalJob()
{
  const AutomaticImageGenerator::Job &job =
    _automaticImageGenerator->currentJob();
  VsectionRoute route;
  QString error;
  if (!_buildAutomaticVsectionRoute(route, error)) {
    _failAutomaticImage(error);
    return;
  }

  _selectAutomaticImageField(job.fieldIndex, false);
  _vsectionRoute = route;
  const int generation = _requestVerticalSection();
  if (generation < 0) {
    _failAutomaticImage(tr("Could not start vertical-section request"));
    return;
  }
  _automaticImageGenerator->setExpectedGeneration(generation);

  const QString fieldName = QString::fromUtf8(
    _params._fields[job.fieldIndex].field_name);
  statusBar()->showMessage(
    tr("Creating automatic image %1 of %2: %3, vertical section %4")
      .arg(_automaticImageGenerator->currentJobNumber())
      .arg(_automaticImageGenerator->jobCount())
      .arg(fieldName)
      .arg(_automaticImageGenerator->currentRoute().name));
  if (_params.images_debug != Params::IMAGE_DEBUG_OFF) {
    cerr << "Automatic image: requesting vertical field "
         << fieldName.toStdString() << ", route "
         << _automaticImageGenerator->currentRoute().name.toStdString()
         << " (" << _automaticImageGenerator->currentJobNumber() << " of "
         << _automaticImageGenerator->jobCount() << ")" << endl;
  }
  _automaticImageTimeoutTimer->start(
    _params.images_request_timeout_secs * 1000);
}

/////////////////////////////////////////////////////////////
// update field state without synthesizing a table interaction

void GuiManager::_selectAutomaticImageField(int fieldIndex,
                                            bool requestHorizontal)
{
  _prevFieldNum = _fieldNum;
  _fieldNum = fieldIndex;
  _gd.prev_field = _gd.h_win.page;
  _setField(fieldIndex);
  _gd.selected_field = fieldIndex;
  _gd.h_win.page = fieldIndex;
  _gd.field_has_changed = requestHorizontal;
  _fieldHasChanged = requestHorizontal;

  // Keep the visible selector consistent for diagnostics without routing the
  // programmatic change back through its user-click handler.
  const int ncols = std::max(1, _params.num_field_menu_cols);
  _fieldTableRow = fieldIndex / ncols;
  _fieldTableCol = fieldIndex % ncols;
  if (_fieldTable) {
    _fieldTable->setCurrentCell(_fieldTableRow, _fieldTableCol);
  }
}

/////////////////////////////////////////////////////////////
// save only after the grid and the configured Product policy are ready

void GuiManager::_checkAutomaticImageReady()
{
  if (!_automaticImagesActive ||
      !_automaticImageGenerator->hasCurrentJob()) {
    return;
  }

  const bool productsRelevant =
    !_automaticImageGenerator->currentJobIsVertical() &&
    _gd.prod.products_on && _gd.prod_mgr;
  const bool productsReady = !productsRelevant ||
    (_params.images_require_all_symprods ?
       _gd.prod_mgr->activeProductsReady() :
       _gd.prod_mgr->activeProductRequestsSettled());
  const bool windsRelevant = _gd.wind_mgr && _gd.wind_mgr->isEnabled();
  const bool windsReady = !windsRelevant ||
    (_automaticImageGenerator->currentJobIsVertical() ?
       _gd.wind_mgr->activeVerticalWindsSettled() :
       _gd.wind_mgr->activeWindsReady());
  if (!_automaticImageGenerator->readyToSave(productsReady && windsReady)) {
    return;
  }

  _automaticImageTimeoutTimer->stop();
  if (productsRelevant && !_params.images_require_all_symprods) {
    const int unavailable =
      _gd.prod_mgr->numActiveProductsUnavailable();
    if (unavailable > 0) {
      ++_automaticBatchProductOmissions;
      cerr << "WARNING - automatic image: saving without "
           << unavailable << " unavailable symbolic product(s)" << endl;
    }
  }
  QString outputPath;
  QString error;
  const bool saved = _automaticImageGenerator->currentJobIsVertical() ?
    _saveAutomaticVerticalImage(outputPath, error) :
    _saveAutomaticHorizontalImage(outputPath, error);
  if (!saved) {
    _automaticImageGenerator->recordFailure();
    cerr << "ERROR - automatic image: " << error.toStdString() << endl;
  } else {
    ++_automaticBatchSaved;
    cerr << "Automatic image saved: " << outputPath.toStdString() << endl;
  }

  _automaticImageGenerator->advance();
  QTimer::singleShot(0, this, [this]() {
    _startAutomaticImageJob();
  });
}

/////////////////////////////////////////////////////////////
// fail the current job immediately and continue the unattended batch

void GuiManager::_failAutomaticImage(const QString &error)
{
  _automaticImageTimeoutTimer->stop();
  _automaticImageGenerator->recordFailure();
  cerr << "ERROR - automatic image: " << error.toStdString() << endl;
  _automaticImageGenerator->advance();
  QTimer::singleShot(0, this, [this]() {
    _startAutomaticImageJob();
  });
}

/////////////////////////////////////////////////////////////
// abandon one field rather than hanging an unattended run forever

void GuiManager::_automaticImageTimedOut()
{
  if (!_automaticImagesActive ||
      !_automaticImageGenerator->hasCurrentJob()) {
    return;
  }
  const AutomaticImageGenerator::Job timedOutJob =
    _automaticImageGenerator->currentJob();
  const int timedOutField = timedOutJob.fieldIndex;
  cerr << "ERROR - automatic image: timed out waiting for field "
       << _params._fields[timedOutField].field_name;
  if (_automaticImageGenerator->currentJobIsVertical()) {
    cerr << ", vertical section "
         << _automaticImageGenerator->currentRoute().name.toStdString();
  } else {
    cerr << " at " << timedOutJob.heightKm << " km, zoom "
         << timedOutJob.zoomName.toStdString();
  }
  cerr << endl;
  if (_gd.mread[timedOutField]) {
    const int expectedGeneration =
      _automaticImageGenerator->expectedGeneration();
    if (_automaticImageGenerator->currentJobIsVertical()) {
      const int generation = expectedGeneration >= 0 ? expectedGeneration :
        _gd.mread[timedOutField]->getVertGeneration();
      _gd.mread[timedOutField]->cancelVertRequest(generation);
    } else {
      const int generation = expectedGeneration >= 0 ? expectedGeneration :
        _gd.mread[timedOutField]->getHorizGeneration();
      _gd.mread[timedOutField]->cancelHorizRequest(generation);
    }
  }
  _automaticImageGenerator->recordFailure();
  ++_automaticBatchTimedOut;
  _automaticImageGenerator->advance();
  QTimer::singleShot(0, this, [this]() {
    _startAutomaticImageJob();
  });
}

/////////////////////////////////////////////////////////////
// write one catalog image using the configured naming policy

bool GuiManager::_saveAutomaticHorizontalImage(QString &path,
                                                QString &error)
{
  path.clear();
  error.clear();
  if (!_horiz || !_automaticImageGenerator->hasCurrentJob()) {
    error = tr("The horizontal view or selected field is unavailable");
    return false;
  }

  const int fieldIndex =
    _automaticImageGenerator->currentJob().fieldIndex;
  MdvReader *reader = _gd.mread[fieldIndex];
  if (!reader || !reader->isValidH() ||
      reader->getHorizCompletedGeneration() !=
        _automaticImageGenerator->expectedGeneration()) {
    error = tr("The completed MDV data do not match the requested field");
    return false;
  }

  QImage image = _horiz->grab().toImage();
  if (image.isNull()) {
    error = tr("Could not capture the horizontal view");
    return false;
  }
  return _publishAutomaticImage(
    image, DateTime(reader->h_date), true, path, error);
}

/////////////////////////////////////////////////////////////
// capture the completed vertical-section widget synchronously

bool GuiManager::_saveAutomaticVerticalImage(QString &path,
                                              QString &error)
{
  path.clear();
  error.clear();
  if (!_vert || !_vertWindow ||
      !_automaticImageGenerator->currentJobIsVertical()) {
    error = tr("The vertical view or vertical job is unavailable");
    return false;
  }
  const int fieldIndex =
    _automaticImageGenerator->currentJob().fieldIndex;
  MdvReader *reader = _gd.mread[fieldIndex];
  if (!reader || !reader->isValidV() ||
      reader->getVertGeneration() !=
        _automaticImageGenerator->expectedGeneration()) {
    error = tr("The completed vertical data do not match the requested field");
    return false;
  }

  // The automatic window may remain hidden, but activating its layouts gives
  // the rendering widget the same configured geometry it has when shown
  // interactively. QWidget::render then paints that exact state synchronously.
  _vertWindow->ensurePolished();
  if (_vertWindow->layout()) {
    _vertWindow->layout()->activate();
  }
  if (_vertWindow->centralWidget() &&
      _vertWindow->centralWidget()->layout()) {
    _vertWindow->centralWidget()->layout()->activate();
  }
  if (_vert->width() < 1 || _vert->height() < 1) {
    error = tr("The vertical view has no drawable area");
    return false;
  }
  QImage image(_vert->size(), QImage::Format_ARGB32_Premultiplied);
  image.fill(QColor(_params.background_color));
  _vert->render(&image);
  return _publishAutomaticImage(
    image, DateTime(reader->v_date), false, path, error);
}

/////////////////////////////////////////////////////////////
// atomically publish one captured view and its configured metadata

bool GuiManager::_publishAutomaticImage(const QImage &image,
                                         const DateTime &imageTime,
                                         bool writeGeoXml,
                                         QString &path,
                                         QString &error)
{
  QByteArray format;
  if (!_automaticImageGenerator->prepareOutput(
        _params, imageTime, path, format, error)) {
    return false;
  }

  QImage outputImage = image;
  if (_params.make_images_transparent) {
    // Match CIDD's catalog convention: the configured canvas background is
    // the transparency key. Exact matching deliberately leaves rendered and
    // anti-aliased pixels intact instead of guessing which colors are data.
    outputImage = image.convertToFormat(QImage::Format_ARGB32);
    const QRgb background = QColor(_params.background_color).rgb();
    const QRgb rgbMask = 0x00ffffff;
    for (int yy = 0; yy < outputImage.height(); ++yy) {
      QRgb *line = reinterpret_cast<QRgb *>(outputImage.scanLine(yy));
      for (int xx = 0; xx < outputImage.width(); ++xx) {
        if ((line[xx] & rgbMask) == (background & rgbMask)) {
          line[xx] = qRgba(qRed(line[xx]), qGreen(line[xx]),
                           qBlue(line[xx]), 0);
        }
      }
    }
  }

  // QSaveFile publishes the catalog image with one atomic rename. Readers
  // therefore see either the previous complete image or the new one, never a
  // partially encoded file while an unattended update is in progress.
  QSaveFile output(path);
  if (!output.open(QIODevice::WriteOnly)) {
    error = tr("Cannot open image for writing: %1").arg(path);
    return false;
  }
  if (!outputImage.save(&output, format.constData())) {
    output.cancelWriting();
    error = tr("Cannot encode image: %1").arg(path);
    return false;
  }
  if (!output.commit()) {
    error = tr("Cannot publish image: %1").arg(path);
    return false;
  }

  // Publish optional sidecars before _latest_data_info. The latter is the
  // catalog notification consumed by downstream processes, so it must never
  // advertise an image whose required companion metadata is incomplete.
  if (writeGeoXml && _params.images_output_geo_xml &&
      !_writeAutomaticGeoXml(path, outputImage, error)) {
    return false;
  }
  if (_params.images_write_latest_data_info &&
      !_writeAutomaticLatestDataInfo(
        path, format, imageTime, error)) {
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// convert one configured route into the display and MDV coordinate forms

bool GuiManager::_buildAutomaticVsectionRoute(VsectionRoute &route,
                                               QString &error) const
{
  route.clear();
  error.clear();
  if (!_automaticImageGenerator->currentJobIsVertical()) {
    error = tr("No automatic vertical-section job is active");
    return false;
  }

  for (const QPointF &point :
       _automaticImageGenerator->currentRoute().points) {
    double x = point.x();
    double y = point.y();
    double lat = 0.0;
    double lon = 0.0;
    if (_params.images_vsection_waypts_in_latlon) {
      lat = point.x();
      lon = point.y();
      _gd.proj.latlon2xy(lat, lon, x, y);
    } else {
      _gd.proj.xy2latlon(x, y, lat, lon);
    }
    route.append(x, y, lat, lon);
  }
  if (route.size() < 2 || route.totalLengthKm() <= 0.0) {
    error = tr("Vertical section %1 has zero usable length")
      .arg(_automaticImageGenerator->currentRoute().name);
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// write CIDD-compatible georeferencing for the complete image canvas

bool GuiManager::_writeAutomaticGeoXml(const QString &imagePath,
                                        const QImage &image,
                                        QString &error) const
{
  const WorldPlot *world = _horiz ? _horiz->getZoomWorld() : nullptr;
  if (!world || image.width() < 1 || image.height() < 1) {
    error = tr("Cannot compute georeferencing for %1").arg(imagePath);
    return false;
  }

  const qreal deviceRatio = image.devicePixelRatio() > 0.0 ?
    image.devicePixelRatio() : 1.0;
  const double logicalWidth = image.width() / deviceRatio;
  const double logicalHeight = image.height() / deviceRatio;
  const double x0 = world->getXWorld(0.0);
  const double x1 = world->getXWorld(logicalWidth);
  const double y0 = world->getYWorld(0.0);
  const double y1 = world->getYWorld(logicalHeight);
  const double west = std::min(x0, x1);
  const double east = std::max(x0, x1);
  const double south = std::min(y0, y1);
  const double north = std::max(y0, y1);
  const double dx = (east - west) / image.width();
  const double dy = (north - south) / image.height();

  const QFileInfo imageInfo(imagePath);
  const QString xmlPath = imageInfo.dir().filePath(
    imageInfo.completeBaseName() + ".xml");
  QSaveFile output(xmlPath);
  if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
    error = tr("Cannot open georeferencing XML for writing: %1")
      .arg(xmlPath);
    return false;
  }

  auto number = [](double value) {
    return QString::number(value, 'g', 15);
  };
  QXmlStreamWriter xml(&output);
  xml.setAutoFormatting(true);
  xml.writeStartElement("GeoImage");
  xml.writeTextElement("px", QString::number(image.width()));
  xml.writeTextElement("py", QString::number(image.height()));
  xml.writeTextElement("pdx", number(dx));
  xml.writeTextElement("pdy", number(dy));

  if (_params.images_use_latlon_in_geo_xml ||
      world->getProjection().getProjType() == Mdvx::PROJ_LATLON) {
    double southLat = south;
    double westLon = west;
    double northLat = north;
    double eastLon = east;
    if (world->getProjection().getProjType() != Mdvx::PROJ_LATLON) {
      MdvxProj projection = world->getProjection();
      projection.xy2latlon(west, south, southLat, westLon);
      projection.xy2latlon(east, north, northLat, eastLon);
    }
    xml.writeStartElement("LatLonBox");
    xml.writeTextElement("north", number(northLat));
    xml.writeTextElement("south", number(southLat));
    xml.writeTextElement("east", number(eastLon));
    xml.writeTextElement("west", number(westLon));
    xml.writeEndElement();
  } else {
    xml.writeStartElement("ProjectionBox");
    xml.writeTextElement("north", number(north));
    xml.writeTextElement("south", number(south));
    xml.writeTextElement("east", number(east));
    xml.writeTextElement("west", number(west));
    xml.writeEndElement();
  }
  xml.writeEndElement();
  xml.writeCharacters("\n");

  if (xml.hasError() || !output.commit()) {
    error = tr("Cannot publish georeferencing XML: %1").arg(xmlPath);
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// advertise the image only after all configured outputs are complete

bool GuiManager::_writeAutomaticLatestDataInfo(
        const QString &imagePath,
        const QByteArray &format,
        const DateTime &imageTime,
        QString &error) const
{
  const QFileInfo imageInfo(imagePath);
  const QString outputRoot = imageInfo.absolutePath();
  const QString relativePath = imageInfo.fileName();

  const std::string root = outputRoot.toStdString();
  const std::string extension = format.toStdString();
  DsLdataInfo latest(root);
  if (_params.images_debug != Params::IMAGE_DEBUG_OFF) {
    latest.setDebug();
  }
  latest.setLatestTime(imageTime.utime());
  latest.setWriter("Lucid");
  latest.setDataFileExt(extension);
  latest.setDataType(extension);
  latest.setUserInfo1("Lucid Output Image");
  latest.setRelDataPath(relativePath.toStdString());
  if (latest.write(imageTime.utime())) {
    error = tr("Cannot write _latest_data_info in %1").arg(outputRoot);
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// close one batch and select the next scheduling action

void GuiManager::_completeAutomaticImageBatch()
{
  _automaticImageTimeoutTimer->stop();
  _automaticImagesActive = false;
  const int batchFailures = std::max(
    0, _automaticImageGenerator->failureCount() -
       _automaticBatchFailureBaseline);
  _automaticTotalFailures += batchFailures;
  _automaticTotalAttempted += _automaticBatchAttempted;
  _automaticTotalSaved += _automaticBatchSaved;
  _automaticTotalTimedOut += _automaticBatchTimedOut;
  _automaticTotalProductOmissions += _automaticBatchProductOmissions;
  ++_automaticCompletedBatches;

  cerr << "Automatic image batch for " << _automaticBatchTime.asString()
       << " completed: attempted=" << _automaticBatchAttempted
       << ", saved=" << _automaticBatchSaved
       << ", failed=" << batchFailures
       << ", timed_out=" << _automaticBatchTimedOut
       << ", images_with_product_omissions="
       << _automaticBatchProductOmissions
       << ", elapsed_secs="
       << (_automaticBatchElapsedTimer.isValid() ?
             _automaticBatchElapsedTimer.elapsed() / 1000.0 : 0.0)
       << endl;

  if (_params.images_creation_mode == Params::CREATE_IMAGES_THEN_EXIT) {
    _finishAutomaticImageRun(QString());
    return;
  }

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    _nextAutomaticImageTime = _automaticBatchTime +
      _params.images_archive_interval_secs;
    if (_nextAutomaticImageTime <= _imagesEndTime) {
      QTimer::singleShot(0, this, [this]() {
        _automaticImageScheduleDue();
      });
    } else {
      _finishAutomaticImageRun(QString());
    }
    return;
  }

  // A realtime run is intentionally long-lived. Coalesce any missed
  // intervals to the newest target already due instead of creating an
  // unbounded queue of obsolete batches.
  const time_t interval = _params.images_schedule_interval_secs;
  const time_t delay = _params.images_schedule_delay_secs;
  const time_t now = std::time(nullptr);
  time_t nextTarget = _automaticBatchTime.utime() + interval;
  if (now >= delay) {
    const time_t latestDueTarget = ((now - delay) / interval) * interval;
    nextTarget = std::max(nextTarget, latestDueTarget);
  }
  _nextAutomaticImageTime.set(nextTarget);

  // Do not restore and immediately re-take the live display when the next
  // coalesced batch is already due. The original snapshot remains owned by
  // the run and is restored once the scheduler reaches a genuine wait.
  if (nextTarget + delay > now) {
    _restoreAutomaticDisplayState();
  }
  _scheduleNextRealtimeImageBatch();
}

/////////////////////////////////////////////////////////////
// terminate a finite run, or a realtime run with invalid configuration

void GuiManager::_finishAutomaticImageRun(const QString &fatalError)
{
  _automaticImageTimeoutTimer->stop();
  _automaticImageScheduleTimer->stop();
  _automaticImagesActive = false;
  if (!fatalError.isEmpty()) {
    ++_automaticTotalFailures;
    cerr << "ERROR - automatic image generation: "
         << fatalError.toStdString() << endl;
  }
  _restoreAutomaticDisplayState();

  const bool success = _automaticTotalFailures == 0;
  if (success) {
    cerr << "Automatic image generation completed successfully";
  } else {
    cerr << "Automatic image generation completed with failures";
  }
  cerr << ": batches=" << _automaticCompletedBatches
       << ", attempted=" << _automaticTotalAttempted
       << ", saved=" << _automaticTotalSaved
       << ", failed=" << _automaticTotalFailures
       << ", timed_out=" << _automaticTotalTimedOut
       << ", images_with_product_omissions="
       << _automaticTotalProductOmissions
       << endl;
  QTimer::singleShot(0, qApp, [success]() {
    QCoreApplication::exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
  });
}

/////////////////////////////////////////////////////
// creating image files in realtime mode

void GuiManager::_createRealtimeImageFiles()
{

  // int interval = _params.images_schedule_interval_secs;
  // int delay = _params.images_schedule_delay_secs;
  // time_t latestRayTime = _readerRayTime.utime();
  
  // initialize the schedule if required

  if (_imagesScheduledTime.utime() == 0) {
    // if (latestRayTime == 0) {
    //   // no data yet
    //   return;
    // }
    // time_t nextUtime = ((latestRayTime / interval) + 1) * interval;
    // _imagesScheduledTime.set(nextUtime + delay);
    // if (_params.debug) {
    //   cerr << "Next scheduled time for images: " 
    //        << _imagesScheduledTime.asString() << endl;
    // }
  }
  
  // if (_readerRayTime > _imagesScheduledTime) {
    
  //   // create images

  //   _timeControl->setArchiveEndTime(_imagesScheduledTime - delay);
  //   _timeControl->setArchiveStartTime
  //     (_timeControl->getArchiveEndTime() - _imagesScanIntervalSecs);
  //   _createImageFilesAllSweeps();

  //   // set next scheduled time
    
  //   time_t nextUtime = ((latestRayTime / interval) + 1) * interval;
  //   _imagesScheduledTime.set(nextUtime + delay);
  //   if (_params.debug) {
  //     cerr << "Next scheduled time for images: "
  //          << _imagesScheduledTime.asString() << endl;
  //   }

  // }

}

/////////////////////////////////////////////////////
// creating image files in archive mode

void GuiManager::_createArchiveImageFiles()
{

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_THEN_EXIT) {
    
    if (_archiveFileList.size() > 0) {

      // using input file list to drive image generation

      while (_archiveFileList.size() > 0) {
        _createImageFilesAllLevels();
        _archiveFileList.erase(_archiveFileList.begin(), 
                               _archiveFileList.begin() + 1);
      }
      
    } else {
      
      // using archive time to drive image generation
      
      while (_timeControl->getStartTime() <= _imagesEndTime) {
        _createImageFilesAllLevels();
        // _timeControl->setStartTime(_timeControl->getStartTime() +
        //                            _imagesScanIntervalSecs);
      }
      
    }
    
  } else if (_params.images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    
    for (DateTime stime = _imagesStartTime;
         stime <= _imagesEndTime;
         stime += _params.images_schedule_interval_secs) {
      
      // _timeControl->setStartTime(stime);
      // _timeControl->setEndTime(_timeControl->getStartTime() +
      //                          _imagesScanIntervalSecs);
      
      _createImageFilesAllLevels();
      
    } // stime
    
  } // if (_params.images_creation_mode ...

}

/////////////////////////////////////////////////////
// creating one image per field, for each level

void GuiManager::_createImageFilesAllLevels()
{
  
  // if (_params.images_set_sweep_index_list) {
    
  //   for (int ii = 0; ii < _params.images_sweep_index_list_n; ii++) {
  //     int index = _params._images_sweep_index_list[ii];
  //     if (index >= 0 && index < (int) _vol.getNSweeps()) {
  //       // _sweepManager.setFileIndex(index);
  //       _createImageFiles();
  //     }
  //   }
    
  // } else {
    
  //   for (size_t index = 0; index < _vol.getNSweeps(); index++) {
  //     // _sweepManager.setFileIndex(index);
  //     _createImageFiles();
  //   }
    
  // }

}

/////////////////////////////////////////////////////
// creating one image per field

void GuiManager::_createImageFiles()
{

  if (_params.debug) {
    cerr << "GuiManager::_createImageFiles()" << endl;
  }

  PMU_auto_register("createImageFiles");

  // plot the data

  // _horiz->setStartOfSweep(true);
  // _vert->setStartOfSweep(true);
  // _handleArchiveData();

  // set times from plots

#ifdef NOTYET
  
  if (_vertMode) {
    _plotStartTime = _vert->getPlotStartTime();
    _plotEndTime = _vert->getPlotEndTime();
  } else {
    _plotStartTime = _horiz->getPlotStartTime();
    _plotEndTime = _horiz->getPlotEndTime();
  }

  // save current field

  int fieldNum = _fieldNum;
  
  // loop through fields

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    
    // select field
    
    _changeField(ii, false);
    
    // create plot
    
    if (_params.debug) {
      cerr << "Creating image for field: " << getSelectedFieldLabel() << endl;
    }
    
    // save image for plot
    
    _saveImageToFile(false);

  }

  // change field back

  _changeField(fieldNum, false);

#endif
  
  if (_params.debug) {
    cerr << "Done creating image files" << endl;
  }

}

string GuiManager::_getOutputPath(bool interactive, string &outputDir, string fileExt)
{
  // set times from plots

  if (_vertMode) {
    // _plotStartTime = _vert->getPlotStartTime();
    // _plotEndTime = _vert->getPlotEndTime();
  } else {
    // _plotStartTime = _horiz->getPlotStartTime();
    // _plotEndTime = _horiz->getPlotEndTime();
  }

  // compute output dir
  outputDir = _params.images_output_dir;
  char dayStr[1024];
  if (_params.images_write_to_day_dir)
  {
    snprintf(dayStr, 1024, "%.4d%.2d%.2d", _plotStartTime.getYear(), _plotStartTime.getMonth(), _plotStartTime.getDay());
    outputDir += PATH_DELIM;
    outputDir += dayStr;
  }

  // make sure output dir exists

  if (ta_makedir_recurse(outputDir.c_str())) {
    string errmsg("Cannot create output dir: " + outputDir);
    cerr << "ERROR - GuiManager::_saveImageToFile()" << endl;
    cerr << "  " << errmsg << endl;
    if (interactive) {
      QMessageBox::critical(this, "Error", errmsg.c_str());
    }
    return(NULL);
  }

  // compute file name

  string fileName;

  // category

  if (strlen(_params.images_file_name_category) > 0) {
    fileName += _params.images_file_name_category;
  }

  // platform

  if (strlen(_params.images_file_name_platform) > 0) {
    fileName += _params.images_file_name_delimiter;
    fileName += _params.images_file_name_platform;
  }

  // time

  if (_params.images_include_time_part_in_file_name) {
    fileName += _params.images_file_name_delimiter;
    char timeStr[1024];
    if (_params.images_include_seconds_in_time_part) {
      snprintf(timeStr, 1024, "%.4d%.2d%.2d%.2d%.2d%.2d",
               _plotStartTime.getYear(),
               _plotStartTime.getMonth(),
               _plotStartTime.getDay(),
               _plotStartTime.getHour(),
               _plotStartTime.getMin(),
               _plotStartTime.getSec());
    } else {
      snprintf(timeStr, 1024, "%.4d%.2d%.2d%.2d%.2d",
               _plotStartTime.getYear(),
               _plotStartTime.getMonth(),
               _plotStartTime.getDay(),
               _plotStartTime.getHour(),
               _plotStartTime.getMin());
    }
    fileName += timeStr;
  }

  // field label

  // // if (_params.images_include_field_label_in_file_name) {
  // //   fileName += _params.images_file_name_delimiter;
  // //   fileName += getSelectedFieldLabel();
  // // }
    
  // scan Id

  // if (_params.images_include_scan_id_in_file_name) {
  //   fileName += _params.images_file_name_delimiter;
  //   fileName += std::to_string(_vol.getScanId());
  // }

  // scan type

  // if (_params.images_include_scan_type_in_file_name) {
  //   fileName += _params.images_file_name_delimiter;
  //   string scanType = "SUR";
  //   if (_vol.getSweeps().size() > 0) {
  //     Radx::SweepMode_t predomSweepMode = _vol.getPredomSweepMode();
  //     scanType = Radx::sweepModeToShortStr(predomSweepMode);
  //   }
  //   fileName += scanType;
  // }

  // extension

  fileName += ".";
  fileName += fileExt;

  // compute output path

  string outputPath(outputDir);
  outputPath += PATH_DELIM;
  outputPath += fileName;

  return(outputPath);
}

/////////////////////////////////////////////////////
// save image to file
// If interactive is true, use dialog boxes to indicate errors or report
// where the image was saved.

void GuiManager::_saveImageToFile(bool interactive)
{

  cerr << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << endl;
  return;
  
  // create image
  QPixmap pixmap;
  if (_vertMode)
    pixmap = _vert->grab();
  else
    pixmap = _horiz->grab();
  QImage image = pixmap.toImage();

  string outputDir;
  string outputPath = _getOutputPath(interactive, outputDir, _params.images_file_name_extension);

  // write the file
  if (!image.save(outputPath.c_str())) {
    string errmsg("Cannot save image to file: " + outputPath);
    cerr << "ERROR - GuiManager::_saveImageToFile()" << endl;
    cerr << "  " << errmsg << endl;
    if (interactive) {
      QMessageBox::critical(this, "Error", errmsg.c_str());
    }
    return;
  }

  if (interactive) {
    string infoMsg("Saved image to file " + outputPath);
    QMessageBox::information(this, "Image Saved", infoMsg.c_str());
  }

  if (_params.debug) {
    cerr << "==>> saved image to file: " << outputPath << endl;
  }

  // write latest data info

  if (_params.images_write_latest_data_info) {
    
    DsLdataInfo ldataInfo(_params.images_output_dir);
    
    string relPath;
    RadxPath::stripDir(_params.images_output_dir, outputPath, relPath);
    
    if(_params.debug) {
      ldataInfo.setDebug();
    }
    ldataInfo.setLatestTime(_plotStartTime.utime());
    ldataInfo.setWriter("Lucid");
    ldataInfo.setDataFileExt(_params.images_file_name_extension);
    ldataInfo.setDataType(_params.images_file_name_extension);
    ldataInfo.setRelDataPath(relPath);
    
    if(ldataInfo.write(_plotStartTime.utime())) {
      cerr << "ERROR - GuiManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.images_write_latest_data_info)

}

/////////////////////////////////////////////////////
// save current view snapshots to the configured image directory

void GuiManager::_saveViewImages()
{
  if (!_horiz) {
    QMessageBox::critical(this, tr("Cannot save images"),
                          tr("The horizontal view is not available."));
    return;
  }

  QString outputDirPath = QString::fromUtf8(_params.images_output_dir);
  outputDirPath.replace("$(HOME)", QDir::homePath());
  if (outputDirPath.startsWith("$HOME")) {
    outputDirPath.replace(0, 5, QDir::homePath());
  }
  if (outputDirPath.trimmed().isEmpty()) {
    QMessageBox::critical(this, tr("Cannot save images"),
                          tr("images_output_dir is empty."));
    return;
  }
  outputDirPath = QDir::cleanPath(outputDirPath);
  QDir outputDir;
  if (!outputDir.mkpath(outputDirPath)) {
    QMessageBox::critical(
      this, tr("Cannot save images"),
      tr("Cannot create the output directory:\n%1").arg(outputDirPath));
    return;
  }
  outputDir.setPath(outputDirPath);

  const time_t selectedTime = _timeControl ?
    _timeControl->getSelectedTime().utime() : time(nullptr);
  const QString timePart = QDateTime::fromSecsSinceEpoch(
    selectedTime, QTimeZone(QTimeZone::UTC))
    .toString("yyyyMMdd_HHmmss'_UTC'");
  const QString fieldPart = safeFileNameComponent(
    QString::fromStdString(getSelectedFieldLabel()));
  const QString commonPart = fieldPart + "_" + timePart;
  const bool saveVertical = _vertWindow && _vert &&
                            _vertWindow->isVisible();

  // Choose one suffix for the complete capture so horizontal and vertical
  // files remain an obvious pair and an earlier snapshot is never replaced.
  QString suffix;
  QString horizPath;
  QString vertPath;
  for (int sequence = 1; ; ++sequence) {
    suffix = sequence == 1 ? QString() :
      QString("_%1").arg(sequence, 2, 10, QChar('0'));
    horizPath = outputDir.filePath(
      "lucid_horiz_" + commonPart + suffix + ".png");
    vertPath = outputDir.filePath(
      "lucid_vert_" + commonPart + suffix + ".png");
    if (!QFileInfo::exists(horizPath) &&
        (!saveVertical || !QFileInfo::exists(vertPath))) {
      break;
    }
  }

  QStringList savedPaths;
  QStringList failedPaths;
  if (_horiz->grab().save(horizPath, "PNG")) {
    savedPaths.push_back(horizPath);
  } else {
    failedPaths.push_back(horizPath);
  }
  if (saveVertical) {
    if (_vert->grab().save(vertPath, "PNG")) {
      savedPaths.push_back(vertPath);
    } else {
      failedPaths.push_back(vertPath);
    }
  }

  if (!failedPaths.isEmpty()) {
    QMessageBox::critical(
      this, tr("Image save failed"),
      tr("Could not save:\n%1").arg(failedPaths.join("\n")));
  } else {
    QMessageBox::information(
      this, tr("Images saved"),
      tr("Saved:\n%1").arg(savedPaths.join("\n")));
  }
}

void GuiManager::ShowContextMenu(const QPoint &pos) {
  _horiz->ShowContextMenu(pos /*, &_vol*/);
}


#ifdef NOTNOW
// Creates the boundary editor dialog and associated event slots
void GuiManager::_createBoundaryEditorDialog()
{
  _boundaryEditorDialog = new QDialog(this);
  _boundaryEditorDialog->setMaximumHeight(368);
  _boundaryEditorDialog->setWindowTitle("Boundary Editor");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignRight(Qt::AlignRight);

  _boundaryEditorDialogLayout = new QGridLayout(_boundaryEditorDialog);
  _boundaryEditorDialogLayout->setVerticalSpacing(4);

  int row = 0;
  _boundaryEditorInfoLabel = new QLabel("Boundary Editor allows you to select\nan area of your radar image", _boundaryEditorDialog);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorInfoLabel, row, 0, 1, 2, alignCenter);
  
  _boundaryEditorDialogLayout->addWidget(new QLabel(" ", _boundaryEditorDialog), ++row, 0, 1, 2, alignCenter);

  QLabel *toolsCaption = new QLabel("Editor Tools:", _boundaryEditorDialog);
  _boundaryEditorDialogLayout->addWidget(toolsCaption, ++row, 0, 1, 2, alignCenter);

  _boundaryEditorPolygonBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorPolygonBtn->setMaximumWidth(130);
  _boundaryEditorPolygonBtn->setText(" Polygon");
  _boundaryEditorPolygonBtn->setIcon(QIcon("images/polygon.png"));
  _boundaryEditorPolygonBtn->setCheckable(TRUE);
  _boundaryEditorPolygonBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorPolygonBtn, ++row, 0);
  connect(_boundaryEditorPolygonBtn, &QPushButton::clicked,
          this, &GuiManager::polygonBtnBoundaryEditorClick);

  _boundaryEditorCircleBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorCircleBtn->setMaximumWidth(130);
  _boundaryEditorCircleBtn->setText(" Circle  ");
  _boundaryEditorCircleBtn->setIcon(QIcon("images/circle.png"));
  _boundaryEditorCircleBtn->setCheckable(TRUE);
  _boundaryEditorCircleBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorCircleBtn, ++row, 0);
  connect(_boundaryEditorCircleBtn, &QPushButton::clicked,
          this, &GuiManager::circleBtnBoundaryEditorClick());

  _circleRadiusSlider = new QSlider(Qt::Horizontal);
  _circleRadiusSlider->setFocusPolicy(Qt::StrongFocus);
  _circleRadiusSlider->setTracking(true);
  _circleRadiusSlider->setSingleStep(1);
  _circleRadiusSlider->setPageStep(0);
  _circleRadiusSlider->setFixedWidth(100);
  _circleRadiusSlider->setToolTip("Set the circle radius");
  _circleRadiusSlider->setMaximumWidth(180);
  _circleRadiusSlider->setValue(50);
  _circleRadiusSlider->setMinimum(8);
  _circleRadiusSlider->setMaximum(200);
  _boundaryEditorDialogLayout->addWidget(_circleRadiusSlider, row, 1);
  connect(_circleRadiusSlider, SIGNAL(valueChanged(int)),
          this, &GuiManager::_circleRadiusSliderValueChanged(int));

  _boundaryEditorBrushBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorBrushBtn->setMaximumWidth(130);
  _boundaryEditorBrushBtn->setText(" Brush ");
  _boundaryEditorBrushBtn->setIcon(QIcon("images/brush.png"));
  _boundaryEditorBrushBtn->setCheckable(TRUE);
  _boundaryEditorBrushBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorBrushBtn, ++row, 0);
  connect(_boundaryEditorBrushBtn, &QPushButton::clicked,
          this, &GuiManager::brushBtnBoundaryEditorClick());

  _brushRadiusSlider = new QSlider(Qt::Horizontal);
  _brushRadiusSlider->setFocusPolicy(Qt::StrongFocus);
  _brushRadiusSlider->setTracking(true);
  _brushRadiusSlider->setSingleStep(1);
  _brushRadiusSlider->setPageStep(0);
  _brushRadiusSlider->setFixedWidth(100);
  _brushRadiusSlider->setToolTip("Set the smart brush radius");
  _brushRadiusSlider->setMaximumWidth(180);
  _brushRadiusSlider->setValue(18);
  _brushRadiusSlider->setMinimum(12);
  _brushRadiusSlider->setMaximum(75);
  _boundaryEditorDialogLayout->addWidget(_brushRadiusSlider, row, 1);
  connect(_brushRadiusSlider, SIGNAL(valueChanged(int)),
          this, &GuiManager::_brushRadiusSliderValueChanged(int));

  _boundaryEditorBrushBtn->setChecked(TRUE);
  _boundaryEditorDialogLayout->addWidget(new QLabel(" ", _boundaryEditorDialog),
                                         ++row, 0, 1, 2, alignCenter);
  
  _boundaryEditorList = new QListWidget(_boundaryEditorDialog);
  QListWidgetItem *newItem5 = new QListWidgetItem;
  newItem5->setText("Boundary5 <none>");
  _boundaryEditorList->insertItem(0, newItem5);
  QListWidgetItem *newItem4 = new QListWidgetItem;
  newItem4->setText("Boundary4 <none>");
  _boundaryEditorList->insertItem(0, newItem4);
  QListWidgetItem *newItem3 = new QListWidgetItem;
  newItem3->setText("Boundary3 <none>");
  _boundaryEditorList->insertItem(0, newItem3);
  QListWidgetItem *newItem2 = new QListWidgetItem;
  newItem2->setText("Boundary2 <none>");
  _boundaryEditorList->insertItem(0, newItem2);
  QListWidgetItem *newItem1 = new QListWidgetItem;
  newItem1->setText("Boundary1");
  _boundaryEditorList->insertItem(0, newItem1);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorList, ++row, 0, 1, 2);
  connect(_boundaryEditorList, SIGNAL(itemClicked(QListWidgetItem*)),
          this, &GuiManager::onBoundaryEditorListItemClicked(QListWidgetItem*));

  // horizontal layout contains the "Clear", "Help", and "Save" buttons
  QHBoxLayout *hLayout = new QHBoxLayout;
  _boundaryEditorDialogLayout->addLayout(hLayout, ++row, 0, 1, 2);
  
  _boundaryEditorClearBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorClearBtn->setText("Clear");
  hLayout->addWidget(_boundaryEditorClearBtn);
  connect(_boundaryEditorClearBtn, &QPushButton::clicked,
          this, &GuiManager::clearBoundaryEditorClick());

  _boundaryEditorHelpBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorHelpBtn->setText("Help");
  hLayout->addWidget(_boundaryEditorHelpBtn);
  connect(_boundaryEditorHelpBtn, &QPushButton::clicked,
          this, &GuiManager::helpBoundaryEditorClick());
  
  _boundaryEditorSaveBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorSaveBtn->setText("Save");
  hLayout->addWidget(_boundaryEditorSaveBtn);

}
#endif

/////////////////////////////////////////////////////////
// read click point from FMQ - i.e. from another app

void GuiManager::_readClickPoint()
{

#ifdef NOTNOW
  bool gotNew = false;
  
  if (_readClickPointFmq(gotNew) == 0) {
    if (gotNew) {
      if (_params.debug) {
        cerr << "====>> gotNewClickInfo" << endl;
      }
      if (_horiz) {
        _horiz->setClickPoint(_clickPointFmq.getAzimuth(),
                              _clickPointFmq.getElevation(),
                              _clickPointFmq.getRangeKm());
      }
    }
  }
#endif
  
}

  
/**********************************************************************
 * HANDLE_CLIENT_EVENT: 
 */

void GuiManager::_handleClientEvent()
{

#ifdef NOTNOW
  
  time_t clock;
  
  if(_gd.debug1) {
    fprintf(stderr,
            "Found Client Event: %d, Args: %s\n",
            _gd.coord_expt->client_event,
            _gd.coord_expt->client_args);
  }
  
  switch(_gd.coord_expt->client_event) {
    case NEW_MDV_AVAIL:
      remote_new_mdv_avail(_gd.coord_expt->client_args);
      break;
      
    case NEW_SPDB_AVAIL:
      remote_new_spdb_avail(_gd.coord_expt->client_args);
      break;

    case RELOAD_DATA:
      // invalidate_all_data();
      // set_redraw_flags(1,1);
      break;

    case SET_FRAME_NUM:
      int	frame;
      if((sscanf(_gd.coord_expt->client_args,"%d",&frame)) == 1) {
        /* Sanity Check */
        if(frame <= 0 ) frame = 1;
        if(frame > _gd.movie.num_frames ) frame = _gd.movie.num_frames ;

        _gd.movie.cur_frame = frame - 1;
      } else {
        fprintf(stderr,"Invalid SET_FRAME_NUM: Args: %s\n",_gd.coord_expt->client_args);
      }
      break;

    case SET_NUM_FRAMES:
      int	nframes;
      if((sscanf(_gd.coord_expt->client_args,"%d",&nframes)) == 1) {
        _setEndFrame(nframes);
      } else {
        fprintf(stderr,"Invalid SET_NUM_FRAMES: Args: %s\n",_gd.coord_expt->client_args);
      }
      break;

    case SET_REALTIME:
      _gd.movie.mode = REALTIME_MODE;
      _gd.movie.cur_frame = _gd.movie.num_frames -1;
      _gd.movie.end_frame = _gd.movie.num_frames -1;
      clock = time(0);
      _gd.movie.start_time = clock - (time_t) ((_gd.movie.num_frames -1) *
                                              _gd.movie.time_interval_mins * 60.0);
      _gd.movie.start_time -= (_gd.movie.start_time % _gd.movie.round_to_seconds);
      _gd.coord_expt->runtime_mode = RUNMODE_REALTIME;	
      _gd.coord_expt->time_seq_num++;

      // reset_time_points();
      // invalidate_all_data();
      // set_redraw_flags(1,1);

      // Set forecast and past time choosers to "now"
      // xv_set(_gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
      // xv_set(_gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);
      // Set movie mode widget to REALTIME 
      // xv_set(_gd.movie_pu->movie_type_st,PANEL_VALUE,0,NULL);

      break;

    case SET_TIME:
      UTIMstruct ts;
      time_t interest_time;

      if((sscanf(_gd.coord_expt->client_args,"%ld %ld %ld %ld %ld %ld",
                 &ts.year,&ts.month,&ts.day, &ts.hour,&ts.min,&ts.sec)) == 6) {
		
        interest_time = UTIMdate_to_unix(&ts);

        _setDisplayTime(interest_time);
        // invalidate_all_data();
        // set_redraw_flags(1,1);
      } else {
        fprintf(stderr,"Invalid SET_TIME Args: %s\n",_gd.coord_expt->client_args);
      }
      break;

    default: {}

  }

  // Reset  the command
  _gd.coord_expt->client_event = NO_MESSAGE;

#endif
  
}


/**********************************************************************
 * CHECK_FOR_EXPIRED_DATA: Check all data and determine if 
 * the data's expiration time has been exceeded - Mark it invalid if so.
 */

void GuiManager::_checkForExpiredData(time_t tm)
{
  int i;

  // Mark all data past the expiration data as invalid
  /* look thru primary data fields */
  for (i=0; i < _gd.num_datafields; i++) {
    /* if data has expired or field should be updated */
    if (_gd.mread[i]->h_mhdr.time_expire < tm ) {
      //        if(_gd.debug1) fprintf(stderr,"Field: %s expired at %s\n",
      //		 _gd.mread[i]->button_name,
      //		 asctime(gmtime(&((time_t) _gd.mread[i]->h_mhdr.time_expire))));
      _gd.mread[i]->h_data_valid = 0;
      _gd.mread[i]->v_data_valid = 0;
    }
  }

  /* Look through wind field data too */
  for (i=0; i < _gd.layers.num_wind_sets; i++ ) {
    if (_gd.layers.wind[i].active) {
      if (_gd.layers.wind[i].wind_u->h_mhdr.time_expire < tm) {
        _gd.layers.wind[i].wind_u->h_data_valid = 0;
        _gd.layers.wind[i].wind_u->v_data_valid = 0;
      }
      if (_gd.layers.wind[i].wind_v->h_mhdr.time_expire < tm) {
        _gd.layers.wind[i].wind_v->h_data_valid = 0;
        _gd.layers.wind[i].wind_v->v_data_valid = 0;
      }
      if (_gd.layers.wind[i].wind_w != NULL && _gd.layers.wind[i].wind_w->h_mhdr.time_expire < tm) {
        _gd.layers.wind[i].wind_w->h_data_valid = 0;
        _gd.layers.wind[i].wind_w->v_data_valid = 0;
      }
    }
  }
}

/**********************************************************************
 * _CHECKFORDATAUPDATES: Check all data and determine if 
 * new data has arrived - Mark it invalid if so.
 */

void GuiManager::_checkForDataUpdates(time_t tm)
{
  DmapAccess dmap;
  int i,j;
  char *start_ptr, *end_ptr;
  char dir_buf[MAX_PATH_LEN];

  if( strlen(_params.datamap_host) < 2 || dmap.reqAllInfo(_params.datamap_host) != 0) {

    // Force a reload of the data
    // reset_data_valid_flags(1,1);
    if (_gd.prod_mgr) {
      _gd.prod_mgr->reset_product_valid_flags();
    }

  } else {   // Got valid  data back from the Data Mapper

    int nsets = dmap.getNInfo();


    if(_gd.debug1) fprintf(stderr,"Found %d Datamapper info sets\n",nsets);

    /* look thru all data fields */

    for (i=0; i < _gd.num_datafields; i++) {

      // pull out dir from URL

      char tmpUrl[MAX_PATH_LEN];
      strncpy(tmpUrl, _gd.mread[i]->url.c_str(), MAX_PATH_LEN - 1);
      end_ptr = strrchr(tmpUrl, '&');
      if(end_ptr == NULL) continue;  // broken URL.
      
      start_ptr =  strrchr(tmpUrl, ':');
      if(start_ptr == NULL) {
	start_ptr =  tmpUrl; // Must be a local file/dir based URL
      } else {
	start_ptr++;  // Move up one character
      }

      strncpy(dir_buf,start_ptr,(size_t) (end_ptr - start_ptr + 1));
      dir_buf[(size_t) (end_ptr - start_ptr)] = '\0'; // Null terminate
	
      // Look through the data mapper info
      for(j = 0;  j < nsets; j++) {
	const DMAP_info_t &info = dmap.getInfo(j);

	// See if any data matches
	if(strstr(info.dir,dir_buf) != NULL) {
          // Note unix_time is signed (time_t)  and info.end_time is unsigned int
          if (_gd.mread[i]->h_date.utime() < (time_t) info.end_time) {
            _gd.mread[i]->h_data_valid = 0;
            _gd.mread[i]->v_data_valid = 0;
          }
	}
      }
    }
    
    for (i=0; i < _gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (_gd.layers.wind[i].active) {
	// pull out dir from URL

        char tmpUrl[MAX_PATH_LEN];
        strncpy(tmpUrl, _gd.mread[i]->url.c_str(), MAX_PATH_LEN - 1);
        
        char tmp2[MAX_PATH_LEN];
        strncpy(tmp2, _gd.layers.wind[i].wind_u->url.c_str(), MAX_PATH_LEN - 1);
	end_ptr = strrchr(tmp2,'&');
	if(end_ptr == NULL) continue;  // broken URL.
        
	start_ptr =  strrchr(tmp2,':');
	if(start_ptr == NULL) {
          start_ptr = tmpUrl; // Must be a local file/dir based URL
	} else {
          start_ptr++;  // Move up one character
	}

	strncpy(dir_buf,start_ptr,(size_t) (end_ptr - start_ptr + 1));
	dir_buf[(size_t) (end_ptr - start_ptr)] = '\0'; // Null terminate
	
	// Look through the data mapper info
	for(j = 0;  j < nsets; j++) {
          const DMAP_info_t &info = dmap.getInfo(j);
          // See if any data matches
          if(strstr(info.dir,dir_buf) != NULL) {
            // Check if that data is more current
            if (_gd.layers.wind[i].wind_u->h_date.utime() < (time_t) info.end_time) {
              _gd.layers.wind[i].wind_u->h_data_valid = 0;
              _gd.layers.wind[i].wind_u->v_data_valid = 0;
              _gd.layers.wind[i].wind_v->h_data_valid = 0;
              _gd.layers.wind[i].wind_v->v_data_valid = 0;
              if (_gd.layers.wind[i].wind_w != NULL) {
		_gd.layers.wind[i].wind_w->h_data_valid = 0;
		_gd.layers.wind[i].wind_w->v_data_valid = 0;
              }
            }
          }  // If a match
	} // For all data mapper info
      }   // If wind layer is active
    }     // For all wind layers

    _gd.prod_mgr->check_product_validity(tm, dmap);
  } // If data mapper info is availible
}

////////////////////////////////////////////////////////////////// 
// _checkWhatNeedsRendering:

void GuiManager::_checkWhatNeedsRendering(int frame_index)
{
  int i;
  
  // If data used to draw plan view is invalid
  // Indicate plan view image needs rerendering
  
  if (_gd.mread[_gd.h_win.page]->h_data_valid == 0 ||
      _gd.prod_mgr->num_products_invalid() > 0) {
    _gd.movie.frame[frame_index].redraw_horiz = 1;
    _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
  }

  for ( i=0; i < _gd.layers.num_wind_sets; i++ ) {
    // Look through wind field data too
    if (_gd.layers.wind[i].active) {
      if (_gd.layers.wind[i].wind_u->h_data_valid == 0) {
        _gd.movie.frame[frame_index].redraw_horiz = 1;
        _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
      if (_gd.layers.wind[i].wind_v->h_data_valid == 0) {
        _gd.movie.frame[frame_index].redraw_horiz = 1;
        _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
      if (_gd.layers.wind[i].wind_w != NULL &&
          _gd.layers.wind[i].wind_w->h_data_valid == 0) {
        _gd.movie.frame[frame_index].redraw_horiz = 1;
        _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
    }
  }

  // Check overlay contours if active

  for(i= 0; i < Constants::NUM_CONT_LAYERS; i++) {
    if(_gd.layers.cont[i].active) {
      if(_gd.mread[_gd.layers.cont[i].field]->h_data_valid == 0) {
	_gd.movie.frame[frame_index].redraw_horiz = 1;
	_gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
    }
  } 

  // If data used to draw cross section is invalid
  // Indicate pcross section image needs rerendering
  
  if (_gd.v_win.active ) {
    if(_gd.mread[_gd.v_win.page]->v_data_valid == 0)  {
      _gd.movie.frame[frame_index].redraw_vert = 1;
      _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
    }

    for ( i=0; i < _gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (_gd.layers.wind[i].active) {
	if (_gd.layers.wind[i].wind_u->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
	if (_gd.layers.wind[i].wind_v->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
	if (_gd.layers.wind[i].wind_w != NULL &&
            _gd.layers.wind[i].wind_w->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
      }
    }

    /* Check overlay contours if active */
    for(i= 0; i < Constants::NUM_CONT_LAYERS; i++) {
      if(_gd.layers.cont[i].active) {
	if(_gd.mread[_gd.layers.cont[i].field]->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
      }
    } 
  }
}

///////////////////////////////////////////
// set text for GUI panels

void GuiManager::_setText(char *text,
                          size_t maxTextLen,
                          const char *format,
                          int val)
{
  if (abs(val) < 9999) {
    snprintf(text, maxTextLen, format, val);
  } else {
    snprintf(text, maxTextLen, format, -9999);
  }
}

void GuiManager::_setText(char *text,
                          size_t maxTextLen,
                          const char *format,
                          double val)
{
  if (fabs(val) < 9999) {
    snprintf(text, maxTextLen, format, val);
  } else {
    snprintf(text, maxTextLen, format, -9999.0);
  }
}

/////////////////////////////
// update data at the latest click point

void GuiManager::updateClickReport(const ClickReport &report)
{
  if (!_clickReportDialog) {
    return;
  }

  DateTime dataTime(report.dataTime);
  char text[256];
  snprintf(text, sizeof(text), "%.4d/%.2d/%.2d",
           dataTime.getYear(), dataTime.getMonth(), dataTime.getDay());
  _clickDate->setText(text);
  snprintf(text, sizeof(text), "%.2d:%.2d:%.2d UTC",
           dataTime.getHour(), dataTime.getMin(), dataTime.getSec());
  _clickTime->setText(text);

  snprintf(text, sizeof(text), "%.5f deg", report.latitude);
  _clickLatitude->setText(text);
  snprintf(text, sizeof(text), "%.5f deg", report.longitude);
  _clickLongitude->setText(text);

  _clickHeightName->setText(report.heightLabel.c_str());
  _clickHeightName->setVisible(report.showHeight);
  _clickHeight->setVisible(report.showHeight);
  snprintf(text, sizeof(text), "%g %s", report.height,
           report.heightUnits.c_str());
  _clickHeight->setText(text);

  _clickXName->setText(report.xLabel.c_str());
  _clickXName->setVisible(report.showX);
  _clickX->setVisible(report.showX);
  snprintf(text, sizeof(text), "%g %s", report.x, report.xUnits.c_str());
  _clickX->setText(text);

  _clickYName->setText(report.yLabel.c_str());
  _clickYName->setVisible(report.showY);
  _clickY->setVisible(report.showY);
  snprintf(text, sizeof(text), "%g %s", report.y, report.yUnits.c_str());
  _clickY->setText(text);

  _clickFieldsTable->setRowCount(report.fields.size());
  for (size_t ii = 0; ii < report.fields.size(); ii++) {
    const ClickReportField &field = report.fields[ii];
    QTableWidgetItem *nameItem =
      new QTableWidgetItem(field.name.c_str());
    snprintf(text, sizeof(text), "%g", field.value);
    QTableWidgetItem *valueItem = new QTableWidgetItem(text);
    valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QTableWidgetItem *unitsItem =
      new QTableWidgetItem(field.units.c_str());
    _clickFieldsTable->setItem(ii, 0, nameItem);
    _clickFieldsTable->setItem(ii, 1, valueItem);
    _clickFieldsTable->setItem(ii, 2, unitsItem);
  }
  _clickFieldsTable->resizeColumnsToContents();
  _clickFieldsTable->resizeRowsToContents();
}

/////////////////////////////
// show data at click point

void GuiManager::showClickDialog()
{

  if (!_clickReportDialog) {
    return;
  }

  // The checkable action is the source of truth.  Click updates may call this
  // repeatedly, so visibility must not be toggled on each new report.
  if (_showClickAct->isChecked()) {
    if (!_clickReportPlaced) {
      QPoint pos;
      pos.setX(x() + width() + 5);
      pos.setY(y());
      _clickReportDialog->move(pos);
      _clickReportPlaced = true;
    }
    _clickReportDialog->show();
    _clickReportDialog->raise();
  } else {
    _clickReportDialog->hide();
  }
}

/////////////////////////////
// show value in data plot

void GuiManager::_showValue()

{
  const bool enabled = _showValueAct->isChecked();
  _horiz->setShowValue(enabled);
  _vert->setShowValue(enabled);
}

/////////////////////////////////////////////////////////////
// retain a completed vertical-section route

void GuiManager::_verticalRouteCompleted(const VsectionRoute &route)
{
  _vsectionRoute = route;

  char message[256];
  snprintf(message, sizeof(message),
           "Vertical section route: %zu points, %.1f km",
           route.size(), route.totalLengthKm());
  // statusBar()->showMessage(message, 5000);

  if (_params.debug) {
    cerr << "Vertical section route completed" << endl;
    cerr << "  points: " << route.size() << endl;
    cerr << "  total length km: " << route.totalLengthKm() << endl;
    const vector<VsectionRoute::Point> &points = route.points();
    for (size_t ii = 0; ii < points.size(); ++ii) {
      cerr << "  waypoint " << ii
           << ": lat, lon = " << points[ii].lat << ", " << points[ii].lon
           << "; x, y = " << points[ii].x << ", " << points[ii].y
           << "; distance km = " << points[ii].cumulativeDistanceKm
           << endl;
    }
  }

  _requestVerticalSection();
}

/////////////////////////////////////////////////////////////
// request the retained route for the currently selected field

int GuiManager::_requestVerticalSection()
{
  if (_vsectionRoute.size() < 2 ||
      _gd.h_win.page < 0 || _gd.h_win.page >= _gd.num_datafields) {
    return -1;
  }

  WayPts wayPts;
  for (const VsectionRoute::Point &point : _vsectionRoute.points()) {
    wayPts.addPoint(point.lat, point.lon);
  }

  MdvReader *reader = _gd.mread[_gd.h_win.page];
  connect(reader, &MdvReader::verticalReadComplete,
          this, &GuiManager::_verticalReadComplete,
          Qt::UniqueConnection);
  // statusBar()->showMessage("Requesting vertical section");
  const int generation = reader->requestVertSection(
    _timeControl->getSelectedTime(), _gd.h_win.page, wayPts);
  _requestVerticalWinds();
  return generation;
}

/////////////////////////////////////////////////////////////
// request horizontal U/V components along the retained section route

int GuiManager::_requestVerticalWinds()
{
  if (_gd.wind_mgr == nullptr || _vsectionRoute.size() < 2) {
    return -1;
  }

  WayPts wayPts;
  for (const VsectionRoute::Point &point : _vsectionRoute.points()) {
    wayPts.addPoint(point.lat, point.lon);
  }
  return _gd.wind_mgr->requestVerticalData(
    _timeControl->getSelectedTime(), _gd.h_win.page, wayPts);
}

/////////////////////////////////////////////////////////////
// display a completed vertical-section read

void GuiManager::_verticalReadComplete(MdvReader *reader)
{
  // Vertical reads run independently for each field.  Ignore a result from a
  // field which was superseded while its request was in flight; the selected
  // field has its own request pending (or has already completed).

  if (_gd.h_win.page < 0 || _gd.h_win.page >= _gd.num_datafields ||
      reader != _gd.mread[_gd.h_win.page]) {
    return;
  }

  // if (reader == nullptr || !reader->isValidV()) {
  //   statusBar()->showMessage("Vertical section data are not available", 5000);
  //   return;
  // }

  _vert->setSection(reader);
  _vertWindow->setSectionInfo(_params._fields[_gd.h_win.page].field_name,
                              reader->v_date,
                              reader->getVertTotalLengthKm());

  // Automatic vertical output is tied to the exact request generation. A
  // failed completion advances immediately; waiting for the timeout would
  // only delay an unattended batch after the MDV layer has already answered.
  if (_automaticImagesActive &&
      _automaticImageGenerator->currentJobIsVertical() &&
      reader == _gd.mread[
        _automaticImageGenerator->currentJob().fieldIndex] &&
      reader->getVertGeneration() ==
        _automaticImageGenerator->expectedGeneration()) {
    if (!reader->isValidV()) {
      _failAutomaticImage(tr("Vertical-section data are not available for %1")
        .arg(_params._fields[_gd.h_win.page].field_name));
    } else if (_automaticImageGenerator->acceptDataCompletion(
                 reader->getVertGeneration())) {
      _checkAutomaticImageReady();
    }
    return;
  }

  if (isMovieBuilding() && _movieIncludesVertical &&
      _movieBuildFrame >= 0 &&
      reader->getVertGeneration() == _movieVerticalRequestGeneration) {
    _movieVerticalReady = true;
    _checkMovieFrameReady();
    return;
  }

  // Field changes during cached playback update the hidden live section. Do
  // not raise the window or replace the frame currently owned by the movie.
  if (isMovieActive()) {
    return;
  }

  _showVertAct->setChecked(true);
  _vertWindow->show();
  _vertWindow->raise();
  _vertWindow->activateWindow();
  // statusBar()->showMessage("Vertical section ready", 5000);
}

/////////////////////////////
// vert window has closed

void GuiManager::vertWindowClosed(QCloseEvent *event)

{
  _showVertAct->setChecked(false);
}

/////////////////////////////////////////////////////
// howto help

void GuiManager::_howto()
{
  string text;
  text += "HOWTO HINTS FOR Lucid\n";
  text += "========================\n";
  text += "\n";
  text += "Use NUMBER keys to display RAW fields\n";
  text += "Use ALT-NUMBER keys to display FILTERED fields\n";
  text += "Hit '.' to toggle between the two latest fields\n";
  text += "\n";
  text += "You can also use the arrow keys to select different fields\n";
  text += "\n";
  text += "Click in main window to get field data at a point.\n";
  text += "\n";
  if (_params.single_segment_vsection) {
    text += "Right-drag and release to select a vertical section.\n";
  } else {
    text += "Right-drag to start a vertical-section route.\n";
    text += "Right-click to add segments and double-click to finish.\n";
    text += "Use Backspace to remove a point or Escape to cancel.\n";
  }
  text += "Use Shift-right-click for the context menu.\n";
  QMessageBox::about(this, tr("Howto dialog"), tr(text.c_str()));
}

void GuiManager::_about()
{

  //QMessageBox::about(this, tr("About Menu"),
  //tr("Lucid is an integrating Cartesian display for weather data."));

  string text;
  
  text += "Lucid is an integrating Cartesian display for weather data.\n\n";
  text += "Get help with Lucid ...  \n ";
  text += "\nReport an issue https://github.com/NCAR/lrose-core/issues \n ";
  text += "\nLucid version ... \n ";  
  text += "\nCopyright UCAR (c) 1990 - 2019  ";
  text += "\nUniversity Corporation for Atmospheric Research (UCAR)  ";  
  text += "\nNational Center for Atmospheric Research (NCAR)   ";  
  text += "\nBoulder, Colorado, USA ";  
  text += "\n\nBSD licence applies - redistribution and use in source and binary";  
  text += " forms, with or without modification, are permitted provided that";  
  text += " the following conditions are met: ";  
  text += "\n1) If the software is modified to produce derivative works,";  
  text += " such modified software should be clearly marked, so as not";  
  text += " to confuse it with the version available from UCAR. ";  
  text += "\n2) Redistributions of source code must retain the above copyright";  
  text += " notice, this list of conditions and the following disclaimer.";  
  text += "\n3) Redistributions in binary form must reproduce the above copyright";  
  text += " notice, this list of conditions and the following disclaimer in the";  
  text += " documentation and/or other materials provided with the distribution.";  
  text += "\n4) Neither the name of UCAR nor the names of its contributors,";  
  text += "if any, may be used to endorse or promote products derived from";  
  text += " this software without specific prior written permission.";  
  text += "\n\nDISCLAIMER: THIS SOFTWARE IS PROVIDED \"AS IS\" AND WITHOUT ANY EXPRESS ";  
  text += " OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED";  
  text += " WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.";  

  QMessageBox::about(this, tr("About Menu"), tr(text.c_str()));
}

//////////////////////////////////////////////////////////////////////////////
// Legacy - to be remove later
//////////////////////////////////////////////////////////////////////////////


/************************************************************************
 * TIMER_FUNC: This routine acts as the main branch point for most of the
 * threads involved in the display. This function gets called through
 * XView's notifier mechanism, which is built on sigalarm().
 * This is parameterized, and usually runs on a 5-100 msec interval.
 * First, any pending IO is handled.
 * Current time tickers, and the display's shmem communications are updated
 * Checks for out-of date images and expired Data are initiated periodically
 * Animation and Rendering are handled.
 * Finally Background Rendering is scheduled
 *
 */

void GuiManager::_autoCreateFunc()
{

    // handle image creation

  if (_params.create_images_automatically) {

    // if we are just creating files in archive mode
    // and then exiting, do that now
    
    if ((_params.images_creation_mode ==
         Params::CREATE_IMAGES_THEN_EXIT) ||
        (_params.images_creation_mode ==
         Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE)) {
      _createArchiveImageFiles();
      close();
      return;
    }
    
    // if we are creating files in realtime mode, do that now
    
    if (_params.images_creation_mode ==
        Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
      // _handleRealtimeData();
      // _createRealtimeImageFiles();
      return;
    }
    
  } // if (_params.images_auto_create)

}


/*************************************************************************
 * SET_DISPLAY_TIME
 */

void GuiManager::_setDisplayTime(time_t utime)
{
  int i,j,found;
  time_t    last_time;
  time_t    target_time;

  movie_frame_t    tmp_frame[Constants::MAX_FRAMES];    /* info about each frame */

  last_time = _gd.movie.start_time;
     
  // Round to the nearest even interval 
  utime -= (utime % _gd.movie.round_to_seconds);

  // Already set to this time
  if(utime == _gd.movie.start_time) return;

  /* if starting time moves more than 2 volume intervals, assume we want archive mode */
  if(abs(last_time - utime) > (2 * _gd.movie.time_interval_mins * 60)) {
    _gd.movie.mode = ARCHIVE_MODE;
    _gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
    _gd.coord_expt->time_seq_num++;
    // xv_set(_gd.movie_pu->movie_type_st,PANEL_VALUE,ARCHIVE_MODE,NULL);

  } else {
    if(_gd.movie.mode == REALTIME_MODE) {
      _gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
      _gd.coord_expt->time_seq_num++;

      // Not Sensible to change start times in real time mode - Ignore;
      _updateMoviePopup();
      return ;
    }
  }

  _gd.movie.start_time = utime;

  // Record the time we're currently on
  target_time = _gd.movie.frame[_gd.movie.cur_frame].time_start;

  // Make a temporary copy
  // Zero out global array
  for (int ii = 0; ii < Constants::MAX_FRAMES; ii++) {
    tmp_frame[ii] = _gd.movie.frame[ii];
    _gd.movie.frame[ii].init();
  }
  // memcpy(tmp_frame,_gd.movie.frame,sizeof(movie_frame_t) * Constants::MAX_FRAMES);
  
  
  // Fill in time points on global array
  // reset_time_points();

  // Search for frames already rendered for this interval and use them
  for(i=0 ; i < _gd.movie.num_frames; i++) {
    found = 0;
    for(j=0; j < Constants::MAX_FRAMES && !found; j++) {
      if(_gd.movie.frame[i].time_start == tmp_frame[j].time_start) {
        found = 1;
        _gd.movie.frame[i] = tmp_frame[j];
        // Render a new time selector for this frame
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
        tmp_frame[j].init();
      }
	 
    }
  }

  // Reuse pixmaps in unused frames
  for(i=0 ; i < _gd.movie.num_frames; i++) {
    if(_gd.movie.frame[i].h_pdev) continue; // Alreday is accounted for.

    found = 0;
#ifdef NOTYET
    for(j=0; j < Constants::MAX_FRAMES && !found; j++) {
      if(tmp_frame[j].h_xid) {
        found = 1;
        _gd.movie.frame[i].h_xid = tmp_frame[j].h_xid;
        _gd.movie.frame[i].v_xid = tmp_frame[j].v_xid;
        _gd.movie.frame[i].redraw_horiz = 1;
        _gd.movie.frame[i].redraw_vert = 1;
        memset(&tmp_frame[j],0,sizeof(movie_frame_t));
      }
    }
#endif
  }

  _gd.movie.cur_frame = 0;
  for(i=0; i < _gd.movie.num_frames; i++) {
    if(target_time >= _gd.movie.frame[i].time_start &&
       target_time <= _gd.movie.frame[i].time_end) {

      _gd.movie.cur_frame = i;
    }
  }

  // Reset gridded and product data validity flags
  // invalidate_all_data();

  _updateMoviePopup();
     
}

/*************************************************************************
 *  SET_END_FRAME
 */
void GuiManager::_setEndFrame(int num_frames)
  
{
  int i,j;
  time_t    target_time;
  movie_frame_t    tmp_frame[Constants::MAX_FRAMES];    /* info about each frame */
  int old_frames;

  _gd.movie.end_frame = num_frames -1;
  
  // Sanity check
  if(_gd.movie.end_frame < 0) _gd.movie.end_frame = 0;
  if(_gd.movie.end_frame >= Constants::MAX_FRAMES) _gd.movie.end_frame = Constants::MAX_FRAMES -1;
  old_frames = _gd.movie.num_frames;
  _gd.movie.num_frames = _gd.movie.end_frame +1;
  
  if(_gd.movie.num_frames > 1) {
    // xv_set(_gd.movie_pu->movie_frame_sl,XV_SHOW,TRUE,NULL);
  } else {
    // xv_set(_gd.movie_pu->movie_frame_sl,XV_SHOW,FALSE,NULL);
  }

  target_time = _gd.movie.frame[_gd.movie.cur_frame].time_start;

  if(_gd.movie.mode == REALTIME_MODE) {
    _gd.movie.cur_frame = _gd.movie.end_frame;
    // Make a temporary copy
    // Zero out global array
    for (int ii = 0; ii < Constants::MAX_FRAMES; ii++) {
      tmp_frame[ii] = _gd.movie.frame[ii];
      _gd.movie.frame[ii].init();
    }
    
    // Start point changes
    _gd.movie.start_time -= (time_t) ((_gd.movie.time_interval_mins * 60.0) *
                                      (_gd.movie.num_frames - old_frames));
    
    // reset_time_points();
    
    if(_gd.movie.num_frames > old_frames) {
      // copy original frames
      for(i = _gd.movie.num_frames - old_frames, j = 0; j < old_frames; i++, j++) {
        _gd.movie.frame[i] = tmp_frame[j];
        
        // Render time selector in reused frame
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
      
      // Mark new frames for allocation & redrawing
      j = _gd.movie.num_frames - old_frames;
      for(i = 0; i < j; i++) {
        _gd.movie.frame[i].redraw_horiz = 1;
        _gd.movie.frame[i].redraw_vert = 1;
      }
    } else {
      for(i = 0, j = old_frames - _gd.movie.num_frames ; j < old_frames; i++, j++) {
        _gd.movie.frame[i] = tmp_frame[j];
        // Render time selector in reused frame
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
      // Copy unused frames too so they get de-allocated
      for(j = 0, i = _gd.movie.num_frames; j < old_frames - _gd.movie.num_frames; i++, j++) {
        _gd.movie.frame[i] = tmp_frame[j];
      }
    }
  } else {
    _gd.movie.cur_frame = 0;
    // Start point remains the same
    // reset_time_points();

    if(_gd.movie.num_frames > old_frames) {
      for(i = _gd.movie.num_frames -1; i < old_frames; i++) {
        _gd.movie.frame[i].redraw_horiz = 1;
        _gd.movie.frame[i].redraw_vert = 1;
      }
      // Render time selector in reused frames
      for(i= 0; i < old_frames; i++) {
#ifdef NOTYET
       draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
    } else {
      // Render time selector in reused frames
      for(i= 0; i < _gd.movie.num_frames; i++) {
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
    }
  }
     
  for(i=0; i < _gd.movie.num_frames; i++) {
    if(target_time >= _gd.movie.frame[i].time_start &&
       target_time <= _gd.movie.frame[i].time_end) {

      _gd.movie.cur_frame = i;
    }
  }
  // Reset gridded and product data validity flags
  // invalidate_all_data();
     
  _updateMoviePopup();

  // adjust_pixmap_allocation();
     
  return;
}

/*************************************************************************
 * UPDATE_MOVIE_PU: Update critical displayed values on the movie popup
 */

void GuiManager::_updateMoviePopup()
{
  int        index;
  char    text[64];
  char    fmt_text[128];
  struct tm tms;

  if(_gd.movie.cur_frame < 0) {
    index =  _gd.movie.num_frames - 1;
  } else {
    index = _gd.movie.cur_frame;
  }
 
  /* Update the Current Frame Begin Time text */
  snprintf(fmt_text,128,"Frame %d: %%H:%%M %%m/%%d/%%Y",index+1);
  if(_params.use_local_timestamps) {
    strftime (text,64,fmt_text,localtime_r(&_gd.movie.frame[index].time_mid,&tms));
  } else {
    strftime (text,64,fmt_text,gmtime_r(&_gd.movie.frame[index].time_mid,&tms));
  }
  // xv_set(_gd.movie_pu->fr_begin_msg,PANEL_LABEL_STRING,text,NULL);
  
  // Update the movie time start text
  if(_params.use_local_timestamps) {
    strftime (text, 64, _params.moviestart_time_format,
              localtime_r(&_gd.movie.start_time,&tms));
  } else {
    strftime (text, 64, _params.moviestart_time_format,
              gmtime_r(&_gd.movie.start_time,&tms));
  }
  // xv_set(_gd.movie_pu->start_time_tx,PANEL_VALUE,text,NULL);

  // xv_set(_gd.movie_pu->movie_type_st,PANEL_VALUE,_gd.movie.mode,NULL);

  if(_gd.debug1) printf("Time Start: %ld, End: %ld\n",_gd.movie.frame[index].time_start,
                       _gd.movie.frame[index].time_end);
   
  /* update the time_interval  text */
  switch(_gd.movie.climo_mode) {
    case Params::CLIMO_REGULAR_INTERVAL:
      snprintf(text,64,"%.2f",_gd.movie.time_interval_mins);
      // xv_set(_gd.movie_pu->time_interval_tx,PANEL_VALUE,text,NULL);
      // xv_set(_gd.movie_pu->min_msg,PANEL_LABEL_STRING,"min",NULL);
      break;

    case Params::CLIMO_DAILY_INTERVAL:
      // xv_set(_gd.movie_pu->time_interval_tx,PANEL_VALUE,"1",NULL);
      // xv_set(_gd.movie_pu->min_msg,PANEL_LABEL_STRING,"day",NULL);
      break;

    case Params::CLIMO_YEARLY_INTERVAL:
      // xv_set(_gd.movie_pu->time_interval_tx,PANEL_VALUE,"1",NULL);
      // xv_set(_gd.movie_pu->min_msg,PANEL_LABEL_STRING,"yr",NULL);
      break;

  }

  /* update the forecast period text */
  snprintf(text,64,"%.2f",_gd.movie.forecast_interval);
  // xv_set(_gd.movie_pu->fcast_per_tx,PANEL_VALUE,text,NULL);

  // xv_set(_gd.movie_pu->movie_frame_sl,
  //        PANEL_MIN_VALUE,_gd.movie.start_frame + 1,
  //        PANEL_MAX_VALUE,_gd.movie.end_frame + 1,
  //        PANEL_VALUE,_gd.movie.cur_frame +1,
  //        NULL);

  /* update the start/end frames text */
  snprintf(text,64,"%d",_gd.movie.end_frame +1);
  // xv_set(_gd.movie_pu->end_frame_tx,PANEL_VALUE,text,NULL);

#ifdef NOTYET
  if(_gd.time_plot)
  {
    _gd.time_plot->Set_times((time_t) _gd.epoch_start,
                            (time_t) _gd.epoch_end,
                            (time_t) _gd.movie.frame[_gd.movie.cur_frame].time_start,
                            (time_t) _gd.movie.frame[_gd.movie.cur_frame].time_end,
                            (time_t)((_gd.movie.time_interval_mins * 60.0) + 0.5),
                            _gd.movie.num_frames);
    _gd.time_plot->Draw(); 
  }
#endif
    

}
