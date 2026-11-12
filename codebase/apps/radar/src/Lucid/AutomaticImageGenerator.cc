// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - see the LICENSE file for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#include "AutomaticImageGenerator.hh"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QImageWriter>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTimeZone>

AutomaticImageGenerator::AutomaticImageGenerator() :
        _jobIndex(-1),
        _expectedGeneration(-1),
        _dataReady(false),
        _failureCount(0)
{
}

/////////////////////////////////////////////////////////////
// build an immutable horizontal-and-vertical job sequence

bool AutomaticImageGenerator::configure(const Params &params,
                                         int numDataFields,
                                         QString &error)
{
  error.clear();
  _jobs.clear();
  _routes.clear();
  _jobIndex = -1;
  _failureCount = 0;
  _resetJobReadiness();

  std::vector<int> fields;
  const int fieldCount = std::min(params.fields_n, numDataFields);
  for (int ii = 0; ii < fieldCount; ++ii) {
    if (params._fields[ii].display_in_menu &&
        std::strlen(params._fields[ii].group_name) > 0) {
      fields.push_back(ii);
    }
  }
  if (fields.empty()) {
    error = "No display fields are enabled for output";
    return false;
  }
  if (fields.size() > 1 &&
      !params.images_include_field_name_in_file_name) {
    error = "images_include_field_name_in_file_name must be true when more "
            "than one field is generated";
    return false;
  }

  // Most products are named from the MDV field, preserving the established
  // catalog convention. If multiple menu entries use that same field (for
  // example to compare render methods or data sources), prefer their button
  // labels and finally a stable field index so no completed image is replaced.
  QHash<QString, int> fieldNameCounts;
  std::vector<QString> outputFieldNames;
  outputFieldNames.reserve(fields.size());
  for (const int field : fields) {
    const QString name = _safeFileNameComponent(
      QString::fromUtf8(params._fields[field].field_name));
    outputFieldNames.push_back(name);
    fieldNameCounts[name]++;
  }
  for (size_t ii = 0; ii < fields.size(); ++ii) {
    if (fieldNameCounts[outputFieldNames[ii]] > 1) {
      outputFieldNames[ii] = _safeFileNameComponent(
        QString::fromUtf8(params._fields[fields[ii]].button_label));
    }
  }
  QHash<QString, int> outputNameCounts;
  for (const QString &name : outputFieldNames) {
    outputNameCounts[name]++;
  }
  for (size_t ii = 0; ii < fields.size(); ++ii) {
    if (outputNameCounts[outputFieldNames[ii]] > 1) {
      outputFieldNames[ii] += QString("_field%1").arg(fields[ii] + 1);
    }
  }

  std::vector<double> heights;
  QSet<QString> heightNames;
  if (params.images_set_height_list) {
    for (int ii = 0; ii < params.images_height_list_n; ++ii) {
      const double height = params._images_height_list[ii];
      if (!std::isfinite(height)) {
        error = "images_height_list contains a non-finite value";
        return false;
      }
      const QString heightName = _heightName(height);
      if (heightNames.contains(heightName)) {
        error = QString("images_height_list contains heights which share "
                        "the output name %1").arg(heightName);
        return false;
      }
      heightNames.insert(heightName);
      heights.push_back(height);
    }
    if (heights.empty()) {
      error = "images_set_height_list is true but images_height_list is empty";
      return false;
    }
  } else {
    if (!std::isfinite(params.start_ht_km)) {
      error = "start_ht_km is not finite";
      return false;
    }
    heights.push_back(params.start_ht_km);
  }

  if (params.zoom_levels_n < 1) {
    error = "No predefined zoom levels are available for output";
    return false;
  }
  std::vector<std::pair<int, QString>> zooms;
  if (params.images_set_zoom_list) {
    if (params.images_zoom_list_n < 1) {
      error = "images_set_zoom_list is true but images_zoom_list is empty";
      return false;
    }
    QSet<QString> requestedLabels;
    QSet<QString> outputLabels;
    for (int ii = 0; ii < params.images_zoom_list_n; ++ii) {
      const QString requested = QString::fromUtf8(
        params._images_zoom_list[ii]).trimmed();
      if (requested.isEmpty() || requestedLabels.contains(requested)) {
        error = QString("images_zoom_list contains an empty or duplicate "
                        "label: %1").arg(requested);
        return false;
      }
      requestedLabels.insert(requested);

      int matchingIndex = -1;
      for (int izoom = 0; izoom < params.zoom_levels_n; ++izoom) {
        if (requested == QString::fromUtf8(
              params._zoom_levels[izoom].label)) {
          if (matchingIndex >= 0) {
            error = QString("zoom_levels contains duplicate label: %1")
              .arg(requested);
            return false;
          }
          matchingIndex = izoom;
        }
      }
      if (matchingIndex < 0) {
        error = QString("images_zoom_list label is not in zoom_levels: %1")
          .arg(requested);
        return false;
      }

      const QString outputLabel = _safeFileNameComponent(requested);
      if (outputLabels.contains(outputLabel)) {
        error = QString("images_zoom_list contains labels which share the "
                        "output name %1").arg(outputLabel);
        return false;
      }
      outputLabels.insert(outputLabel);
      zooms.push_back({matchingIndex, outputLabel});
    }
  } else {
    zooms.push_back({0, _safeFileNameComponent(
      QString::fromUtf8(params._zoom_levels[0].label))});
  }

  // Height and zoom form the stable outer dimensions. Field remains innermost
  // because every selection requires a distinct MDV request regardless of the
  // current display geometry.
  for (const double height : heights) {
    for (const auto &zoom : zooms) {
      for (size_t ii = 0; ii < fields.size(); ++ii) {
        _jobs.push_back({JobType::Horizontal, fields[ii], height,
                         zoom.first, zoom.second, outputFieldNames[ii], -1});
      }
    }
  }

  if (params.generate_vsection_images) {
    if (params.images_vsection_spec_n < 1) {
      error = "generate_vsection_images is true but images_vsection_spec "
              "is empty";
      return false;
    }
    QSet<QString> routeNames;
    for (int ii = 0; ii < params.images_vsection_spec_n; ++ii) {
      const Params::images_vsection_spec_t &spec =
        params._images_vsection_spec[ii];
      const QString rawName = QString::fromUtf8(spec.vsection_label).trimmed();
      const QString routeName = _safeFileNameComponent(rawName);
      if (rawName.isEmpty() || routeNames.contains(routeName)) {
        error = QString("images_vsection_spec contains an empty or "
                        "filename-equivalent label: %1").arg(rawName);
        return false;
      }

      std::vector<QPointF> points;
      if (!_parseRoutePoints(QString::fromUtf8(spec.waypt_locs), points) ||
          points.size() != static_cast<size_t>(spec.n_waypts) ||
          points.size() < 2) {
        error = QString("Invalid waypt_locs or n_waypts for vertical "
                        "section %1").arg(rawName);
        return false;
      }
      routeNames.insert(routeName);
      _routes.push_back({routeName, std::move(points)});
    }

    for (size_t iroute = 0; iroute < _routes.size(); ++iroute) {
      for (size_t ifield = 0; ifield < fields.size(); ++ifield) {
        _jobs.push_back({JobType::Vertical, fields[ifield], 0.0, -1,
                         QString(), outputFieldNames[ifield],
                         static_cast<int>(iroute)});
      }
    }
  }
  return true;
}

