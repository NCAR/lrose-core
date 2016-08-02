/////////////////////////////////////////////////////////////
// Beam.hh
//
// Beam object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <string>
#include <vector>
#include <cstdio>
#include "Params.hh"
#include "Complex.hh"
#include "Pulse.hh"
#include "MomentsMgr.hh"
#include "InterestMap.hh"
#include "GateSpectra.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // constructor

  Beam (const string &prog_name,
	const Params &params,
	int max_trips,
	const deque<Pulse *> pulse_queue,
	double az,
	const MomentsMgr *momentsMgr);
  
  // destructor
  
  ~Beam();

  // Create interest maps.
  // These are static on the class, and should be created before any
  // beams are constructed.
 
  static int createInterestMaps(const Params &params);
  
  // compute moments
  //
  // Set combinedSpectraFile to NULL to prevent printing.
  //
  // Returns 0 on success, -1 on failure
  
  int computeMoments(int &combinedPrintCount,
		     FILE *combinedSpectraFile);

  int computeMomentsSinglePol(int &combinedPrintCount,
			      FILE *combinedSpectraFile);

  int computeMomentsDualPol(int &combinedPrintCount,
			    FILE *combinedSpectraFile);

  // compute REC
  
  void computeRec(const deque<Beam *> beams, int midBeamIndex);

  // filter clutter, using rec values to decide whether to apply the filter
  
  void filterClutterUsingRec();

  // get methods

  int getNSamples() const { return _nSamples; }
  int get_maxTrips() const { return _maxTrips; }
  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getPrf() const { return _prf; }
  double getPrt() const { return _prt; }
  time_t getTime() const { return _time; }
  int getNGatesPulse() const { return _nGatesPulse; }
  int getNGatesOut() const { return _nGatesOut; }
  const MomentsMgr *getMomentsMgr() const { return _momentsMgr; }
  
  double getStartRange() const { return _momentsMgr->getStartRange(); }
  double getGateSpacing() const { return _momentsMgr->getGateSpacing(); }

  const int* getFlags() const { return _flags; }
  const int* getTripFlag() const { return _tripFlag; }
  const int* getVcensor() const { return _vcensor; }
  const int* getCensor() const { return _censor; }
  const int* getZinfill() const { return _zinfill; }

  const double* getPowerDbm() const { return _powerDbm; }
  const double* getSnr() const { return _snr; }

  const double* getNoiseDbm() const { return _noiseDbm; }
  const double* getSnrn() const { return _snrn; }
  const double* getDbzn() const { return _dbzn; }

  const double* getDbzt() const { return _dbzt; }
  const double* getDbz() const { return _dbz; }
  const double* getVel() const { return _vel; }
  const double* getWidth() const { return _width; }

  const double* getClut() const { return _clut; }
  const double* getDbzf() const { return _dbzf; }
  const double* getVelf() const { return _velf; }
  const double* getWidthf() const { return _widthf; }

  const double* getDbzi() const { return _dbzi; }
  const double* getVeli() const { return _veli; }
  const double* getWidthi() const { return _widthi; }

  const double* getLeakage() const { return _leakage; }
  const double* getVtexture() const { return _vtexture; }
  const double* getVspin() const { return _vspin; }
  const double* getFcensor() const { return _fcensor; }
  const double* getZtexture() const { return _ztexture; }

  const double* getPhidp() const { return _phidp; }
  const double* getKdp() const { return _kdp; }
  const double* getZdr() const { return _zdr; }
  const double* getRhohv() const { return _rhohv; }

  const double* getRecDbzDbzDiffSq() const { return _recDbzDiffSq; }
  const double* getRecDbzSpinChange() const { return _recDbzSpinChange; }
  const double* getRecDbzTexture() const { return _recDbzTexture; }
  const double* getRecSqrtTexture() const { return _recSqrtTexture; }
  const double* getRecDbzSpin() const { return _recDbzSpin; }
  const double* getRecWtSpin() const { return _recWtSpin; }
  const double* getRecVel() const { return _recVel; }
  const double* getRecVelSdev() const { return _recVelSdev; }
  const double* getRecWidth() const { return _recWidth; }
  const double* getRecClutDbzNarrow() const { return _recClutDbzNarrow; }
  const double* getRecClutRatioNarrow() const { return _recClutRatioNarrow; }
  const double* getRecClutRatioWide() const { return _recClutRatioWide; }
  const double* getRecClutWxPeakRatio() const { return _recClutWxPeakRatio; }
  const double* getRecClutWxPeakSep() const { return _recClutWxPeakSep; }
  const double* getRecClutWxPeakSepSdev() const {
    return _recClutWxPeakSepSdev;
  }
  const double* getRecZdrSdev() const { return _recZdrSdev; }
  const double* getRecRhohvSdev() const { return _recRhohvSdev; }
  const double* getRec() const { return _rec; }
  const double* getRecFlag() const { return _recFlag; }
  
protected:
  
