/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/**************************************************************************/
/**
 * Structs for handling time series in radar data
 *
 ***************************************************************************/

#ifndef _DS_RADAR_TS_H_
#define _DS_RADAR_TS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>
#include <rapformats/ds_radar.h>
#include <rapformats/ds_radar_calib.h>

/* packet IDs for time series data */

#define TS_INFO_ID 77100  /* ts_pulse_hdr_t */
#define TS_PULSE_ID 77101 /* ts_ops_info_t */

#define RVP8_INFO_ID 77000 /* struct rvp8PulseHdr */
#define RVP8_PULSE_ID 77001 /* struct rvp8PulseInfo */

/* sizes */

#define TS_MAX_CHAN        2  /* Two channels for standard Dual-Pol */
#define TS_RADAR_NAME_LEN 32
#define TS_SITE_NAME_LEN  32
#define TS_GATE_MASK_LEN  512

/* number of gates taken up storing the burst */
 
#define RVP8_NGATES_BURST 2

/* missing val */

#define DS_RADAR_TS_MISSING -9999

/**************************************************************************/
/**
 * \enum ts_xmit_rcv_mode_t
 *
 * Transmit and receive modes.
 *
 ***************************************************************************/

typedef enum {

  /* Single polarization */

  TS_SINGLE_POL = 0, 

  /* Dual pol, alternating transmission, copolar receiver only
   * (CP2 SBand) */

  TS_ALT_HV_CO_ONLY = 1, 

  /* Dual pol, alternating transmission, co-polar and cross-polar
   * receivers (SPOL with Mitch Switch and receiver in 
   * switching mode, CHILL) */

  TS_ALT_HV_CO_CROSS = 2,

  /* Dual pol, alternating transmission, fixed H and V receivers (SPOL
   * with Mitch Switch and receive7rs in fixed mode) */

  TS_ALT_HV_FIXED_HV = 3,
  
  /* Dual pol, simultaneous transmission, fixed H and V receivers (NEXRAD
   * upgrade, SPOL with T and receivers in fixed mode) */

  TS_SIM_HV_FIXED_HV = 4,
  
  /* Dual pol, simultaneous transmission, switching H and V receivers
   * (SPOL with T and receivers in switching mode) */

  TS_SIM_HV_SWITCHED_HV = 5,

  /* Dual pol, H transmission, fixed H and V receivers (CP2 X band) */

  TS_H_ONLY_FIXED_HV = 6,

  /* Dual pol, V transmission, fixed H and V receivers */

  TS_V_ONLY_FIXED_HV = 7,

  TS_XMIT_RCV_MODE_UNKNOWN = 999

} ts_xmit_rcv_mode_t;

/**************************************************************************/
/**
 * \enum ts_scan_mode_t
 *
 * Antenna scanning mode
 *
 ***************************************************************************/

typedef enum {
  TS_SCAN_MODE_UNKNOWN = DS_RADAR_UNKNOWN_MODE,
  TS_SCAN_MODE_CALIBRATION = DS_RADAR_CALIBRATION_MODE,
  TS_SCAN_MODE_SECTOR = DS_RADAR_SECTOR_MODE,
  TS_SCAN_MODE_COPLANE = DS_RADAR_COPLANE_MODE,
  TS_SCAN_MODE_RHI = DS_RADAR_RHI_MODE,
  TS_SCAN_MODE_VERTICAL_POINTING = DS_RADAR_VERTICAL_POINTING_MODE,
  TS_SCAN_MODE_TARGET = DS_RADAR_TARGET_MODE,
  TS_SCAN_MODE_MANUAL = DS_RADAR_MANUAL_MODE,
  TS_SCAN_MODE_IDLE = DS_RADAR_IDLE_MODE,
  TS_SCAN_MODE_SURVEILLANCE = DS_RADAR_SURVEILLANCE_MODE,
  TS_SCAN_MODE_AIRBORNE = DS_RADAR_AIRBORNE_MODE,
  TS_SCAN_MODE_HORIZONTAL = DS_RADAR_HORIZONTAL_MODE,
  TS_SCAN_MODE_SUNSCAN = DS_RADAR_SUNSCAN_MODE,
  TS_SCAN_MODE_POINTING = DS_RADAR_POINTING_MODE,
  TS_SCAN_MODE_FOLLOW_VEHICLE = DS_RADAR_FOLLOW_VEHICLE_MODE,
  TS_SCAN_MODE_EL_SURV = DS_RADAR_EL_SURV_MODE,
  TS_SCAN_MODE_MANPPI = DS_RADAR_MANPPI_MODE,
  TS_SCAN_MODE_MANRHI = DS_RADAR_MANRHI_MODE,
  TS_SCAN_MODE_SUNSCAN_RHI = DS_RADAR_SUNSCAN_RHI_MODE
} ts_scan_mode_t;

