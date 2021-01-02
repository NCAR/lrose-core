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
#ifdef __cplusplus
 extern "C" {
#endif

   /************************************************
    * gmap.h
    */
   
   /*************************************************
    * gmap()
    *
    * Pass in unfiltered magnitude spectrum, clutter_width and noise power.
    * Filtered array is filled in.
    * Returns power removed.
    */
   
   extern double gmap(const double *unfiltered,
		      int n_samples,
		      double clutter_width,
		      double noise_power,
		      double *filtered);
   
   /*
    * typedefs for IPP routines
    */
   
   typedef float Ipp32f;
   typedef unsigned char Ipp8u;
   
   typedef struct {
     Ipp32f  re;
     Ipp32f  im;
   } Ipp32fc;
   
   typedef struct {
     int n_samples;
   } IppsDFTSpec_C_32fc;

#define IPP_FFT_DIV_FWD_BY_N 1
#define ippAlgHintFast 2

   /*
    * sigmet defines
    */

#define NINT(fvalue) ((SINT4)floor(0.5+(double)(fvalue)))
#define SQUARE(value) ((value)*(value))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define FPI   (3.14159265358979)
#define SQRT2PI (2.506628274)
#define TRUE 1
#define FALSE 0

#define POISSONNOISERANK( TERM, SIZE ) \
  ( -log( (((FLT4)((SIZE) - (TERM))) - 0.5) / (FLT4)(SIZE) ) )

#define RVP8MAXPULSES 1024
#define SIGNED signed

#define WIN_RECT          0	/* Window codes, used elsewhere */
#define WIN_HAMMING       1	/*   within IRIS as well. */
#define WIN_BLACKMAN      2
#define WIN_BLKMANEX      3
#define WIN_VONHANN       4
#define WIN_ADAPTIVE      5

#define SSGSM_FILL_NEARBY  0x0001 /* Fill likely points if near others */
#define SSGSM_FILL_GAPS    0x0002 /* Fill single-point gaps */
#define SSGSM_DESPECKLE    0x0004 /* Kill single-point speckles */

typedef   SIGNED char  SINT1;
typedef unsigned char  UINT1;
typedef   SIGNED short SINT2;
typedef unsigned short UINT2;
typedef   SIGNED long  SINT4;
typedef unsigned long  UINT4;
typedef float           FLT4;
typedef double          FLT8;

typedef unsigned short  BIN2;
typedef unsigned long   BIN4;

   struct rvp8DftConf { /* Specifications for computing DFTs */
     SINT4 iSpecSize ;  /* Actual spectrum size to use */
     SINT4 iWinType ;   /* Window selection, one of WIN_xxx */
     FLT4  fReNormExp ; /* Renormalized spectrum weighting exponent */
     IppsDFTSpec_C_32fc *pDFTSpec ; /* Precomputations for FFT/DFT */
     Ipp8u *pDFTWork ;	       /* Dedicated working buffer */
     Ipp32fc window[RVP8MAXPULSES] ;  /* Precomputed window */
     FLT4 fCorDots[4][RVP8MAXPULSES] ; /* Dot product terms for autocorels */
     UINT4 iMagic ;		  /* Magic number to indicate validity */
   } ;

#define SPFILTMAXPTS 30

struct winGaussClutterModel {
  FLT4 fClutterWidth ; SINT4 iWinType, iSpecSize, iMainLobePts ;
  FLT4 fModelPwr[ SPFILTMAXPTS ] ;
} ;

struct specSort {		/* Element of rank ordered spectrum */
  FLT4  fdBVal ;		/*   Spectral value in dB */
  SINT2 iIndex ;		/*   Location in original spectrum */
  char  pad6x2[2] ;
} ;

extern FLT4 fSpecFilterGMAP
( FLT4 *fSpecLin_a, FLT4 fClutterWidth_a, FLT4 fNoisePower_a,
  const struct rvp8DftConf *dftConf_a, struct winGaussClutterModel *WGCM_a );

extern void specSortFromDFT
( struct specSort *specSort_a, const Ipp32fc *pDFT_a, SINT4 iSpecSize_a );

extern void specSortFromSpecLog
( struct specSort *specSort_a, const FLT4 *fSpecLog_a, SINT4 iSpecSize_a );

extern SINT4 iSpecSortNoiseAnalysis
( const struct specSort *specSort_a, SINT4 iSpecSize_a, FLT4 *fNoiseTerm_a );

extern void specSortGenSignalMask
( const struct specSort *specSort_a, SINT4 iSpecSize_a, SINT4 iSigIndex_a,
  UINT1 *lSigMask_a, UINT4 iFlags_a );

extern int specSortCompare( const void *arg1_a, const void *arg2_a );

extern void specLogFromSpecLin
( FLT4 *fSpecLog_a, const FLT4 *fSpecLin_a, SINT4 iSpecSize_a );

extern void specLinFromSpecLog
( FLT4 *fSpecLin_a, const FLT4 *fSpecLog_a, SINT4 iSpecSize_a );

extern void initWinGaussClutterModel
( struct winGaussClutterModel *model_a, FLT4 fClutterWidth_a,
  const struct rvp8DftConf *dftConf_a );

#ifdef __cplusplus
}
#endif

