/////////////////////////////////////////////////////////////
// ClutProb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1005
//
///////////////////////////////////////////////////////////////

#ifndef ClutProb_hh
#define ClutProb_hh

////////////////////////
// This class

class ClutProb {
  
public:
  
  ClutProb();
  ~ClutProb();
  
  // combine two probability objects
  
  void ClutProb::combine(const ClutProb &prob1,
                         const ClutProb &prob2);

  // compute probability of clutter, based on the ratio of
  // power near 0 to power away from 0.
  
  void compute(int nSamples,
               const double *magnitude,
               double nyquist,
               double maxClutterVel,
               double initNotchWidth);
  
  // get fields

  inline double getPowerNarrow() const { return _powerNarrow; }
  inline double getRatioNarrow() const { return _ratioNarrow; }
  inline double getRatioWide() const { return _ratioWide; }
  inline bool  getClutterFound() const { return _clutterFound; }
  inline int getClutterPos() const { return _clutterPos; }
  inline double getClutterPeak() const { return _clutterPeak; }
  inline int getWeatherPos() const { return _weatherPos; }
  inline double getWeatherPeak() const { return _weatherPeak; }
  inline double getClutWxPeakRatio() const { return _clutWxPeakRatio; }
  inline double getClutWxPeakSeparation() const {
    return _clutWxPeakSeparation;
  }

  static const double missingDbl;

private:

  double _powerNarrow;
  double _ratioNarrow;
  double _ratioWide;
  bool _clutterFound;
  int _clutterPos;
  int _weatherPos;
  double _clutterPeak;
  double _weatherPeak;
  double _clutWxPeakRatio;
  double _clutWxPeakSeparation;

};

#endif

