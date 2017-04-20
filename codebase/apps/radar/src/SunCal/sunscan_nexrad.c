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

/* includes */

#include "sunscan_nexrad.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* parameters */
  
static double gridDeltaAz = 0.2;
static double gridDeltaEl = 0.2;
static double gridStartAz = -3.0;
static double gridStartEl = -2.0;
/* static int startGate = 400; */
/* static int endGate = 800; */
static double maxValidDrxPowerDbm = -60.0;
static double validEdgeBelowPeakDb = 8.0;
static double minAngleOffsetForNoisePower = 2.0;
static double maxSolidAngleForMeanCorr = 2.0;
static double solidAngleForSS = 1.0;

/* missing value */

static double _missing = -9999.0;

/* noise in each channel */

static double _noiseDbmH = -115.0;
static double _noiseDbmV = -115.0;

/* location and theoretical sun angle */

static double _latitude = 0.0;
static double _longitude = 0.0;
static double _altitudeM = 0.0;
static double _prevSunTime = 0.0;
static double _meanSunTime;
static double _meanSunEl, _meanSunAz;

/* input pulse queue
 * this is an array of pointers that are
 * reordered as new pulses are added
 * pulses are added at the back of the queue
 * and removed from the front
 */

static NexradPulse_t **_pulseQueue = NULL;
static int *_nPulses = 0;

/* raw beam array, before interpolation */
/* this is computed from the incoming pulses */

/* static Beam _rawBeamArray[RAW_MAX_NAZ][RAW_MAX_NEL]; */
static NexradBeam_t **_rawBeamArray;
static int _rawMaxAz;
static int _rawMaxEl[NEXRAD_RAW_MAX_NAZ];
static double _prevAzOffset = -9999;

/* interpolated beams */
/* on regular grid */

static NexradBeam_t _interpBeamArray[nexradGridNAz][nexradGridNEl];

/* interpolated power arrays */
/* on regular grid */

static double **_interpDbm;
static double **_interpDbmH;
static double **_interpDbmV;

/* solar results - power is mean of H and V channels */

static double _maxPowerDbm;
static double _quadPowerDbm;
static double _pwrWtCentroidAzError;
static double _pwrWtCentroidElError;
static double _quadFitCentroidAzError;
static double _quadFitCentroidElError;
static double _elAzWidthRatio;

/* solar results - power is for H channel */

static double _maxPowerDbmH;
static double _quadPowerDbmH;
static double _pwrWtCentroidAzErrorH;
static double _pwrWtCentroidElErrorH;
static double _quadFitCentroidAzErrorH;
static double _quadFitCentroidElErrorH;
static double _elAzWidthRatioH;

/* solar results - power is for V channel */

static double _maxPowerDbmV;
static double _quadPowerDbmV;
static double _pwrWtCentroidAzErrorV;
static double _pwrWtCentroidElErrorV;
static double _quadFitCentroidAzErrorV;
static double _quadFitCentroidElErrorV;
static double _elAzWidthRatioV;
static double _elAzwidthRatioDiffHV;

static double _meanSS;
static double _meanZdr;
static double _meanCorr00;

/* receiver gain */
static double _rxGainHdB;
static double _rxGainVdB;

/*****************************************************
 * static file-scope functions
 *****************************************************/

/*****************************************************
 * compute sun position using NOVA routines 
 */

static void computePosnNova(double stime, double *el, double *az);

/*****************************************************
 * check for north crossing
 * and adjust accordingly */

static void adjustForNorthCrossing(double *az0, double *az1);

/*****************************************************
 * condition az to between 0 and 360 
 */

static double conditionAz(double az);

/*****************************************************
 * condition el to between -180 and 180
 */

static double conditionEl(double el);

/*****************************************************
 * condition angle delta to between -180 and 180
 */

static double conditionAngleDelta(double delta);

/*****************************************************
 * compute diff between 2 angles: (ang1 - ang2) 
 */

static double computeAngleDiff(double ang1, double ang2);

/*****************************************************
 * compute mean of 2 angles: ang1 + ((ang2 - ang1)/2) 
 */

static double computeAngleMean(double ang1, double ang2);

/*****************************************************
 * compute mean power of time series 
 */

static double meanPower(const nexrad_complex_t *c1, int len);

/*****************************************************
 * compute mean conjugate product of series 
 */

static nexrad_complex_t meanConjugateProduct(const nexrad_complex_t *c1,
                                              const nexrad_complex_t *c2,
                                              int len);

/*****************************************************
 * compute sum
 */

static nexrad_complex_t complexSum(const nexrad_complex_t *c1,
                                  const nexrad_complex_t *c2);

/*****************************************************
 * mean of complex sum 
 */

static nexrad_complex_t complexMean(nexrad_complex_t *sum, double nn);

/*****************************************************
 * magnitude of complex val
 */

static double complexMag(nexrad_complex_t *val);

/*****************************************************
 * compute arg in degrees 
 */

/* static double complexArgDeg(const nexrad_complex_t *cc); */
  
/*****************************************************
 * get gate IQ in H channel
 */

static nexrad_complex_t *getGateIqH(int igate);

/*****************************************************
 * get gate IQ in V channel
 */

static nexrad_complex_t *getGateIqV(int igate);

/*****************************************************
 * Compare for sort on elevation angles 
 */

static int compareBeamEl(const void *lhs, const void *rhs);

/*****************************************************
 * sort the raw beam data by elevation 
 */
    
static void sortRawBeamsByEl();

/*****************************************************
 * correct powers by subtracting the noise
 */
    
static void correctPowersForNoise();

/*****************************************************
 * compute the maximum power 
 */
    
static void computeMaxPower();

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