/**************************************************************************/
/**
 * \enum ts_prf_mode_t
 *
 * PRF mode - fixed or dual-prt
 *
 ***************************************************************************/

typedef enum {
  TS_PRF_MODE_FIXED = 0, 
  TS_PRF_MODE_STAGGERED_2_3,
  TS_PRF_MODE_STAGGERED_3_4,
  TS_PRF_MODE_STAGGERED_4_5,
  TS_PRF_MODE_UNKNOWN = 99
} ts_prf_mode_t;

/**************************************************************************/
/**
 * \enum ts_iq_format_t
 *
 * Encoding of IQ data
 *
 ***************************************************************************/

typedef enum {
  TS_IQ_ENCODING_FLOAT32 = 0,      /* 4-byte floats, IQ data is in power */
  TS_IQ_ENCODING_SCALED_INT16 = 1, /* scaled 2-byte ints, the IQ data is in dBM.
                                    * float = (10**((int16 * scale) + bias)) / 10.0 */
  TS_IQ_ENCODING_SIGMET_INT16 = 2, /* SIGMET packed int16s representing floats */
  TS_IQ_ENCODING_UNKNOWN = 99
} ts_iq_encoding_t;

/**************************************************************************/
/**
 * \enum ts_xmit_waveform_t
 *
 * Transmitter waveform
 *
 ***************************************************************************/

typedef enum {
  TS_XMIT_PHASE_MODE_FIXED = 0,  /* Klystron */
  TS_XMIT_PHASE_MODE_RANDOM = 1, /* Magnetron */
  TS_XMIT_PHASE_MODE_SZ864 = 2,  /* SAchinanda-Zrnic 8/64 phase encoded */
  TS_XMIT_PHASE_MODE_UNKNOWN = 99
} ts_xmit_phase_mode_t;

/**************************************************************************/
/**
 * \enum  ts_pulse_hdr_t
 *
 * Time-series pulse header
 *
 ***************************************************************************/

typedef struct {

  /** RVP8-specific support */

  ui08 iFlags;      /* Control flags - set to 255 if no RVP8 processor */
  ui08 iAqMode;     /* Sequence numbers for acquisition mode */
  ui08 iPolarBits;  /* State of polarization control bits */
  ui08 iVIQPerBin;  /* (I,Q) vectors/bin, i.e., # of Rx channels */
  ui08 iTgBank;     /* Trigger bank number (one of TBANK_xxx) */
  ui08 spare1[3];

  ui16 iTxPhase;    /* Phase angle of transmit data */
  ui16 iAz;         /* Antenna azimuth * 65535 / 360 */
  ui16 iEl;         /* Antenna elevation * 65535 / 360 */
  si16 iNumVecs;    /* Actual (I,Q) vectors (burst+data) for each Rx, */
  si16 iMaxVecs;    /*   and maximum number (which sets data stride). */
  ui16 iTgWave;     /* Trigger waveform sequence number within a bank */

  ui32 iBtimeAPI;  /* Local time (ms) when pulse arrived in API */
  ui32 iSysTime;   /* IFD system clock time (see rvp8PulseInfo.fSyClkMHz) */
  ui32 iPrevPRT;   /* SysTime ticks to previous/next pulse */
  ui32 iNextPRT;   /* SysTime ticks to previous/next pulse */

  ui32 uiqPerm[2]; /* User specified bits (Permanent) */
  ui32 uiqOnce[2]; /* User specified bits (One-Shot) */

  si32 iDataOff[TS_MAX_CHAN]; /* Offset of start of this pulse in fIQ[] */
  fl32 fBurstMag[TS_MAX_CHAN];/* Burst pulse magnitude (amplitude, not power) */
  ui16 iBurstArg[TS_MAX_CHAN]; /* Burst phase changes (PrevPulse - ThisPulse) */
  ui16 iWrapIQ[TS_MAX_CHAN];  /* Data wraparound count for validity check */

  ui32 spare2[4];

} _rvp8_pulse_hdr_t;

