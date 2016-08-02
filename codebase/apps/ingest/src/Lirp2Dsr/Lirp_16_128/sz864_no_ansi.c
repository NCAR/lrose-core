/***************************************************************************
 * sz864.c
 *
 * C implementation of SZ 8/64 algorithm for phase decoding.
 *
 * NCAR, Boulder, Colorado, USA
 *
 * July 2003
 *
 ****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include "sz864.h"
#include <fftw.h>

#define N_SAMPLES 64

/**************************************************************************
 * file scope variables
 */

/*
 * general
 */

static const double _censoredVal = -9999.0;
static const double _smallValue = 1.0e-9;
static const double _pi = 3.14159265358979323846;
static const int true = 1;
static const int false = 0;

/*
 * SZ coding
 */

static const int _phaseCodeN = 8;
static const int _nSamples = N_SAMPLES;
static Complex_t _modCode12[N_SAMPLES];

/*
 * parameters
 */

static double _wavelengthMeters = 0.10;
static double _noiseValueDbm = -84.0;
static double _signalToNoiseRatioThreshold = 3.0;
static double _szStrongToWeakPowerRatioThreshold = 45.0;
static double _szOutOfTripPowerRatioThreshold = 6.0;
static int _szOutOfTripPowerNReplicas = 3;

/*
 * censoring flag values
 */

static const int _censorOnTotalPower = 1;
static const int _censorOnPowerRatio = 2;
static const int _censorOnReplicas = 4;

/*
 * 3/4 notch characteristics
 */

static const int _szNotchWidth75 = 48;
static const int _szPowerWidth75 = 16;
static const double _szFracPower75 = 0.25;

/*
 * Hanning window
 */

static double _hanning[N_SAMPLES];

/*
 * deconvolution matrix
 */

static double _deconvMatrix75[N_SAMPLES * N_SAMPLES];

/*
 * fft plans
 */

static fftw_plan _fftPlanFwd;
static fftw_plan _fftPlanInv;

/**************************************************************************
 *********************** Private interface ********************************
 **************************************************************************/

/*
 * file scope function prototypes
 */

static void _initPhaseCodes();
  
static void _initDeconMatrix(int notchWidth,
			     int powerWidth,
			     double fracPower,
			     double *deconvMatrix);
  
static void _initHanning(double *window);

static void _velWidthFromTd(const Complex_t *IQ,
			    double prtSecs,
			    double *vel, double *width);
  
static void _velWidthFromFft(const double *magnitude,
			     double prtSecs,
			     double *vel, double *width);
  
static void _subModCode(const Complex_t *in, const Complex_t *code,
			Complex_t *diff);

static void _applyNotch75(int startIndex,
			  Complex_t *in, Complex_t *notched);

static void _applyNotch(int startIndex, Complex_t *in,
			int notchWidth, int powerWidth,
			double fracPower, Complex_t *notched);
  
static void _computeMag(const Complex_t *in, double *mag);

static void _normalizeMagSum(const Complex_t *in, double *normMag);

static int _checkSnThreshold(const Complex_t *IQ, double *meanPower);
     
static void _cohereTrip1_to_Trip2(const Complex_t *trip1,
				  const Complex_t *delta12,
				  Complex_t *trip2);

static void _cohereTrip2_to_Trip1(const Complex_t *trip2,
				  const Complex_t *delta12,
				  Complex_t *trip1);
  
static double _computeMeanPower(const Complex_t *IQ);

static double _computeR1(const Complex_t *IQ);

static void _applyHanningWindow(const Complex_t *in, Complex_t *out);
  
static void _computeSpectralNoise(const double *powerCentered,
				  double *noiseMean,
				  double *noiseSdev);
  
static int _computeNotchStart(int notchWidth,
			      double vel,
			      double prtSecs);
static int _hasReplicas(double *magnitude);
  
static void _invertMatrix(double *data, int nn);

static void _fftFwd(const Complex_t *in, Complex_t *out);
  
static void _fftInv(const Complex_t *in, Complex_t *out);
  
static int _compareDoubles(const void *v1, const void *v2);

/**************************************************************************
 *********************** Public functions *********************************
 **************************************************************************/

/**************************************************************************
 * Initialize module.
 *
 * Call this before any other functions.
 */

void szInit()

{

  /*
   * initialize the fft plans
   */
  
  _fftPlanFwd = fftw_create_plan(_nSamples, FFTW_FORWARD, FFTW_MEASURE);
  _fftPlanInv = fftw_create_plan(_nSamples, FFTW_BACKWARD, FFTW_MEASURE);

  /*
   * initialize the phase codes
   */

  _initPhaseCodes();

  /*
   * initialize the deconvolution matrix
   */

  _initDeconMatrix(_szNotchWidth75, _szPowerWidth75,
		   _szFracPower75, _deconvMatrix75);

  /*
   * initialize the hanning window
   */

  _initHanning(_hanning);

}

/**************************************************************************
 * Set parameters.
 *
 * The parameters are set to default values in the initialization.
 * Use these functions to change the values from the default.
 */

void szSetWavelength(double wavelength) {
  _wavelengthMeters = wavelength;
}

void szSetNoiseValueDbm(double dbm) {
  _noiseValueDbm = dbm;
}

void szSetSignalToNoiseRatioThreshold(double db) {
  _signalToNoiseRatioThreshold = db;
}

void szSetSzStrongToWeakPowerRatioThreshold(double db) {
  _szStrongToWeakPowerRatioThreshold = db;
}

void szSetSzOutOfTripPowerRatioThreshold(double db) {
  _szOutOfTripPowerRatioThreshold = db;
}

void szSetSzOutOfTripPowerNReplicas(int n) {
  _szOutOfTripPowerNReplicas = n;
}

/**************************************************************************
 * Get number of samples.
 */

int szGetNSamples() {
  return _nSamples;
}

/**************************************************************************
 * Compute the moments using SZ decoding and pulse pair moments estimators.
 *
 * Trip decoding follows method in report:
 *    Signal Design and Processing Techniques for WSR88D Ambiguity
 *    Resolution. NSSL. Sachidananda et. al. June 1998, Part 2.
 *    Section 5.4.1, page 78.
 *
 * Pulse pair moments estimation follows the methods outlined in
 *    Doviak and Zrnic (1993), in the following sections:
 *    Velocity: section 6.4.1
 *    Width:    section 6.5.1
 *
 * Inputs
 * ------
 *
 *   IQ: IQ time series, length _nSamples
 *
 *   delta12: Measured modulation code from trip 1 to trip 2.
 * 
 *            Note that the phase data which comes from the RVP8 headers is
 *            in fact the delta21. You need to compute the complex conjugate
 *            of this to get delta12 (i.e. multiply imaginary part by -1.0)
 *           
 *            delta12 should be a pointer into a complex array:
 * 
 *              extended_delta12[_nSamples + 4];
 *              delta12 = extended_delta12 + 4;
 *
 *            The extended array should contain the modulation codes
 *            starting at 4 pulses BEFORE the first pulse in the IQ array.
 *            This allows the algorithm access to modulation codes for up
 *            to 5 trips.
 *           
 *    prtSecs: Pulse Repetition Time (secs)
 *
 *  Outputs
 *  -------
 *
 *    power1: trip 1 power (dBm)
 *    vel1:   trip 1 velocity (m/s), positive away from the radar
 *    width1: trip 1 spectral width (m/s)
 *    flags1: trip 1 censoring flags
 *            Values: 0 = no censoring
 *                    1 = censor on total power using SNR
 * 
 *    power2: trip 2 power (dBm)
 *    vel2:   trip 2 velocity (m/s), positive away from the radar
 *    width2: trip 2 spectral width (m/s)
 *    flags2: trip 2 censoring flags
 *            Values: 0 = no censoring
 *                    1 = censor on total power using SNR
 *                    2 = censor on ratio between strong and weak trip power
 *                    4 = censor on out-of-trip power in weak trip
 *
 *    If any of the power, vel or width values are censored, they are
 *    set to _censoredVal = -9999.0;
 */

