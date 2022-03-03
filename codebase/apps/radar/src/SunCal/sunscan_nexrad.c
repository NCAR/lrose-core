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
#include <sys/stat.h>

/* macros */

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define MAX_PATH_LEN 1024

/************************************************
 * structs
 */

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
} novas_site_into;

/* grid params */

#define gridNAz 121
#define gridNEl 81
static double gridStartAz = -3.0;
static double gridStartEl = -2.0;
static double gridDeltaAz = 0.05;
static double gridDeltaEl = 0.05;

/* processing parameters */

static double maxValidDrxPowerDbm = -60.0;
static double validEdgeBelowPeakDb = 8.0;
static double minAngleOffsetForNoisePower = 2.0;
static double maxSolidAngleForMeanCorr = 2.0;
static double solidAngleForSS = 1.0;

/* debugging */

int _debug = 0;

/* missing value */

static double _missing = -9999.0;

/* noise in each channel */

static double _noiseDbmH = -115.0;
static double _noiseDbmV = -115.0;

/* location and theoretical sun angle */

static double _latitude = 0.0;
static double _longitude = 0.0;
static double _altitudeM = 0.0;
static double _meanSunTime;
static double _meanSunEl, _meanSunAz;

/* input pulse queue
 * this is an array of pointers that are
 * reordered as new pulses are added
 * pulses are added at the back of the queue
 * and removed from the front
 */

static solar_pulse_t *_pulseQueue = NULL;
static int _nSamples = 0;
static int _nPulsesInQueue = 0;

/* raw beam array, before interpolation */
/* this is computed from the incoming pulses */

static int *_elevAllocRaw = NULL; /* allocation for beams in el */
static int *_elevCountRaw = NULL; /* active count of beams in el */
static solar_beam_t **_rawBeamArray = NULL;  /* [gridNAz][variable] */
static double _prevAzOffset = -9999;

/* properties of current beam */

double _beamTime;
double _beamAz, _offsetAz;
double _beamEl, _offsetEl;

/* beams interpolated onto a regular grid */

static solar_beam_t **_interpBeamArray = NULL;  /* [gridNAz][gridNel] */

/* interpolated power arrays on regular grid */

static double **_interpDbm = NULL;   /* [gridNAz][gridNel] */
static double **_interpDbmH = NULL;  /* [gridNAz][gridNel] */
static double **_interpDbmV = NULL;  /* [gridNAz][gridNel] */

/* solar results - power is mean of H and V channels */

static double _maxPowerDbm;
static double _quadPowerDbm;
static double _pwrWtCentroidAzError;
static double _pwrWtCentroidElError;
static double _quadFitCentroidAzError;
static double _quadFitCentroidElError;
static double _elAzWidthRatio;

/* solar results - power is for H channel only */

static double _maxPowerDbmH;
static double _quadPowerDbmH;
static double _pwrWtCentroidAzErrorH;
static double _pwrWtCentroidElErrorH;
static double _quadFitCentroidAzErrorH;
static double _quadFitCentroidElErrorH;
static double _elAzWidthRatioH;

/* solar results - power is for V channel only */

static double _maxPowerDbmV;
static double _quadPowerDbmV;
static double _pwrWtCentroidAzErrorV;
static double _pwrWtCentroidElErrorV;
static double _quadFitCentroidAzErrorV;
static double _quadFitCentroidElErrorV;
static double _elAzWidthRatioV;
static double _elAzwidthRatioDiffHV;

/* mean stats on the solar disk */

static double _meanSS;
static double _meanZdr;
static double _meanCorr00;

/* receiver gain */

static double _rxGainHdB;
static double _rxGainVdB;

/*****************************************************
 * access to results
 *****************************************************/

void nexradSolarSetDebug(int level) { _debug = level; }

int nexradSolarGetGridNAz() { return gridNAz; }
int nexradSolarGetGridNEl() { return gridNEl; }
double nexradSolarGetGridStartAz() { return gridStartAz; }
double nexradSolarGetGridStartEl() { return gridStartEl; }
double nexradSolarGetGridDeltaAz() { return gridDeltaAz; }
double nexradSolarGetGridDeltaEl() { return gridDeltaEl; }

double nexradSolarGetMissingVal() { return _missing; }
double nexradSolarGetNoiseDbmH() { return _noiseDbmH; }
double nexradSolarGetNoiseDbmV() { return _noiseDbmV; }

double nexradSolarGetLatitude() { return _latitude; }
double nexradSolarGetLongitude() { return _longitude; }
double nexradSolarGetAltitudeM() { return _altitudeM; }

double nexradSolarGetMeanSunTime() { return _meanSunTime; }
double nexradSolarGetMeanSunEl() { return _meanSunEl; }
double nexradSolarGetMeanSunAz() { return _meanSunAz; }

solar_beam_t **nexradSolarGetInterpBeamArray() { return _interpBeamArray; }
double **nexradSolarGetInterpDbm() { return _interpDbm; }
double **nexradSolarGetInterpDbmH() { return _interpDbmH; }
double **nexradSolarGetInterpDbmV() { return _interpDbmV; }

double nexradSolarGetMaxPowerDbm() { return _maxPowerDbm; }
double nexradSolarGetQuadPowerDbm() { return _quadPowerDbm; }

