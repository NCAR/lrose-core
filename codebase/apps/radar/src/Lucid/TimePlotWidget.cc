// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** BSD licence applies - see the source distribution for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////
// TimePlotWidget.cc
////////////////////////////////////////////////////////////////

#include "TimePlotWidget.hh"

#include <algorithm>
#include <cmath>

#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

#include "Constants.hh"

namespace {

QColor colorOr(const char *name, const QColor &fallback)
{
  const QColor color(name);
  return color.isValid() ? color : fallback;
}

}

TimePlotWidget::TimePlotWidget(QWidget *parent, const Params &params) :
        QWidget(parent),
        _params(params),
        _movieStart(0.0),
        _movieEnd(0.0),
        _viewStart(0.0),
        _viewEnd(0.0),
        _pressViewStart(0.0),
        _pressViewEnd(0.0),
        _frameIntervalSecs(1.0),
        _nFrames(0),
        _selectedFrame(0),
        _dragMode(DragMode::None)
{
  setMinimumHeight(92);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setMouseTracking(true);
  setToolTip("Left drag: zoom time; middle drag: pan time; "
             "right drag: expand movie; double-click: fit movie");
}

QSize TimePlotWidget::sizeHint() const
{
  return QSize(800, 92);
}

void TimePlotWidget::setMovieState(time_t startTime, int nFrames,
                                   double frameIntervalSecs,
                                   int selectedFrame)
{
  const double newStart = static_cast<double>(startTime);
  const double newInterval = std::max(1.0, frameIntervalSecs);
  const int newNFrames = std::max(1, nFrames);
  const double newEnd = newStart + newNFrames * newInterval;
  const bool rangeChanged = fabs(newStart - _movieStart) > 0.01 ||
                            fabs(newEnd - _movieEnd) > 0.01;

  _movieStart = newStart;
  _movieEnd = newEnd;
  _frameIntervalSecs = newInterval;
  _nFrames = newNFrames;
  _selectedFrame = std::clamp(selectedFrame, 0, newNFrames - 1);
  if (rangeChanged || _viewEnd <= _viewStart) {
    fitMovie();
  } else {
    update();
  }
}

void TimePlotWidget::fitMovie()
{
  _viewStart = _movieStart;
  _viewEnd = _movieEnd;
  update();
}

QRectF TimePlotWidget::_plotRect() const
{
  return QRectF(10.0, 4.0, std::max(1, width() - 20),
                std::max(1, height() - 26));
}

double TimePlotWidget::_pixelForTime(double time) const
{
  const QRectF plot = _plotRect();
  if (_viewEnd <= _viewStart) {
    return plot.left();
  }
  return plot.left() + (time - _viewStart) * plot.width() /
                       (_viewEnd - _viewStart);
}

double TimePlotWidget::_timeForPixel(double x, double start, double end) const
{
  const QRectF plot = _plotRect();
  const double fraction = (x - plot.left()) / plot.width();
  return start + fraction * (end - start);
}

double TimePlotWidget::_floorToFrame(double time) const
{
  const double frame = floor((time - _movieStart) / _frameIntervalSecs);
  return _movieStart + frame * _frameIntervalSecs;
}

double TimePlotWidget::_ceilToFrame(double time) const
{
  const double frame = ceil((time - _movieStart) / _frameIntervalSecs);
  return _movieStart + frame * _frameIntervalSecs;
}