private:

  static const double _missingDbl = -9999.0;

  string _progName;
  const Params &_params;
  int _maxTrips;
  
  int _nSamples;
  int _halfNSamples;
  
  vector<Pulse *> _pulses;
  vector<Pulse *> _prevPulses;
  Complex_t *_delta12;
  Complex_t **_iq;
  Complex_t **_iqh;
  Complex_t **_iqv;
  
  double _el;
  double _az;

  double _prf;
  double _prt;
  
  time_t _time;

  int _nGatesPulse;
  int _nGatesOut;

  const MomentsMgr *_momentsMgr;
  
  int *_flags;
  int *_tripFlag;
  int *_vcensor;
  int *_censor;
  int *_zinfill;

  double *_powerDbm;
  double *_snr;

  double *_noiseDbm;
  double *_snrn;
  double *_dbzn;

  double *_dbzt;
  double *_dbz;
  double *_vel;
  double *_width;

  double *_clut;
  double *_dbzf;
  double *_velf;
  double *_widthf;

  double *_dbzi;
  double *_veli;
  double *_widthi;

  double *_leakage;
  double *_vtexture;
  double *_vspin;
  double *_fcensor;
  double *_ztexture;

  double *_phidp;
  double *_kdp;
  double *_zdr;
  double *_rhohv;

  bool _recVarsReady;

  double *_recDbzDiffSq;
  double *_recDbzSpinChange;

  double *_recDbzTexture;
  double *_recSqrtTexture;
  double *_recDbzSpin;
  double *_recWtSpin;
  double *_recVel;
  double *_recVelSdev;
  double *_recWidth;

  double *_recClutDbzNarrow;
  double *_recClutRatioNarrow;
  double *_recClutRatioWide;
  double *_recClutWxPeakRatio;
  double *_recClutWxPeakSep;
  double *_recClutWxPeakSepSdev;

  double *_recZdrSdev;
  double *_recRhohvSdev;
  double *_rec;
  double *_recFlag;
  
  // details of spectra at each gate, saved for later use

  vector<GateSpectra *> _gateSpectra;

  // zdr flags are static on the class, since we need continuity beteen beams

  static bool _zvFlagReady;
  static int _zvFlag;
  static int _nZdr;
  static double _sumZdr;

  // interest maps are static on the class, since they should
  // only be computed once

  static InterestMap *_dbzTextureInterestMap;
  static InterestMap *_sqrtTextureInterestMap;
  static InterestMap *_dbzSpinInterestMap;
  static InterestMap *_wtSpinInterestMap;
  static InterestMap *_velInterestMap;
  static InterestMap *_widthInterestMap;
  static InterestMap *_velSdevInterestMap;
  static InterestMap *_zdrSdevInterestMap;
  static InterestMap *_rhohvInterestMap;
  static InterestMap *_rhohvSdevInterestMap;
  static InterestMap *_clutRatioNarrowInterestMap;
  static InterestMap *_clutRatioWideInterestMap;
  static InterestMap *_clutWxPeakRatioInterestMap;
  static InterestMap *_clutWxPeakSepInterestMap;
  static InterestMap *_clutWxPeakSepSdevInterestMap;

  // private functions
  
  void _allocArrays();

  void _freeArrays();
  void _freeArray(int* &array);
  void _freeArray(double* &array);

  void _computeRecBeamLimits(const deque<Beam *> beams,
			     int midBeamIndex,
			     int &minIndex,
			     int &maxIndex) const;
  
  void _computeRecVars();

  double _computeAzDiff(double az1, double az2) const;

  double _computeMeanVelocity(double vel1, double vel2, double nyquist);
  
  double _velDiff2Angle(double vel1, double vel2, double nyquist);

  Complex_t _complexProduct(Complex_t c1, Complex_t c2);
  
  Complex_t _meanConjugateProduct(const Complex_t *c1,
				  const Complex_t *c2,
				  int len);

  double _velFromComplexAng(Complex_t cVal, double nyquist);
  
#ifdef JUNK

  void _filterSpikes(double *dbzf,
		     double *velf,
		     double *widthf);
  
  void _filterDregs(double nyquist,
		    double *dbzf,
		    double *velf,
		    double *widthf);
  
  void _computeVelCensoring(const double *vel,
			    double nyquist,
			    double *vtexture,
			    double *vspin,
			    int *vcensor);
  
#endif

  void _computeZTexture(const double *dbz,
			double *ztexture);

#ifdef JUNK

  void _performInfilling(const double *dbz,
			 const double *vel,
			 const double *width,
			 const double *ztexture,
			 int *zinfill,
			 double *dbzi,
			 double *veli,
			 double *widthi);
  
#endif

  double _computeInterest(double xx, double x0, double x1);

#ifdef JUNK

  void _printPhaseDrift(const Pulse *pulse);

#endif
  void _addSpectrumToFile(FILE *specFile, int count, time_t beamTime,
			  double el, double az, int gateNum,
			  double snr, double vel, double width,
			  int nSamples, const Complex_t *iq);
#ifdef JUNK

#endif

  void _computeIsWx(Complex_t **IQ, bool *isWx);

  static int _convertInterestMapToVector(const string &label,
					 const Params::interest_map_point_t *map,
					 int nPoints,
					 vector<InterestMap::ImPoint> &pts);
};

#endif