void szComputeMomentsPp(const Complex_t *IQ,
			const Complex_t *delta12,
			double prtSecs,
			double *power1, double *vel1,
			double *width1, int *flags1,
			double *power2, double *vel2,
			double *width2, int *flags2)
     
{

  int ii;

  // Initialize return vals.

  *power1 = *vel1 = *width1 = _censoredVal;
  *power2 = *vel2 = *width2 = _censoredVal;
  *flags1 = *flags2 = 0;

  // Check thresholding on total power.
  // As a side effect, powerTotal is computed.
  
  double powerTotal;
  if (_checkSnThreshold(IQ, &powerTotal)) {
    *flags1 = _censorOnTotalPower;
    *flags2 = _censorOnTotalPower;
    return;
  }

  // IQ data comes in cohered to trip 1.

  const Complex_t *iqTrip1 = IQ;
      
  // Cohere to trip 2.
  
  Complex_t iqTrip2[_nSamples];
  _cohereTrip1_to_Trip2(iqTrip1, delta12, iqTrip2);
  
  // Compute R1 for each trip.
  
  double r1Trip1 = _computeR1(iqTrip1);
  double r1Trip2 = _computeR1(iqTrip2);

  // Determine which trip is strongest by comparing R1 from 
  // each trip. The trip with the higher R1 value is assumed
  // to be the strong trip.
  
  int trip1IsStrong;
  const Complex_t *iqStrong;
  
  if (r1Trip1 > r1Trip2) {
    trip1IsStrong = true;
    iqStrong = iqTrip1;
  } else {
    trip1IsStrong = false;
    iqStrong = iqTrip2;
  }

  // Compute moments for strong trip (pulse-pair).

  double velStrong, widthStrong;
  _velWidthFromTd(iqStrong, prtSecs, &velStrong, &widthStrong);
  
  // Apply hanning window to strong trip.
  
  Complex_t hanningStrong[_nSamples];
  _applyHanningWindow(iqStrong, hanningStrong);
  
  // Compute FFT for strong trip.
  
  Complex_t strongSpec[_nSamples];
  _fftFwd(hanningStrong, strongSpec);
  
  // Apply 3/4 notch to strong trip. The power left in the part outside 
  // the notch is adjusted so that it represents the power for the
  // entire weak trip spectrum.
  
  Complex_t notchedStrong[_nSamples];
  int notchStart = _computeNotchStart(_szNotchWidth75, velStrong, prtSecs);
  _applyNotch75(notchStart, strongSpec, notchedStrong);

  // Compute weak trip power.

  double powerWeak = _computeMeanPower(notchedStrong);
  
  // Compute strong power by subtracting weak trip power from total power.
  
  double powerStrong = powerTotal - powerWeak;
  double powerRatioDb = 10.0 * log10(powerStrong / powerWeak);
  
  if (powerRatioDb > _szStrongToWeakPowerRatioThreshold) {
    if (trip1IsStrong) {
      *power1 = powerStrong;
      *vel1 = -1.0 * velStrong;
      *width1 = widthStrong;
      *flags2 = _censorOnPowerRatio;
    } else {
      *power2 = powerStrong;
      *vel2 =  -1.0 * velStrong;
      *width2 = widthStrong;
      *flags1 = _censorOnPowerRatio;
    }
    return;
  }
  
  // Invert the notched spectra into the time domain.
  
  Complex_t notchedTd[_nSamples];
  _fftInv(notchedStrong, notchedTd);
  
  // Change coherence to the weaker trip.
  
  Complex_t weakTd[_nSamples];
  if (trip1IsStrong) {
    _cohereTrip1_to_Trip2(notchedTd, delta12, weakTd);
  } else {
    _cohereTrip2_to_Trip1(notchedTd, delta12, weakTd);
  }

  // Compute FFT of cohered weak trip.
  
  Complex_t weakSpec[_nSamples];
  _fftFwd(weakTd, weakSpec);
  
  // Compute magnitude of cohered weak spectrum.
  
  double weakMag[_nSamples];
  _computeMag(weakSpec, weakMag);
  
  // Deconvolve weak trip spectrum,
  // by multiplying with the deconvoltion matrix.
  
  double weakMagDecon[_nSamples];
  memset(weakMagDecon, 0, sizeof(weakMagDecon));
  int irow;
  for (irow = 0; irow < _nSamples; irow++) {
    double *decon = _deconvMatrix75 + irow * _nSamples;
    double *mag = weakMag;
    double sum = 0.0;
    int icol;
    for (icol = 0; icol < _nSamples; icol++, decon++, mag++) {
      sum += *decon * *mag;
    }
    if (sum > 0) {
      weakMagDecon[irow] = sum;
    }
  }
  
  // Check for replicas in the weak spectra.
  // If replicas exist, this indicates leakage of out-of-trip power
  // into the weak trip, so censor the weak trip.

  int weakHasReplicas = _hasReplicas(weakMagDecon);
  if (weakHasReplicas) {
    if (trip1IsStrong) {
      *power1 = powerStrong;
      *vel1 = -1.0 * velStrong;
      *width1 = widthStrong;
      *flags2 = _censorOnReplicas;
    } else {
      *power2 = powerStrong;
      *vel2 = -1.0 * velStrong;
      *width2 = widthStrong;
      *flags1 = _censorOnReplicas;
    }
    return;
  }

  // Compute deconvoluted weak trip complex spectrum, by scaling the
  // original spectrum with the ratio of the deconvoluted magnitudes
  // with the original magnitudes.

  Complex_t weakSpecDecon[_nSamples];
  double *wmd = weakMagDecon;
  double *wm = weakMag;
  Complex_t *wsd = weakSpecDecon;
  Complex_t *ws = weakSpec;
  for (ii = 0; ii < _nSamples; ii++, wmd++, wm++, wsd++, ws++) {
    double ratio = *wmd / *wm;
    wsd->re = ws->re * ratio;
    wsd->im = ws->im * ratio;
  }

  // Inverse FFT to get deconvoluted weak trip back into time domain.

  Complex_t weakTdDecon[_nSamples];
  _fftInv(weakSpecDecon, weakTdDecon);
  
  // Compute vel and width for weak trip in time domain (pulse-pair).
  
  double velWeak = 0.0, widthWeak = 0.0;
  _velWidthFromTd(weakTdDecon, prtSecs, &velWeak, &widthWeak);
  
  // Set return vals, changing sign of velocity so that positive
  // represents motion away from the radar.
  
  if (trip1IsStrong) {
    *power1 = powerStrong;
    *vel1 = -1.0 * velStrong;
    *width1 = widthStrong;
    *power2 = powerWeak;
    *vel2 = -1.0 * velWeak;
    *width2 = widthWeak;
  } else {
    *power1 = powerWeak;
    *vel1 = -1.0 * velWeak;
    *width1 = widthWeak;
    *power2 = powerStrong;
    *vel2 = -1.0 * velStrong;
    *width2 = widthStrong;
  }
  
}

