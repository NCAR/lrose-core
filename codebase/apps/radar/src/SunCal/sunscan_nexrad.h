/****************************************************************
 * sunscan_nexrad.h
 *
 * C implementation of sunscan code, for use by NEXRAD
 *
 * Mike Dixon, EOL, NCAR
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * April 2017
 *
*****************************************************************/

#ifndef sunscan_nexrad_h
#define sunscan_nexrad_h

/* const variables */

#define RAD_TO_DEG 57.29577951308092
#define DEG_TO_RAD 0.01745329251994372

/* macros */

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

/* raw beam array, before interpolation
 * this is computed from the incoming pulses 
 */

#define RAW_MAX_NAZ 1000
#define RAW_MAX_NEL 1000

/* number of gates and samples */

#define maxGates 2000
#define nSamples 128

/* interpolated beams on regular grid */

#define gridNAz 31
#define gridNEl 21

/************************************************
 * structs for holding pulse and beam data
 */

/******************************
 * Pulse implementation example
 */


typedef struct {
  
  /* meta data */
  
  /* int nGates; /* number of gates */
  double time; /* time in secs and fractions from 1 Jan 1970 */
  double prt; /* pulse repetition time (secs) */
  double el; /* elevation angle (deg) */
  double az; /* azimuth angle (deg) */
  
  /* IQ data */
  
  float iq[maxGates * 2];
  
} Pulse;

/*****************************
 * Beam implementation example
 *
 * Note that the meta-data  time, prt, el and az are not actually
 * used in this code, they are just included for context.
 */

typedef struct {
  
  /* meta data */
  
  /* int nSamples; /* number of pulse samples in beam */
  int nGates; /* number of gates */
  double time; /* time for the center pulse of beam */
  double prt; /* pulse repetition time (secs) */
  double el; /* elevation angle for center of beam (deg) */
  double az; /* azimuth angle for center of beam(deg) */
  double elOffset; /* elevation offset to theoretical sun center (deg) */
  double azOffset; /* azimuth offset to theoretical sun center (deg) */
  
  /* Array of Pulses */
  
  Pulse pulses[nSamples]; /* pulses for this beam */
  
  /* moments */
  
  double powerH; /* power for H channel I*I+Q*Q */
  double powerV; /* power for V channel I*I+Q*Q */
  double dbmH;   /* power for H channel in dBm */
  double dbmV;   /* power for V channel in dBm */
  double dbm;    /* mean of dbmH and dbmV */
  double corrHV;  /* correlation between H and V */
  double phaseHV; /* mean phase between H and V */
  double dbBelowPeak; /* peak sun power minus mean dbm */
  double zdr; /* dbmH minus dbmV */
  double SS; /* 1.0 / (zdr^2) */

} Beam;

/***************************************************************************
 * struct site_info: data for the observer's location.  The atmospheric 
 *                   parameters are used only by the refraction 
 *                   function called from function 'equ_to_hor'.
 *                   Additional parameters can be added to this 
 *                   structure if a more sophisticated refraction model 
 *                   is employed.
 *                    
 *  latitude         = geodetic latitude in degrees; north positive.
 *  longitude        = geodetic longitude in degrees; east positive.
 *  height           = height of the observer in meters.
 *  temperature      = temperature (degrees Celsius).
 *  pressure         = atmospheric pressure (millibars)
 */

typedef struct
{
  double latitude;
  double longitude;
  double height;
  double temperature;
  double pressure;
} site_info;

/*****************************************************
 * Complex math object 
 */

typedef struct {
  double re;
  double im;
} Complex_t;

/*****************************************************
 * compute sun position using NOVA routines
 */

extern void rsts_SunNovasComputePosAtTime
  (site_info here, double deltat,
   double *SunAz, double *SunEl, double distanceAU);

/*****************************************************
 * initialize the lat/lon/alt for which sun position
 * is computed lat/lon in degrees, alt_m in meters
 */

extern void setLocation(double lat, double lon, double alt_m);

/*****************************************************
 * compute sun position using NOVA routines 
 */

extern void computePosnNova(double stime, double *el, double *az);

/*****************************************************
 * check for north crossing
 * and adjust accordingly */

extern void adjustForNorthCrossing(double *az0, double *az1);

/*****************************************************
 * condition az to between 0 and 360 
 */

extern double conditionAz(double az);

/*****************************************************
 * condition el to between -180 and 180
 */