static int quadFit(int n,
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
    
static void computeMeanSunLocation();

/*****************************************************
 * Compute sun centroid for given power array 
 */

static int computeSunCentroid(double **interpDbm,
                              double maxPowerDbm,
                              double *quadPowerDbm,
                              double *pwrWtCentroidAzError,
                              double *pwrWtCentroidElError,
                              double *quadFitCentroidAzError,
                              double *quadFitCentroidElError,
                              double *elAzWidthRatio);

/*****************************************************
 *
 * Compute sun centroid for mean, H and V channels 
 */

static int computeSunCentroidAllChannels();

/*****************************************************
 * compute mean ZDR and SS ratio
 */
    
static int computeMeanZdrAndSS();

/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/

/************* implementation code below ******************/

/*****************************************************
 * compute sun position using NOVA routines
 */

void rsts_SunNovasComputePosAtTime
  (nexrad_site_info here, double deltat,
   double *SunAz, double *SunEl, double distanceAU)
{
  /* do what is needed here */
}

/*****************************************************
 * initialize the lat/lon/alt for which sun position
 * is computed lat/lon in degrees, alt_m in meters
 */

void nexradSetLocation(double lat, double lon, double alt_m)

{
  _prevSunTime = 0.0;
  _latitude = lat;
  _longitude = lon;
  _altitudeM = alt_m;
}

/*****************************************************
 * initialize the pulse queue
 */

void nexradInitPulseQueue()
  
{
  if (_pulseQueue != NULL) {
    // already done, only initialize once
    return;
  }
  _pulseQueue =
    (NexradPulse_t **) malloc(nexradNSamples * sizeof(NexradPulse_t *));
  memset((void *) _pulseQueue, 0, nexradNSamples * sizeof(NexradPulse_t *));
  _nPulses = 0;
}
  
/*****************************************************
 * create a pulse
 */

NexradPulse_t *nexradCreatePulse(int n_gates)
  
{
  NexradPulse_t *pulse = (NexradPulse_t *) malloc(sizeof(NexradPulse_t));
  pulse->nGates = n_gates;
  pulse->time = _missing;
  pulse->prt = _missing;
  pulse->el = _missing;
  pulse->az = _missing;
  pulse->iq = (float *) malloc(n_gates * 2 * sizeof(float));
  memset(pulse->iq, 0, n_gates * 2 * sizeof(float));
  return pulse;
}
  
/*****************************************************
 * resize a pulse
 */

void nexradResizePulse(NexradPulse_t *pulse, int n_gates)
  
{
  if (pulse->iq) {
    free(pulse->iq);
  }
  pulse->nGates = n_gates;
  pulse->iq = (float *) malloc(n_gates * 2 * sizeof(float));
  memset(pulse->iq, 0, n_gates * 2 * sizeof(float));
}
  
/*****************************************************
 * initialize a beam
 * is computed lat/lon in degrees, alt_m in meters
 */

void initBeam(NexradBeam_t *beam)

{
  beam->nGates = 0;
  beam->time = _missing;
  beam->prt = _missing;
  beam->el = _missing;
  beam->az = _missing;
  beam->elOffset = _missing;
  beam->azOffset = _missing;
  beam->powerH = _missing;
  beam->powerV = _missing;
  beam->dbmH = _missing;
  beam->dbmV = _missing;
  beam->dbm = _missing;
  beam->corrHV = _missing;
  beam->phaseHV = _missing;
  beam->dbBelowPeak = _missing;
  beam->zdr = _missing;
  beam->ratioDbmVH = _missing;
  beam->SS = _missing;
  beam->rvvhh0.re = 0.0;
  beam->rvvhh0.im = 0.0;
}

/*****************************************************
 * compute sun position using NOVA routines 
 */

void computePosnNova(double stime, double *el, double *az)
{
  /* check if time has changed */
  /* if not do not recalculate */

  double now = time(NULL);
  if (fabs(now - _prevSunTime) < 1) {
    /* time has not changed more than 1 sec */
    return;
  }
  _prevSunTime = now;

  /* set up site info */
  double tempC = 20;
  double pressureMb = 1013;

  nexrad_site_info site = { _latitude, _longitude, _altitudeM, tempC, pressureMb };
  
  /* set time */
  time_t ttime = (time_t) stime;
  double deltat = -0.45;
  
  /* compute sun posn */
  rsts_SunNovasComputePosAtTime(site, deltat, az, el, ttime);

}

/*****************************************************/
/* check for north crossing */
/* and adjust accordingly */

void adjustForNorthCrossing(double *az0, double *az1)
{
  if (*az0 - *az1 > 180) {
    *az0 -= 360.0;
  } else if (*az0 - *az1 < -180) {
    *az1 -= 360.0;
  }
}

/*****************************************************/
/* condition az to between 0 and 360 */

double conditionAz(double az)
{
  while (az < 0.0) {
    az += 360.0;
  }
  while (az > 360.0) {
    az -= 360.0;
  }
  return az;
}

/*****************************************************/
/* condition el to between -180 and 180 */

double conditionEl(double el)
{
  while (el < -180.0) {
    el += 360.0;
  }
  while (el > 180.0) {
    el -= 360.0;
  }
  return el;
}

/*****************************************************/
/* condition angle delta to between -180 and 180 */

double conditionAngleDelta(double delta)
{
  if (delta < -180.0) {
    delta += 360.0;
  } else if (delta > 180.0) {
    delta -= 360.0;
  }
  return delta;
}

/*****************************************************/
/* compute diff between 2 angles: (ang1 - ang2) */

double computeAngleDiff(double ang1, double ang2)
{
  double delta = conditionAngleDelta(ang1 - ang2);
  return delta;
}

/*****************************************************/
/* */
/* compute mean of 2 angles: ang1 + ((ang2 - ang1)/2) */

double computeAngleMean(double ang1, double ang2)
{
  double delta = conditionAngleDelta(ang2 - ang1);
  double mean = ang1 + delta / 2.0;
  if (ang1 > 180 || ang2 > 180) {
    mean = conditionAz(mean);
  } else {
    mean = conditionEl(mean);
  }
  return mean;
}

/*****************************************************
 * check if current beam is indexed to the grid
 * returns 0 on success, -1 on failure
 *
 * We keep track of the previous valid indexed azimuth found
 */

int isBeamIndexedToGrid()

{

  /* find pulses either side of mid point of queue */

  int midIndex0 = nexradNSamples / 2;
  int midIndex1 = midIndex0 + 1;
  NexradPulse_t *pulse0 = _pulseQueue[midIndex0];
  NexradPulse_t *pulse1 = _pulseQueue[midIndex1];
  
  /* compute angles at mid queue ? i.e. in center of beam */
  
  double az0 = pulse0->az;
  double az1 = pulse1->az;

  /* adjust az angles if they cross north */

  adjustForNorthCrossing(&az0, &az1);
  
  /* order the azimuths */

  if (az0 > az1) {
    double tmp = az0;
    az0 = az1; 
    az1 = tmp;
  }

  /* compute mean azimuth and elevation */
  
  double az = computeAngleMean(az0, az1);
  if (az < 0) {
    az += 360.0;
  }
  double el = computeAngleMean(pulse0->el, pulse1->el);

  /* compute cosine of elevation for correcting azimuth relative to sun */

  double cosel = cos(el * DEG_TO_RAD);

  /* compute angles relative to sun position */
  
  double midTime = (pulse0->time + pulse1->time) / 2.0;
  double sunEl, sunAz;
  computePosnNova(midTime, &sunEl, &sunAz);
  
  /* compute az offsets for 2 center pulses */
    
  double offsetAz0 = computeAngleDiff(az0, sunAz) * cosel;
  double offsetAz1 = computeAngleDiff(az1, sunAz) * cosel;
    
  /* compute grid az closest to the offset az */
    
  double roundedOffsetAz =
    (floor (offsetAz0 / gridDeltaAz + 0.5)) * gridDeltaAz;
    
  /* have we moved at least half grid point since last beam? */
    
  if (fabs(offsetAz0 - _prevAzOffset) < gridDeltaAz / 2) {
    return -1;
  }
    
  /* is the azimuth correct? */
    
  if (offsetAz0 > roundedOffsetAz || offsetAz1 < roundedOffsetAz) {
    return -1;
  }
    
  /* is this azimuth contained in the grid? */
    
  int azIndex = -1;
  azIndex = (int) ((roundedOffsetAz - gridStartAz) / gridDeltaAz + 0.5);
  if (azIndex < 0 || azIndex > nexradGridNAz - 1) {
    /* outside grid - failure */
    return -1;
  }

  /* save az offset   */
  _prevAzOffset = roundedOffsetAz;

  /* success */

  return 0;

}

/*****************************************************/
/* compute mean power of time series */

double meanPower(const nexrad_complex_t *c1, int len)
{
  double sum = 0.0;
  int ipos;
  if (len < 1) {
    return 0.0;
  }
  for (ipos = 0; ipos < len; ipos++, c1++) {
    sum += ((c1->re * c1->re) + (c1->im * c1->im));
  }
  return sum / len;
}

/*****************************************************/
/* compute mean conjugate product of series */

nexrad_complex_t meanConjugateProduct(const nexrad_complex_t *c1,
                                       const nexrad_complex_t *c2,
                                       int len)
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;
  int ipos;

  for (ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) + (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) - (c1->re * c2->im));
  }

  nexrad_complex_t meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