double nexradSolarGetQuadFitCentroidAzError() { return _quadFitCentroidAzError; }
double nexradSolarGetQuadFitCentroidElError() { return _quadFitCentroidElError; }
double nexradSolarGetElAzWidthRatio() { return _elAzWidthRatio; }

double nexradSolarGetMeanSS() { return _meanSS; }
double nexradSolarGetMeanZdr() { return _meanZdr; }
double nexradSolarGetMeanCorr00() { return _meanCorr00; }

double nexradSolarGetRxGainHdB() { return _rxGainHdB; }
double nexradSolarGetRxGainVdB() { return _rxGainVdB; }

/*****************************************************
 * static file-scope functions
 *****************************************************/

/*****************************************************
 * compute sun position using NOVA routines 
 */

extern void rsts_SunNovasComputePosAtTime
  (novas_site_into here, double deltat,
   double *SunAz, double *SunEl, time_t timeIn);

static void _computePosnNova(double stime, double *el, double *az)
{
  /* set up site info */
  double tempC = 20;
  double pressureMb = 1013;

  novas_site_into site = { _latitude, _longitude, _altitudeM, tempC, pressureMb };

  /* set time */
  time_t ttime = (time_t) stime;
  double deltat = -0.45;

  /* compute sun posn */
  rsts_SunNovasComputePosAtTime(site, deltat, az, el, ttime);

}

/*****************************************************
 * allocate arrays
 */

static void _allocArrays()

{

  int iaz;

  if (_elevAllocRaw == NULL) {
    _elevAllocRaw = malloc(gridNAz * sizeof(int));
  }
  if (_elevCountRaw == NULL) {
    _elevCountRaw = malloc(gridNAz * sizeof(int));
  }
  
  if (_rawBeamArray == NULL) {
    _rawBeamArray = malloc(gridNAz * sizeof(solar_beam_t *));
    for (iaz = 0; iaz < gridNAz; iaz++) {
      _rawBeamArray[iaz] = malloc(gridNEl * sizeof(solar_beam_t));
      _elevAllocRaw[iaz] = gridNEl;
      _elevCountRaw[iaz] = 0;
    }
  }
  
  if (_interpBeamArray == NULL) {
    _interpBeamArray = malloc(gridNAz * sizeof(solar_beam_t *));
    for (iaz = 0; iaz < gridNAz; iaz++) {
      _interpBeamArray[iaz] = malloc(gridNEl * sizeof(solar_beam_t));
    }
  }

  if (_interpDbm == NULL) {
    _interpDbm = malloc(gridNAz * sizeof(double *));
    for (iaz = 0; iaz < gridNAz; iaz++) {
      _interpDbm[iaz] = malloc(gridNEl * sizeof(double));
    }
  }

  if (_interpDbmH == NULL) {
    _interpDbmH = malloc(gridNAz * sizeof(double *));
    for (iaz = 0; iaz < gridNAz; iaz++) {
      _interpDbmH[iaz] = malloc(gridNEl * sizeof(double));
    }
  }

  if (_interpDbmV == NULL) {
    _interpDbmV = malloc(gridNAz * sizeof(double *));
    for (iaz = 0; iaz < gridNAz; iaz++) {
      _interpDbmV[iaz] = malloc(gridNEl * sizeof(double));
    }
  }

}

/*****************************************************
 * free arrays
 */

static void _freeArrays()

{

  int iaz;

  if (_elevAllocRaw != NULL) {
    free(_elevAllocRaw);
    _elevAllocRaw = NULL;
  }
  
  if (_elevCountRaw != NULL) {
    free(_elevCountRaw);
    _elevCountRaw = NULL;
  }
  
  if (_rawBeamArray != NULL) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      free(_rawBeamArray[iaz]);
    }
    free(_rawBeamArray);
    _rawBeamArray = NULL;
  }

  if (_interpBeamArray != NULL) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      free(_interpBeamArray[iaz]);
    }
    free(_interpBeamArray);
    _interpBeamArray = NULL;
  }

  if (_interpDbm != NULL) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      free(_interpDbm[iaz]);
    }
    free(_interpDbm);
    _interpDbm = NULL;
  }

  if (_interpDbmH != NULL) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      free(_interpDbmH[iaz]);
    }
    free(_interpDbmH);
    _interpDbmH = NULL;
  }

  if (_interpDbmV != NULL) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      free(_interpDbmV[iaz]);
    }
    free(_interpDbmV);
    _interpDbmV = NULL;
  }

}

/*****************************************************
 * initialize the pulse queue
 */

void _initPulseQueue(int n_samples)

{

  /* check for existence */

  if (_pulseQueue != NULL) {
    if (n_samples == _nSamples) {
      // already done
      return;
    }
  }
  
  // free up if needed

  if (_pulseQueue != NULL) {
    free(_pulseQueue);
  }

  /* create */

  _pulseQueue = malloc(n_samples * sizeof(solar_pulse_t));
  memset(_pulseQueue, 0, n_samples * sizeof(solar_pulse_t));
  
  _nSamples = n_samples;
  _nPulsesInQueue = 0;
  
}

/*****************************************************
 * delete the pulse queue
 */

void _freePulseQueue()