extern double conditionEl(double el);

/*****************************************************
 * condition angle delta to between -180 and 180
 */

extern double conditionAngleDelta(double delta);

/*****************************************************
 * compute diff between 2 angles: (ang1 - ang2) 
 */

extern double computeAngleDiff(double ang1, double ang2);

/*****************************************************
 * compute mean of 2 angles: ang1 + ((ang2 - ang1)/2) 
 */

extern double computeAngleMean(double ang1, double ang2);

/*****************************************************
 * Check if beam is indexed to grid
 * returns 0 on success, -1 on failure
 */

extern int isBeamIndexedToGrid();

/*****************************************************
 * compute mean power of time series 
 */

extern double meanPower(const Complex_t *c1, int len);

/*****************************************************
 * compute mean conjugate product of series 
 */

extern Complex_t meanConjugateProduct(const Complex_t *c1,
                                      const Complex_t *c2,
                                      int len);

/*****************************************************
 * compute sum
 */

extern Complex_t complexSum(const Complex_t *c1,
                            const Complex_t *c2);
/*****************************************************
 * mean of complex sum 
 */

extern Complex_t meanSumMean(Complex_t *sum, double nn);

/*****************************************************
 * magnitude of complex val
 */

extern double commplexMag(Complex_t *val);

/*****************************************************
 * compute arg in degrees 
 */

extern double complexArgDeg(const Complex_t *cc);
  
/*****************************************************
 * get gate IQ in H channel
 */

extern Complex_t *getGateIqH(int igate);

/*****************************************************
 * get gate IQ in V channel
 */

extern Complex_t *getGateIqV(int igate);

/*****************************************************
 * compute sun moments in dual-pol simultaneous mode
 * load up Beam with moments
 */

extern int computeMoments(int startGate,
                          int endGate,
                          Beam *beam);

/*****************************************************
 * Add a beam to the raw beam array 
 */

extern int addBeam(Beam *beam);

/*****************************************************
 * Compare for sort on elevation angles 
 */

extern int compareBeamEl(const void *lhs, const void *rhs);

/*****************************************************
 * sort the raw beam data by elevation 
 */
    
extern void sortRawBeamsByEl();

/*****************************************************
 * interp ppi moments onto regular 2-D grid
 *
 * global 2D array of Beam objects to store the interpolated data:
 * Beam _interpBeamArray[gridNAz][gridNEl];
 * double _interpDbmH[gridNAz][gridNEl];
 * double _interpDbmV[gridNAz][gridNEl];
 * double _interpDbm[gridNAz][gridNEl];
 */

extern void interpMoments();

/*****************************************************
 * correct powers by subtracting the noise
 */
    
extern void correctPowersForNoise();

/*****************************************************
 * compute the maximum power 
 */
    
extern void computeMaxPower();

/*****************************************************
 * quadFit : fit a quadratic to a data series
 *
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  a? - quadratic coefficients (cc - bias, bb - linear, aa - squared)
 *  std_error - standard error of estimate
 *  r_squared - correlation coefficient squared
 *
 * Returns 0 on success, -1 on error.
 *
 *****************************************************/

extern int quadFit(int n,
                   const double *x,
                   const double *y,
                   double *cc,
                   double *bb,
                   double *aa,
                   double *std_error_est,
                   double *r_squared);

/*****************************************************
 * compute the sun location for the mean time of the scan 
 */
    
extern double computeMeanSunLocation();

/*****************************************************
 * Compute sun centroid for given power array 
 */

extern int computeSunCentroid(double **interpDbm,
                              double maxPowerDbm,
                              double *quadPowerDbm,
                              double *pwrWtCentroidAzError,
                              double *pwrWtCentroidElError,
                              double *quadFitCentroidAzError,
                              double *quadFitCentroidElError);

/*****************************************************
 *
 * Compute sun centroid for mean, H and V channels 
 */

extern int computeSunCentroidAllChannels();

/*****************************************************
 * compute mean ZDR and SS ratio
 */
    
extern int computeMeanZdrAndSS(double solidAngle);
  
/*****************************************************
 * compute receiver gain
 * based on solar flux from Penticton
 *
 * Reference: On Measuring WSR-88D Antenna Gain Using Solar Flux.
 *            Dale Sirmans, Bill Urell, ROC Engineering Branch
 *            2001/01/03.
 */
   
extern int computeReceiverGain();