/*****************************************************/
/* compute sum */

nexrad_complex_t complexSum(const nexrad_complex_t *c1,
                           const nexrad_complex_t *c2)
{
  nexrad_complex_t sum;
  sum.re = c1->re + c2->re;
  sum.im = c1->im + c2->im;
  return sum;
}

/* mean of complex sum */

nexrad_complex_t complexMean(nexrad_complex_t *sum, double nn)
{
  nexrad_complex_t mean;
  mean.re = sum->re / nn;
  mean.im = sum->im / nn;
  return mean;
}

/*****************************************************/
/* mean of complex sum */

double complexMag(nexrad_complex_t *val)
{
  return sqrt(val->re * val->re + val->im * val->im);
}

/*****************************************************/
/* compute arg in degrees */

double argDeg(const nexrad_complex_t *cc)
  
{
  double arg = 0.0;
  if (cc->re != 0.0 || cc->im != 0.0) {
    arg = atan2(cc->im, cc->re);
  }
  arg *= RAD_TO_DEG;
  return arg;
}

nexrad_complex_t *getGateIqH(int igate)
{
  nexrad_complex_t *val = (nexrad_complex_t *) malloc(sizeof(nexrad_complex_t));
  /* read from data array ... */
  return val;
}

nexrad_complex_t *getGateIqV(int igate)
{
  nexrad_complex_t *val = (nexrad_complex_t *) malloc(sizeof(nexrad_complex_t));
  /* read from data array ... */
  return val;
}

/*****************************************************/
/* compute sun moments in dual-pol simultaneous mode */
/* load up Beam with moments */

int computeMoments(int startGate,
                   int endGate,
                   NexradBeam_t *beam)

