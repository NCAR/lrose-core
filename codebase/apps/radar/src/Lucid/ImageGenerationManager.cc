// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - see the LICENSE file for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include "ImageGenerationManager.hh"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <limits>
#include <utility>

#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QSaveFile>
#include <QStringList>
#include <QXmlStreamWriter>

#include <dsserver/DsLdataInfo.hh>
#include <Mdv/MdvxProj.hh>

#include "WorldPlot.hh"

using std::cerr;
using std::endl;

ImageGenerationManager::ImageGenerationManager(
        const Params &params,
        int numDataFields,
        Callbacks callbacks,
        QObject *parent) :
        QObject(parent),
        _params(params),
        _numDataFields(numDataFields),
        _callbacks(std::move(callbacks)),
        _started(false),
        _active(false),
        _batchFailureBaseline(0),
        _batchAttempted(0),
        _batchSaved(0),
        _batchTimedOut(0),
        _batchProductOmissions(0),
        _totalFailures(0),
        _totalAttempted(0),
        _totalSaved(0),
        _totalTimedOut(0),
        _totalProductOmissions(0),
        _completedBatches(0)
{
  _imagesStartTime.set(_params.images_archive_start_time);
  _imagesEndTime.set(_params.images_archive_end_time);

  _timeoutTimer.setSingleShot(true);
  connect(&_timeoutTimer, &QTimer::timeout,
          this, [this]() { _jobTimedOut(); });
  _scheduleTimer.setSingleShot(true);
  connect(&_scheduleTimer, &QTimer::timeout,
          this, [this]() { _scheduleDue(); });
}

/////////////////////////////////////////////////////////////
// initialize the configured schedule exactly once

void ImageGenerationManager::start()
{
  if (_started) {
    return;
  }
  _started = true;

  QString error;
  if (!_validateConfiguration(error) ||
      !_generator.configure(_params, _numDataFields, error)) {
    _finishRun(error);
    return;
  }

  switch (_params.images_creation_mode) {
    case Params::CREATE_IMAGES_THEN_EXIT:
      _startBatch(_imagesStartTime);
      break;

    case Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE:
      _nextImageTime = _imagesStartTime;
      _scheduleDue();
      break;

    case Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE: {
      const time_t now = std::time(nullptr);
      const time_t interval = _params.images_schedule_interval_secs;
      const time_t delay = _params.images_schedule_delay_secs;
      time_t target = (now / interval) * interval;
      if (target + delay < now) {
        target += interval;
      }
      _nextImageTime.set(target);
      _scheduleNextRealtimeBatch();
      break;
    }

    default:
      _finishRun(tr("Unknown images_creation_mode"));
      break;
  }
}

/////////////////////////////////////////////////////////////
// reject schedules which cannot make deterministic progress

bool ImageGenerationManager::_validateConfiguration(QString &error) const
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
// start one complete field/height/zoom/route batch

void ImageGenerationManager::_startBatch(const DateTime &imageTime)
{
  if (_active) {
    return;
  }
  if (!imageTime.isValid()) {
    _finishRun(tr("Automatic image target time is invalid"));
    return;
  }

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE &&
      _callbacks.saveDisplayState) {
    _callbacks.saveDisplayState();
  }

  _batchTime = imageTime;
  _batchFailureBaseline = _generator.failureCount();
  _batchAttempted = 0;
  _batchSaved = 0;
  _batchTimedOut = 0;
  _batchProductOmissions = 0;
  _batchElapsedTimer.start();
  _active = true;

  if (_callbacks.batchStarted) {
    _callbacks.batchStarted(imageTime);
  }
  if (!_generator.start()) {
    _active = false;
    _finishRun(tr("No automatic image jobs were configured"));
    return;
  }

  cerr << "Automatic image batch started for "
       << imageTime.asString() << ", "
       << _generator.jobCount() << " image(s)" << endl;
  _requestCurrentJob();
}

/////////////////////////////////////////////////////////////
// hand the current immutable job to the display controller

void ImageGenerationManager::_requestCurrentJob()
{
  if (!_active || !_generator.hasCurrentJob()) {
    if (_active) {
      _completeBatch();
    }
    return;
  }

  ++_batchAttempted;
  _timeoutTimer.start(_params.images_request_timeout_secs * 1000);
  if (_callbacks.jobRequested) {
    _callbacks.jobRequested();
  } else {
    failCurrentJob(tr("No automatic image job callback is configured"));
  }
}

/////////////////////////////////////////////////////////////
// capture and publish after data plus optional layers have settled

