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

  struct Job {
    int fieldIndex = -1;
    double heightKm = 0.0;
    QString outputFieldName;
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

  void setExpectedGeneration(int generation);
  int expectedGeneration() const;
  bool acceptGridCompletion(int generation);
  bool readyToSave(bool productsReady) const;

  void recordFailure();
  int failureCount() const;

  bool prepareOutput(const Params &params,
                     const DateTime &imageTime,
                     QString &path,
                     QString &outputRoot,
                     QByteArray &format,
                     QString &error) const;

private:

  std::vector<Job> _jobs;
  int _jobIndex;
  int _expectedGeneration;
  bool _gridReady;
  int _failureCount;

  void _resetJobReadiness();
  static QString _safeFileNameComponent(QString value);
  static QString _heightName(double heightKm);
};

#endif
