// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:33:50 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// KdpSband.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// Kdp for SBand - based on Bringi code
//
////////////////////////////////////////////////////////////////

#ifndef KdpSband_hh
#define KdpSband_hh

#include <string>
#include <vector>
using namespace std;

////////////////////////
// This class

class KdpSband {
  
public:

  // constructor
  
  KdpSband();
  
  // destructor
  
  ~KdpSband();

  // run 
  
  int compute(int nGates,
	      const double *range,
	      const double *zh,
	      const double *zdr,
	      const double *phidp,
	      const double *rhohv,
	      const double *snr,
	      double *kdp);

  // set pulsing mode

  void setSlant45() { foldRange = 360.0; }
  void setAltHv() { foldRange = 180.0; }

protected:
  
private:

  static const int MAX_GATES = 5000;
  static const int EXTRA_GATES = 500; // for padding at each end of arrays
  static const int EXTRA_HALF = 250; // for padding at each end of arrays
  static const int FIR3ORDER = 30;
  static const int mgood = 10;
  static const int mbad = 5;
  static const int unfoldLen = 5;
  static const int maxNadp = 30;
  
  static const double fir3gain;
  static const double fir3coef[FIR3ORDER+1];

  double foldRange;

  void msr(double &ymn, double &sd, const double *y, int n);

  void lse(double &a, double &b, const double *x, const double *y, int n);

  void unfoldPhidp(int nGates, double *phidp, double *range,
		   double thrs_phi);

};

#endif