/**************************************************************************
 * Compute the moments using SZ decoding and FFT moments estimators.
 *
 * Trip decoding follows method in report:
 *    Signal Design and Processing Techniques for WSR88D Ambiguity
 *    Resolution. NSSL. Sachidananda et. al. June 1998, Part 2.
 *    Section 5.4.1, page 78.
 *
 * FFT moments estimation follows the methods outlined in
 *    Doviak and Zrnic (1993), in the following sections:
 *    Velocity: section 6.4.2
 *    Width:    section 6.5.2
 *
 * Inputs
 * ------
 *
 *   IQ: IQ time series, length _nSamples
 *
 *   delta12: Measured modulation code from trip 1 to trip 2.
 * 
 *            Note that the phase data which comes from the RVP8 headers is
 *            in fact the delta21. You need to compute the complex conjugate
 *            of this to get delta12 (i.e. multiply imaginary part by -1.0)
 *           
 *            delta12 should be a pointer into a complex array:
 * 
 *              extended_delta12[_nSamples + 4];
 *              delta12 = extended_delta12 + 4;
 *
 *            The extended array should contain the modulation codes
 *            starting at 4 pulses BEFORE the first pulse in the IQ array.
 *            This allows the algorithm access to modulation codes for up
 *            to 5 trips.
 *           
 *    prtSecs: Pulse Repetition Time (secs)
 *
 *  Outputs
 *  -------
 *
 *    power1: trip 1 power (dBm)
 *    vel1:   trip 1 velocity (m/s), positive away from the radar
 *    width1: trip 1 spectral width (m/s)
 *    flags1: trip 1 censoring flags
 *            Values: 0 = no censoring
 *                    1 = censor on total power using SNR
 * 
 *    power2: trip 2 power (dBm)
 *    vel2:   trip 2 velocity (m/s), positive away from the radar
 *    width2: trip 2 spectral width (m/s)
 *    flags2: trip 2 censoring flags
 *            Values: 0 = no censoring
 *                    1 = censor on total power using SNR
 *                    2 = censor on ratio between strong and weak trip power
 *                    4 = censor on out-of-trip power in weak trip
 *
 *    If any of the power, vel or width values are censored, they are
 *    set to _censoredVal = -9999.0;
 */

void szComputeMomentsFft(const Complex_t *IQ,
			 const Complex_t *delta12,
			 double prtSecs,
			 double *power1, double *vel1,
			 double *width1, int *flags1,
			 double *power2, double *vel2,
			 double *width2, int *flags2)
     
{

  // Initialize return vals.

  *power1 = *vel1 = *width1 = _censoredVal;
  *power2 = *vel2 = *width2 = _censoredVal;
  *flags1 = *flags2 = 0;

  // Check thresholding on total power.
  // As a side effect, powerTotal is computed.

  double powerTotal;
  if (_checkSnThreshold(IQ, &powerTotal)) {
    *flags1 = _censorOnTotalPower;
    *flags2 = _censorOnTotalPower;
    return;
  }

  // IQ data comes in cohered to trip 1.

  const Complex_t *iqTrip1 = IQ;
      
  // Cohere to trip 2.
  
  Complex_t iqTrip2[_nSamples];
  _cohereTrip1_to_Trip2(iqTrip1, delta12, iqTrip2);
  
  // Compute R1 for each trip.
  
  double r1Trip1 = _computeR1(iqTrip1);
  double r1Trip2 = _computeR1(iqTrip2);

  // Determine which trip is strongest by comparing R1 from 
  // each trip. The trip with the higher R1 value is assumed
  // to be the strong trip.
  
  int trip1IsStrong;
  const Complex_t *iqStrong;
  
  if (r1Trip1 > r1Trip2) {
    trip1IsStrong = true;
    iqStrong = iqTrip1;
  } else {
    trip1IsStrong = false;
    iqStrong = iqTrip2;
  }

  // Apply hanning window to strong trip.
  
  Complex_t hanningStrong[_nSamples];
  _applyHanningWindow(iqStrong, hanningStrong);
  
  // Compute FFT for strong trip.
  
  Complex_t strongSpec[_nSamples];
  _fftFwd(hanningStrong, strongSpec);
 
  // Compute moments for strong trip - FFT methods.
  
  double velStrong, widthStrong;
  double strongMag[_nSamples];
  _computeMag(strongSpec, strongMag);
  _velWidthFromFft(strongMag, prtSecs, &velStrong, &widthStrong);
  
  // Apply 3/4 notch to strong trip. The power left in the part outside 
  // the notch is adjusted so that it represents the power for the
  // entire weak trip spectrum.
  
  Complex_t notchedStrong[_nSamples];
  int notchStart = _computeNotchStart(_szNotchWidth75, velStrong, prtSecs);
  _applyNotch75(notchStart, strongSpec, notchedStrong);
   
  // Compute weak trip power.

  double powerWeak = _computeMeanPower(notchedStrong);
  
  // Correct strong power by subtracting weak trip power.
  
  double powerStrong = powerTotal - powerWeak;

  // Check for censoring based on power ratio.

  double powerRatioDb = 10.0 * log10(powerStrong / powerWeak);
  
  if (powerRatioDb > _szStrongToWeakPowerRatioThreshold) {
    if (trip1IsStrong) {
      *power1 = powerStrong;
      *vel1 = -1.0 * velStrong;
      *width1 = widthStrong;
      *flags2 = _censorOnPowerRatio;
    } else {
      *power2 = powerStrong;
      *vel2 =  -1.0 * velStrong;
      *width2 = widthStrong;
      *flags1 = _censorOnPowerRatio;
    }
    return;
  }
  
  // invert the notched spectra into the time domain
  
  Complex_t notchedTd[_nSamples];
  _fftInv(notchedStrong, notchedTd);
 
  // Change coherence to the weaker trip.
  
  Complex_t weakTd[_nSamples];
  if (trip1IsStrong) {
    _cohereTrip1_to_Trip2(notchedTd, delta12, weakTd);
  } else {
    _cohereTrip2_to_Trip1(notchedTd, delta12, weakTd);
  }
  
  // Compute FFT of cohered weak trip.
  
  Complex_t weakSpec[_nSamples];
  _fftFwd(weakTd, weakSpec);
  
  // Compute magnitude of cohered weak spectrum.
  
  double weakMag[_nSamples];
  _computeMag(weakSpec, weakMag);
 
  // Deconvolve weak trip spectrum,
  // by multiplying with the deconvoltion matrix.
  
  double weakMagDecon[_nSamples];
  memset(weakMagDecon, 0, sizeof(weakMagDecon));
  int irow;
  for (irow = 0; irow < _nSamples; irow++) {
    double *decon = _deconvMatrix75 + irow * _nSamples;
    double *mag = weakMag;
    double sum = 0.0;
    int icol;
    for (icol = 0; icol < _nSamples; icol++, decon++, mag++) {
      sum += *decon * *mag;
    }
    if (sum > 0) {
      weakMagDecon[irow] = sum;
    }
  }

  // Check for replicas in the weak spectra.
  // If replicas exist, this indicates leakage of out-of-trip power
  // into the weak trip, so censor the weak trip.
  
  int weakHasReplicas = _hasReplicas(weakMagDecon);
  if (weakHasReplicas) {
    if (trip1IsStrong) {
      *power1 = powerStrong;
      *vel1 = -1.0 * velStrong;
      *width1 = widthStrong;
      *flags2 = _censorOnReplicas;
    } else {
      *power2 = powerStrong;
      *vel2 = -1.0 * velStrong;
      *width2 = widthStrong;
      *flags1 = _censorOnReplicas;
    }
    return;
  }
  
  // Compute vel and width for weak trip - FFT method.
  
  double velWeak = 0.0, widthWeak = 0.0;
  _velWidthFromFft(weakMagDecon, prtSecs, &velWeak, &widthWeak);
  
  // set return vals
  
  if (trip1IsStrong) {
    *power1 = powerStrong;
    *vel1 = -1.0 * velStrong;
    *width1 = widthStrong;
    *power2 = powerWeak;
    *vel2 = -1.0 * velWeak;
    *width2 = widthWeak;
  } else {
    *power1 = powerWeak;
    *vel1 = -1.0 * velWeak;
    *width1 = widthWeak;
    *power2 = powerStrong;
    *vel2 = -1.0 * velStrong;
    *width2 = widthStrong;
  }
  
}

