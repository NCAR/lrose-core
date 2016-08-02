/////////////////////////////////////////////////////////////
// ClutFilter.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2004
//
///////////////////////////////////////////////////////////////

#ifndef ClutFilter_HH
#define ClutFilter_HH

#include <string>
using namespace std;

////////////////////////
// This class

class ClutFilter {
  
public:

  // constructor

  ClutFilter();

  // destructor
  
  ~ClutFilter();

  // perform filtering
  
  static void run(const double *rawMag, 
		  int nSamples,
		  double max_clutter_vel,
		  double init_notch_width,
		  double nyquist,
		  double *filteredMag,
		  bool &clutterFound,
		  int &notchStart,
		  int &notchEnd,
		  double &powerRemoved,
		  double &vel,
		  double &width);
  
  // apply an aggressive notch
  
  static void notch(const double *rawMag, 
		    int nSamples,
		    double max_clutter_vel,
		    double nyquist,
		    double *filteredMag,
		    bool &clutterFound,
		    int &notchStart,
		    int &notchEnd,
		    double &powerRemoved);

  // locate wx and clutter peaks
  
  static void ClutFilter::locateWxAndClutter(const double *power,
                                             int nSamples,
                                             double max_clutter_vel,
                                             double init_notch_width,
                                             double nyquist,
                                             int &notchWidth,
                                             bool &clutterFound,
                                             int &clutterPos,
                                             double &clutterPeak,
                                             int &weatherPos,
                                             double &weatherPeak);

  // fit gaussian to spectrum

  static void fitGaussian(const double *magnitude,
                          int nSamples, 
                          int weatherPos,
                          double minMagnitude,
                          double nyquist,
                          double &vel,
                          double &width,
                          double *gaussian);
  
protected:
private:
  
};

#endif