{
  
  /* initialize summation quantities */
  
  int igate;
  double sumPowerH = 0.0;
  double sumPowerV = 0.0;
  double nn = 0.0;
  nexrad_complex_t sumRvvhh0;
  sumRvvhh0.re = 0.0;
  sumRvvhh0.im = 0.0;

  /* loop through gates to be used for sun computations */
  
  for (igate = startGate; igate <= endGate; igate++, nn++) {
    
    /* get the I/Q data time series for the gate */
    /* the getGateIq() functions must be provided externally. */
    
    const nexrad_complex_t *iqh = getGateIqH(igate);
    const nexrad_complex_t *iqv = getGateIqV(igate);

    /* compute lag 0 covariance = power */
    
    double lag0_h = meanPower(iqh, nexradNSamples - 1);
    double lag0_v = meanPower(iqv, nexradNSamples - 1);
    
    /* check power for interference */

    double dbmH = 10.0 * log10(lag0_h);
    double dbmV = 10.0 * log10(lag0_v);
    double meanDbm = (dbmH + dbmV) / 2.0;

    if (meanDbm > maxValidDrxPowerDbm) {
      /* don't use this gate - probably interference */
      continue;
    }
    
    /* compute lag0 conjugate product, for correlation */

    nexrad_complex_t lag0_hv =
      meanConjugateProduct(iqh, iqv, nexradNSamples - 1);
    
    /* sum up */

    sumPowerH += lag0_h;
    sumPowerV += lag0_v;
    sumRvvhh0 = complexSum(&sumRvvhh0, &lag0_hv);
    
  } /* igate */

  /* sanity check */

  if (nn < 3) {
    fprintf(stderr, "Warning - computeMoments\n");
    fprintf(stderr, "  Insufficient good data found\n");
    fprintf(stderr, "  az, el: %lg, %lg\n", beam->az, beam->el);
    fprintf(stderr, "  nn: %g\n", nn);
    return -1;
  }
  
  /* compute mean moments */

  beam->powerH = sumPowerH / nn;
  beam->powerV = sumPowerV / nn;
   
  beam->dbmH = 10.0 * log10(beam->powerH);
  beam->dbmV = 10.0 * log10(beam->powerV);
  beam->dbm = (beam->dbmH + beam->dbmV)/2.0;
  beam->zdr = beam->dbmH - beam->dbmV;
  beam->SS = 1.0 / (2.0 * beam->zdr);
 
  beam->rvvhh0 = complexMean(&sumRvvhh0, nn);
  double corrMag = complexMag(&sumRvvhh0) / nn;
  beam->corrHV = corrMag / sqrt(beam->powerH * beam->powerV);
  beam->phaseHV = argDeg(&sumRvvhh0);

  /* compute sun angle offset */
  
  double sunEl, sunAz;
  computePosnNova(beam->time, &sunEl, &sunAz);
  double cosel = cos(beam->el * DEG_TO_RAD);
  beam->azOffset = computeAngleDiff(beam->az, sunAz) * cosel;
  beam->elOffset = computeAngleDiff(beam->el, sunEl);
    
  return 0;

}

/*****************************************************/
/* Add a beam to the raw beam array */

int addBeam(NexradBeam_t *beam)
{

  /* compute the azimuth index */

  int azIndex = (int) ((beam->azOffset - gridStartAz) / gridDeltaAz + 0.5);

  /* check for array space */

  if (azIndex > NEXRAD_RAW_MAX_NAZ - 1) {
    fprintf(stderr, "ERROR - azIndex too great: %d\n", azIndex);
  }

  /* keep track of array sizes */

  if (azIndex > _rawMaxAz) {
    _rawMaxAz = azIndex;
    _rawMaxEl[azIndex] = 0;
  }

  if (azIndex >= 0 || azIndex < nexradGridNAz) {

    /* check for array space */
    
    if (_rawMaxEl[azIndex] > NEXRAD_RAW_MAX_NEL - 2) {
      fprintf(stderr, "ERROR - elIndex too great: %d\n", _rawMaxEl[azIndex]);
      return -1;
    }

    /* increment counter for array use */

    _rawMaxEl[azIndex]++;

    /* copy beam to the array */

    _rawBeamArray[azIndex][_rawMaxEl[azIndex]] = *beam;

  }

  return 0;

}

/*****************************************************
 * Compare for sort on elevation angles
 */

int compareBeamEl(const void *lhs, const void *rhs)
{
  const NexradBeam_t *lbeam = (NexradBeam_t *) lhs;
  const NexradBeam_t *rbeam = (NexradBeam_t *) rhs;
  if (lbeam->elOffset < rbeam->elOffset) {
    return 1;
  } else {
    return 0;
  }
}

/*****************************************************
 * sort the raw beam data by elevation
 */
    
void sortRawBeamsByEl()
{
  int iaz;
  for (iaz = 0; iaz < _rawMaxAz; iaz++) {
    NexradBeam_t *beams = _rawBeamArray[iaz];
    qsort(beams, _rawMaxEl[iaz], sizeof(NexradBeam_t), compareBeamEl);
  }
}

/*****************************************************
 * interp ppi moments onto regular 2-D grid
 *
 * global 2D array of Beam objects to store the interpolated data:
 * Beam _interpBeamArray[nexradGridNAz][nexradGridNEl];
 * double _interpDbmH[nexradGridNAz][nexradGridNEl];
 * double _interpDbmV[nexradGridNAz][nexradGridNEl];
 * double _interpDbm[nexradGridNAz][nexradGridNEl];
 */

void interpMomentsToRegularGrid() 
{

  /* loop through azimuths */
  int iaz, iel, ii;
  for (iaz = 0; iaz < nexradGridNAz; iaz++) {
    double azOffset = gridStartAz + iaz * gridDeltaAz;
    
    /* find elevation straddle if available */
    
    for (iel = 0; iel < nexradGridNEl; iel++) {
      double elOffset = gridStartEl + iel * gridDeltaEl;

      /* find the raw moments which straddle this elevation */

      NexradBeam_t *raw = _rawBeamArray[iaz];
      
      for (ii = 0; ii < _rawMaxEl[iaz] - 1; ii++) {
        
        NexradBeam_t raw0 = raw[ii];
        NexradBeam_t raw1 = raw[ii+1];
        double elOffset0 = raw0.elOffset;
        double elOffset1 = raw1.elOffset;

        /* is the elevation between these two values */
        if (elOffset0 > elOffset || elOffset1 < elOffset) {
          continue;
        }
        
        /* compute interpolation weights */
        double wt0 = 1.0;
        double wt1 = 0.0;
        if (elOffset0 != elOffset1) {
          wt1 = (elOffset - elOffset0) / (elOffset1 - elOffset0);
          wt0 = 1.0 - wt1;
        }

        /* compute interpolated values */
        
        NexradBeam_t *interp = &_interpBeamArray[iaz][iel];

        interp->time = (wt0 * raw0.time) + (wt1 * raw1.time);
        interp->az = (wt0 * raw0.az) + (wt1 * raw1.az);
        interp->el = (wt0 * raw0.el) + (wt1 * raw1.el);
        interp->elOffset = elOffset;
        interp->azOffset = azOffset;
        interp->powerH = (wt0 * raw0.powerH) + (wt1 * raw1.powerH);
        interp->powerV = (wt0 * raw0.powerV) + (wt1 * raw1.powerV);
        interp->dbmH = (wt0 * raw0.dbmH) + (wt1 * raw1.dbmV);
        interp->dbmV = (wt0 * raw0.dbmV) + (wt1 * raw1.dbmV);
        interp->dbm = (wt0 * raw0.dbm) + (wt1 * raw1.dbm);
        interp->zdr = (wt0 * raw0.zdr) + (wt1 * raw1.zdr);
        interp->SS = (wt0 * raw0.SS) + (wt1 * raw1.SS);
        interp->corrHV = (wt0 * raw0.corrHV) + (wt1 * raw1.corrHV);
        interp->phaseHV = (wt0 * raw0.phaseHV) + (wt1 * raw1.phaseHV);

        _interpDbmH[iaz][iel] = interp->dbmH; 
        _interpDbmV[iaz][iel] = interp->dbmV; 
        _interpDbm[iaz][iel] = interp->dbm; 

        break;

      } /* ii */
    } /* iel */
  } /* iaz */
} 

