// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/* 	$Id: IndexFields.hh,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */

# ifndef INDEXFIELDS_HH
# define INDEXFIELDS_HH

# define     VR_NDX 1
# define    DBM_NDX 2
# define    DBZ_NDX 3
# define   CDBZ_NDX 4
# define    NCP_NDX 5
# define     SW_NDX 6
# define    ZDR_NDX 7
# define  PHIDP_NDX 8
# define  RHOHV_NDX 9
# define    KDP_NDX 10
# define    LDR_NDX 11
# define   NKDP_NDX 12
# define   CPHI_NDX 13
# define   NPHI_NDX 14
# define  NSDEV_NDX 15
# define  RSDEV_NDX 16
// Kdp rainfall rate
# define  KRATE_NDX 17
# define     VT_NDX 18
# define   DBMV_NDX 19
// ZR rainfall rate
# define  ZRATE_NDX 20
// Zdr rainfall rate
# define  DRATE_NDX 21
// Hdr (Hail Detection)
# define    HDR_NDX 22
// NSSL Kdp rainfall rate
# define  NRATE_NDX 23
# define   MKDP_NDX 24
# define   CKDP_NDX 25
// CSU Kdp rainfall rate
# define  CRATE_NDX 26

// kdp rainfall accumulation
# define    KAC_NDX 27
// zr rainfall accumulation
# define    ZAC_NDX 28
// zdr rainfall accumulation
# define    DAC_NDX 29
// hybrid rainfall accumulation
# define    HAC_NDX 30
// cross pole power
# define   DX_NDX 31
// particle type
# define   PD_NDX 32

// mystery fields
# define RH_NDX 33
# define AH_NDX 34
# define RV_NDX 35
# define AV_NDX 36

# define    DBZV_NDX 37
# define      ST_NDX 38
# define      AZ_NDX 39
# define      EL_NDX 40

# define  IQ_NORM_NDX 41
# define   IQ_ANG_NDX 42
# define     LDRV_NDX 43
# define   N_NDX      44
# define  DELTA_N_NDX 45

// drop size distribution stuff
# define   N0_NDX 46
# define   MU_NDX 47
# define  LAM_NDX 48
# define   D0_NDX 49
# define   RR_NDX 50
# define  DKD_NDX 51 
# define  RES_NDX 52
# define   NT_NDX 53
# define  RNX_NDX 54
# define  RZD_NDX 55
# define  RKD_NDX 56
# define  LWC_NDX 57

# define MAX_FIELD_INDEX 64

# endif  /* INDEXFIELDS_HH */