/**************************************************************************
 * Clean up module.
 *
 * Call this when the module is no longer needed.
 */

void szCleanUp()
     
{
  fftw_destroy_plan(_fftPlanFwd);
  fftw_destroy_plan(_fftPlanInv);
}

/**************************************************************************
 *********************** Private functions ********************************
 **************************************************************************/

/**************************************************************************
 * Initialize the theoretical command phase codes.
 *
 * See Report: Signal Design and Processing Techniques
 *             for WSR88D Ambiguity Resolution. NSSL.
 *             Sachidananda et. al. June 1998, Part 2.
 *             Equation 2.1, page 4.
 */

static void _initPhaseCodes()
  
{
  
  int ii;
  Complex_t switchCode[_nSamples];
  Complex_t *trip1Code;
  Complex_t trip2Code[_nSamples];
  double ratio = (double) _phaseCodeN / (double) _nSamples;
  double angle = 0.0;
  
  for (ii = 0; ii < _nSamples; ii++) {
    
    double code = (double) ii * (double) ii * ratio;
    double deltaAngle = code * _pi;
    angle += deltaAngle;

    switchCode[ii].re = cos(angle);
    switchCode[ii].im = sin(angle);
    
  }

  // The switching code is the code for trip 1.
  
  trip1Code = switchCode;

  // The code for trip 2 is the trip 1 code shifted left by 1

  for (ii = 0; ii < _nSamples; ii++) {
    int jj = (ii - 1 + _nSamples) % _nSamples;
    trip2Code[ii] = trip1Code[jj];
  }
  
  // compute modulation code from trip1 to trip2, by subtracting
  // the codes
  
  _subModCode(trip1Code, trip2Code, _modCode12);

}

/**************************************************************************
 * Initialize the deconvolution matrix.
 *
 * The deconvolution matrix is computed for a given notch width.
 * The theoretical commanded phases are assumed.
 *
 * See Report: Signal Design and Processing Techniques
 *             for WSR88D Ambiguity Resolution. NSSL.
 *             Sachidananda et. al. June 1998, Part 2.
 *             Section 2.3, page 9.
 *
 * Inputs:
 *  notchWidth: width of notch
 *  powerWidth: width outside notch = _nSamples - notchWidth
 *  fracPower: fraction of total power within notch.
 *
 * Output:
 *  deconvMatrix - size (_nSamples * _nSamples)
 */

static void _initDeconMatrix(int notchWidth,
			     int powerWidth,
			     double fracPower,
			     double *deconvMatrix)
  
{
  
  // compute the spectrum of the modulation code from trip 1 to trip 2

  Complex_t modSpec12[_nSamples];
  _fftFwd(_modCode12, modSpec12);
  
  // apply a 3/4 notch to the spectrum
  
  Complex_t notchedSpec12[_nSamples];
  _applyNotch(0, modSpec12, notchWidth, powerWidth, fracPower, notchedSpec12);
  
  // invert the notched spectra into the time domain
  
  Complex_t notchedCode12[_nSamples];
  _fftInv(notchedSpec12, notchedCode12);
  
  // cohere notched time series to trip 1
  
  Complex_t cohered12[_nSamples];
  _subModCode(notchedCode12, _modCode12, cohered12);
  
  // compute cohered spectra
  
  Complex_t coheredSpec12[_nSamples];
  _fftFwd(cohered12, coheredSpec12);
  
  // compute normalized magnitude
  
  double normMag12[_nSamples];
  _normalizeMagSum(coheredSpec12, normMag12);
  
  // Load up convolution matrix
  // First create mag array of double length so we can
  // use it as the source for each line without worrying about wrapping
  // Then we load up each row, moving to the right by one element
  // for each row.
  
  double mag2[_nSamples * 2];
  memcpy(mag2, normMag12, sizeof(normMag12));
  memcpy(mag2 + _nSamples, normMag12, sizeof(normMag12));
  
  double convMatrix[_nSamples * _nSamples];
  int irow;
  for (irow = 0; irow < _nSamples; irow++) {
    memcpy(convMatrix + irow * _nSamples,
	   mag2 + _nSamples - irow, sizeof(normMag12));
  }
  
  // Set small values to 0 to improve the condition of the matrix.
  // This aids in inversion.
  
  double *conv = convMatrix;
  int ii;
  for (ii = 0; ii < _nSamples * _nSamples; ii++, conv++) {
    if (fabs(*conv) < _smallValue) {
      *conv = 0;
    }
  }
  
  // Invert the matrix
  
  memcpy(deconvMatrix, convMatrix, sizeof(convMatrix));
  _invertMatrix(deconvMatrix, _nSamples);
  
  // Set small values to 0 to improve condition of deconvolution matrix.
  
  double *deconv = deconvMatrix;
  for (ii = 0; ii < _nSamples * _nSamples; ii++, deconv++) {
    if (fabs(*deconv) < _smallValue) {
      *deconv = 0;
    }
  }

}

