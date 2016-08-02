/////////////////////////////////////////////////////////////
// InterestMap.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// Handles interest mapping. Converts a data value into an
// interest value based on a linear function.
//
///////////////////////////////////////////////////////////////

#ifndef InterestMap_hh
#define InterestMap_hh

#include <string>
#include <vector>
using namespace std;

class InterestMap {
  
public:

  class ImPoint {
  public:
    ImPoint(double val, double interest) {
      _val = val;
      _interest = interest;
    }
    double getVal() const { return _val; }
    double getInterest() const { return _interest; }
  private:
    double _val;
    double _interest;
  };

  InterestMap(const string &label,
	      const vector<ImPoint> &map,
	      double weight);

  ~InterestMap();

  // get interest for a given val

  void getInterest(double val, double &interest, double &wt);

  // accumulate interest based on value

  void accumInterest(double val,
		     double &sumInterest, double &sumWt);
  
protected:
private:
  
  static const double _missingDbl = -9999.0;
  static const int _nLut = 10001;

  string _label;
  vector<ImPoint> _map;
  double _weight;

  bool _mapLoaded;
  double _minVal;
  double _maxVal;
  double _dVal;

  double *_lut;

};

#endif