/*****************************************************/
/* correct powers by subtracting the noise */
    
void correctPowersForNoise()
  
{
  int iel, iaz;
  double noisePowerH = pow(10.0, _noiseDbmH / 10.0);
  double noisePowerV = pow(10.0, _noiseDbmV / 10.0);
  for (iel = 0; iel < nexradGridNEl; iel++) {
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      beam->powerH -= noisePowerH;
      beam->powerV -= noisePowerV;
      if (beam->powerH <= 0) {
        beam->powerH = 1.0e-12;
      }
      if (beam->powerV <= 0) {
        beam->powerV = 1.0e-12;
      }
      beam->dbmH = 10.0 * log10(beam->powerH);
      beam->dbmV = 10.0 * log10(beam->powerV);
    } /* iaz */
  } /* iel */
}

/*****************************************************/
/* compute the maximum power */
    
void computeMaxPower()
{
  
  /* max power for each channel, and mean of channels */

  _maxPowerDbmH = -120.0;
  _maxPowerDbmV = -120.0;
  _maxPowerDbm = -120.0;
  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->dbmH <= maxValidDrxPowerDbm) {
        _maxPowerDbmH = MAX(_maxPowerDbmH, beam->dbmH);
      }
      if (beam->dbmV <= maxValidDrxPowerDbm) {
        _maxPowerDbmV = MAX(_maxPowerDbmV, beam->dbmV);
      }
      if (beam->dbm <= maxValidDrxPowerDbm) {
        _maxPowerDbm = MAX(_maxPowerDbm, beam->dbm);
      }
    }
  }
   
  /* compute dbm below peak */
  for (iel = 0; iel < nexradGridNEl; iel++) {
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      beam->dbBelowPeak = beam->dbm - _maxPowerDbm;
    }
  }
}

/*****************************************************/
/* quadFit : fit a quadratic to a data series */
/* */
/*  n: number of points in (x, y) data set */
/*  x: array of x data */
/*  y: array of y data */
/*  a? - quadratic coefficients (cc - bias, bb - linear, aa - squared) */
/*  std_error - standard error of estimate */
/*  r_squared - correlation coefficient squared */
/* */
/* Returns 0 on success, -1 on error. */
/* */
/*****************************************************/

int quadFit(int n,
            const double *x,
            const double *y,
            double *cc,
            double *bb,
            double *aa,
            double *std_error_est,
            double *r_squared)
  
{
  
  long i;

  double sumx = 0.0, sumx2 = 0.0, sumx3 = 0.0, sumx4 = 0.0;
  double sumy = 0.0, sumxy = 0.0, sumx2y = 0.0;
  double dn;
  double term1, term2, term3, term4, term5;
  double diff;
  double ymean, sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  double xval, yval;

  if (n < 4)
    return (-1);

  dn = (double) n;
  
  /* sum the various terms */

  for (i = 0; i < n; i++) {

    xval = x[i];
    yval = y[i];

    sumx = sumx + xval;
    sumx2 += xval * xval;
    sumx3 += xval * xval * xval;
    sumx4 += xval * xval * xval * xval;
    sumy += yval;
    sumxy += xval  * yval;
    sumx2y += xval * xval * yval;

  }

  ymean = sumy / dn;

  /* compute the coefficients */

  term1 = sumx2 * sumy / dn - sumx2y;
  term2 = sumx * sumx / dn - sumx2;
  term3 = sumx2 * sumx / dn - sumx3;
  term4 = sumx * sumy / dn - sumxy;
  term5 = sumx2 * sumx2 / dn - sumx4;

  *aa = (term1 * term2 / term3 - term4) / (term5 * term2 / term3 - term3);
  *bb = (term4 - term3 * *aa) / term2;
  *cc = (sumy - sumx * *bb  - sumx2 * *aa) / dn;

  /* compute the sum of the residuals */

  for (i = 0; i < n; i++) {
    xval = x[i];
    yval = y[i];
    diff = (yval - *cc - *bb * xval - *aa * xval * xval);
    sum_of_residuals += diff * diff;
    sum_dy_squared += (yval - ymean) * (yval - ymean);
  }

  /* compute standard error of estimate and r-squared */
  
  *std_error_est = sqrt(sum_of_residuals / (dn - 3.0));
  *r_squared = ((sum_dy_squared - sum_of_residuals) /
                sum_dy_squared);
  
  return 0;

}

/*****************************************************/
/* compute the sun location for the mean time of the scan */
    
void computeMeanSunLocation()
  
{
  double sumTime = 0.0;
  double nn = 0.0;
  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->dbBelowPeak > validEdgeBelowPeakDb) {
        sumTime += beam->time;
        nn++;
      }
    } /* iaz */
  } /* iel */

  /* compute mean time */
  _meanSunTime = sumTime / nn;
  
  /* compute mean sun location */
  
  computePosnNova(_meanSunTime, &_meanSunEl, &_meanSunAz);
  
}