typedef struct {

  si32 version;      /* version number of this struct format */
  si32 radarId;      /* to match ids between ops info and pulse headers */

  ui32 pulseSeqNum;  /* pulse sequence number */

  ui32 timeSecsUTC;  /* UTC time in secs since Jan 1 1970 */
  ui32 timeNanoSecs; /* partial secs - nanosecs */

  fl32 elevation;    /* antenna elevation */
  fl32 azimuth;      /* antenna azimuth */
  
  fl32 prt;          /* time since previous pulse, secs */
  fl32 prtNext;      /* time to next pulse, secs, if available */
  
  fl32 pulseWidth;   /* nanosecs */

  ui16 nGates;  /* number of gates of IQ data */
  si16 tiltNum; /* scan strategy tilt num - if available, otherwise set to -1 */ 
  si32 volNum;  /* scan strategy vol  num - if available, otherwise set to -1 */ 

  ui08 nChannels;  /* number of channels in the IQ data */
  ui08 iqEncoding; /* ts_iq_encoding_t */
  ui08 hvFlag;     /* 1 = H transmit, 0 = V transmit */

  ui08 antennaTransition; /* antenna is in transition, 1 = TRUE, 0 = FALSE */

  ui08 phaseCohered;      /* received phases are cohered to burst phase
                           * 1 = TRUE, 0 = FALSE, applies to
                           * TS_XMIT_PHASE_MODE_RANDOM and TS_XMIT_PHASE_MODE_SZ864 */
  
  ui08 spare1[3];
  
  ui32 status; /* general status flag - optional use */

  ui32 nData; /* number of data values in the IQ array
               * nData = nChannels + (nGates + nGatesBurst) * 2 */

  ui16 iqOffset[TS_MAX_CHAN];  /* Index of gate 0 for this channel, in IQ[] array */
  fl32 burstMag[TS_MAX_CHAN];  /* Burst pulse magnitude (amplitude, not power) */
  fl32 burstArg[TS_MAX_CHAN];  /* Burst phase phase (deg) */
  fl32 burstArgDiff[TS_MAX_CHAN]; /* Burst phase changes (PrevPulse - ThisPulse) deg */

  fl32 measXmitPowerDbmH; /* measured H power in dBm */
  fl32 measXmitPowerDbmV; /* measured V power in dBm */

  fl32 scale; /* for use with TS_IQ_ENCODING_SCALED_INT16
               *         and  TS_IQ_ENCODING_FLOAT32 */
  fl32 bias;  /* for use with TS_IQ_ENCODING_SCALED_INT16 */

  ui32 nGatesBurst; /* number of gates at start of data array
                     * holding burst IQ information - the RVP8 stores
                     * burst data in the first RVP8_NGATES_BURST gates.
                     * For other systems this should normally be 0 */

  si32 scanMode;  /* ts_scan_mode_t */
  fl32 targetEl;  /* target elevation for ppis */
  fl32 targetAz;  /* target azimuth for rhis */

  ui32 spare2[16];

  /** RVP8-specific support */
  
  _rvp8_pulse_hdr_t rvp8;

} ts_pulse_hdr_t;

/**************************************************************************/
/**
 *
 * \enum  ts_ops_info_t
 *
 * Operations information
 * Sent every so often, or if something changes
 *
 ***************************************************************************/

/* RVP8-specific support */