bool AutomaticImageGenerator::start()
{
  if (_jobs.empty()) {
    return false;
  }
  _jobIndex = 0;
  _resetJobReadiness();
  return true;
}

bool AutomaticImageGenerator::advance()
{
  if (!hasCurrentJob()) {
    return false;
  }
  ++_jobIndex;
  _resetJobReadiness();
  return hasCurrentJob();
}

bool AutomaticImageGenerator::hasCurrentJob() const
{
  return _jobIndex >= 0 && _jobIndex < static_cast<int>(_jobs.size());
}

const AutomaticImageGenerator::Job &
AutomaticImageGenerator::currentJob() const
{
  return _jobs.at(_jobIndex);
}

int AutomaticImageGenerator::currentJobNumber() const
{
  return hasCurrentJob() ? _jobIndex + 1 : 0;
}

int AutomaticImageGenerator::jobCount() const
{
  return static_cast<int>(_jobs.size());
}

bool AutomaticImageGenerator::hasMultipleHeights() const
{
  bool found = false;
  double firstHeight = 0.0;
  for (const Job &job : _jobs) {
    if (job.type != JobType::Horizontal) {
      continue;
    }
    if (!found) {
      firstHeight = job.heightKm;
      found = true;
      continue;
    }
    if (job.heightKm != firstHeight) {
      return true;
    }
  }
  return false;
}

