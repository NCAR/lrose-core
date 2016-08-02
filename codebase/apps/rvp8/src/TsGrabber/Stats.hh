// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Stats.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Data accumulation and stats at a gate or over multiple gates
//
////////////////////////////////////////////////////////////////

#ifndef Stats_HH
#define Stats_HH

#include <rvp8_rap/RapComplex.hh>

using namespace std;

////////////////////////
// This class

class Stats {
  
public:

  // constructor

  Stats();

  // destructor
  
  ~Stats();

  // initialize - set memvbers to 0

  void init();

  // sum up alternating information
  
  void addToAlternating(double ii0, double qq0,
			bool haveChan1,
			double ii1, double qq1,
			bool isHoriz);

  // compute alternating stats
  // Assumes data has been added
  
  void computeAlternating();

  // data

  double nn0, nn1;
  double nnH, nnV;
  
  double sumPower0, sumPower1;
  double meanDbm0, meanDbm1;
  
  double sumPowerHc, sumPowerHx;
  double sumPowerVc, sumPowerVx;

  double meanDbmHc, meanDbmHx;
  double meanDbmVc, meanDbmVx;

  double noiseDbmHc, noiseDbmHx;
  double noiseDbmVc, noiseDbmVx;
  
  RapComplex sumConjProd01H;
  RapComplex sumConjProd01V;
  
  double corr01H;
  double corr01V;
  
  double arg01H;
  double arg01V;

protected:
  
private:

};

#endif