typedef struct {
  
  ui32 iVersion; /* Version of this public structure
                  *  0: Initial public element layout
                  *  1: Added fDBzCalib, iSampleSize, iMeanAngleSync, 
                  *     iFlags, iPlaybackVersion
                  * Set to 9999 if no RVP8 processor */

  ui32 iMajorMode;    /* RVP8 major mode, one of PMODE_xxx */  
  ui32 iPolarization; /* Polarization selection, one of POL_xxx */
  ui32 iPhaseModSeq;  /* Tx phase modulation, one of PHSEQ_xxx */

  ui16 iTaskSweep;    /* Sweep number within a task */
  ui16 iTaskAuxNum;   /* Auxiliary sequence number */
  ui32 iTaskScanType; /* Scan geometry, one of SCAN_xxx */
  ui32 spare1[3];

  char sTaskName[32];  /* Name of the task (NULL terminated) */

  char sSiteName[32];  /* Data are from this site
                        * (NULL terminated) */

  ui32 iAqMode;     /* Acquisition mode (filled in by local API) */
  ui32 iUnfoldMode; /* Dual-PRF unfolding  0:None, 1:2/3, 2:3/4, 3:4/5 */

  ui32 iPWidthCode;  /* Pulse width selection (0 to RVP8NPWIDS-1), */
  fl32 fPWidthUSec; /*   and actual width in microseconds */

  fl32 fDBzCalib;       /* Calibration reflectivity (dBZ at 1 km) */
  si32 iSampleSize;      /* Number of pulses per ray */
  ui32 iMeanAngleSync;  /* Mean angle sync ray spacing */

  ui32 iFlags; /* Various bit flags */

  si32 iPlaybackVersion; /* Allows playback tagging, zeroed initially */

  fl32 fSyClkMHz;      /* IFD system (XTAL) clock frequency (MHz) */
  fl32 fWavelengthCM;  /* Radar wavelength (cm) */
  fl32 fSaturationDBM; /* Power (dBm) corresponding to MAG(I,Q) of 1.0 */

  fl32 fRangeMaskRes;   /* Spacing between range bins (meters) */
  ui16 iRangeMask[TS_GATE_MASK_LEN]; /* Bit mask of bins that have
                                      * actually been selected at the
                                      * above spacing. */

  fl32 fNoiseDBm[TS_MAX_CHAN];    /* Noise level in dBm, and standard */
  fl32 fNoiseStdvDB[TS_MAX_CHAN]; /*   deviation of measurement in dB. */
  fl32 fNoiseRangeKM;		/* Range (km) and PRF (Hz) at which the */
  fl32 fNoisePRFHz;		/*   last noise sample was taken. */

  ui16 iGparmLatchSts[2];	/* Copies of latched and immediate status */
  ui16 iGparmImmedSts[6];	/*   words from the GPARM structure. */
  ui16 iGparmDiagBits[4];	/* Copies of GPARM diagnostic bits */

  char sVersionString[12];	/* IRIS/RDA version (full Major/Minor string) */
				/*   that created the data. */

  fl32 spare2[4];

} _rvp8_ops_info_t;
  
typedef struct {

  si32 version;      /* version number of this struct format */
  si32 radarId;      /* to match ids between ops info and pulse headers */

  /* location */

  fl32 altitudeM;    /* altitude in meters */
  fl32 latitudeDeg;  /* degrees */
  fl32 longitudeDeg; /* degrees */

  /* gate geometry */

  fl32 startRangeM;  /* range to center of first gate - meters */
  fl32 gateSpacingM; /* spacing between gates - meters */

  /* operational modes */

  si32 scanMode;          /* ts_scan_mode_t */
  si32 xmitRcvMode;       /* ts_xmit_rcv_mode_t */
  si32 prfMode;           /* ts_prf_mode_t */
  si32 xmitPhaseMode;     /* ts_xmit_phase_mode_t */

  /* radar parameters */

  fl32 wavelengthCm; /* radar wavelength in cm */
  fl32 beamWidthDegH; /* beam width H in degrees */
  fl32 beamWidthDegV; /* beam width V in degrees */

  /* scan angles */

  fl32 targetEl;  /* target elevation for ppis */
  fl32 targetAz;  /* target azimuth for rhis */
  fl32 sectorAz1; /* az 1 for sector ppis */
  fl32 sectorAz2; /* az 2 for sector ppis */
  fl32 rhiEl1;    /* el 1 for rhis */
  fl32 rhiEl2;    /* el 2 for rhis */

  fl32 spare1[40];
  
  /* radar and site name */
  
  char radarName[TS_RADAR_NAME_LEN]; /**< UTF-8 encoded radar name */
  char siteName[TS_SITE_NAME_LEN]; /**< UTF-8 encoded radar name */

  /* calibration */

  ds_radar_calib_t calib;
  
  /* RVP8-specific support */

  _rvp8_ops_info_t rvp8;

} ts_ops_info_t;

extern void ts_pulse_hdr_init(ts_pulse_hdr_t *pulse);
extern void ts_ops_info_init(ts_ops_info_t *info);
extern void ts_pulse_hdr_print(FILE *out, const char *spacer,
                               ts_pulse_hdr_t *pulse);
extern void ts_ops_info_print(FILE *out, const char *spacer,
                              ts_ops_info_t *info);

#ifdef __cplusplus
}
#endif

#endif