bool AutomaticImageGenerator::hasMultipleZooms() const
{
  bool found = false;
  int firstZoom = -1;
  for (const Job &job : _jobs) {
    if (job.type != JobType::Horizontal) {
      continue;
    }
    if (!found) {
      firstZoom = job.zoomIndex;
      found = true;
      continue;
    }
    if (job.zoomIndex != firstZoom) {
      return true;
    }
  }
  return false;
}

bool AutomaticImageGenerator::currentJobIsHorizontal() const
{
  return hasCurrentJob() && currentJob().type == JobType::Horizontal;
}

bool AutomaticImageGenerator::currentJobIsVertical() const
{
  return hasCurrentJob() && currentJob().type == JobType::Vertical;
}

const AutomaticImageGenerator::Route &
AutomaticImageGenerator::currentRoute() const
{
  return _routes.at(currentJob().routeIndex);
}

void AutomaticImageGenerator::setExpectedGeneration(int generation)
{
  _expectedGeneration = generation;
}

int AutomaticImageGenerator::expectedGeneration() const
{
  return _expectedGeneration;
}

bool AutomaticImageGenerator::acceptDataCompletion(int generation)
{
  if (!hasCurrentJob() || generation < 0 ||
      generation != _expectedGeneration) {
    return false;
  }
  _dataReady = true;
  return true;
}

bool AutomaticImageGenerator::readyToSave(bool productsReady) const
{
  return hasCurrentJob() && _dataReady && productsReady;
}

void AutomaticImageGenerator::recordFailure()
{
  ++_failureCount;
}

int AutomaticImageGenerator::failureCount() const
{
  return _failureCount;
}

/////////////////////////////////////////////////////////////
// construct and create the catalog destination for the current job

