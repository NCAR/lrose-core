/**
 * @file Histo.hh
 * @brief
 * @class Histo
 * @brief
 */

#ifndef Histo_h
#define Histo_h

#include <vector>

class Histo
{
public:
  Histo(const double res, const double min, const double max);
  ~Histo();
  void clear(void);
  void addValue(double d);
  bool getMedian(double &m) const;
  bool getPercentile(double pct, double &m) const;
private:

  bool _pcntile(double pct, double &m) const;

  double _binMin;
  double _binMax;
  double _binDelta;
  int _nbin;
  std::vector<double> _bin;
  std::vector<double> _counts;
  double _nc;
};

#endif