/*****************************************************/
/* Compute sun centroid for given power array */

int computeSunCentroid(double **interpDbm,
                       double maxPowerDbm,
                       double *quadPowerDbm,
                       double *pwrWtCentroidAzError,
                       double *pwrWtCentroidElError,
                       double *quadFitCentroidAzError,
                       double *quadFitCentroidElError,
                       double *elAzWidthRatio)

{

  /* initialize */

  *quadPowerDbm = -120.0;
  *pwrWtCentroidAzError = 0.0;
  *pwrWtCentroidElError = 0.0;
  *quadFitCentroidAzError = 0.0;
  *quadFitCentroidElError = 0.0;

  /* first estimate the 2-D power-weighted centroid */

  double sumWtAz = 0.0;
  double sumWtEl = 0.0;
  double sumPower = 0.0;
  double count = 0.0;

  double edgePowerThreshold = maxPowerDbm - validEdgeBelowPeakDb;

  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      double dbm = interpDbm[iaz][iel];
      double power = pow(10.0, dbm / 10.0);
      if (dbm >= edgePowerThreshold && dbm <= maxValidDrxPowerDbm) {
        double az = beam->azOffset;
        double el = beam->elOffset;
        sumPower += power;
        sumWtAz += az * power;
        sumWtEl += el * power;
        count++;
      } /* if (dbm >= edgePowerThreshold ? */
    } /* iaz */
  } /* iel */

  if (count == 0) {
    /* no valid data */
    fprintf(stderr, "Cannot estimate solar centroid:\n");
    fprintf(stderr, "  no measured power\n");
    return -1;
  }

  *pwrWtCentroidAzError = sumWtAz / sumPower;
  *pwrWtCentroidElError = sumWtEl / sumPower;

  double gridMaxAz = gridStartAz + nexradGridNAz * gridDeltaAz;
  double gridMaxEl = gridStartEl + nexradGridNEl * gridDeltaEl;
  
  if (*pwrWtCentroidAzError < gridStartAz ||
      *pwrWtCentroidAzError > gridMaxAz ||
      *pwrWtCentroidElError < gridStartEl ||
      *pwrWtCentroidElError > gridMaxEl) {
    fprintf(stderr, "Estimated centroid outside grid:\n");
    fprintf(stderr, "  pwrWtCentroidAzError: %g\n", *pwrWtCentroidAzError);
    fprintf(stderr, "  pwrWtCentroidElError: %g\n", *pwrWtCentroidElError);
    fprintf(stderr, "  Setting quad offsets to 0\n");
    return -1;
  }

  /* compute the grid index location of the centroid */
  /* in azimuth and elevation */
  
  int elCentroidIndex =
    (int) ((*pwrWtCentroidElError - gridStartEl) / gridDeltaEl);

  int azCentroidIndex =
    (int) ((*pwrWtCentroidAzError - gridStartAz) / gridDeltaAz);
  
  /* fit parabola in azimuth to refine the azimuth centroid */
  /* this is done for the grid row at the elevation centroid */

  int fitIsGood = 1;
  double azArray[nexradGridNAz];
  double azDbm[nexradGridNAz];
  double widthAz3Db = _missing;

  for (iaz = 0; iaz < nexradGridNAz; iaz++) {
    double dbm = interpDbm[iaz][elCentroidIndex]; /* row for el centroid */
    if (dbm >= edgePowerThreshold) {
      double az = gridStartAz + iaz * gridDeltaAz;
      azArray[iaz] = az;
      /* add 200 to dbm to ensure real roots */
      azDbm[iaz] = dbm + 200;
    }
  }
  
  double ccAz, bbAz, aaAz, errEstAz, rSqAz;
  if (quadFit(nexradGridNAz,
              azArray, azDbm,
              &ccAz, &bbAz, &aaAz,
              &errEstAz, &rSqAz) == 0) {
    double rootTerm = bbAz * bbAz - 4.0 * aaAz * ccAz;
    double rootTerm2 = bbAz * bbAz - 2.0 * aaAz * ccAz;
    if (rSqAz > 0.9 && rootTerm >= 0) {
      /* good fit, real roots, so override centroid */
      double root1 = (-bbAz - sqrt(rootTerm)) / (2.0 * aaAz); 
      double root2 = (-bbAz + sqrt(rootTerm)) / (2.0 * aaAz);
      *quadFitCentroidAzError = (root1 + root2) / 2.0;
      if (rootTerm2 >= 0) {
        widthAz3Db = -(sqrt(rootTerm2) / aaAz);
      }
    } else {
      fitIsGood = 0;
    }
  } else {
    fitIsGood = 0;
  }
  
  /* fit parabola in elevation to refine the elevation centroid */
  /* this is done for the grid column at the azimuth centroid */

  double elArray[nexradGridNEl];
  double elDbm[nexradGridNEl];
  double widthEl3Db = _missing;
  
  for (iel = 0; iel < nexradGridNEl; iel++) {
    double dbm = interpDbm[azCentroidIndex][iel]; /* column for az centroid */
    if (dbm >= edgePowerThreshold) {
      double el = gridStartEl + iel * gridDeltaEl;
      elArray[iel] = el;
      /* add 200 to dbm to ensure real roots */
      elDbm[iel] = dbm + 200;
    }
  }
  
  double ccEl, bbEl, aaEl, errEstEl, rSqEl;
  if (quadFit(nexradGridNEl,
              elArray, elDbm,
              &ccEl, &bbEl, &aaEl,
              &errEstEl, &rSqEl) == 0) {
    double rootTerm = bbEl * bbEl - 4.0 * aaEl * ccEl;
    double rootTerm2 = bbEl * bbEl - 2.0 * aaEl * ccEl;
    if (rSqEl > 0.9 && rootTerm >= 0) {
      /* good fit, real roots, so override centroid */
      double root1 = (-bbEl - sqrt(rootTerm)) / (2.0 * aaEl); 
      double root2 = (-bbEl + sqrt(rootTerm)) / (2.0 * aaEl);
      *quadFitCentroidElError = (root1 + root2) / 2.0;
      if (rootTerm2 >= 0) {
        widthEl3Db = -(sqrt(rootTerm2) / aaEl);
      }
    } else {
      fitIsGood = 0;
    }
  } else {
    fitIsGood = 0;
  }

  // compute the width ratio

  *elAzWidthRatio = _missing;
  if (widthEl3Db != _missing && widthAz3Db != _missing) {
    *elAzWidthRatio = widthEl3Db / widthAz3Db;
  }

  /* set power from quadratic fits */

  if (fitIsGood) {
    *quadPowerDbm = (ccAz + ccEl) / 2.0 - 200.0;
  }

  return 0;

}