bool AutomaticImageGenerator::prepareOutput(const Params &params,
                                             const DateTime &imageTime,
                                             QString &path,
                                             QByteArray &format,
                                             QString &error) const
{
  path.clear();
  format.clear();
  error.clear();
  if (!hasCurrentJob()) {
    error = "No automatic image job is active";
    return false;
  }

  QString extension = QString::fromUtf8(
    params.images_file_name_extension).trimmed().toLower();
  while (extension.startsWith('.')) {
    extension.remove(0, 1);
  }
  format = extension.toLatin1();
  if (extension.isEmpty() ||
      !QImageWriter::supportedImageFormats().contains(format)) {
    error = QString("Unsupported images_file_name_extension: %1")
      .arg(extension);
    return false;
  }

  QString outputRoot = QString::fromUtf8(params.images_output_dir).trimmed();
  outputRoot.replace("$(HOME)", QDir::homePath());
  if (outputRoot.startsWith("$HOME")) {
    outputRoot.replace(0, 5, QDir::homePath());
  }
  if (outputRoot.isEmpty()) {
    error = "images_output_dir is empty";
    return false;
  }

  QString relativeDir;
  if (params.images_write_to_day_dir) {
    relativeDir = QString("%1%2%3")
      .arg(imageTime.getYear(), 4, 10, QChar('0'))
      .arg(imageTime.getMonth(), 2, 10, QChar('0'))
      .arg(imageTime.getDay(), 2, 10, QChar('0'));
  }
  QString viewSubdir = QDir::cleanPath(QString::fromUtf8(
    currentJobIsHorizontal() ? params.images_horiz_subdir :
                               params.images_vsect_subdir).trimmed());
  if (viewSubdir == ".") {
    viewSubdir.clear();
  }
  if (QDir::isAbsolutePath(viewSubdir) || viewSubdir == ".." ||
      viewSubdir.startsWith("../") || viewSubdir.contains("/../")) {
    error = "Image subdirectory must remain below images_output_dir";
    return false;
  }
  if (!viewSubdir.isEmpty()) {
    relativeDir = relativeDir.isEmpty() ? viewSubdir :
      QDir(relativeDir).filePath(viewSubdir);
  }

  outputRoot = QDir::cleanPath(outputRoot);
  QDir directoryCreator;
  if (!directoryCreator.mkpath(outputRoot)) {
    error = QString("Cannot create images_output_dir: %1").arg(outputRoot);
    return false;
  }
  QDir outputDir(outputRoot);
  if (!relativeDir.isEmpty() && !outputDir.mkpath(relativeDir)) {
    error = QString("Cannot create automatic image directory: %1")
      .arg(outputDir.filePath(relativeDir));
    return false;
  }
  if (!relativeDir.isEmpty() && !outputDir.cd(relativeDir)) {
    error = QString("Cannot access automatic image directory: %1")
      .arg(relativeDir);
    return false;
  }

  const QString delimiter = QString::fromUtf8(
    params.images_file_name_delimiter);
  if (delimiter.contains('/') || delimiter.contains('\\')) {
    error = "images_file_name_delimiter cannot contain a path separator";
    return false;
  }

  QStringList nameParts;
  auto appendNamePart = [&](const char *value) {
    const QString part = QString::fromUtf8(value).trimmed();
    if (!part.isEmpty()) {
      nameParts.push_back(_safeFileNameComponent(part));
    }
  };
  appendNamePart(params.images_file_name_category);
  appendNamePart(params.images_file_name_platform);
  if (params.images_include_time_part_in_file_name) {
    const QString timeFormat = params.images_include_seconds_in_time_part ?
      "yyyyMMddHHmmss" : "yyyyMMddHHmm";
    nameParts.push_back(QDateTime::fromSecsSinceEpoch(
      imageTime.utime(), QTimeZone(QTimeZone::UTC)).toString(timeFormat));
  }

  QString productName;
  if (params.images_include_field_name_in_file_name) {
    productName = currentJob().outputFieldName;
  }
  productName += QString::fromUtf8(currentJobIsHorizontal() ?
    params.image_field_suffix_horiz : params.image_field_suffix_vsect);
  if (!productName.trimmed().isEmpty()) {
    nameParts.push_back(_safeFileNameComponent(productName));
  }
  if (currentJobIsHorizontal() && hasMultipleHeights()) {
    nameParts.push_back(_heightName(currentJob().heightKm));
  }
  if (currentJobIsHorizontal() && hasMultipleZooms()) {
    nameParts.push_back(currentJob().zoomName);
  }
  if (currentJobIsVertical()) {
    nameParts.push_back(currentRoute().name);
  }
  if (nameParts.empty()) {
    nameParts.push_back("image");
  }

  path = outputDir.filePath(nameParts.join(delimiter) + "." + extension);
  return true;
}

void AutomaticImageGenerator::_resetJobReadiness()
{
  _expectedGeneration = -1;
  _dataReady = false;
}

QString AutomaticImageGenerator::_safeFileNameComponent(QString value)
{
  value = value.trimmed();
  value.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
  value.remove(QRegularExpression("^_+|_+$"));
  return value.isEmpty() ? QString("field") : value;
}

QString AutomaticImageGenerator::_heightName(double heightKm)
{
  QString value = QString::number(heightKm, 'f', 6);
  while (value.contains('.') && value.endsWith('0')) {
    value.chop(1);
  }
  if (value.endsWith('.')) {
    value.chop(1);
  }
  return _safeFileNameComponent(value + "km");
}

bool AutomaticImageGenerator::_parseRoutePoints(
        const QString &text,
        std::vector<QPointF> &points)
{
  points.clear();
  const QRegularExpression pairExpression(
    R"(\(\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)\s*,\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)\s*\))");
  QRegularExpressionMatchIterator matches =
    pairExpression.globalMatch(text);
  while (matches.hasNext()) {
    const QRegularExpressionMatch match = matches.next();
    bool firstOk = false;
    bool secondOk = false;
    const double first = match.captured(1).toDouble(&firstOk);
    const double second = match.captured(2).toDouble(&secondOk);
    if (!firstOk || !secondOk || !std::isfinite(first) ||
        !std::isfinite(second)) {
      points.clear();
      return false;
    }
    points.emplace_back(first, second);
  }

  QString remainder = text;
  remainder.remove(pairExpression);
  if (!remainder.trimmed().isEmpty() || points.size() < 2) {
    points.clear();
    return false;
  }
  for (size_t ii = 1; ii < points.size(); ++ii) {
    if (points[ii] != points[0]) {
      return true;
    }
  }
  points.clear();
  return false;
}
