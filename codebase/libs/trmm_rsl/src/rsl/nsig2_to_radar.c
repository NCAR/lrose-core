/*************************************************************/
/*                                                           */
/*    Function: nsig2_to_radar.c                             */
/*                                                           */
/*    John H. Merritt                                        */
/*    Space Applications Corporation                         */
/*    NASA/GSFC                                              */
/*    TRMM/Code 910.1                                        */
/*                                                           */
/*  Copyright 1996, 1997                                     */
/*************************************************************/

/* The trick here is to reuse as much code from nsig_to_radar.c.
 * To do that, #define NSIG_VER2, separates all version 2 code
 * from version 1 code in nsig_to_radar.c, nsig.c, and nsig.h
 */

#define NSIG_VER2

/* 'static' forces all routines in nsig.c to be static for this code. */
#include "nsig.c"
#include "nsig_to_radar.c"