void ImageGenerationManager::checkReady(bool dependenciesReady,
                                        int unavailableProducts)
{
  if (!_active || !_generator.readyToSave(dependenciesReady)) {
    return;
  }

  _timeoutTimer.stop();
  if (!_params.images_require_all_symprods && unavailableProducts > 0) {
    ++_batchProductOmissions;
    cerr << "WARNING - automatic image: saving without "
         << unavailableProducts
         << " unavailable symbolic product(s)" << endl;
  }

  Capture capture;
  QString error;
  QString path;
  const bool captured = _callbacks.captureCurrentJob &&
    _callbacks.captureCurrentJob(capture, error);
  const bool saved = captured && _publishImage(capture, path, error);
  _completeCurrentJob(saved, path, error);
}

/////////////////////////////////////////////////////////////
// fail one job immediately and continue the unattended batch

void ImageGenerationManager::failCurrentJob(const QString &error)
{
  if (!_active || !_generator.hasCurrentJob()) {
    return;
  }
  _timeoutTimer.stop();
  _completeCurrentJob(false, QString(), error);
}

void ImageGenerationManager::_completeCurrentJob(bool saved,
                                                  const QString &path,
                                                  const QString &error)
{
  if (saved) {
    ++_batchSaved;
    cerr << "Automatic image saved: " << path.toStdString() << endl;
  } else {
    _generator.recordFailure();
    cerr << "ERROR - automatic image: " << error.toStdString() << endl;
  }

  _generator.advance();
  QTimer::singleShot(0, this, [this]() { _requestCurrentJob(); });
}

/////////////////////////////////////////////////////////////
// abandon one request rather than hanging an unattended run

void ImageGenerationManager::_jobTimedOut()
{
  if (!_active || !_generator.hasCurrentJob()) {
    return;
  }

  const AutomaticImageGenerator::Job job = _generator.currentJob();
  cerr << "ERROR - automatic image: timed out waiting for field "
       << _params._fields[job.fieldIndex].field_name;
  if (_generator.currentJobIsVertical()) {
    cerr << ", vertical section "
         << _generator.currentRoute().name.toStdString();
  } else {
    cerr << " at " << job.heightKm << " km, zoom "
         << job.zoomName.toStdString();
  }
  cerr << endl;

  if (_callbacks.cancelTimedOutRequest) {
    _callbacks.cancelTimedOutRequest(job, _generator.expectedGeneration());
  }
  _generator.recordFailure();
  ++_batchTimedOut;
  _generator.advance();
  QTimer::singleShot(0, this, [this]() { _requestCurrentJob(); });
}

/////////////////////////////////////////////////////////////
// atomically publish one image and its configured metadata

