// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - see the LICENSE file for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef AutomaticImageGenerator_HH
#define AutomaticImageGenerator_HH

#include <QByteArray>
#include <QPointF>
#include <QString>
#include <vector>

#include <toolsa/DateTime.hh>

#include "Params.hh"

// AutomaticImageGenerator owns the deterministic part of unattended output:
// job ordering, exact-request readiness, failure accounting, and catalog
// naming. GuiManager remains responsible for changing live display state and
// capturing widgets after this class declares the current job ready.

class AutomaticImageGenerator
{
public:

  enum class JobType {
    Horizontal,
    Vertical
  };

  struct Route {
    QString name;
    std::vector<QPointF> points;
  };

  struct Job {
    JobType type = JobType::Horizontal;
    int fieldIndex = -1;
    double heightKm = 0.0;
    int zoomIndex = -1;
    QString zoomName;
    QString outputFieldName;
    int routeIndex = -1;
  };

  AutomaticImageGenerator();

  bool configure(const Params &params, int numDataFields, QString &error);
  bool start();
  bool advance();

  bool hasCurrentJob() const;
  const Job &currentJob() const;
  int currentJobNumber() const;
  int jobCount() const;
  bool hasMultipleHeights() const;
  bool hasMultipleZooms() const;
  bool currentJobIsHorizontal() const;
  bool currentJobIsVertical() const;
  const Route &currentRoute() const;

  void setExpectedGeneration(int generation);
  int expectedGeneration() const;
  bool acceptDataCompletion(int generation);
  bool readyToSave(bool productsReady) const;

  void recordFailure();
  int failureCount() const;

  bool prepareOutput(const Params &params,
                     const DateTime &imageTime,
                     QString &path,
                     QByteArray &format,
                     QString &error) const;

private:

  std::vector<Job> _jobs;
  std::vector<Route> _routes;
  int _jobIndex;
  int _expectedGeneration;
  bool _dataReady;
  int _failureCount;

  void _resetJobReadiness();
  static QString _safeFileNameComponent(QString value);
  static QString _heightName(double heightKm);
  static bool _parseRoutePoints(const QString &text,
                                std::vector<QPointF> &points);
};

#endif