/**************************************************************************
 * Initalize the hanning window.
 *
 * The Hanning Window function was obtained from:
 *   Digital Signal Processing, Roberts and Mullis, 1987.
 *   Publisher: Addison Wesley.
 *   Page 183.
 *
 *   wH(k) = w(k) [ 0.5 + 0.5 cos((2.PI.k)/N) ]
 *
 * The window coefficients are adjusted so that after applying the
 * window, the mean power will be the same as before applying the
 * window. 
 */

static void _initHanning(double *window)
     
{
  
  int ii;
  double sumsq, rms;

  // compute window coefficients

  for (ii = 0; ii < _nSamples; ii++) {
    double ang = 2.0 * M_PI * ((ii + 0.5) / (double) _nSamples - 0.5);
    window[ii] = 0.5 * (1.0 + cos(ang));
  }

  // adjust window coefficients to keep mean power constant

  sumsq = 0.0;
  for (ii = 0; ii < _nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  rms = sqrt(sumsq / _nSamples);
  for (ii = 0; ii < _nSamples; ii++) {
    window[ii] /= rms;
  }
  
}
  
/**************************************************************************
 * Compute velocity and width in time domain.
 *
 * Inputs:
 *  IQ: time series, length _nSamples
 *  prtSecs: Pulse Repetition Time (secs)
 *
 * Outputs:
 *  vel: velocity (m/s)
 *  width: spectral width (m/s)
 *
 * Method:
 *  Follows the methods outlined in Doviak and Zrnic (1993),
 *  in the following sections:
 *    Velocity: section 6.4.1, eq 6.19, p 132.
 *    Width:    section 6.5.1, eq 6.32, p 138.
 */

static void _velWidthFromTd(const Complex_t *IQ,
			    double prtSecs,
			    double *vel, double *width)
  
{

  int ii;

  // compute a, b, r1
  
  double a = 0.0, b = 0.0;
  
  const Complex_t *iq0 = IQ;
  const Complex_t *iq1 = IQ + 1;
  
  for (ii = 0; ii < _nSamples - 1; ii++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
  }
  double r1_val = sqrt(a * a + b * b) / _nSamples;
  
  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  iq0 = IQ;
  const Complex_t *iq2 = IQ + 2;

  for (ii = 0; ii < _nSamples - 2; ii++, iq0++, iq2++) {
    c += ((iq0->re * iq2->re) + (iq0->im * iq2->im));
    d += ((iq0->re * iq2->im) - (iq2->re * iq0->im));
  }
  double r2_val = sqrt(c * c + d * d) / _nSamples;
  
  // velocity
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  double nyqFac = nyquist / M_PI;
  if (a == 0.0 && b == 0.0) {
    *vel = 0.0;
  } else {
    *vel = nyqFac * atan2(b, a);
  }
  
  // width from R1R2

  double r1r2_fac = (2.0 * nyquist) / (M_PI * sqrt(6.0));
  double ln_r1r2 = log(r1_val/r2_val);
  if (ln_r1r2 > 0) {
    *width = r1r2_fac * sqrt(ln_r1r2);
  } else {
    *width = r1r2_fac * -1.0 * sqrt(fabs(ln_r1r2));
  }
  
}

/**************************************************************************
 * Compute velocity and width in FFT domain.
 *
 * Inputs:
 *  magnitude: spectrum, length _nSamples
 *  prtSecs: Pulse Repetition Time (secs)
 *
 * Outputs:
 *  vel: velocity (m/s)
 *  width: spectral width (m/s)
 *
 * Method:
 *  Follows the methods outlined in Doviak and Zrnic (1993),
 *  in the following sections:
 *    Velocity: section 6.4.2, eq 6.24, p 135.
 *    Width:    section 6.5.2, eq 6.34, p 140.
 *
 * We confine the data used in computing the velocity and width to
 * the region in the vicinity of the spectral peak which is above the
 * spectral noise. This prevents the widths from becoming large for
 * noisy data.
 */

static void _velWidthFromFft(const double *magnitude,
			     double prtSecs,
			     double *vel, double *width)
  
{

  int ii;
  int kCent = _nSamples / 2; // central index
  
  // find max magnitude
  
  double maxMag = 0.0;
  int kMax = 0;
  const double *mp = magnitude;
  for (ii = 0; ii < _nSamples; ii++, mp++) {
    if (*mp > maxMag) {
      kMax = ii;
      maxMag = *mp;
    }
  }
  if (kMax >= kCent) {
    kMax -= _nSamples;
  }

  // center power array on the max value
  // compute the mean power
  
  double powerCentered[_nSamples];
  int kOffset = kCent - kMax;
  double sumPower = 0.0;
  for (ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + kOffset) % _nSamples;
    double powr = magnitude[ii] * magnitude[ii];
    powerCentered[jj] = powr;
    sumPower += powr;
  }
  double meanPower = sumPower / _nSamples;
  
  // compute noise floor

  double noiseMean, noiseSdev;
  _computeSpectralNoise(powerCentered, &noiseMean, &noiseSdev);
  double noiseThreshold = noiseMean + noiseSdev;

  // if the signal is noisy, we use the entire spectrum to compute
  // the width. Otherwise we trim down kStart and kEnd to give us
  // the region around the peak which is above the spectral noise.

  int kStart = 0;
  int kEnd = _nSamples - 1;
  
  if (meanPower > noiseMean + 3 * noiseSdev) {
    
    // moving away from the peak, find the points in the spectrum
    // where the signal drops below the noise threshold for at
    // least 2 consecutive points

    int nTest = 2;
    int count = 0;
    kStart = kCent - 1;
    double *pw = powerCentered + kStart;
    for (ii = kStart; ii >= 0; ii--, pw--) {
      if (*pw < noiseThreshold) {
	count ++;
	if (count >= nTest) {
	  break;
	}
      } else {
	count = 0;
      }
      kStart = ii;
    }
    
    count = 0;
    kEnd = kCent + 1;
    pw = powerCentered + kEnd;
    for (ii = kEnd; ii < _nSamples; ii++, pw++) {
      if (*pw < noiseThreshold) {
	count ++;
	if (count >= nTest) {
	  break;
	}
      } else {
	count = 0;
      }
      kEnd = ii;
    }
    
  }
  
  // compute vel and width, using only that region centered on
  // the peak which is above the noise
  
  double sumP = 0.0;
  double sumK = 0.0;
  double sumK2 = 0.0;
  double *pw = powerCentered + kStart;
  for (ii = kStart; ii <= kEnd; ii++, pw++) {
    double phase = (double) ii;
    double pwr = *pw;
    sumP += pwr;
    sumK += pwr * phase;
    sumK2 += pwr * phase * phase;
  }
  double meanK = 0.0;
  double sdevK = 0.0;
  if (sumP > 0.0) {
    meanK = sumK / sumP;
    double diff = (sumK2 / sumP) - (meanK * meanK);
    if (diff > 0) {
      sdevK = sqrt(diff);
    }
  }

  // set return values

  double velFac = _wavelengthMeters / (2.0 * _nSamples * prtSecs);
  *vel = velFac * (meanK - kOffset);
  *width = velFac * sdevK;

}