{

  // free up if needed

  if (_pulseQueue != NULL) {
    free(_pulseQueue);
    _pulseQueue = NULL;
  }
  _nPulsesInQueue = 0;

}

/*****************************************************/
/* check for north crossing */
/* and adjust accordingly */

static void _adjustForNorthCrossing(double *az0, double *az1)
{
  if (*az0 - *az1 > 180) {
    *az0 -= 360.0;
  } else if (*az0 - *az1 < -180) {
    *az1 -= 360.0;
  }
}

/*****************************************************/
/* condition az to between 0 and 360 */

static double _conditionAz(double az)
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

static double _conditionEl(double el)
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

static double _conditionAngleDelta(double delta)
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

static double _computeAngleDiff(double ang1, double ang2)
{
  double delta = _conditionAngleDelta(ang1 - ang2);
  return delta;
}

/*****************************************************/
/* */
/* compute mean of 2 angles: ang1 + ((ang2 - ang1)/2) */

static double _computeAngleMean(double ang1, double ang2)
{
  double delta = _conditionAngleDelta(ang2 - ang1);
  double mean = ang1 + delta / 2.0;
  if (ang1 > 180 || ang2 > 180) {
    mean = _conditionAz(mean);
  } else {
    mean = _conditionEl(mean);
  }
  return mean;
}

/*****************************************************
 * initialize a beam
 * is computed lat/lon in degrees, alt_m in meters
 */

static void _initBeam(solar_beam_t *beam)

{
  beam->time = 0;
  beam->el = 0;
  beam->az = 0;
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
 * check if current beam is indexed to the grid
 * returns 0 on success, -1 on failure
 *
 * We keep track of the previous valid indexed azimuth found
 */

static int _readyForBeam()

{

  /* ensure the queue is full */

  if (_nPulsesInQueue != _nSamples) {
    return -1;
  }

  /* find pulses either side of mid point of queue */

  int midIndex0 = _nSamples / 2;
  int midIndex1 = midIndex0 + 1;
  solar_pulse_t *pulse0 = &_pulseQueue[midIndex0];
  solar_pulse_t *pulse1 = &_pulseQueue[midIndex1];

  /* compute angles at mid queue ? i.e. in center of beam */

  double az0 = pulse0->az;
  double az1 = pulse1->az;

  /* adjust az angles if they cross north */

  _adjustForNorthCrossing(&az0, &az1);

  /* order the azimuths */

  if (az0 > az1) {
    double tmp = az0;
    az0 = az1; 
    az1 = tmp;
  }

  /* compute mean azimuth and elevation */
  
  _beamAz = _computeAngleMean(az0, az1);
  if (_beamAz < 0) {
    _beamAz += 360.0;
  }
  _beamEl = _computeAngleMean(pulse0->el, pulse1->el);
  
  /* compute cosine of elevation for correcting azimuth relative to sun */

  double cosel = cos(_beamEl * DEG_TO_RAD);

  /* compute angles relative to sun position */

  _beamTime = (pulse0->time + pulse1->time) / 2.0;
  double sunEl = 0.0, sunAz = 0.0;
  _computePosnNova(_beamTime, &sunEl, &sunAz);

  /* compute az offsets for 2 center pulses */

  double offsetAz0 = _computeAngleDiff(az0, sunAz) * cosel;
  double offsetAz1 = _computeAngleDiff(az1, sunAz) * cosel;

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
  if (azIndex < 0 || azIndex > gridNAz - 1) {
    /* outside grid - failure */
    return -1;
  }

  /* save az offset   */

  _prevAzOffset = roundedOffsetAz;

  /* success */

  _offsetAz = _computeAngleMean(offsetAz0, offsetAz1);
  _offsetEl = _computeAngleDiff(_beamEl, sunEl);
  
  return 0;

}

/*****************************************************/
/* compute sum */

static solar_complex_t _complexSum(const solar_complex_t *c1,
                                   const solar_complex_t *c2)
{
  solar_complex_t sum;
  sum.re = c1->re + c2->re;
  sum.im = c1->im + c2->im;
  return sum;
}

/* mean of complex sum */

static solar_complex_t _complexMean(solar_complex_t *sum, double nn)
{
  solar_complex_t mean;
  mean.re = sum->re / nn;
  mean.im = sum->im / nn;
  return mean;
}

/*****************************************************/
/* mean of complex sum */

static double _complexMag(solar_complex_t *val)
{
  return sqrt(val->re * val->re + val->im * val->im);
}

/*****************************************************/
/* compute arg in degrees */

static double _argDeg(const solar_complex_t *cc)

{
  double arg = 0.0;
  if (cc->re != 0.0 || cc->im != 0.0) {
    arg = atan2(cc->im, cc->re);
  }
  arg *= RAD_TO_DEG;
  return arg;
}

/*****************************************************
 * Compare for sort on elevation angles
 */

static int _compareBeamEl(const void *lhs, const void *rhs)
{
  const solar_beam_t *lbeam = (solar_beam_t *) lhs;
  const solar_beam_t *rbeam = (solar_beam_t *) rhs;
  if (lbeam->elOffset > rbeam->elOffset) {
    return 1;
  } else {
    return 0;
  }
}

/*****************************************************
 * sort the raw beam data by elevation
 */
    
static void _sortRawBeamsByEl()
{
  int iaz;
  for (iaz = 0; iaz < gridNAz; iaz++) {
    solar_beam_t *beams = _rawBeamArray[iaz];
    qsort(beams, _elevCountRaw[iaz], sizeof(solar_beam_t), _compareBeamEl);
  }
}

/*****************************************************/
/* compute the maximum power */
    
static void _computeMaxPower()
{
  
  /* max power for each channel, and mean of channels */

  _maxPowerDbmH = -120.0;
  _maxPowerDbmV = -120.0;
  _maxPowerDbm = -120.0;
  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
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
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
      beam->dbBelowPeak = beam->dbm - _maxPowerDbm;
    }
  }
}