void TimePlotWidget::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.fillRect(rect(), colorOr(_params.margin_color, Qt::black));
  if (_movieEnd <= _movieStart || _viewEnd <= _viewStart) {
    return;
  }

  const QRectF plot = _plotRect();
  const double bandTop = plot.top() + 2.0;
  const double bandBottom = bandTop + 17.0;
  const double axisY = plot.bottom();
  const QColor axisColor = colorOr(_params.time_axis_color, Qt::white);
  const QColor epochColor = colorOr(_params.epoch_indicator_color,
                                    QColor("orange"));
  const QColor frameColor = colorOr(_params.time_frame_color,
                                    QColor("gray"));

  painter.save();
  painter.setClipRect(plot.adjusted(0.0, 0.0, 1.0, 1.0));

  // The selected frame is filled before the epoch outline so its boundaries
  // remain visible even when the time view is tightly zoomed.
  const double selectedStart = _movieStart +
    _selectedFrame * _frameIntervalSecs;
  const double selectedEnd = selectedStart + _frameIntervalSecs;
  painter.fillRect(QRectF(_pixelForTime(selectedStart), bandTop,
                          _pixelForTime(selectedEnd) -
                            _pixelForTime(selectedStart),
                          bandBottom - bandTop), frameColor);

  const double movieLeft = _pixelForTime(_movieStart);
  const double movieRight = _pixelForTime(_movieEnd);
  painter.setPen(QPen(epochColor, 2));
  painter.drawRect(QRectF(movieLeft, bandTop,
                          movieRight - movieLeft, bandBottom - bandTop));

  const double framePixels = fabs(_pixelForTime(_movieStart +
                                                 _frameIntervalSecs) -
                                  movieLeft);
  int dividerStep = 1;
  if (framePixels > 0.0 && framePixels < 4.0) {
    dividerStep = std::max(1, static_cast<int>(ceil(4.0 / framePixels)));
  }
  painter.setPen(QPen(epochColor, 1));
  for (int ii = dividerStep; ii < _nFrames; ii += dividerStep) {
    const double x = _pixelForTime(_movieStart +
                                   ii * _frameIntervalSecs);
    painter.drawLine(QPointF(x, bandTop), QPointF(x, bandBottom));
  }

  painter.setPen(QPen(axisColor, 1));
  painter.drawLine(QPointF(plot.left(), axisY),
                   QPointF(plot.right(), axisY));

  QFont axisFont = font();
  axisFont.setPixelSize(10);
  painter.setFont(axisFont);
  const int labelWidth = painter.fontMetrics().horizontalAdvance("00:00:00");
  const double minTickSecs = (_viewEnd - _viewStart) *
                             (labelWidth + 12.0) / plot.width();
  const int canonical[] = {
    1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 900, 1800,
    3600, 7200, 10800, 21600, 43200, 86400, 172800, 604800
  };
  int tickSecs = canonical[sizeof(canonical) / sizeof(canonical[0]) - 1];
  for (int candidate : canonical) {
    if (candidate >= minTickSecs) {
      tickSecs = candidate;
      break;
    }
  }
  const qint64 firstTick =
    static_cast<qint64>(ceil(_viewStart / tickSecs)) * tickSecs;
  double previousLabelRight = plot.left() - 1.0;
  for (qint64 tick = firstTick; tick <= _viewEnd; tick += tickSecs) {
    const double x = _pixelForTime(static_cast<double>(tick));
    painter.drawLine(QPointF(x, axisY), QPointF(x, axisY - 4.0));
    const QDateTime dateTime = QDateTime::fromSecsSinceEpoch(tick, Qt::UTC);
    QString label = dateTime.toString("HH:mm:ss");
    if (tickSecs >= 86400 || dateTime.time() == QTime(0, 0)) {
      label = dateTime.toString("MM/dd HH:mm");
    }
    const int textWidth = painter.fontMetrics().horizontalAdvance(label);
    const double textLeft = x - textWidth / 2.0;
    if (textLeft > previousLabelRight + 4.0 &&
        textLeft + textWidth <= plot.right()) {
      painter.drawText(QPointF(textLeft, height() - 4.0), label);
      previousLabelRight = textLeft + textWidth;
    }
  }

  const double now = static_cast<double>(time(nullptr));
  if (now >= _viewStart && now <= _viewEnd) {
    painter.setPen(QPen(colorOr(_params.now_time_color, Qt::red), 1));
    const double x = _pixelForTime(now);
    painter.drawLine(QPointF(x, plot.top()), QPointF(x, axisY));
  }
  painter.restore();

  if (_dragMode == DragMode::Zoom || _dragMode == DragMode::Expand) {
    const double left = std::min(_dragStart.x(), _dragCurrent.x());
    const double right = std::max(_dragStart.x(), _dragCurrent.x());
    QColor dragColor = _dragMode == DragMode::Zoom ? QColor("cyan") :
                                                    epochColor;
    dragColor.setAlpha(70);
    painter.fillRect(QRectF(left, plot.top(), right - left, plot.height()),
                     dragColor);
    painter.setPen(QPen(dragColor.lighter(180), 1, Qt::DashLine));
    painter.drawRect(QRectF(left, plot.top(), right - left, plot.height()));
  }
}