/**************************************************************************
 * Subtract modulation code from IQ time series.
 *
 * Inputs:
 *  iq: IQ time series, length _nSamples.
 *  code: phase code, length _nSamples.
 *
 * Output:
 *  diff: iq - code.
 */

void _subModCode(const Complex_t *iq, const Complex_t *code, Complex_t *diff)
  
{

  int ii;
  for (ii = 0; ii < _nSamples; ii++, iq++, code++, diff++) {
    diff->re = (iq->re * code->re + iq->im * code->im);
    diff->im = (iq->im * code->re - iq->re * code->im);
  }
  
}

/**************************************************************************
 * Apply a 3/4 notch to a spectrum, given the start point of the notch.
 *
 * Inputs:
 *  startIndex: start index for notch
 *  in: complex spectrum, n = _nSamples
 *
 * Output:
 *  notched: spectrum with all values within the notch limits set to 0.
 *           The power values are set so that the mean power from the
 *           values within the notch is equal to the expected power from
 *           the entire spectrum, assuming an even distribution of power
 *           within the notch and outside it.
 */

static void _applyNotch75(int startIndex,
			  Complex_t *in,
			  Complex_t *notched)

{
  _applyNotch(startIndex, in,
	      _szNotchWidth75, _szPowerWidth75, _szFracPower75,
	      notched);
}

/**************************************************************************
 * Apply a notch to a spectrum, given the start point of the notch
 * and the notch width.
 *
 * Inputs:
 *  startIndex: start index for notch
 *  in: complex spectrum, n = _nSamples
 *  notchWidth: width of notch
 *  powerWidth: width outside notch = _nSamples - notchWidth
 *  fracPower: fraction of total power within notch.
 *
 * Output:
 *  notched: spectrum with all values within the notch limits set to 0.
 *           The power values are set so that the mean power from the
 *           values within the notch is equal to the expected power from
 *           the entire spectrum, assuming an even distribution of power
 *           within the notch and outside it.
 */

static void _applyNotch(int startIndex,
			Complex_t *in,
			int notchWidth,
			int powerWidth,
			double fracPower,
			Complex_t *notched)
  
{
  
  // compute the end index for the notch
  
  int iStart = startIndex;
  int iEnd = iStart + notchWidth - 1;
  if (iStart < 0 && iEnd < 0) {
    iStart += _nSamples;
    iEnd += _nSamples;
  }

  // apply the notch, taking into account the folding of the
  // notch around the ends of the array

  if (iStart >= 0 && iEnd < _nSamples) {

    // notch does not wrap, copy array then zero out the notch
    
    memcpy(notched, in, _nSamples * sizeof(Complex_t));
    memset(notched + iStart, 0, notchWidth * sizeof(Complex_t));
    
  } else {

    // notch wraps, non-notch region is centered
    // zero out array, copy non-notch region

    int copyStart;
    if (iStart < 0) {
      // notch wraps at lower end
      copyStart = iEnd + 1;
    } else {
      // notch wraps at upper end
      // copyStart = _nSamples - (iEnd + 1);
      copyStart = iEnd - _nSamples + 1;
    }
    
    memset(notched, 0, _nSamples * sizeof(Complex_t));
    memcpy(notched + copyStart, in + copyStart,
	   powerWidth * sizeof(Complex_t));

  }
  
  // modify power so that power in part left is equal to
  // theoretical distributed power
  
  Complex_t *nn = notched;
  double mult = 1.0 / sqrt(fracPower);
  int ii;
  for (ii = 0; ii < _nSamples; ii++, nn++) {
    nn->re *= mult;
    nn->im *= mult;
  }

}


/**************************************************************************
 * Compute magnitudes for a given complex spectrum.
 *
 * Set the magnitude to 0 if it is less than a small value.
 *
 * Input: complex spectrum, n = _nSamples.
 * Output: array of magnitudes.
 */

static void _computeMag(const Complex_t *in, double *mag)

{

  int ii;
  for (ii = 0; ii < _nSamples; ii++, in++, mag++) {
    double mm = sqrt(in->re * in->re + in->im * in->im);
    if (mm > _smallValue) {
      *mag = mm;
    } else {
      *mag = 0.0;
    }
  }

}

/**************************************************************************
 * Compute normalize magnitudes in a spectrum, so that the sum of
 * the magnitudes is 1.0.
 *
 * Input: complex spectrum, n = _nSamples.
 * Output: array of normalized magnitudes.
 */

static void _normalizeMagSum(const Complex_t *in, double *normMag)
     
{
  
  int ii;
  double sum = 0.0;
  double *norm = normMag;
  for (ii = 0; ii < _nSamples; ii++, in++, norm++) {
    *norm = sqrt(in->re * in->re + in->im * in->im);
    sum += *norm;
  }
  norm = normMag;
  for (ii = 0; ii < _nSamples; ii++, norm++) {
    (*norm) /= sum;
  }

}

/**************************************************************************
 * Check if the mean power in an IQ time series exceeds the noise
 * value by the signal-to-noise threshold.
 * 
 * Input: IQ time-series, length _nSamples.
 *
 * Returns: true (1) of exceeds SNR, false (0) if not.
 *
 * Side-effect: computes mean power, passes this back in meanPower.
 */

static int _checkSnThreshold(const Complex_t *IQ,
			     double *meanPower)
     
{
  *meanPower = _computeMeanPower(IQ);
  double dbm = 10.0 * log10(*meanPower);
  if (dbm < _noiseValueDbm + _signalToNoiseRatioThreshold) {
    return true;
  }
  return false;
}
  
/**************************************************************************
 * Change coherence of an IQ time-series cohered to trip1, to be
 * cohered to trip2.
 *
 * This is done by subtracting the conjugate of delta12.
 *
 * Inputs:
 *   trip1: IQ time series, length _nSamples, cohered to trip1.
 *   delta12: array of measured phase differences between
 *            pulse[i] and pulse[i-1], i = 0 to _nSamples.
 *
 * Output:
 *  trip2: IQ time series, length _nSamples, cohered to trip2.
 */

static void _cohereTrip1_to_Trip2(const Complex_t *trip1,
				  const Complex_t *delta12,
				  Complex_t *trip2)
  
{
  
  const Complex_t *t1 = trip1;
  Complex_t *t2 = trip2;
  const Complex_t *dd = delta12;
  int ii;
  for (ii = 0; ii < _nSamples; ii++, t1++, t2++, dd++) {
    t2->re = t1->re * (dd->re * -1.0) + t1->im * dd->im;
    t2->im = t1->im * (dd->re * -1.0) - t1->re * dd->im;
  }

}

/**************************************************************************
 * Change coherence of an IQ time-series cohered to trip2, to be
 * cohered to trip1.
 *
 * This is done by subtracting delta12.
 *
 * Inputs:
 *   trip2: IQ time series, length _nSamples, cohered to trip2.
 *   delta12: array of measured phase differences between
 *            pulse[i] and pulse[i-1], i = 0 to _nSamples.
 *
 * Output:
 *  trip1: IQ time series, length _nSamples, cohered to trip1.
 */