/*****************************************************/
/* correct powers by subtracting the noise */
    
static void _correctPowersForNoise()
  
{
  int iel, iaz;
  double noisePowerH = pow(10.0, _noiseDbmH / 10.0);
  double noisePowerV = pow(10.0, _noiseDbmV / 10.0);
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
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
      beam->dbm = (beam->dbmH + beam->dbmV) / 2.0;
    } /* iaz */
  } /* iel */
}

/*****************************************************
 * compute mean sun moments for a beam,
 * using moments from pulses in queue */

static int _computeMoments(solar_beam_t *beam)

{

  /* initialize summation quantities */

  int ipulse;
  int nn = 0;
  double sumPowerH = 0.0;
  double sumPowerV = 0.0;
  solar_complex_t sumRvvhh0;
  sumRvvhh0.re = 0.0;
  sumRvvhh0.im = 0.0;

  /* loop through pulses */

  for (ipulse = 0; ipulse < _nSamples; ipulse++, nn++) {

    solar_pulse_t *pulse = &_pulseQueue[ipulse];

    sumPowerH += pulse->powerH;
    sumPowerV += pulse->powerV;
    sumRvvhh0 = _complexSum(&sumRvvhh0, &pulse->rvvhh0);

  } /* ipulse */

  /* compute mean moments */

  beam->powerH = sumPowerH / nn;
  beam->powerV = sumPowerV / nn;

  beam->dbmH = 10.0 * log10(beam->powerH);
  beam->dbmV = 10.0 * log10(beam->powerV);
  beam->dbm = (beam->dbmH + beam->dbmV)/2.0;
  beam->zdr = beam->dbmH - beam->dbmV;
  beam->SS = 1.0 / (2.0 * beam->zdr);

  beam->rvvhh0 = _complexMean(&sumRvvhh0, nn);
  double corrMag = _complexMag(&sumRvvhh0) / nn;
  beam->corrHV = corrMag / sqrt(beam->powerH * beam->powerV);
  beam->phaseHV = _argDeg(&sumRvvhh0);

  beam->azOffset = _offsetAz;
  beam->elOffset = _offsetEl;

  return 0;

}

/*****************************************************/
/* Add a beam to the raw beam array */

static int _addBeamToRawArray(solar_beam_t *beam)
{

  /* compute the azimuth index */
  
  int azIndex = (int) ((beam->azOffset - gridStartAz) / gridDeltaAz + 0.5);
  if (azIndex < 0 || azIndex > gridNAz - 1) {
    /* out of bounds */
    return -1;
  }
  
  /* increment elevation count */
  
  _elevCountRaw[azIndex]++;
  
  /* alloc more space as needed for beams in elevation */
  
  if (_elevCountRaw[azIndex] > _elevAllocRaw[azIndex]) {
    int newSize = _elevAllocRaw[azIndex] + gridNEl;
    _rawBeamArray[azIndex] =
      realloc(_rawBeamArray[azIndex], newSize * sizeof(solar_beam_t));
    _elevAllocRaw[azIndex] = newSize;
  }

  /* copy beam to the array */
  
  _rawBeamArray[azIndex][_elevCountRaw[azIndex] - 1] = *beam;

  return 0;

}

/*****************************************************
 * interp ppi moments onto regular 2-D grid
 *
 * global 2D array of Beam objects to store the interpolated data:
 * Beam _interpBeamArray[gridNAz][gridNEl];
 * double _interpDbmH[gridNAz][gridNEl];
 * double _interpDbmV[gridNAz][gridNEl];
 * double _interpDbm[gridNAz][gridNEl];
 */

