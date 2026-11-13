// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - see the LICENSE file for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef ImageGenerationManager_HH
#define ImageGenerationManager_HH

#include <functional>

#include <QElapsedTimer>
#include <QImage>
#include <QObject>
#include <QString>
#include <QTimer>

#include <toolsa/DateTime.hh>

#include "AutomaticImageGenerator.hh"
#include "Params.hh"

class WorldPlot;

// Own the unattended image lifecycle without owning any display widgets.
// GuiManager supplies synchronous callbacks for operations which must touch
// live application state; it owns this object and therefore outlives every
// callback registered here.

class ImageGenerationManager : public QObject
{
public:

  struct Capture {
    QImage image;
    DateTime dataTime;
    const WorldPlot *world = nullptr;
    bool writeGeoXml = false;
  };

  struct Callbacks {
    std::function<void(const DateTime &)> batchStarted;
    std::function<void()> saveDisplayState;
    std::function<void()> restoreDisplayState;
    std::function<void()> jobRequested;
    std::function<void(const AutomaticImageGenerator::Job &, int)>
      cancelTimedOutRequest;
    std::function<bool(Capture &, QString &)> captureCurrentJob;
    std::function<void(bool)> runFinished;
  };

  ImageGenerationManager(const Params &params,
                         int numDataFields,
                         Callbacks callbacks,
                         QObject *parent = nullptr);

  bool isStarted() const { return _started; }
  bool isActive() const { return _active; }
  bool hasCurrentJob() const { return _generator.hasCurrentJob(); }
  bool currentJobIsHorizontal() const
    { return _generator.currentJobIsHorizontal(); }
  bool currentJobIsVertical() const
    { return _generator.currentJobIsVertical(); }
  const AutomaticImageGenerator::Job &currentJob() const
    { return _generator.currentJob(); }
  const AutomaticImageGenerator::Route &currentRoute() const
    { return _generator.currentRoute(); }
  int currentJobNumber() const { return _generator.currentJobNumber(); }
  int jobCount() const { return _generator.jobCount(); }
  int expectedGeneration() const { return _generator.expectedGeneration(); }

  void start();
  void setExpectedGeneration(int generation)
    { _generator.setExpectedGeneration(generation); }
  bool acceptDataCompletion(int generation)
    { return _generator.acceptDataCompletion(generation); }
  void checkReady(bool dependenciesReady, int unavailableProducts);
  void failCurrentJob(const QString &error);

private:

  const Params &_params;
  int _numDataFields;
  Callbacks _callbacks;
  AutomaticImageGenerator _generator;
  QTimer _timeoutTimer;
  QTimer _scheduleTimer;

  bool _started;
  bool _active;
  DateTime _imagesStartTime;
  DateTime _imagesEndTime;
  DateTime _batchTime;
  DateTime _nextImageTime;

  int _batchFailureBaseline;
  int _batchAttempted;
  int _batchSaved;
  int _batchTimedOut;
  int _batchProductOmissions;
  QElapsedTimer _batchElapsedTimer;

  int _totalFailures;
  int _totalAttempted;
  int _totalSaved;
  int _totalTimedOut;
  int _totalProductOmissions;
  int _completedBatches;

  bool _validateConfiguration(QString &error) const;
  void _startBatch(const DateTime &imageTime);
  void _requestCurrentJob();
  void _completeCurrentJob(bool saved,
                           const QString &path,
                           const QString &error);
  void _completeBatch();
  void _finishRun(const QString &fatalError);
  void _scheduleNextRealtimeBatch();
  void _scheduleDue();
  void _jobTimedOut();

  bool _publishImage(const Capture &capture,
                     QString &path,
                     QString &error);
  bool _writeGeoXml(const QString &imagePath,
                    const QImage &image,
                    const WorldPlot &world,
                    QString &error) const;
  bool _writeLatestDataInfo(const QString &imagePath,
                            const QByteArray &format,
                            const DateTime &imageTime,
                            QString &error) const;
};

#endif