void TimePlotWidget::mousePressEvent(QMouseEvent *event)
{
  if (!_plotRect().contains(event->position())) {
    return;
  }
  if (event->button() == Qt::LeftButton) {
    _dragMode = DragMode::Zoom;
  } else if (event->button() == Qt::MiddleButton) {
    _dragMode = DragMode::Pan;
    setCursor(Qt::ClosedHandCursor);
  } else if (event->button() == Qt::RightButton) {
    _dragMode = DragMode::Expand;
  } else {
    return;
  }
  _dragStart = event->position().toPoint();
  _dragCurrent = _dragStart;
  _pressViewStart = _viewStart;
  _pressViewEnd = _viewEnd;
  event->accept();
}

void TimePlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (_dragMode == DragMode::None) {
    return;
  }
  _dragCurrent = event->position().toPoint();
  if (_dragMode == DragMode::Pan) {
    const QRectF plot = _plotRect();
    const double delta = -(_dragCurrent.x() - _dragStart.x()) *
                         (_pressViewEnd - _pressViewStart) / plot.width();
    const double frameDelta = round(delta / _frameIntervalSecs);
    _viewStart = _pressViewStart + frameDelta * _frameIntervalSecs;
    _viewEnd = _pressViewEnd + frameDelta * _frameIntervalSecs;
  }
  update();
  event->accept();
}

void TimePlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (_dragMode == DragMode::None) {
    return;
  }
  _dragCurrent = event->position().toPoint();
  const DragMode completedMode = _dragMode;
  _dragMode = DragMode::None;
  unsetCursor();
  if (completedMode == DragMode::Zoom) {
    _finishZoom();
  } else if (completedMode == DragMode::Pan) {
    _finishPan();
  } else if (completedMode == DragMode::Expand) {
    _finishExpand();
  }
  update();
  event->accept();
}

void TimePlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    _dragMode = DragMode::None;
    fitMovie();
    event->accept();
  }
}

void TimePlotWidget::_finishZoom()
{
  const QRectF plot = _plotRect();
  const double left = std::clamp(
    static_cast<double>(std::min(_dragStart.x(), _dragCurrent.x())),
    plot.left(), plot.right());
  const double right = std::clamp(
    static_cast<double>(std::max(_dragStart.x(), _dragCurrent.x())),
    plot.left(), plot.right());
  if (right - left < 4.0) {
    return;
  }
  const double start = _floorToFrame(
    _timeForPixel(left, _pressViewStart, _pressViewEnd));
  const double end = _ceilToFrame(
    _timeForPixel(right, _pressViewStart, _pressViewEnd));
  if (end - start >= _frameIntervalSecs) {
    _viewStart = start;
    _viewEnd = end;
  }
}

void TimePlotWidget::_finishPan()
{
  const QRectF plot = _plotRect();
  const double delta = -(_dragCurrent.x() - _dragStart.x()) *
                       (_pressViewEnd - _pressViewStart) / plot.width();
  const double frameDelta = round(delta / _frameIntervalSecs);
  _viewStart = _pressViewStart + frameDelta * _frameIntervalSecs;
  _viewEnd = _pressViewEnd + frameDelta * _frameIntervalSecs;
}

void TimePlotWidget::_finishExpand()
{
  const QRectF plot = _plotRect();
  const double left = std::clamp(
    static_cast<double>(std::min(_dragStart.x(), _dragCurrent.x())),
    plot.left(), plot.right());
  const double right = std::clamp(
    static_cast<double>(std::max(_dragStart.x(), _dragCurrent.x())),
    plot.left(), plot.right());
  if (right - left < 4.0) {
    return;
  }

  const double leftFraction = (left - plot.left()) / plot.width();
  const double rightFraction = (right - plot.left()) / plot.width();
  const double fractionSpan = rightFraction - leftFraction;
  if (fractionSpan <= 0.0) {
    return;
  }

  const double oldDuration = _movieEnd - _movieStart;
  const double rawDuration = oldDuration / fractionSpan;
  const double rawStart = _movieStart - leftFraction * rawDuration;
  const double rawEnd = rawStart + rawDuration;
  const double newStart = _floorToFrame(rawStart);
  const double newEnd = _ceilToFrame(rawEnd);
  const int newNFrames = static_cast<int>(
    llround((newEnd - newStart) / _frameIntervalSecs));
  if (newNFrames < _nFrames || newNFrames > Constants::MAX_FRAMES) {
    if (newNFrames > Constants::MAX_FRAMES) {
      QToolTip::showText(mapToGlobal(_dragCurrent),
                         "Expanded movie would exceed the frame limit", this);
    }
    return;
  }
  if (_expansionCallback) {
    _expansionCallback(static_cast<time_t>(llround(newStart)), newNFrames);
  }
}