bool ImageGenerationManager::_publishImage(const Capture &capture,
                                            QString &path,
                                            QString &error)
{
  if (capture.image.isNull()) {
    error = tr("The captured image is empty");
    return false;
  }

  QByteArray format;
  if (!_generator.prepareOutput(
        _params, capture.dataTime, path, format, error)) {
    return false;
  }

  QImage outputImage = capture.image;
  if (_params.make_images_transparent) {
    // The configured canvas background is the catalog transparency key.
    // Exact matching preserves rendered and anti-aliased data pixels.
    outputImage = capture.image.convertToFormat(QImage::Format_ARGB32);
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

  // Atomic publication prevents downstream readers from observing a partly
  // encoded image while an unattended update is in progress.
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

  // Sidecars precede _latest_data_info because that file is the publication
  // notification consumed by downstream catalog readers.
  if (capture.writeGeoXml && _params.images_output_geo_xml) {
    if (!capture.world ||
        !_writeGeoXml(path, outputImage, *capture.world, error)) {
      if (error.isEmpty()) {
        error = tr("No horizontal world geometry is available for %1")
          .arg(path);
      }
      return false;
    }
  }
  if (_params.images_write_latest_data_info &&
      !_writeLatestDataInfo(path, format, capture.dataTime, error)) {
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// write CIDD-compatible georeferencing for the image canvas

bool ImageGenerationManager::_writeGeoXml(const QString &imagePath,
                                           const QImage &image,
                                           const WorldPlot &world,
                                           QString &error) const
{
  if (image.width() < 1 || image.height() < 1) {
    error = tr("Cannot compute georeferencing for %1").arg(imagePath);
    return false;
  }

  const qreal deviceRatio = image.devicePixelRatio() > 0.0 ?
    image.devicePixelRatio() : 1.0;
  const double logicalWidth = image.width() / deviceRatio;
  const double logicalHeight = image.height() / deviceRatio;
  const double x0 = world.getXWorld(0.0);
  const double x1 = world.getXWorld(logicalWidth);
  const double y0 = world.getYWorld(0.0);
  const double y1 = world.getYWorld(logicalHeight);
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
      world.getProjection().getProjType() == Mdvx::PROJ_LATLON) {
    double southLat = south;
    double westLon = west;
    double northLat = north;
    double eastLon = east;
    if (world.getProjection().getProjType() != Mdvx::PROJ_LATLON) {
      MdvxProj projection = world.getProjection();
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
// advertise an image only after all configured files exist

bool ImageGenerationManager::_writeLatestDataInfo(
        const QString &imagePath,
        const QByteArray &format,
        const DateTime &imageTime,
        QString &error) const
{
  const QFileInfo imageInfo(imagePath);
  const std::string root = imageInfo.absolutePath().toStdString();
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
  latest.setRelDataPath(imageInfo.fileName().toStdString());
  if (latest.write(imageTime.utime())) {
    error = tr("Cannot write _latest_data_info in %1")
      .arg(imageInfo.absolutePath());
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////
// close one batch and choose the next scheduling action

void ImageGenerationManager::_completeBatch()
{
  _timeoutTimer.stop();
  _active = false;
  const int batchFailures = std::max(
    0, _generator.failureCount() - _batchFailureBaseline);
  _totalFailures += batchFailures;
  _totalAttempted += _batchAttempted;
  _totalSaved += _batchSaved;
  _totalTimedOut += _batchTimedOut;
  _totalProductOmissions += _batchProductOmissions;
  ++_completedBatches;

  cerr << "Automatic image batch for " << _batchTime.asString()
       << " completed: attempted=" << _batchAttempted
       << ", saved=" << _batchSaved
       << ", failed=" << batchFailures
       << ", timed_out=" << _batchTimedOut
       << ", images_with_product_omissions="
       << _batchProductOmissions
       << ", elapsed_secs="
       << (_batchElapsedTimer.isValid() ?
             _batchElapsedTimer.elapsed() / 1000.0 : 0.0)
       << endl;

  if (_params.images_creation_mode == Params::CREATE_IMAGES_THEN_EXIT) {
    _finishRun(QString());
    return;
  }

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    _nextImageTime = _batchTime + _params.images_archive_interval_secs;
    if (_nextImageTime <= _imagesEndTime) {
      QTimer::singleShot(0, this, [this]() { _scheduleDue(); });
    } else {
      _finishRun(QString());
    }
    return;
  }

  // Coalesce missed realtime intervals to the newest target already due so
  // a slow data source cannot create an unbounded queue of obsolete batches.
  const time_t interval = _params.images_schedule_interval_secs;
  const time_t delay = _params.images_schedule_delay_secs;
  const time_t now = std::time(nullptr);
  time_t nextTarget = _batchTime.utime() + interval;
  if (now >= delay) {
    const time_t latestDueTarget = ((now - delay) / interval) * interval;
    nextTarget = std::max(nextTarget, latestDueTarget);
  }
  _nextImageTime.set(nextTarget);

  // Keep control of the display when the coalesced batch is immediately due;
  // restore it only when the scheduler reaches a genuine idle interval.
  if (nextTarget + delay > now && _callbacks.restoreDisplayState) {
    _callbacks.restoreDisplayState();
  }
  _scheduleNextRealtimeBatch();
}

void ImageGenerationManager::_scheduleNextRealtimeBatch()
{
  const time_t dueTime = _nextImageTime.utime() +
    _params.images_schedule_delay_secs;
  const qint64 waitMillis = std::max<qint64>(
    0, static_cast<qint64>(dueTime - std::time(nullptr)) * 1000);
  const int timerMillis = static_cast<int>(std::min<qint64>(
    waitMillis, std::numeric_limits<int>::max()));
  cerr << "Next automatic image target: "
       << _nextImageTime.asString()
       << ", generation begins after "
       << DateTime(dueTime).asString() << endl;
  _scheduleTimer.start(timerMillis);
}

void ImageGenerationManager::_scheduleDue()
{
  if (_active) {
    return;
  }
  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
    const time_t dueTime = _nextImageTime.utime() +
      _params.images_schedule_delay_secs;
    if (std::time(nullptr) < dueTime) {
      _scheduleNextRealtimeBatch();
      return;
    }
  }
  _startBatch(_nextImageTime);
}

/////////////////////////////////////////////////////////////
// terminate a finite run or a realtime run with bad configuration

void ImageGenerationManager::_finishRun(const QString &fatalError)
{
  _timeoutTimer.stop();
  _scheduleTimer.stop();
  _active = false;
  if (!fatalError.isEmpty()) {
    ++_totalFailures;
    cerr << "ERROR - automatic image generation: "
         << fatalError.toStdString() << endl;
  }
  if (_callbacks.restoreDisplayState) {
    _callbacks.restoreDisplayState();
  }

  const bool success = _totalFailures == 0;
  cerr << "Automatic image generation completed"
       << (success ? " successfully" : " with failures")
       << ": batches=" << _completedBatches
       << ", attempted=" << _totalAttempted
       << ", saved=" << _totalSaved
       << ", failed=" << _totalFailures
       << ", timed_out=" << _totalTimedOut
       << ", images_with_product_omissions="
       << _totalProductOmissions << endl;
  if (_callbacks.runFinished) {
    _callbacks.runFinished(success);
  }
}