/*****************************************************/
/* */
/* Compute sun centroid for mean, H and V channels */

int computeSunCentroidAllChannels()

{
  /* compute centroid for mean dbm (mean of H and V) */
  
  if (computeSunCentroid(_interpDbm,
                         _maxPowerDbm,
                         &_quadPowerDbm,
                         &_pwrWtCentroidAzError,
                         &_pwrWtCentroidElError,
                         &_quadFitCentroidAzError,
                         &_quadFitCentroidElError,
                         &_elAzWidthRatio)) {
    return -1;
  }
  
  /* compute centroid for H channel */
  
  if (computeSunCentroid(_interpDbmH,
                         _maxPowerDbmH,
                         &_quadPowerDbmH,
                         &_pwrWtCentroidAzErrorH,
                         &_pwrWtCentroidElErrorH,
                         &_quadFitCentroidAzErrorH,
                         &_quadFitCentroidElErrorH,
                         &_elAzWidthRatioH)) {
    return -1;
  }
  
  /* compute centroid for V channel */
  
  if (computeSunCentroid(_interpDbmV,
                         _maxPowerDbmV,
                         &_quadPowerDbmV,
                         &_pwrWtCentroidAzErrorV,
                         &_pwrWtCentroidElErrorV,
                         &_quadFitCentroidAzErrorV,
                         &_quadFitCentroidElErrorV,
                         &_elAzWidthRatioV)) {
    return -1;
  }

  _elAzwidthRatioDiffHV = _elAzWidthRatioH - _elAzWidthRatioV;

  return 0;
 
}

/*****************************************************/
/* compute mean ZDR and SS ratio */
    
int computeMeanZdrAndSS()
  
{

  double sumZdr = 0.0;
  double sumSS = 0.0;
  double nn = 0.0;

  double searchRadius = solidAngleForSS / 2.0;
  
  /* for points within the required solid angle, */
  /* sum up stats */

  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {
    double el = gridStartEl + iel * gridDeltaEl;
    double elOffset = el - _quadFitCentroidElError;
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      double az = gridStartAz + iaz * gridDeltaAz;
      double azOffset = az - _quadFitCentroidAzError;
      double offset = sqrt(elOffset * elOffset + azOffset * azOffset);
      if (offset <= searchRadius) {
        NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
        sumZdr += beam->zdr;
        sumSS += beam->SS;
        nn++;
      }
    } /* iaz */
  } /* iel */
  
  /* if too few points, cannot compute mean */

  if (nn < 1) {
    _meanSS = _missing;
    _meanZdr = _missing;
    return -1;
  }

  _meanZdr = sumZdr / nn;
  _meanSS = sumSS / nn;

  return 0;

}

/*****************************************************/
/* compute receiver gain */
/* based on solar flux from Penticton */
/* */
/* Reference: On Measuring WSR-88D Antenna Gain Using Solar Flux. */
/*            Dale Sirmans, Bill Urell, ROC Engineering Branch */
/*            2001/01/03. */
    
void computeReceiverGain()
  
{

  /* beam width correction for solar obs - Penticton */

  double solarRadioWidth = 0.57;
  double radarBeamWidth = 0.92; /* example */
  double kk = pow((1.0 + 0.18 * pow((solarRadioWidth / radarBeamWidth), 2.0)), 2.0);

  /* frequency of radar and solar observatory */

  double radarFreqMhz = 2809.0; /* example */
  double solarFreqMhz = 2800.0;

  /* estimated received power given solar flux */
  
  /* double beamWidthRad = radarBeamWidth * DEG_TO_RAD; */
  double radarWavelengthM = (2.99735e8 / (radarFreqMhz * 1.0e6));

  /* antenna gains - from previous cal */

  double antennaGainHdB = 44.95; /* example */
  double antennaGainH = pow(10.0, antennaGainHdB / 10.0);
  double antennaGainVdB = 45.32; /* example */
  double antennaGainV = pow(10.0, antennaGainVdB / 10.0);

  /* waveguide gains - from previous cal */

  double waveguideGainHdB = -1.16; /* example */
  double waveguideGainH = pow(10.0, waveguideGainHdB / 10.0);
  double waveguideGainVdB = -1.44; /* example */
  double waveguideGainV = pow(10.0, waveguideGainVdB / 10.0);

  /* Observed flux */
  /* 'fluxobsflux' column from Penticton flux table   */

  double fluxSolarFreq = 135.0; /* example */
  double fluxRadarFreq = 
    (0.0002 * fluxSolarFreq - 0.01) * (radarFreqMhz - solarFreqMhz) + fluxSolarFreq;
  
  /* noise bandwidth from pulse width */

  double pulseWidthUs = 1.5; /* example */
  double noiseBandWidthHz = 1.0e6 / pulseWidthUs;

  /* gain H */

  double PrHWatts = ((antennaGainH * waveguideGainH *
                      radarWavelengthM * radarWavelengthM * fluxRadarFreq *
                      1.0e-22 * noiseBandWidthHz) /
                     (4 * M_PI * 2.0 * kk));
  double PrHdBm = 10.0 * log10(PrHWatts) + 30.0;
  _rxGainHdB = _quadPowerDbmH - PrHdBm;
  
  /* gain V */

  double PrVWatts = ((antennaGainV * waveguideGainV *
                      radarWavelengthM * radarWavelengthM * fluxRadarFreq *
                      1.0e-22 * noiseBandWidthHz) /
                     (4 * M_PI * 2.0 * kk));
  double PrVdBm = 10.0 * log10(PrVWatts) + 30.0;
  _rxGainVdB = _quadPowerDbmV - PrVdBm;
  
}