static void _cohereTrip2_to_Trip1(const Complex_t *trip2,
				  const Complex_t *delta12,
				  Complex_t *trip1)
  
{
  
  Complex_t *t1 = trip1;
  const Complex_t *t2 = trip2;
  const Complex_t *dd = delta12;
  int ii;
  
  for (ii = 0; ii < _nSamples; ii++, t1++, t2++, dd++) {
    t1->re = t2->re * dd->re + t2->im * dd->im;
    t1->im = t2->im * dd->re - t2->re * dd->im;
  }

}

/**************************************************************************
 * Compute mean power for an IQ time series.
 *
 * Input: IQ time-series, length _nSamples.
 *
 * Returns: mean power
 *
 * See Doviak and Zrnic (1993), Eq 6.6, p 126.
 *
 */

static double _computeMeanPower(const Complex_t *IQ)

{
  
  int ii;
  double p = 0.0;
  
  if (_nSamples < 1) {
    return 0.0;
  }
  
  for (ii = 0; ii < _nSamples; ii++, IQ++) {
    p += ((IQ->re * IQ->re) + (IQ->im * IQ->im));
  }

  return (p / _nSamples);
  
}

/**************************************************************************
 * Compute R1, the lag-1 auto-correlation coefficient.
 *
 * Input: IQ time-series, length _nSamples.
 *
 * Returns: R1
 *
 * See Doviak and Zrnic (1993), Eq 6.18, p 132.
 *
 */

static double _computeR1(const Complex_t *IQ)

{

  double a = 0.0, b = 0.0;
  const Complex_t *iq0 = IQ;
  const Complex_t *iq1 = IQ + 1;
  int ii;

  if (_nSamples < 1) {
    return 0.0;
  }

  for (ii = 0; ii < _nSamples - 1; ii++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
  }

  return (sqrt(a * a + b * b) / _nSamples);

}
  
/**************************************************************************
 * Apply Hanning window to an IQ time-series.
 *
 * This operation consists of multiplying each complex value in the
 * IQ series by the corresponding Hanning coefficient.
 * 
 */

static void _applyHanningWindow(const Complex_t *in, Complex_t *out)
  
{
  
  const double *ww = _hanning;
  const Complex_t *inp = in;
  Complex_t *outp = out;
  int ii;

  for (ii = 0; ii < _nSamples; ii++, ww++, inp++, outp++) {
    outp->re = inp->re * *ww;
    outp->im = inp->im * *ww;
  }

}
  
/**************************************************************************
 * Compute mean and standard deviation of noise for the spectrum.
 *
 * This is used in the computation of velocity and width. It allows
 * us to limit our computations to the region around the spectral 
 * peak which is above the noise value, and not broaded the width
 * by considering spectral points which are actually noise.
 *
 * The input is the array of power values in the spectrum, centered
 * on the spectral peak.
 *
 * We compute the mean power for 3 regions of the spectrum:
 *   1. 1/8 at lower end plus 1/8 at upper end
 *   2. 1/4 at lower end
 *   3. 1/4 at uppoer end
 *
 * We estimate the noise to be the least of these 3 values because if
 * there is a weather echo it will not affect both ends of the
 * spectrum unless the width is very high. In this casecase we
 * probably have a bad signal/noise ratio anyway
 */

static void _computeSpectralNoise(const double *powerCentered,
				  double *noiseMean,
				  double *noiseSdev)
  
{
  
  int ii;
  int nby4 = _nSamples / 4;
  int nby8 = _nSamples / 8;
  
  // Compute mean power for the two 1/8 parts at each end

  double sumEnds = 0.0;
  double sumSqEnds = 0.0;
  const double *pw = powerCentered;
  for (ii = 0; ii < nby8; ii++, pw++) {
    sumEnds += *pw;
    sumSqEnds += *pw * *pw;
  }
  pw = powerCentered + _nSamples - nby8 - 1;
  for (ii = 0; ii < nby8; ii++, pw++) {
    sumEnds += *pw;
    sumSqEnds += *pw * *pw;
  }
  double meanEnds = sumEnds / (2.0 * nby8);

  // Compute mean power for the 1/4 at the lower end

  double sumLower = 0.0;
  double sumSqLower = 0.0;
  pw = powerCentered;
  for (ii = 0; ii < nby4; ii++, pw++) {
    sumLower += *pw;
    sumSqLower += *pw * *pw;
  }
  double meanLower = sumLower / (double) nby4;
  
  // Compute mean power for the 1/4 at the upper end
  
  double sumUpper = 0.0;
  double sumSqUpper = 0.0;
  pw = powerCentered + _nSamples - nby4 - 1;
  for (ii = 0; ii < nby4; ii++, pw++) {
    sumUpper += *pw;
    sumSqUpper += *pw * *pw;
  }
  double meanUpper = sumUpper / (double) nby4;

  // find the minimum value from the 3 options

  if (meanEnds < meanLower && meanEnds < meanUpper) {

    double diff = (sumSqEnds / (2.0 * nby8)) - (meanEnds * meanEnds);
    double sdev = 0.0;
    if (diff > 0) {
      sdev = sqrt(diff);
    }
    *noiseMean = meanEnds;
    *noiseSdev = sdev;

  } else if (meanLower < meanUpper) {
    
    double diff = (sumSqLower / nby4) - (meanLower * meanLower);
    double sdev = 0.0;
    if (diff > 0) {
      sdev = sqrt(diff);
    }
    *noiseMean = meanLower;
    *noiseSdev = sdev;

  } else {

    double diff = (sumSqUpper / nby4) - (meanUpper * meanUpper);
    double sdev = 0.0;
    if (diff > 0) {
      sdev = sqrt(diff);
    }
    *noiseMean = meanUpper;
    *noiseSdev = sdev;

  }

}

/**************************************************************************
 * Compute notch start position.
 *
 * This notch must be centered as far as possible from the spectral
 * peak. The location of the spectral peak is inferred from the
 * velocity value.
 *
 * Returns the index for the start of the notch
 */

static int _computeNotchStart(int notchWidth,
			      double vel,
			      double prtSecs)
  
{
  
  // compute the nyquist
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  
  // compute the location of the spectral peak based on the
  // velocity
  
  double integ;
  double dPeak = modf(vel / (2.0 * nyquist) + 1.0, &integ);
  dPeak *= _nSamples;

  // if half width is even, add 0.5 to peak position to
  // evenly space the notch around the spectral peak
  
  int halfWidth = notchWidth / 2;
  int peakIndex;
  if (notchWidth == halfWidth * 2) {
    peakIndex = (int) (dPeak + 0.5);
  } else {
    peakIndex = (int) dPeak;
  }
  
  // compute the start position;
  
  int startIndex = peakIndex - halfWidth;

  return startIndex;
  
}

