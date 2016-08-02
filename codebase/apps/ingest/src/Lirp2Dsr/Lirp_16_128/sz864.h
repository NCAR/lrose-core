/***************************************************************************
 * sz864.h
 *
 * C implementation of SZ 8/64 algorithm for phase decoding.
 *
 * NCAR, Boulder, Colorado, USA
 *
 * July 2003
 *
 ****************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
  double re;
  double im;
} Complex_t;
#define Complex_hh

/*
 * SZ864 API
 */

/**************************************************************************
 * Initialize module.
 *
 * Call this before any other functions.
 */

extern void szInit();

/**************************************************************************
 * set parameters
 */

extern void szSetWavelength(double wavelength);
extern void szSetNoiseValueDbm(double dbm);
extern void szSetSignalToNoiseRatioThreshold(double db);
extern void szSetSzStrongToWeakPowerRatioThreshold(double db);
extern void szSetSzOutOfTripPowerRatioThreshold(double db);
extern void szSetSzOutOfTripPowerNReplicas(int n);

/**************************************************************************
 * get number of samples
 */

extern int szGetNSamples();

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

extern void szComputeMomentsPp(const Complex_t *IQ,
			       const Complex_t *delta12,
			       double prtSecs,
			       double *power1, double *vel1,
			       double *width1, int *flags1,
			       double *power2, double *vel2,
			       double *width2, int *flags2);

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

extern void szComputeMomentsFft(const Complex_t *IQ,
				const Complex_t *delta12,
				double prtSecs,
				double *power1, double *vel1,
				double *width1, int *flags1,
				double *power2, double *vel2,
				double *width2, int *flags2);

/**************************************************************************
 * Clean up module.
 *
 * Call this when the module is no longer needed.
 */

extern void szCleanUp();

#ifdef __cplusplus
}
#endif