static void _interpMomentsToRegularGrid() 
{

  int iaz, iel, ii;

  /* initialize */

  for (iaz = 0; iaz < gridNAz; iaz++) {
    for (iel = 0; iel < gridNEl; iel++) {
      solar_beam_t *interp = &_interpBeamArray[iaz][iel];
      _initBeam(interp);
      _interpDbmH[iaz][iel] = _missing;
      _interpDbmV[iaz][iel] = _missing;
      _interpDbm[iaz][iel] = _missing;
    }
  }

  /* loop through azimuths */
  for (iaz = 0; iaz < gridNAz; iaz++) {
    double azOffset = gridStartAz + iaz * gridDeltaAz;
    
    /* find elevation straddle if available */
    
    for (iel = 0; iel < gridNEl; iel++) {
      double elOffset = gridStartEl + iel * gridDeltaEl;

      /* find the raw moments which straddle this elevation */

      solar_beam_t *raw = _rawBeamArray[iaz];
      
      for (ii = 0; ii < _elevCountRaw[iaz] - 1; ii++) {
        
        solar_beam_t raw0 = raw[ii];
        solar_beam_t raw1 = raw[ii+1];
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
        
        solar_beam_t *interp = &_interpBeamArray[iaz][iel];

        interp->time = (wt0 * raw0.time) + (wt1 * raw1.time);

        interp->az = (wt0 * raw0.az) + (wt1 * raw1.az);
        interp->el = (wt0 * raw0.el) + (wt1 * raw1.el);
        interp->elOffset = elOffset;
        interp->azOffset = azOffset;
        interp->powerH = (wt0 * raw0.powerH) + (wt1 * raw1.powerH);
        interp->powerV = (wt0 * raw0.powerV) + (wt1 * raw1.powerV);
        interp->dbmH = (wt0 * raw0.dbmH) + (wt1 * raw1.dbmH);
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

static int _quadFit(int n,
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
    
static void _computeMeanSunLocation()
  
{
  double sumTime = 0.0;
  double nn = 0.0;
  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
      if (beam->dbBelowPeak <= validEdgeBelowPeakDb) {
        sumTime += beam->time;
        nn++;
      }
    } /* iaz */
  } /* iel */

  /* compute mean time */
  _meanSunTime = sumTime / nn;
  
  /* compute mean sun location */
  
  _computePosnNova(_meanSunTime, &_meanSunEl, &_meanSunAz);
  
}

/*****************************************************/
/* Compute sun centroid for given power array */

static int _computeSunCentroid(double **interpDbm,
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

  if (_debug > 0) {
    fprintf(stderr, "===>> computeSunCentroid()\n");
    fprintf(stderr, "  maxPowerDbm: %g\n", maxPowerDbm);
    fprintf(stderr, "  validEdgeBelowPeakDb: %g\n", validEdgeBelowPeakDb);
    fprintf(stderr, "  edgePowerThreshold: %g\n", edgePowerThreshold);
  }

  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
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
    fprintf(stderr, "WARNING - cannot estimate solar centroid:\n");
    fprintf(stderr, "  no measured power\n");
    return -1;
  }

  *pwrWtCentroidAzError = sumWtAz / sumPower;
  *pwrWtCentroidElError = sumWtEl / sumPower;

  if (_debug > 0) {
    fprintf(stderr, "  pwrWtCentroidAzError: %lg\n", *pwrWtCentroidAzError);
    fprintf(stderr, "  pwrWtCentroidElError: %lg\n", *pwrWtCentroidElError);
  }
    
  double gridMaxAz = gridStartAz + gridNAz * gridDeltaAz;
  double gridMaxEl = gridStartEl + gridNEl * gridDeltaEl;
  
  if (*pwrWtCentroidAzError < gridStartAz ||
      *pwrWtCentroidAzError > gridMaxAz ||
      *pwrWtCentroidElError < gridStartEl ||
      *pwrWtCentroidElError > gridMaxEl) {
    fprintf(stderr, "WARNING- estimated centroid outside grid:\n");
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
  double azArray[gridNAz];
  double azDbm[gridNAz];
  double widthAz3Db = _missing;
  int nvalid = 0;

  for (iaz = 0; iaz < gridNAz; iaz++) {
    if (interpDbm[iaz][elCentroidIndex] == _missing) {
      continue;
    }
    double dbm = interpDbm[iaz][elCentroidIndex]; /* row for el centroid */
    if (dbm >= edgePowerThreshold) {
      double az = gridStartAz + iaz * gridDeltaAz;
      azArray[nvalid] = az;
      /* add 200 to dbm to ensure real roots */
      azDbm[nvalid] = dbm + 200;
      nvalid++;
    }
  }
  
  double ccAz, bbAz, aaAz, errEstAz, rSqAz;
  if (_quadFit(nvalid,
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

  double elArray[gridNEl];
  double elDbm[gridNEl];
  double widthEl3Db = _missing;
  nvalid = 0;
  
  for (iel = 0; iel < gridNEl; iel++) {
    if (interpDbm[azCentroidIndex][iel] == _missing) {
      continue;
    }
    double dbm = interpDbm[azCentroidIndex][iel]; /* column for az centroid */
    if (dbm >= edgePowerThreshold) {
      double el = gridStartEl + iel * gridDeltaEl;
      elArray[nvalid] = el;
      /* add 200 to dbm to ensure real roots */
      elDbm[nvalid] = dbm + 200;
      nvalid++;
    }
  }
  
  double ccEl, bbEl, aaEl, errEstEl, rSqEl;
  if (_quadFit(nvalid,
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

  if (_debug > 0) {
    fprintf(stderr, "  quadFitCentroidAzError: %g\n", *quadFitCentroidAzError);
    fprintf(stderr, "  quadFitCentroidElError: %g\n", *quadFitCentroidElError);
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

  if (_debug > 0) {
    fprintf(stderr, "  elAzWidthRatio: %g\n", *elAzWidthRatio);
    fprintf(stderr, "  quadPowerDbm: %g\n", *quadPowerDbm);
  }

  return 0;

}

/*****************************************************/
/* */
/* Compute sun centroid for mean, H and V channels */

static int _computeSunCentroidAllChannels()

{
  /* compute centroid for mean dbm (mean of H and V) */

  if (_debug > 0) {
    fprintf(stderr, "============== quadratic mean fit ============\n");
  }

  if (_computeSunCentroid(_interpDbm,
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
  
  if (_debug > 0) {
    fprintf(stderr, "============== quadratic H fit ============\n");
  }
  
  if (_computeSunCentroid(_interpDbmH,
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
  
  if (_debug > 0) {
    fprintf(stderr, "============== quadratic V fit ============\n");
  }
  
  if (_computeSunCentroid(_interpDbmV,
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
    
static int _computeMeanZdrAndSS()
  
{

  double sumZdr = 0.0;
  double sumSS = 0.0;
  double nn = 0.0;

  double searchRadius = solidAngleForSS / 2.0;
  
  /* for points within the required solid angle, */
  /* sum up stats */

  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {
    double el = gridStartEl + iel * gridDeltaEl;
    double elOffset = el - _quadFitCentroidElError;
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
      double az = gridStartAz + iaz * gridDeltaAz;
      double azOffset = az - _quadFitCentroidAzError;
      double offset = sqrt(elOffset * elOffset + azOffset * azOffset);
      if (offset <= searchRadius) {
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

/////////////////////////////////////////////////
// compute the mean noise power for each channel
    
static void _computeMeanNoise()
  
{

  double sumNoiseDbmH = 0.0;
  double sumNoiseDbmV = 0.0;
  double nBeamsNoise = 0.0;

  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {

    double elOffset = gridStartEl + iel * gridDeltaEl;
    
    for (iaz = 0; iaz < gridNAz; iaz++) {

      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
      
      double offsetAz = gridStartAz + iaz * gridDeltaAz;
      double offset = sqrt(elOffset * elOffset + offsetAz * offsetAz);

      if (offset < minAngleOffsetForNoisePower) {
        continue;
      }
      
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

static void _computePowerRatios()

{

  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
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
    
static void _computeSunCorr()
  
{

  double sumPowerH = 0.0;
  double sumPowerV = 0.0;
  solar_complex_t sumRvvhh0;
  sumRvvhh0.re = 0.0;
  sumRvvhh0.im = 0.0;
  double nn = 0.0;
  
  int iel, iaz;
  for (iel = 0; iel < gridNEl; iel++) {
    double dEl = gridStartEl + iel * gridDeltaEl;
    for (iaz = 0; iaz < gridNAz; iaz++) {
      solar_beam_t *beam = &_interpBeamArray[iaz][iel];
      if (beam->time == 0.0) {
        continue;
      }
      double dAz = gridStartAz + iaz * gridDeltaAz;
      double angDist = sqrt(dEl * dEl + dAz * dAz);
      if (angDist > maxSolidAngleForMeanCorr) {
        continue;
      }
      sumPowerH += beam->powerH;
      sumPowerV += beam->powerV;
      sumRvvhh0 = _complexSum(&sumRvvhh0, &beam->rvvhh0);
      nn++;
    }
  }
  
  if (nn < 2) {
    return;
  }
  
  double meanPowerH = sumPowerH / nn;
  double meanPowerV = sumPowerV / nn;
  solar_complex_t meanRvvhh0;
  meanRvvhh0 = _complexMean(&sumRvvhh0, nn);
  
  if (meanPowerH > 0 && meanPowerV > 0) {
    _meanCorr00 = _complexMag(&meanRvvhh0) / sqrt(meanPowerH * meanPowerV);
  }
  
}

/**********************************************************/
/**********************************************************/
/*************** Public API code below ********************/
/**********************************************************/
/**********************************************************/

/*****************************************************
 * initialize the module for processing
 */

void nexradSolarInit(int n_samples)

{
  
  _initPulseQueue(n_samples);
  _allocArrays();

}

/*****************************************************
 * free up the memory used in the module
 */

void nexradSolarFree()

{

  _freePulseQueue();
  _freeArrays();

}

/*****************************************************
 * initialize the lat/lon/alt for which sun position
 * is computed lat/lon in degrees, alt_m in meters
 */

void nexradSolarSetLocation(double lat, double lon, double alt_m)

{
  _latitude = lat;
  _longitude = lon;
  _altitudeM = alt_m;
}

/*****************************************************
 * clear the pulse queue
 */

void nexradSolarClearPulseQueue()

{
  _nPulsesInQueue = 0;
}

/*****************************************************
 * add a pulse to the queue
 */

void nexradSolarAddPulseToQueue(solar_pulse_t *pulse)

{

  /* move pulses by 1 towards front of queue */

  memmove(_pulseQueue, _pulseQueue + 1,
          (_nSamples - 1) * sizeof(solar_pulse_t));
  
  /* copy pulse to slot at back of queue */
  
  memcpy(_pulseQueue + (_nSamples - 1), pulse, sizeof(solar_pulse_t));

  /* update number of samples */

  if (_nPulsesInQueue < _nSamples) {
    _nPulsesInQueue++;
  }

  // process beam if ready

  if (_readyForBeam() == 0) {

    /* create beam */
    solar_beam_t *beam = malloc(sizeof(solar_beam_t));
    _initBeam(beam);
    beam->time = _beamTime;
    beam->el = _beamEl;
    beam->az = _beamAz;

    /* compute moments on the beam */

    _computeMoments(beam);

    /* add the beam to the raw array */

    if (_addBeamToRawArray(beam)) {
      /* failed, free it up */
      free(beam);
    }

  }

}

/**************************************************
 * perform analysis
 *
 * Returns 0 on success, -1 on failure
 */

void nexradSolarPerformAnalysis()
{

  /* sort the raw moments and interp onto a regular grid */
  
  _sortRawBeamsByEl();
  _interpMomentsToRegularGrid();
    
  /* compute the min power for each channel */
  
  _computeMeanNoise();

  /* adjust powers for noise */
  
  _correctPowersForNoise();

  /* compute power ratios */
  
  _computePowerRatios();

  /* compute the max power */
  
  _computeMaxPower();

  /* compute mean sun location */
  
  _computeMeanSunLocation();
  
  /* if (_params.debug >= Params::DEBUG_VERBOSE) { */
  /*   cerr << "========== mean sun location ==========" << endl; */
  /*   cerr << "  calTime: " << DateTime::strm((time_t) _calTime) << endl; */
  /*   cerr << "  _meanSunEl: " << _meanSunEl << endl; */
  /*   cerr << "  _meanSunAz: " << _meanSunAz << endl; */
  /* } */

  /* compute sun centroid */
  
  _computeSunCentroidAllChannels();

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
  
  _computeSunCorr();
  
  /* compute results from moments */
  
  _computeMeanZdrAndSS();

  /* write out results */

  /* if (_writeGriddedTextFiles()) { */
  /*   return -1; */
  /* } */
  /* if (_writeSummaryText()) { */
  /*   return -1; */
  /* } */
    
}

/*****************************************************/
/* compute receiver gain */
/* based on solar flux from Penticton */
/* */
/* Reference: On Measuring WSR-88D Antenna Gain Using Solar Flux. */
/*            Dale Sirmans, Bill Urell, ROC Engineering Branch */
/*            2001/01/03. */
    
void nexradSolarComputeReceiverGain()
  
{

  /* beam width correction for solar obs - Penticton */

  double solarRadioWidth = 0.57;
  double radarBeamWidth = 0.92; /* example */
  double kk = pow((1.0 + 0.18 * pow((solarRadioWidth / radarBeamWidth), 2.0)), 2.0);

  /* frequency of radar and solar observatory */

  double radarFreqMhz = 2809.0; /* example */
  double solarFreqMhz = 2800.0;

  /* wavelength */

  double radarWavelengthM = (2.99735e8 / (radarFreqMhz * 1.0e6));

  /* estimated received power given solar flux */
  
  /* double beamWidthRad = radarBeamWidth * DEG_TO_RAD; */

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

/********************************************************
 * _makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing.
 *
 * Returns -1 on error, 0 otherwise.
 */

static int _makedir(const char *path)
{
  struct stat stat_buf;
  /*
   * Status the directory to see if it already exists.
   */
  if (stat(path, &stat_buf) == 0) {
    return 0;
  }
  /*
   * Directory doesn't exist, create it.
   */
  if (mkdir(path,
	    S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
    /*
     * failed
     * check if dir has been made bu some other process
     * in the mean time, in which case return success
     */
    if (stat(path, &stat_buf) == 0) {
      return 0;
    }
    return -1;
  }
  return 0;
}


/********************************************************
 * _makedir_recurse()
 *
 * Utility routine to create a directory recursively.
 * If the directory already exists, does nothing.
 * Otherwise it recurses through the path, making all
 * needed directories.
 *
 * Returns -1 on error, 0 otherwise.
 */

static int _makedir_recurse(const char *path)
{

  char up_dir[MAX_PATH_LEN];
  char *last_delim;
  struct stat dir_stat;
  int delim = '/';
  
  /*
   * Status the directory to see if it already exists.
   * '/' dir will always exist, so this stops the recursion
   * automatically.
   */
  
  if (stat(path, &dir_stat) == 0) {
    return 0;
  }
  
  /*
   * create up dir - one up the directory tree -
   * by searching for the previous delim and removing it
   * from the string.
   * If no delim, try to make the directory non-recursively.
   */
  
  strncpy(up_dir, path, MAX_PATH_LEN);
  last_delim = strrchr(up_dir, delim);
  if (last_delim == NULL) {
    return (_makedir(up_dir));
  }
  *last_delim = '\0';
  
  /*
   * make the up dir
   */
  
  if (_makedir_recurse(up_dir)) {
    return -1;
  }

  /*
   * make this dir
   */

  if (_makedir(path)) {
    return -1;
  } else {
    return 0;
  }

}

/************************************************************
 * write out debug file for data at given offset from 'time'
 */

static int _writeGriddedFieldDebug(const char *dirPath,
                                   const char *fieldName,
                                   int offset)
  
{

  int iaz, iel;

  /* compute file path */

  char path[MAX_PATH_LEN];
  sprintf(path, "%s/%s.debug.txt", dirPath, fieldName);
  if (_debug) {
    fprintf(stderr, "writing debug nexrad moments file: %s\n", path);
  }

  // open file
  
  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    fprintf(stderr, "ERROR - nexradSolarWriteGriddedTextFiles\n");
    fprintf(stderr, "  Cannot open debug file for writing: %s\n", path);
    perror("  ");
    return -1;
  }

  // compute the min value, set missing to this

  double minVal = 1.0e99;
  
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      const solar_beam_t *moments = &_interpBeamArray[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val != _missing) {
        if (val < minVal) {
          minVal = val;
        }
      }
    }
  }

  // write out grid details
  
  fprintf(out, "# minEl, deltaEl, nEl: %g %g %d\n",
          gridStartEl, gridDeltaEl, gridNEl);
  fprintf(out, "# minAz, deltaAz, nAz: %g %g %d\n",
          gridStartAz, gridDeltaAz, gridNAz);
  fprintf(out, "# sun offset az, el (deg): %g %g\n",
          _quadFitCentroidAzError, _quadFitCentroidElError);
  
  // write out grid data
  
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      double el = gridStartEl + iel * gridDeltaEl;
      double az = gridStartAz + iaz * gridDeltaAz;
      const solar_beam_t *moments = &_interpBeamArray[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val == _missing) {
        fprintf(out, " el, az, val: %10.5f %10.5f %10.5f\n", el, az, minVal);
      } else {
        fprintf(out, " el, az, val: %10.5f %10.5f %10.5f\n", el, az, val);
      }
    } // iaz
  } // iel

  fclose(out);
  return 0;

}

/////////////////////////////////////////////////////////////
// write out file for data at given offset from 'time' member

static int _writeGriddedField(const char *dirPath,
                              const char *fieldName,
                              int offset)
  
{

  int iaz, iel;

  // compute file path

  char path[MAX_PATH_LEN];
  sprintf(path, "%s/%s.txt", dirPath, fieldName);
  if (_debug) {
    fprintf(stderr, "writing nexrad moments file: %s\n", path);
  }

  // open file
  
  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    fprintf(stderr, "ERROR - nexradSolarWriteGriddedTextFiles\n");
    fprintf(stderr, "  Cannot open file for writing: %s\n", path);
    perror("  ");
    return -1;
  }

  // compute the min value, set missing to this

  double minVal = 1.0e99;
  
  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      const solar_beam_t *moments = &_interpBeamArray[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val != _missing) {
        if (val < minVal) {
          minVal = val;
        }
      }
    }
  }

  // write out grid details

  fprintf(out, "%g %g %d\n", gridStartAz, gridDeltaAz, gridNAz);
  fprintf(out, "%g %g %d\n", gridStartEl, gridDeltaEl, gridNEl);

  // write out grid data

  for (iel = 0; iel < gridNEl; iel++) {
    for (iaz = 0; iaz < gridNAz; iaz++) {
      const solar_beam_t *moments = &_interpBeamArray[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val == _missing) {
        fprintf(out, " %10.3f", minVal);
      } else {
        fprintf(out, " %10.3f", val);
      }
    } // iaz
    fprintf(out, "\n");
  } // iel

  // write out sun centroid
  
  fprintf(out, "%g %g\n", _quadFitCentroidAzError, _quadFitCentroidElError);
  
  fclose(out);

  if (_debug >= 2) {
    _writeGriddedFieldDebug(dirPath, fieldName, offset);
  }

  return 0;

}

/////////////////////////////////////////
// write gridded results to text files

int nexradSolarWriteGriddedTextFiles(const char *output_dir)

{

  // create the directory for the output files
  
  struct tm *mtime;
  time_t meanTime = (time_t) _meanSunTime;
  mtime = gmtime(&meanTime);
  char dirPath[MAX_PATH_LEN];
  sprintf(dirPath, "%s/%.4d%.2d%.2d_%.2d%.2d%.2d",
          output_dir,
          mtime->tm_year + 1900,
          mtime->tm_mon + 1,
          mtime->tm_mday,
          mtime->tm_hour,
          mtime->tm_min,
          mtime->tm_sec);

  if (_makedir_recurse(dirPath)) {
    fprintf(stderr, "ERROR - nexradSolarWriteGriddedTextFiles\n");
    fprintf(stderr, "  Cannot create output dir: %s\n", dirPath);
    perror("  ");
    return -1;
  }

  // write out various data sets

  solar_beam_t moments;
  int offset = 0;

  offset = (char *) &moments.dbm - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbm", offset);
  
  offset = (char *) &moments.dbBelowPeak - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbBelowPeak", offset);
  
  offset = (char *) &moments.dbmH - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbmH", offset);
  
  offset = (char *) &moments.dbm - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbm", offset);
  
  offset = (char *) &moments.dbmV - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbmV", offset);
  
  offset = (char *) &moments.corrHV - (char *) &moments.time;
  _writeGriddedField(dirPath, "corrHV", offset);
  
  offset = (char *) &moments.phaseHV - (char *) &moments.time;
  _writeGriddedField(dirPath, "phaseHV", offset);
    
  offset = (char *) &moments.zdr - (char *) &moments.time;
  _writeGriddedField(dirPath, "zdr", offset);
  
  offset = (char *) &moments.SS - (char *) &moments.time;
  _writeGriddedField(dirPath, "SS", offset);
    
  return 0;

}

