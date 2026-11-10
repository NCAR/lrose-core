// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** BSD licence applies - see the source distribution for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////
// TimePlotWidget.hh
//
// Compact movie-time axis owned by the time controller dialog.
////////////////////////////////////////////////////////////////

#ifndef TimePlotWidget_HH
#define TimePlotWidget_HH

#include <ctime>
#include <functional>

#include <QPoint>
#include <QRectF>
#include <QWidget>

#include "Params.hh"

class QMouseEvent;
class QPaintEvent;

class TimePlotWidget : public QWidget
{
public:
  explicit TimePlotWidget(QWidget *parent, const Params &params);

  void setMovieState(time_t startTime, int nFrames,
                     double frameIntervalSecs, int selectedFrame);
  void setExpansionCallback(
    const std::function<void(time_t, int)> &callback) {
    _expansionCallback = callback;
  }
  void fitMovie();

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  enum class DragMode { None, Zoom, Pan, Expand };

  const Params &_params;
  double _movieStart;
  double _movieEnd;
  double _viewStart;
  double _viewEnd;
  double _pressViewStart;
  double _pressViewEnd;
  double _frameIntervalSecs;
  int _nFrames;
  int _selectedFrame;
  DragMode _dragMode;
  QPoint _dragStart;
  QPoint _dragCurrent;
  std::function<void(time_t, int)> _expansionCallback;

  QRectF _plotRect() const;
  double _pixelForTime(double time) const;
  double _timeForPixel(double x, double start, double end) const;
  double _floorToFrame(double time) const;
  double _ceilToFrame(double time) const;
  void _finishZoom();
  void _finishPan();
  void _finishExpand();
};

#endif
