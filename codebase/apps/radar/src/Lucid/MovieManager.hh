// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - see the LICENSE file for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef MovieManager_HH
#define MovieManager_HH

#include <cstdint>
#include <functional>

#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>

#include <toolsa/DateTime.hh>

#include "Params.hh"

class QTimer;

// Own the immutable movie cache lifecycle without owning display widgets.
// GuiManager supplies synchronous callbacks for view capture and MDV request
// operations; it owns this object and therefore outlives those callbacks.

class MovieManager : public QObject
{
public:

  struct Callbacks {
    std::function<void(int, bool)> clearFrames;
    std::function<void()> resetProducts;
    std::function<int(bool)> beginFrameRequest;
    std::function<bool(int, bool, QString &)> captureFrame;
    std::function<bool(int, bool)> showFrame;
    std::function<bool(int, bool)> hasFrame;
    std::function<QImage(int, bool)> frameImage;
    std::function<void()> stopPlayback;
    std::function<void(uint64_t, int)> frameRendered;
    std::function<void(uint64_t, int, const QString &)> frameCaptureFailed;
    std::function<QStringList()> pendingDependencies;
    std::function<void(const QString &)> showProgress;
  };

  MovieManager(const Params &params,
               Callbacks callbacks,
               QObject *parent = nullptr);

  uint64_t prepareBuild(int frameCount,
                        bool includeVertical,
                        const QString &fieldLabel);
  void beginFrameBuild(uint64_t buildId, int frameIndex, int frameCount);
  void cancelBuild();

  void expectGridGeneration(int frameIndex, int generation);
  bool markGridReady(int frameIndex, int generation);
  bool markVerticalReady(int generation);
  void checkFrameReady(bool dependenciesReady);

  bool showFrame(int frameIndex);
  void stopPlayback();
  bool exportLoop(int frameCount,
                  const DateTime &startTime,
                  QString &message) const;

  int buildFrame() const { return _buildFrame; }
  bool includesVertical() const { return _includesVertical; }
  int verticalRequestGeneration() const
    { return _verticalRequestGeneration; }

private:

  const Params &_params;
  Callbacks _callbacks;
  uint64_t _nextBuildId;
  uint64_t _activeBuildId;
  int _buildFrame;
  bool _gridReady;
  int _gridRequestGeneration;
  bool _includesVertical;
  bool _verticalReady;
  int _verticalRequestGeneration;
  QString _fieldLabel;
  QTimer *_frameTimeoutTimer;

  void _frameTimedOut();
  static QString _safeFileNameComponent(QString value);
};

#endif