/////////////////////////////////////////////////
// compute the mean noise power for each channel
    
void computeMeanNoise()
  
{

  double sumNoiseDbmH = 0.0;
  double sumNoiseDbmV = 0.0;
  double nBeamsNoise = 0.0;

  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {

    double elOffset = gridStartEl + iel * gridDeltaEl;
    
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {

      double offsetAz = gridStartAz + iaz * gridDeltaAz;

      double offset = sqrt(elOffset * elOffset + offsetAz * offsetAz);

      if (offset < minAngleOffsetForNoisePower) {
        continue;
      }
      
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      
      if (beam->dbmH != _missing &&
          beam->dbmV != _missing &&
          beam->dbmH < maxValidDrxPowerDbm) {
        
        sumNoiseDbmH += beam->dbmH;
        sumNoiseDbmV += beam->dbmV;
        nBeamsNoise++;
      }
      
    } /* iaz */

  } /* iel */

  if (nBeamsNoise == 0) {
    /* use defaults */
    return;
  }

  _noiseDbmH = sumNoiseDbmH / nBeamsNoise;
  _noiseDbmV = sumNoiseDbmV / nBeamsNoise;

}

/**********************************************
 * compute power ratios
 */

void computePowerRatios()

{

  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->dbmV != _missing && beam->dbmH != _missing) {
        beam->ratioDbmVH = beam->dbmV - beam->dbmH;
        beam->SS = beam->ratioDbmVH * 2.0;
      }
    } /* iaz */
  } /* iel */

}

/*********************************************
 * compute mean correlation for sun disk
 */
    
void computeSunCorr()
  
{

  double sumPowerH = 0.0;
  double sumPowerV = 0.0;
  nexrad_complex_t sumRvvhh0;
  sumRvvhh0.re = 0.0;
  sumRvvhh0.im = 0.0;
  double nn = 0.0;
  
  int iel, iaz;
  for (iel = 0; iel < nexradGridNEl; iel++) {
    double dEl = gridStartEl + iel * gridDeltaEl;
    for (iaz = 0; iaz < nexradGridNAz; iaz++) {
      double dAz = gridStartAz + iaz * gridDeltaAz;
      double angDist = sqrt(dEl * dEl + dAz * dAz);
      if (angDist > maxSolidAngleForMeanCorr) {
        continue;
      }
      NexradBeam_t *beam = &_interpBeamArray[iaz][iel];
      sumPowerH += beam->powerH;
      sumPowerV += beam->powerV;
      sumRvvhh0 = complexSum(&sumRvvhh0, &beam->rvvhh0);
      nn++;
    }
  }
  
  if (nn < 2) {
    return;
  }
  
  double meanPowerH = sumPowerH / nn;
  double meanPowerV = sumPowerV / nn;
  nexrad_complex_t meanRvvhh0;
  meanRvvhh0 = complexMean(&sumRvvhh0, nn);
  
  if (meanPowerH > 0 && meanPowerV > 0) {
    _meanCorr00 = complexMag(&meanRvvhh0) / sqrt(meanPowerH * meanPowerV);
  }
  
}

/**************************************************
 * perform analysis
 *
 * Returns 0 on success, -1 on failure
 */

void performAnalysis()
{

  /* sort the raw moments and interp onto a regular grid */
  
  sortRawBeamsByEl();
  interpMomentsToRegularGrid();
    
  /* compute the min power for each channel */
  
  computeMeanNoise();

  /* adjust powers for noise */
  
  correctPowersForNoise();

  /* compute power ratios */
  
  computePowerRatios();

  /* compute the max power */
  
  computeMaxPower();

  /* compute mean sun location */
  
  computeMeanSunLocation();
  
  /* if (_params.debug >= Params::DEBUG_VERBOSE) { */
  /*   cerr << "========== mean sun location ==========" << endl; */
  /*   cerr << "  calTime: " << DateTime::strm((time_t) _calTime) << endl; */
  /*   cerr << "  _meanSunEl: " << _meanSunEl << endl; */
  /*   cerr << "  _meanSunAz: " << _meanSunAz << endl; */
  /* } */

  /* compute sun centroid */
  
  computeSunCentroidAllChannels();

  
  /* if (_params.debug) { */
  /*   cerr << "============================" << endl; */
  /*   if (_dualPol) { */
  /*     cerr << "_sunCentroidAzOffsetHc: " << _sunCentroidAzOffsetHc << endl; */
  /*     cerr << "_sunCentroidElOffsetHc: " << _sunCentroidElOffsetHc << endl; */
  /*     cerr << "_sunCentroidAzOffsetVc: " << _sunCentroidAzOffsetVc << endl; */
  /*     cerr << "_sunCentroidElOffsetVc: " << _sunCentroidElOffsetVc << endl; */
  /*     cerr << "Stats for ellipses at -3dB: " */
  /*          << "widthRatioElAzHc, widthRatioElAzVc, widthRatioElAzDiffHV: " */
  /*          << _widthRatioElAzHc << ", " */
  /*          << _widthRatioElAzVc << ", " */
  /*          << _widthRatioElAzDiffHV << endl; */
      
  /*   } */
  /*   cerr << "_sunCentroidAzOffset: " << _sunCentroidAzOffset << endl; */
  /*   cerr << "_sunCentroidElOffset: " << _sunCentroidElOffset << endl; */
  /*   cerr << "============================" << endl; */
  /* } */
  
  /* compute the mean correlation for sun disk */
  
  computeSunCorr();
  
  /* compute results from moments */
  
  computeMeanZdrAndSS();

  /* write out results */

  /* if (_writeGriddedTextFiles()) { */
  /*   return -1; */
  /* } */
  /* if (_writeSummaryText()) { */
  /*   return -1; */
  /* } */
    
}

