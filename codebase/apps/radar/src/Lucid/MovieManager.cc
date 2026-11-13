// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - see the LICENSE file for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include "MovieManager.hh"

#include <utility>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTimeZone>

MovieManager::MovieManager(const Params &params,
                           Callbacks callbacks,
                           QObject *parent) :
        QObject(parent),
        _params(params),
        _callbacks(std::move(callbacks)),
        _buildFrame(-1),
        _gridReady(false),
        _includesVertical(false),
        _verticalReady(false),
        _verticalRequestGeneration(-1)
{
}

/////////////////////////////////////////////////////////////
// initialize one cache from a stable display configuration

void MovieManager::prepareBuild(int frameCount,
                                bool includeVertical,
                                const QString &fieldLabel)
{
  _buildFrame = -1;
  _gridReady = false;
  _includesVertical = includeVertical;
  _verticalReady = !_includesVertical;
  _verticalRequestGeneration = -1;
  _fieldLabel = fieldLabel;

  if (_callbacks.clearFrames) {
    _callbacks.clearFrames(frameCount, _includesVertical);
  }
  if (_callbacks.resetProducts) {
    // Frame zero may already be the live frame, but its symbolic products
    // still belong to a new immutable cache and must be requested again.
    _callbacks.resetProducts();
  }
}

/////////////////////////////////////////////////////////////
// begin the exact horizontal and optional vertical requests for one frame

void MovieManager::beginFrameBuild(int frameIndex, int frameCount)
{
  _buildFrame = frameIndex;
  _gridReady = false;
  _verticalReady = !_includesVertical;
  _verticalRequestGeneration = -1;

  if (_callbacks.beginFrameRequest) {
    _verticalRequestGeneration =
      _callbacks.beginFrameRequest(_includesVertical);
  }
  if (_includesVertical && _verticalRequestGeneration < 0) {
    // A route which can no longer produce a request must not deadlock the
    // cache. The vertical capture will retain its current no-data state.
    _verticalReady = true;
  }
  if (_callbacks.showProgress) {
    _callbacks.showProgress(
      tr("Building movie frame %1 of %2")
        .arg(frameIndex + 1).arg(frameCount));
  }
}

void MovieManager::cancelBuild()
{
  _buildFrame = -1;
  _gridReady = false;
  _includesVertical = false;
  _verticalReady = false;
  _verticalRequestGeneration = -1;
}

void MovieManager::markGridReady(int frameIndex)
{
  if (_buildFrame == frameIndex) {
    _gridReady = true;
  }
}

bool MovieManager::markVerticalReady(int generation)
{
  if (_buildFrame < 0 || !_includesVertical ||
      generation != _verticalRequestGeneration) {
    return false;
  }
  _verticalReady = true;
  return true;
}

/////////////////////////////////////////////////////////////
// publish a frame only after every optional display layer has settled

void MovieManager::checkFrameReady(bool dependenciesReady)
{
  if (_buildFrame < 0 || !_gridReady || !_verticalReady ||
      !dependenciesReady) {
    return;
  }

  const int completedFrame = _buildFrame;
  _buildFrame = -1;
  _gridReady = false;
  _verticalReady = false;
  if (_callbacks.captureFrame &&
      _callbacks.captureFrame(completedFrame, _includesVertical) &&
      _callbacks.frameRendered) {
    _callbacks.frameRendered(completedFrame);
  }
}

bool MovieManager::showFrame(int frameIndex)
{
  return _callbacks.showFrame &&
    _callbacks.showFrame(frameIndex, _includesVertical);
}

void MovieManager::stopPlayback()
{
  if (_callbacks.stopPlayback) {
    _callbacks.stopPlayback();
  }
}

/////////////////////////////////////////////////////////////
// write the complete cache as paired horizontal/vertical frame sequences

bool MovieManager::exportLoop(int frameCount,
                              const DateTime &startTime,
                              QString &message) const
{
  message.clear();
  if (frameCount < 1) {
    message = tr("The movie contains no frames.");
    return false;
  }
  if (!_callbacks.hasFrame || !_callbacks.frameImage) {
    message = tr("The movie views are not available.");
    return false;
  }
  for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
    if (!_callbacks.hasFrame(frameIndex, false) ||
        (_includesVertical &&
         !_callbacks.hasFrame(frameIndex, true))) {
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

  QString extension = _safeFileNameComponent(
    QString::fromUtf8(_params.images_file_name_extension)).toLower();
  if (extension.startsWith('.')) {
    extension.remove(0, 1);
  }
  if (extension.isEmpty() || extension == "field") {
    extension = "png";
  }
  const QString fieldPart = _safeFileNameComponent(_fieldLabel);
  const QString startPart = QDateTime::fromSecsSinceEpoch(
      startTime.utime(), QTimeZone(QTimeZone::UTC))
    .toString("yyyyMMdd_HHmmss'_UTC'");

  // One suffix pairs both views and prevents a repeated export from replacing
  // even a partially retained sequence from an earlier attempt.
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
        (_includesVertical && QFileInfo::exists(outputDir.filePath(
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
    if (!_callbacks.frameImage(frameIndex, false).save(horizontalPath)) {
      message = tr("Could not save movie frame:\n%1")
        .arg(horizontalPath);
      return false;
    }
    horizontalFiles.push_back(horizontalPath);

    if (_includesVertical) {
      const QString verticalPath = outputDir.filePath(
        "lucid_vert_" + commonPart);
      if (!_callbacks.frameImage(frameIndex, true).save(verticalPath)) {
        message = tr("Could not save movie frame:\n%1")
          .arg(verticalPath);
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
      (_includesVertical &&
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

QString MovieManager::_safeFileNameComponent(QString value)
{
  value = value.trimmed();
  value.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
  value.remove(QRegularExpression("^_+|_+$"));
  return value.isEmpty() ? QString("field") : value;
}