/**************************************************************************
 * Check if spectrum has replicas.
 * This is used to check for out-of-trip power, for censoring.
 *
 * Passed in: spectrum magnitudes. Assumed length _nSamples.
 *
 * The method is as follows:
 *   1. Find the peak magnitude.
 *   2. Using the peak as the central point, divide the spectrum into
 *      8 equal parts, folding around the ends as necessary.
 *   3. Find the max magnitude in each part, i.e. the original peak
 *      plus 7 maxima in the other parts.
 *   4. Sort the max values.
 *   5. Consider only the values from the 6 parts with the lowest max
 *      values. In other words, ignore the 2nd-to-highest peak, since this
 *      may be related to clutter.
 *   6. From the lowest 6 values, count up the number of values which
 *      exceed the peak minus _szOutOfTripPowerRatioThreshold. This
 *      finds out how many significant replicas exits.
 *   7. If the number of significant replicas exceeds
 *      _szOutOfTripPowerNReplicas, return true. Otherwise return false.
 */

static int _hasReplicas(double *magnitude)
  
{
  
  // find max magnitude
  
  double maxMag = 0.0;
  int kMax = 0;
  double *mp = magnitude;
  int ii;
  for (ii = 0; ii < _nSamples; ii++, mp++) {
    if (*mp > maxMag) {
      kMax = ii;
      maxMag = *mp;
    }
  }

  // set peak[0] to the max mag

  double peaks[8];
  peaks[0] = maxMag;

  // Create a magnitude array of twice the length of the original.
  // This simplifies the search for max values, since we do not
  // need to worry about folding around the end of the array.

  double mag2[_nSamples * 2];
  memcpy(mag2, magnitude, _nSamples * sizeof(double));
  memcpy(mag2 + _nSamples, magnitude, _nSamples * sizeof(double));

  // find each of 7 other peaks
  
  int kStart = kMax + _nSamples / 16;
  int binWidth = _nSamples / 8;

  for (ii = 1; ii < 8; ii++, kStart += binWidth) {
    double maxInBin = 0.0;
    double *mp2 = mag2 + kStart;
    int k;
    for (k = 0; k < binWidth; k++, mp2++) {
      if (*mp2 > maxInBin) {
	maxInBin = *mp2;
      }
    } // k
    peaks[ii] = maxInBin;
  } // ii

  // sort the peaks

  qsort(peaks, 8, sizeof(double), _compareDoubles);

  // count the number of peaks for which the replica power ratio
  // exceeds the threshold

  double ratio = pow(10.0, _szOutOfTripPowerRatioThreshold / 10.0);
  double testVal = peaks[7] / ratio;
  int count = 0;
  for (ii = 0; ii < 6; ii++) {
    if (peaks[ii] > testVal) {
      count++;
    }
  }

  // return true if the number of significant replicas exceeds 
  // the parameter _szOutOfTripPowerNReplicas

  if (count >= _szOutOfTripPowerNReplicas) {
    return true;
  }

  return false;

}

/**************************************************************************
 * Invert square matrix.
 *
 * This routine uses LU decomposition.
 *
 * Any standard matrix inversion routine could be used in place of
 * this routine.
 */

static void _invertMatrix(double *data, int nn)

{
  
  int i, j, k;

  if (data[0] != 0.0) {
    for (i = 1; i < nn; i++) {
      data[i] /= data[0]; // normalize row 0
    }
  }

  for (i = 1; i < nn; i++)  { 
    for (j = i; j < nn; j++)  { // do a column of L
      double sum = 0.0;
      for (k = 0; k < i; k++) {
	sum += data[j*nn+k] * data[k*nn+i];
      }
      data[j*nn+i] -= sum;
    } // j
    
    if (i == nn - 1) {
      continue;
    }
    for (j = i+1; j < nn; j++)  {  // do a row of U
      double sum = 0.0;
      for (k = 0; k < i; k++) {
	sum += data[i*nn+k]*data[k*nn+j];
      }
      data[i*nn+j] = (data[i*nn+j]-sum) / data[i*nn+i];
    }
  } // i

  // invert L
  
  for (i = 0; i < nn; i++) {
    for (j = i; j < nn; j++)  {
      double x = 1.0;
      if (i != j) {
	x = 0.0;
	for (k = i; k < j; k++) {
	  x -= data[j*nn+k]*data[k*nn+i];
	}
      }
      data[j*nn+i] = x / data[j*nn+j];
    } // j
  }

  // invert U

  for (i = 0; i < nn; i++) {
    for (j = i; j < nn; j++)  {
      if (i == j) continue;
      double sum = 0.0;
      for (k = i; k < j; k++) {
	sum += data[k*nn+j]*( (i==k) ? 1.0 : data[i*nn+k] );
      }
      data[i*nn+j] = -sum;
    }
  }
  
  // final inversion

  for (i = 0; i < nn; i++) {
    for (j = 0; j < nn; j++)  {
      double sum = 0.0;
      for (k = ((i>j)?i:j); k < nn; k++ ) {
	sum += ((j==k)?1.0:data[j*nn+k])*data[k*nn+i];
      }
      data[j*nn+i] = sum;
    }
  }

}

/**************************************************************************
 * Compute forward fft. We normalize by sqrt(n) in both directions.
 *
 * For the purposes of this work, the public-domain fftw package is
 * used. See www.fftw.org.
 *
 * Any suitable FFT package may be used in place of fftw. For an
 * operational system based on Linux/Intel, it is likely that the
 * Intel Integrated Performance Primitives (Intel IPP) library would
 * be used. Similarly the Athlon library would be used for Athlon
 * hardware
 */

static void _fftFwd(const Complex_t *in, Complex_t *out)
  
{

  int ii;

  fftw_one(_fftPlanFwd, (fftw_complex *) in, (fftw_complex *) out);

  /*
   * adjust by sqrt(n)
   */

  Complex_t *oo = out;
  double corr = sqrt((double) _nSamples);
  for (ii = 0; ii < _nSamples; ii++, oo++) {
    oo->re /= corr;
    oo->im /= corr;
  }

}

/**************************************************************************
 * Compute inverse fft. We normalize by sqrt(n) in both directions.
 *
 * For the purposes of this work, the public-domain fftw package is
 * used. See www.fftw.org.
 *
 * Any suitable FFT package may be used in place of fftw. For an
 * operational system based on Linux/Intel, it is likely that the
 * Intel Integrated Performance Primitives (Intel IPP) library would
 * be used. Similarly the Athlon library would be used for Athlon
 * hardware
 */

static void _fftInv(const Complex_t *in, Complex_t *out)
  
{
  
  int ii;

  fftw_one(_fftPlanInv, (fftw_complex *) in, (fftw_complex *) out);
  
  /*
   * adjust by sqrt(n)
   */
  
  Complex_t *oo = out;
  double corr = sqrt((double) _nSamples);
  for (ii = 0; ii < _nSamples; ii++, oo++) {
    oo->re /= corr;
    oo->im /= corr;
  }

}

/*****************************************************************************
 * Define function to be used for sorting (lowest to highest).
 *
 * This is a standard type of function to be used with qsort().
 * See man qsort.
 */

static int _compareDoubles(const void *v1, const void *v2)

{
  double *d1 = (double *) v1;
  double *d2 = (double *) v2;
  if (*d1 > *d2) {
    return 1;
  } else {
      return -1;
    }
}
