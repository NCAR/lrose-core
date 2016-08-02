/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            Mike Kolander
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBTSDISTK
/******************************************************************

	 Writes one VOS from a RSL radar structure into one 1B-51/1C-51 HDF
	 file.

	 A 1B-51/1C-51 HDF file contains multiple VOS's recorded by a radar
	 site during a 1-hour time period, and the HDF file is named using
	 the date/hour of the constituent VOS's.

	 Functions defined herein perform tasks preparatory to building a
	 TSDIS toolkit 'L1B_1C_GV' structure, the contents of which are
	 written into the HDF file by the TSDIS toolkit. Construction of the
	 toolkit 'L1B_1C_GV' structure is done via the subroutines defined
	 in RSL file 'radar_to_hdf_2.c'.

  -----------------------------------------------------------------
	 Libraries required for execution of this code :
      -ltsdistk                    : TSDIS toolkit
      -lmfhdf -ldf -ljpeg -lz      : HDF
      -lrsl                        : rsl
      -lm                          : C math

  -----------------------------------------------------------------
*******************************************************************/

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

/* TSDIS toolkit function and structure definitions. */
#include "IO.h"
#include "IO_GV.h"
/* RSL function and structure definitions. */
#include <trmm_rsl/rsl.h>
/* Parameter definitions for 1B-51 and 1C-51 HDF
	 file handling applications using the TSDIS toolkit. */
#include "toolkit_1BC-51_appl.h"

/*************************************************************/
/*                                                           */
/*                Function Prototypes                        */
/*                                                           */
/*************************************************************/
void RSL_set_tkMetaDataString(char *string, int param);
void RSL_set_hdf_qc_parameters(float *inParm);
void radarVolumesSave(Volume *v[MAX_RADAR_VOLUMES], Radar *radar);
void radarVolumesRestore(Radar *radar, Volume *v[MAX_RADAR_VOLUMES]);
int nextVolume(Radar *radar, int last_volume);
Volume *maskBuild(Volume *cv, Volume *ucv);
void metaDataWrite(IO_HANDLE *granuleHandle, Radar *radar,
									 char *hdfFileName, int fileAccessMode);
int tkVosDimensions(VosSize *vs, Radar *radar);
Ray *first_ray_in_sweep(Sweep *sweep);
int rslVosDimensions(VosSize *vs, Radar *radar, float maxRange);
int L1GVtemplateInit(VosSize *vs, Radar *radar, char *hdfFileName, 
										 float maxRange);
static int hdfFileOpen(IO_HANDLE *granuleHandle, VosSize *vs, 
											 char *hdfFileName, Radar *radar);
int radarPrep1B51(Radar *radar);
int radarPrep1C51(Radar *radar);
int nullGranuleCreate(char *hdfFileName, IO_HANDLE *granuleHandle,
											Radar *radar);
int RSL_radar_to_hdf(Radar *radar, char *hdfFileName);

extern L1B_1C_GV *gvl1Build(Radar *radar, float *qcParm, VosSize *vs,
														int productID);
extern int radar_verbose_flag;

/* The 1st non-NULL ray in each volume is widely used. Hence global. */
Ray *first_ray_in_volume[MAX_RADAR_VOLUMES];



/*************************************************************/
/*                                                           */
/*                  RSL_set_tkMetaDataString                 */
/*                                                           */
/*************************************************************/
static struct
{
  char GenInputDate[128];
  char AlgorithmVersion[128];
  int ProductVersion;
  char SoftwareVersion[128];
} tkMetaDataString;

void RSL_set_tkMetaDataString(char *string, int param)
{
/* Allows the application 'level_1' to store metadata strings into
   the tkMetaDataString buffers for later insertion by RSL function 
   'metaDataWrite()' into the HDF file. 
   Call **before** RSL_radar_to_hdf().
*/

#define CP_TKMETA(s, str) \
	memset(tkMetaDataString.s, '\0', sizeof(tkMetaDataString.s));\
	strncpy(tkMetaDataString.s, str, sizeof(tkMetaDataString.s)-1)


  if (param == TK_GEN_DATE_INPUT_FILES) {
	CP_TKMETA(GenInputDate, string);
  /* 1C-51 kludge...
	 To decide whether or not to write a VOS into a 1C-51 HDF file
	 (See 'hdf1C51Create()' in file 'level_1.c'), I need to know
	 something about the times of the VOSs already in the file;
	 ie, if no satellite overpass, the file is to contain just one
	 VOS from each half-hour. The only efficient way to do this is
	 by encoding time info into a metadata field.

	 The 'GenInputDate' metaData string contains info indicating
	 which hourly time slots are filled by the VOSs contained in the
	 HDF file. (See function 'metaDataWrite()' in this file for details.)

	 Called by 'hdfFilePeek()' in file 'level_1.c'.
  */  
  }
  else if (param == TK_ALGORITHM_VERSION) {
	CP_TKMETA(AlgorithmVersion, string);

  } else if (param == TK_SOFTWARE_VERSION) {
	CP_TKMETA(SoftwareVersion, string);

	if (radar_verbose_flag)
	  fprintf(stderr, "TK_SOFTWARE_VERSION = <%s>\n", string);

  } else if (param == TK_PRODUCT_VERSION) {
	sscanf(string, "%d", &tkMetaDataString.ProductVersion);


  } else {
	fprintf(stderr, "RSL_set_tkMetaDataString: Unknown param==%d!!\n", param);
  }
}

/*************************************************************/
/*                                                           */
/*                 RSL_set_hdf_qc_parameters                 */
/*                                                           */
/*************************************************************/
static float qcParm[NUMBER_QC_PARAMS] =
{
	 NOVAL_FLOAT, NOVAL_FLOAT, NOVAL_FLOAT, NOVAL_FLOAT, NOVAL_FLOAT,
	 NOVAL_FLOAT, NOVAL_FLOAT, NOVAL_FLOAT, NOVAL_FLOAT, NOVAL_FLOAT
};

void RSL_set_hdf_qc_parameters(float *inParm)
{
/* Stores 1C-51 QC parameters for later insertion into 1C-51 HDF
	 file. Call **before** RSL_radar_to_hdf().
*/
	int j;
	
	for (j=0; j<NUMBER_QC_PARAMS; j++)
	  qcParm[j] = inParm[j];
}

/*************************************************************/
/*                                                           */
/*                     radarVolumesSave                      */
/*                                                           */
/*************************************************************/
void radarVolumesSave(Volume *v[MAX_RADAR_VOLUMES], Radar *radar)
{
/* Save array of radar volume pointers. */
	int j;
	
	for (j=0; j<radar->h.nvolumes; j++)
	  v[j] = radar->v[j];
}
		
/*************************************************************/
/*                                                           */
/*                      radarVolumesRestore                  */
/*                                                           */
/*************************************************************/
void radarVolumesRestore(Radar *radar, Volume *v[MAX_RADAR_VOLUMES])
{
/* Restore array of radar volume pointers. */	
	int j;

	for (j=0; j<radar->h.nvolumes; j++)
	  radar->v[j] = v[j];
}

/*************************************************************/
/*                                                           */
/*                      nextVolume                           */
/*                                                           */
/*************************************************************/
int nextVolume(Radar *radar, int last_volume)
/* Find the index of the next volume in the radar structure, given
	 the index of the last volume found. 
	 Returns:
	   index of next volume, if success.
		 -1 if failure.
*/
{
	int j;
	
	for (j=last_volume+1; j<radar->h.nvolumes; j++)
		if (radar->v[j] != NULL)
		  return(j);       /* Found volume. Return the index. */

	return(-1);   /* No volume found. Return bogus index. */
}

/*************************************************************/
/*                                                           */
/*                          maskBuild                        */
/*                                                           */
/*************************************************************/
Volume *maskBuild(Volume *cv, Volume *ucv)
{
/* Create a mask volume. A mask value, in conjunction with a corresponding 
	 corrected reflectivity value, enables the future recovery of a 
	 uncorrected value from a 1C-51 HDF file, since the original, uncorrected
	 reflectivity values are not stored in 1C-51 HDF files.

	 This function used for both reflectivity 'DZ' and differential reflectivity
	 'ZD'.

	 cv: corrected volume pointer.    (CZ or CD)
	 ucv: uncorrected volume pointer. (DZ or ZD)
	 mv: mask volume pointer.         (MZ or MD)
*/
	Volume *mv;
	int sindex, rindex, bindex;

	
  mv = RSL_copy_volume(cv);
	mv->h.f = MZ_F;         /* MZ_F identical to MD_F. Can use either. */
	mv->h.invf = MZ_INVF;   /* MZ_INVF identical to MD_INVF. Can use either. */
	for (sindex=0; sindex<cv->h.nsweeps; sindex++)
	{
		if (cv->sweep[sindex] == NULL) continue;
		mv->sweep[sindex]->h.f = MZ_F;
		mv->sweep[sindex]->h.invf = MZ_INVF;
	  for (rindex=0; rindex<cv->sweep[sindex]->h.nrays; rindex++)
		{
			if (cv->sweep[sindex]->ray[rindex] == NULL) continue;
			mv->sweep[sindex]->ray[rindex]->h.f = MZ_F;
			mv->sweep[sindex]->ray[rindex]->h.invf = MZ_INVF;
	    for (bindex=0; bindex<cv->sweep[sindex]->ray[rindex]->h.nbins; bindex++)
			  /* Has the Z_value been corrected by the QC algorithm? */

	      if (cv->sweep[sindex]->ray[rindex]->range[bindex] == cv->h.invf(BADVAL))
		mv->sweep[sindex]->ray[rindex]->range[bindex] = 1;
	      else  /* Uncorrected Z_value. */
		mv->sweep[sindex]->ray[rindex]->range[bindex] = 0;
		}  /* end for (rindex=0; ... */
	}  /* end for (sindex=0; ... */
		 
	return(mv);
}

/*************************************************************/
/*                                                           */
/*                         metaDataWrite                     */
/*                                                           */
/*************************************************************/
void metaDataWrite(IO_HANDLE *granuleHandle, Radar *radar,
									 char *hdfFileName, int fileAccessMode)
{
/* Write out some metadata values into toolkit structures. */
  /*
   * YOU REALLY DON'T KNOW, A PRIORI, WHAT DATATYPE THE METADATA ITEM IS.
   * TRIAL and ERROR is how we figured some of them out.
   *
   * THEY SHOULD ALL BE STRINGS!!!!!
   */
	char buf[1024];
	int intVal, vindex;
	float floatVal;
	DATE_STR tkdate;
	TIME_STR tktime;

	vindex = nextVolume(radar, -1);  /* Find 1st non-NULL rsl volume. */
	
	/* ----------------- Core MetaData ---------------*/
	/* Begin/end date of the HDF granule. */
	tkdate.tkyear = (short) radar->h.year;
	tkdate.tkmonth = (short) radar->h.month;
	tkdate.tkday = (short) radar->h.day;
	TKwriteMetadataInt(granuleHandle, TK_BEGIN_DATE, &tkdate);
	TKwriteMetadataInt(granuleHandle, TK_END_DATE, &tkdate);
	/* Begin time of the HDF granule. */
	tktime.tkhour = (int8) radar->h.hour;
	tktime.tkminute = (int8) 0;
	tktime.tksecond = (int8) 0;
	TKwriteMetadataInt(granuleHandle, TK_BEGIN_TIME, &tktime);
	/* End time of the HDF granule. */
	tktime.tkhour = (int8) radar->h.hour;
	tktime.tkminute = (int8) 59;
	tktime.tksecond = (int8) 59;
	TKwriteMetadataInt(granuleHandle, TK_END_TIME, &tktime);

	/* Longitude & Bounding Coordinates */
	floatVal = radar->h.lond + radar->h.lonm/60.0 + radar->h.lons/3600.0;
	TKwriteMetadataFloat(granuleHandle, TK_CENTER_POINT_LON, &floatVal);
	floatVal = floatVal - 2.0;
	TKwriteMetadataFloat(granuleHandle, TK_WEST_BOUND_COORD, &floatVal);
	floatVal = floatVal + 4.0;
	TKwriteMetadataFloat(granuleHandle, TK_EAST_BOUND_COORD, &floatVal);
	/* Latitude & Bounding Coordinates */
	floatVal = radar->h.latd + radar->h.latm/60.0 + radar->h.lats/3600.0;
	TKwriteMetadataFloat(granuleHandle, TK_CENTER_POINT_LAT, &floatVal);
	floatVal = floatVal - 2.0;
	TKwriteMetadataFloat(granuleHandle, TK_SOUTH_BOUND_COORD, &floatVal);
	floatVal = floatVal + 4.0;
	TKwriteMetadataFloat(granuleHandle, TK_NORTH_BOUND_COORD, &floatVal);

	TKwriteMetadataChar(granuleHandle, TK_CONTACT, "Danny Rosenfeld");



	/* ----------------- PS MetaData ---------------*/
	TKwriteMetadataChar(granuleHandle, TK_ALGORITHM_VERSION,
						tkMetaDataString.AlgorithmVersion);
	TKwriteMetadataInt(granuleHandle, TK_PRODUCT_VERSION,
						&tkMetaDataString.ProductVersion);
	TKwriteMetadataChar(granuleHandle, TK_SOFTWARE_VERSION,
						tkMetaDataString.SoftwareVersion);
	TKwriteMetadataChar(granuleHandle, TK_MAX_VALID_CHANNEL, "70 dBz");
	TKwriteMetadataChar(granuleHandle, TK_MIN_VALID_CHANNEL, "-20 dBz");
	if (radar->h.nvolumes > 0)
	{
	  floatVal = first_ray_in_volume[vindex]->h.wavelength;
	}
	else  /* No radar volumes. Creating empty granule. */
	{
		TKwriteMetadataChar(granuleHandle, TK_ANOMALY_FLAG, "EMPTY: REASON UNKNOWN");
	  floatVal = 0.0;
	}
	TKwriteMetadataFloat(granuleHandle, TK_RADAR_WAVELENGTH, &floatVal);

	floatVal = -20.0;  /* MIN_REFL_THRESHOLD */
	TKwriteMetadataFloat(granuleHandle, TK_MIN_REF_THRESHOLD, &floatVal);
	TKwriteMetadataChar(granuleHandle, TK_RADAR_NAME, radar->h.radar_name);
	TKwriteMetadataChar(granuleHandle, TK_RADAR_CITY, radar->h.city);
	TKwriteMetadataChar(granuleHandle, TK_RADAR_STATE, radar->h.state);
	TKwriteMetadataChar(granuleHandle, TK_RADAR_COUNTRY, radar->h.country);
	
	intVal = (int)TKgetNvos(granuleHandle);
	TKwriteMetadataInt(granuleHandle, TK_NUM_VOS, &intVal);
	TKwriteMetadataFloat(granuleHandle, TK_GV_DZCAL, &qcParm[ZCAL]);
	floatVal = X;  /* Mask scale factor */
	TKwriteMetadataFloat(granuleHandle, TK_GV_L1C_SCALE, &floatVal);
	floatVal = 0.0; /* Correction for gaseous two-way attenuation */
	TKwriteMetadataFloat(granuleHandle, TK_GV_ALPHA, &floatVal);

	TKwriteMetadataChar(granuleHandle, TK_INPUT_FILES, "8mm tape files");
	TKwriteMetadataChar(granuleHandle, TK_DATA_CENTER_SRC, radar->h.radar_name);

	/* I really, really hate this 1C-51 kludge, but...
		 To decide (within function 'hdf1C51Create') whether or not to write
		 a VOS into a 1C-51 file, I need to know something about the times
		 of the VOSs already in the file; ie, if no satellite overpass, the
		 file is to contain just one VOS from each half-hour.
		 The only efficient way to do this is by encoding info into a metadata
		 field.

		 So... I use the 'GEN_DATE_INPUT_FILES' metaData field. The intial
		 value of this field is the string 'unKNOWN'. When a VOS from the
		 first half_of_the_hour is written into the file, change the first
		 char 'u' to upper_case. When a VOS from the second half_of_the_hour
		 is written into the file, change the second char 'n' to upper_case.
		 The final string, after both time slots are filled, is entirely
		 upper_case: 'UNKOWN'.
	*/
	if (granuleHandle->productID == TK_L1C_GV)  /* 1C-51 file? */
	{
		/* If new file, then intitialize 'GEN_DATE_INPUT_FILES' string.
		   If file exists, then get the existing string from the file. */
		if (fileAccessMode == TK_NEW_FILE) strcpy(buf, "unKNOWN");
		else strcpy(buf, tkMetaDataString.GenInputDate);
		if (radar->h.nvolumes > 0)  /* Don't mess with an empty granule. */
		{
			if (radar->h.minute < 30)  /* 1st half_of_the_hour? */
	      buf[0] = (char) toupper((int)buf[0]);  /* Convert char to upper_case */
			else  /* 2nd half_of_the_hour */
	      buf[1] = (char) toupper((int)buf[1]);  /* Convert char to upper_case */
		}
	}
	else  /* 1B-51 file */
	{
		strcpy(buf, "UNKNOWN");
	}
	TKwriteMetadataChar(granuleHandle, TK_GEN_DATE_INPUT_FILES, buf);
}

/*************************************************************/
/*                                                           */
/*                         newPhysicalSweep                  */
/*                                                           */
/*************************************************************/
int newPhysicalSweep(Sweep *phys_sweep, Sweep *sweep)
{
	/* Checks if the rsl 'sweep' belongs to a different physical sweep
		 than 'phys_sweep'.
		 Returns:  1, if sweep is a new physical sweep
		           0, if not new physical sweep
				  -1, if error.
	*/
  int nrays, iray;

  /* If elevations don't match, new sweep. */
  if (phys_sweep->h.elev != sweep->h.elev) return(1);

  /* Check the first 50 rays of both sweeps.
   * If azimuths don't match, new sweep.
   */
  nrays = 50;
  if (sweep->h.nrays < nrays) nrays = sweep->h.nrays;
  if (phys_sweep->h.nrays < nrays) nrays = phys_sweep->h.nrays;
  for (iray=0; iray<nrays; iray++)
  {
	if (sweep->ray[iray] == NULL) continue;
	if (phys_sweep->ray[iray] == NULL) continue;
	if (phys_sweep->ray[iray]->h.azimuth != sweep->ray[iray]->h.azimuth)
	  return(1);  /* New physical sweep */
  }

  /* No new physical sweep. */
  return(0);
}

/*************************************************************/
/*                                                           */
/*                       tkVosDimensions                     */
/*                                                           */
/*************************************************************/
int tkVosDimensions(VosSize *vs, Radar *radar)
{
	/* Logically reconfigures the VOS in the rsl radar structure into
		 a sequence of physical sweeps as required for the toolkit
		 L1GV structure.

		 Records required toolkit gvl1 dimensions in the 'VosSize->tk'
		 structure. When we later load the toolkit gvl1 structure with
		 actual data values, vs->tk.ncell[tk_sindex][pindex] will be zero
		 for those data types not collected by the radar during physical
		 sweep 'tk_sindex'.

		 The algorithm used herein to locate physical sweeps is as simple
		 as possible. It works for all VOSs processed to date. However,
		 if more complicated radar scanning regimes turn up, this function
		 will surely require an overhaul and additional complexity.

		 Algorithm: Within the rsl radar structure, walks horizontally
		 across the radar volumes at one sweep level, checking for elev
		 and azim values different from the last physical sweep located.
		 Then increments the sweep level and repeats at the next rsl sweep
		 level.

		 Returns: OK, if success.
		          <0, if error.
  */
	int pindex, sindex, tk_sindex, status;
	Sweep *sweep;
	
	/* Note: 'vs->tk' structure contains all zeroes upon entry into this 
		 function. */
	sindex = 0;      /* rsl sweep index */
	tk_sindex = -1;  /* toolkit sweep index */
	while (sindex < vs->rsl.maxNsweep) /* for each rsl sweep... */
	{
		for (pindex=0; pindex<vs->tk.nparm; pindex++) /* for each rsl volume */
		{
			if (sindex >= vs->rsl.nsweep[pindex]) continue;
			sweep = vs->rsl.v[pindex]->sweep[sindex];
			/* No null sweep ptrs allowed in rsl ptr array. */
			if (sweep == NULL) return(QUIT);
			/* Check for a new physical sweep. */
			if (tk_sindex == -1)
			{
				tk_sindex++;
				vs->tk.nray[tk_sindex] = vs->rsl.nray[pindex][sindex];
				vs->rsl.sweep[tk_sindex] = sweep;
			}
			else if ((status=newPhysicalSweep(vs->rsl.sweep[tk_sindex], sweep)))
			{
				if (status < 0) return(status);
				tk_sindex++;
				if (tk_sindex >= MAX_SWEEP)
				{
					if (radar_verbose_flag)
					  fprintf(stderr, "tkVosDimensions(): Too many toolkit sweeps.\n");
					return(QUIT);
				}
				vs->tk.nray[tk_sindex] = vs->rsl.nray[pindex][sindex];
				vs->rsl.sweep[tk_sindex] = sweep;
			} /* end if (newPhysicalSweep */

			vs->tk.ncell[tk_sindex][pindex] = vs->rsl.ncell[pindex][sindex];
		} /* end for (pindex=0;... */

		sindex++;
	} /* end while (sindex < vs->rsl.maxNsweep) */
	vs->tk.nsweep = tk_sindex + 1;
	return(OK);
}

/*************************************************************/
/*                                                           */
/*                 first_ray_in_sweep                        */
/*                                                           */
/*************************************************************/
Ray *first_ray_in_sweep(Sweep *sweep)
{
  /*
   * Return the first non-NULL ray in the sweep.
   * Returns NULL, if error.
   */
  int iray;

  if (sweep == NULL) return(NULL);
  for (iray=0; iray<sweep->h.nrays; iray++)
  {
	if (sweep->ray[iray] != NULL)
	  return(sweep->ray[iray]);
  }

  return(NULL);
}
			 
/*************************************************************/
/*                                                           */
/*                      rslVosDimensions                     */
/*                                                           */
/*************************************************************/
int rslVosDimensions(VosSize *vs, Radar *radar, float maxRange)
{
	/* Scopes out all dimensions of the VOS contained in the rsl 
		 radar structure. Records dimensions in the 'VosSize->rsl'
		 structure.

		 The ncell values may be less than the actual number of 
		 bins in the radar structure, since the 1B-51/1C-51 standards 
		 call for range truncation. Truncates range as necessary.
		   For 1B-51: Max range is the lesser of:  
				 1: max_range from radar structure, and 
				 2: 230 km
			 
			 For 1C-51: Max range is the lesser of:
				 1: max_range from radar structure, and 
				 2: 200 km

		 Returns OK, or
		         <0, if error.
	*/

  /* Must differentiate between rsl and vosSize array indices,
	 since the rsl arrays may contain NULL elements. */
  int ivolume, isweep, iray;  /* Indices for rsl arrays. */
  int Vindex, Sindex;         /* Indices for non-NULL rsl array elements. */
  int nrays_in_sweep;
  float maxRangeActual = 0.0;
  Ray *ray;
  Ray *longest_ray_in_sweep;

  /* Initialize the 'VosSize' structure. */
  memset(vs, '\0', sizeof(VosSize));
  vs->rsl.sweep[0] = NULL;

/*
  if (radar_verbose_flag)
    fprintf(stderr, "RSL VOS Dimensions...\n");
*/
  Vindex = -1;
  for (ivolume=0; ivolume<radar->h.nvolumes; ivolume++)
  {
	if (radar->v[ivolume] == NULL) continue;
	Vindex++;
	vs->rsl.v[Vindex] = radar->v[ivolume];
	Sindex = -1;
	for (isweep=0; isweep<radar->v[ivolume]->h.nsweeps; isweep++)
	{
	  if (radar->v[ivolume]->sweep[isweep] == NULL) continue;
	  Sindex++;
	  longest_ray_in_sweep = first_ray_in_sweep(radar->v[ivolume]->sweep[isweep]);
	  if (longest_ray_in_sweep == NULL)
	  {
		fprintf(stderr, "rslVosDimensions(): no rays in sweep:%d ???\n\n",
				isweep);
		return(QUIT);
	  }
	  nrays_in_sweep = 0;
	  if (Sindex == 0)  /* First sweep in volume? */
		first_ray_in_volume[ivolume] = longest_ray_in_sweep;
	  /*
	   * Go thru the entire sweep to find the number of 
	   * rays, and the longest ray.
	   */
	  for (iray=0; iray<radar->v[ivolume]->sweep[isweep]->h.nrays; iray++)
	  {
		if (radar->v[ivolume]->sweep[isweep]->ray[iray] == NULL) continue;
		ray = radar->v[ivolume]->sweep[isweep]->ray[iray];
		nrays_in_sweep++;
		if (ray->h.nbins > longest_ray_in_sweep->h.nbins)
		  longest_ray_in_sweep = ray;
	  } /* end for (iray=0;... */

	  if (Sindex == 0)  /* 1st sweep of this volume? */
	  {
		/* Find max range (km) of data in this sweep. */
		maxRangeActual = (longest_ray_in_sweep->h.range_bin1 + 
						  longest_ray_in_sweep->h.nbins *
						  longest_ray_in_sweep->h.gate_size) / 1000.0; /*km*/
		if (maxRangeActual > maxRange+0.5)   /* Truncate range at maxRange km */
		  vs->rsl.ncell[Vindex][Sindex] = (int) 
			((maxRange*1000.0 - longest_ray_in_sweep->h.range_bin1) /
			 longest_ray_in_sweep->h.gate_size);
		else
		  vs->rsl.ncell[Vindex][Sindex] = (int) 
			((maxRangeActual*1000.0 - longest_ray_in_sweep->h.range_bin1) /
			 longest_ray_in_sweep->h.gate_size);
		if (vs->rsl.ncell[Vindex][Sindex] > MAX_CELL)
		{
		  fprintf(stderr, "rslVosDimensions(): ncell[parm%d]=%d > MAX_CELL=%d ",
				  Vindex, vs->rsl.ncell[Vindex][Sindex], MAX_CELL);
		  fprintf(stderr, " gate_size:%d\n", longest_ray_in_sweep->h.gate_size);
		  return(QUIT);
		}
	  } /* end if (Sindex == 0) */
	  else /* Not the 1st sweep of volume. */
	  {
		if (longest_ray_in_sweep->h.nbins > vs->rsl.ncell[Vindex][0])
		  vs->rsl.ncell[Vindex][Sindex] = vs->rsl.ncell[Vindex][0];
		else
		  vs->rsl.ncell[Vindex][Sindex] = longest_ray_in_sweep->h.nbins;
	  }

	  vs->rsl.nray[Vindex][Sindex] = nrays_in_sweep;
	  if (vs->rsl.nray[Vindex][Sindex] > vs->rsl.maxNray)
	  {
		vs->rsl.maxNray = vs->rsl.nray[Vindex][Sindex];
		if (vs->rsl.maxNray > MAX_RAY)
		{
		  fprintf(stderr, "rslVosDimensions(): v[%d]->sweep[%d].nray=%d > MAX_RAY=%d\n", 
				  ivolume, isweep, vs->rsl.nray[Vindex][Sindex], MAX_RAY);
		  return(QUIT);
		}
	  }
	} /* end for (isweep=0;... */
	vs->rsl.nsweep[Vindex] = Sindex + 1;
	if (vs->rsl.nsweep[Vindex] > vs->rsl.maxNsweep)
	{
	  vs->rsl.maxNsweep = vs->rsl.nsweep[Vindex];
	  if (vs->rsl.maxNsweep > MAX_SWEEP)
	  {
		fprintf(stderr, "rslVosDimensions(): v[%d].nsweep=%d > MAX_SWEEP=%d\n", 
				ivolume, vs->rsl.nsweep[Vindex], MAX_SWEEP);
		return(QUIT);
	  }
	}
/*
  if (radar_verbose_flag)
  {
  fprintf(stderr, 
  "  vIndex:%2d nsweeps:%d cellSize(m):%4d ncells:%d maxRng(km):%.1f\n",
  ivolume, vs->rsl.nsweep[Vindex], longest_ray_in_sweep->h.gate_size,
  longest_ray_in_sweep->h.nbins, maxRangeActual);
  }
*/
  } /* for (ivolume=0;... */
  vs->tk.nparm = Vindex + 1;
  if ((vs->tk.nparm == 0) || (vs->tk.nparm > MAX_PARM))
  {
	fprintf(stderr, "rslVosDimensions(): Invalid nparm=%d\n", vs->tk.nparm);
	return(QUIT);
  }

  return(OK);
}

/*************************************************************/
/*                                                           */
/*                       L1GVtemplateInit                    */
/*                                                           */
/*************************************************************/
int L1GVtemplateInit(VosSize *vs, Radar *radar, char *hdfFileName, 
										 float maxRange)
{
/* Create a toolkit 'Level_1B_1C_GV' template_node for the VOS 
	 contained in the radar structure. To do this:
	 1: Based on the data contained in the rsl structure, initialize
	    the following toolkit arrays:
			'TKnparm' : no. of volumes in radar structure (DZ, VR, ZD, etc).
			'TKnsweep': max no. of sweeps/volume, over all volumes.
			'TKnray' : max no. of rays per sweep, over all volumes.
			'TKncell': max no. of cells (bins) per ray, over all volumes, all sweeps.
	    These values are  parameters for the 'TKsetL1GVtemplate()'
			toolkit function call.
	 2: Call TKsetL1GVtemplate().

	 Returns:
	   OK, if success.
		 <0, if failure.
*/
	int pindex, status;
	int32 TKnparm[MAX_VOS], TKnsweep[MAX_VOS], TKnray[MAX_VOS];
	int32 TKncell[MAX_VOS][MAX_PARM];

	/* Scope out all dimensions of the VOS contained in the rsl structure. */
	status = rslVosDimensions(vs, radar, maxRange);
	if (status < 0) return(status);
	/* Set the toolkit VOS dimensions. These are different from the rsl
		 dimensions. The toolkit L1GV structure is organized as a
		 sequence of physical sweeps, while RSL is organized as a
		 sequence of logical "volumes".
  */
	status = tkVosDimensions(vs, radar);
	if (status < 0) return(status);

	/* Fill required toolkit array values for TKsetL1GVtemplate() call. */
	TKnparm[0] = vs->tk.nparm;
	TKnsweep[0] = vs->tk.nsweep;
	TKnray[0] = vs->rsl.maxNray;
/*
	if (radar_verbose_flag)
		fprintf(stderr, "Toolkit VOS Dimensions...\n");
*/
	for (pindex=0; pindex<vs->tk.nparm; pindex++)
	{
	  TKncell[0][pindex] = vs->rsl.ncell[pindex][0];
/*
		if (radar_verbose_flag)
		  fprintf(stderr, "  pIndex:%d nsweep:%d nray:%d ncell:%d\n", pindex, 
							(int)TKnsweep[0], (int)TKnray[0], (int)TKncell[0][pindex]);
*/
	}

	/* Create a toolkit template_node for this VOS. */
	status = TKsetL1GVtemplate(1, TKnparm, TKncell, TKnray, TKnsweep, hdfFileName);
	if (status != TK_SUCCESS)
	{
		fprintf(stderr, "L1GVtemplateInit(): *****TKsetL1GVtemplate() error\n");
		return(ABORT);
	}
	
	return(OK);   /* Successful template_node creation. */
}

/*************************************************************/
/*                                                           */
/*                       hdfFileOpen                         */
/*                                                           */
/*************************************************************/
static int hdfFileOpen(IO_HANDLE *granuleHandle, VosSize *vs, 
											 char *hdfFileName, Radar *radar)
{
/* Create a toolkit template node for the new VOS, and then open
	 a HDF file in which to write the VOS.

	 Returns:
	    OK, if success.
	    <0, if failure.
*/
	char fileAccessMode;
	int productID, status;
	float maxRange;
	struct stat buf;

	/* Based on the product desired, set the maximum range of radar data 
		 values for the HDF file.
	     For 1B-51: Max range = 230.0 km.  
		   For 1C-51: Max range = 200.0 km
	*/
	productID = (int)granuleHandle->productID;
	if (productID == TK_L1B_GV)
	  maxRange = MAX_RANGE_1B51;
	else if (productID == TK_L1C_GV)
	  maxRange = MAX_RANGE_1C51;
	else	
	{
		fprintf(stderr, "hdfFileOpen(): Invalid product type\n");
		return(ABORT);
	}

	/* Create a L1BGV template node for the new VOS.  */
/*
	if (radar_verbose_flag)
	  fprintf(stderr, "\n****** Creating toolkit template_node for VOS ...\n");
*/
	status = L1GVtemplateInit(vs, radar, hdfFileName, maxRange);
	if (status < 0) return(status);

	/* Determine if the HDF file to which this VOS belongs already exists.*/
	if (stat(hdfFileName, &buf) == 0)
	{
		/* The HDF file already exists. We will append this VOS to it. */
	  fileAccessMode = TK_APPEND;
		if (radar_verbose_flag)
		fprintf(stderr, "\n****** Opening HDF file: %s to append VOS ...\n",
					 hdfFileName);
	}
	else  /* The HDF file does not exist. We must create it. */
	{
		fileAccessMode = TK_NEW_FILE;
		if (radar_verbose_flag)
		fprintf(stderr, "\n****** Opening new HDF file: %s to write VOS ...\n",
					 hdfFileName);
	}

	/* Finally, open the HDF file. */
	status = TKopen(hdfFileName, productID, fileAccessMode, granuleHandle); 
	if (status != TK_SUCCESS)
	{
		if (radar_verbose_flag)
		  fprintf(stderr, "level_1(): ***** TKopen() error\n");
		return(ABORT);
	}

	/* Find the slot number in the HDF granule for this VOS. */
	vs->vos_num = TKgetNvos(granuleHandle) - 1;

	/* Write metadata fields into HDF file. */
	metaDataWrite(granuleHandle, radar, hdfFileName, fileAccessMode);

	return(OK);
}

/*************************************************************/
/*                                                           */
/*                       radarPrep1B51                       */
/*                                                           */
/*************************************************************/
int radarPrep1B51(Radar *radar)
{
/*
	 Prepare the RSL radar structure for 1B-51 processing: 
	 Null the pointers to RSL volumes not required for 1B-51.

	 Returns OK.
*/
	int j;
	
	/* For 1B-51, we need only the DZ, VR, and ZD data volumes. NULL the 
		 pointers to all other volumes. */
	for (j=0; j<radar->h.nvolumes; j++)
	{
		if (radar->v[j] == NULL)
		  continue;
		if ((j == DZ_INDEX) || (j == VR_INDEX) || (j == ZD_INDEX))
		  continue;
		radar->v[j] = NULL;
	}  /* end for (j=0; ... */

	return(OK);
}

/*************************************************************/
/*                                                           */
/*                       radarPrep1C51                       */
/*                                                           */
/*************************************************************/
int radarPrep1C51(Radar *radar)
{
/*
	 Prepare the RSL radar structure for 1C-51 processing. 
	 1. Check that the necessary RSL volumes exist.
	 2. Create mask volumes.
	 3. Null the pointers to RSL volumes not required for 1C-51.

	 Returns:
	 OK, if success.
	 <0, if failure.
*/
	int j;
	
	if (radar->v[DZ_INDEX] != NULL)  /* Is there a DZ volume? */
	{
		if (radar->v[CZ_INDEX] == NULL) /* DZ exists, hence CZ should exist. */
		{
			fprintf(stderr, "RSL_radar_to_hdf(): CZ volume expected but not found.\n");
			return(QUIT);
		}
		else  /* Both DZ and CZ volumes exist. */
		{
			/* Construct mask volume MZ */
			radar->v[MZ_INDEX] = maskBuild(radar->v[CZ_INDEX], radar->v[DZ_INDEX]);
			/* We're now finished with the RSL CZ data, so let the CZ volume point 
				 to the uncorrected volume DZ. We will later use it,
				 in combination with the mask volume MZ, to create the HDF CZ volume. 
				 (The HDF CZ values differ from the RSL CZ values.) 
			*/
			radar->v[CZ_INDEX] = radar->v[DZ_INDEX];
		}
	}  /* end if (radar->v[DZ_INDEX] != NULL) */

	/* For 1C-51, we need only the following volumes:
		     CZ: QC'ed reflectivity, which we will later obtain from DZ and MZ
			   MZ: mask to obtain DZ from CZ
			 NULL the pointers to all other volumes.
	*/
	for (j=0; j<radar->h.nvolumes; j++)
	{
		if (radar->v[j] == NULL)
		  continue;
		if ((j == CZ_INDEX) || (j == MZ_INDEX))
		  continue;
		radar->v[j] = NULL;
	}  /* end for (j=0; ... */

	return(OK);
}

/*************************************************************/
/*                                                           */
/*                    nullGranuleCreate                      */
/*                                                           */
/*************************************************************/
int nullGranuleCreate(char *hdfFileName, IO_HANDLE *granuleHandle,
											Radar *radar)
{
/* 
	 Create an HDF file containing an empty granule.
*/
	int status;
	struct stat buf;
	/* Following arrays required by toolkit function 'TKsetL1GVtemplate()'*/
	int32 TKnparm[MAX_VOS], TKnsweep[MAX_VOS], TKnray[MAX_VOS];
	int32 TKncell[MAX_VOS][MAX_PARM];
	
	/* Check if this HDF file already exists. If it exists, abort. */
	if (stat(hdfFileName, &buf) == 0)
	{
		if (radar_verbose_flag)
		fprintf(stderr, "\nnullGranuleCreate(): File %s already exists.\n",
					 hdfFileName);
		return(ABORT);
	}

	/* Fill toolkit array values for subsequent TKsetL1GVtemplate() call. */
	TKnparm[0] = TKnsweep[0] = TKnray[0] = TKncell[0][0] = 0;
	/* Create a L1 GV template_node. */
	status = TKsetL1GVtemplate(0, TKnparm, TKncell, TKnray, TKnsweep, hdfFileName);
	if (status != TK_SUCCESS)
	{
		fprintf(stderr, "nullGranuleCreate(): ***** TKsetL1GVtemplate() error\n");
		return(ABORT);
	}
	
	/* Open the HDF file. */
	if (radar_verbose_flag)
	  fprintf(stderr, "\n\n****** Opening new HDF file: %s for empty granule...\n",
					hdfFileName);
	status = TKopen(hdfFileName, TK_L1C_GV, TK_NEW_FILE, granuleHandle); 
	if (status != TK_SUCCESS)
	{
		if (radar_verbose_flag)
		  fprintf(stderr, "nullGranuleCreate(): ***** TKopen() error\n");
		return(ABORT);
	}
	/* Write metadata fields into HDF file. */
	metaDataWrite(granuleHandle, radar, hdfFileName, TK_NEW_FILE);
	/* Close the HDF file */
	if (radar_verbose_flag)
	  fprintf(stderr, "\n****** Closing HDF file: %s ...\n\n", hdfFileName);
	status = TKclose(granuleHandle);
	if (status != TK_SUCCESS)
	{
		if (radar_verbose_flag)
		  fprintf(stderr, "nullGranuleCreate(): ***** TKclose() error\n");
		return(ABORT);
	}

	return(OK);
}

/*************************************************************/
/*                                                           */
/*                      RSL_radar_to_hdf                     */
/*                                                           */
/*************************************************************/
int RSL_radar_to_hdf(Radar *radar, char *hdfFileName)
{
/*
	 Writes one VOS from a RSL radar structure into one HDF file.
	 Returns:
	   OK , if success.
	   <0 , if failure. (See Error code definitions at top of file.)
*/
	char product[8];
	int status;
	L1B_1C_GV *gvl1;              /* Toolkit structure for VOS storage. */
	IO_HANDLE granuleHandle;      /* Toolkit file_descriptor structure. */
	VosSize vs;                   /* Storage of VOS dimensions. */
	Volume *v[MAX_RADAR_VOLUMES]; /* Storage of radar volume pointers. */

	if (radar == NULL) return(ABORT);

	if (radar->h.nvolumes == 0)
	/* Create an HDF file to contain an empty granule. */
	{
		status = nullGranuleCreate(hdfFileName, &granuleHandle, radar);
		return(status);
	}
	/* We will, within functions radarPrep1B51() and radarPrep1C51(), 
		 manipulate the array of radar volume pointers radar->v[]. 
		 Hence we here save the array radar->v[] in v[], so we can restore 
		 radar->v[] to its original condition before leaving this function.
	*/
	radarVolumesSave(v, radar);

	/* Get the desired product out of the HDF filename. */
	sscanf(strrchr(hdfFileName, '/'), "/%4s", product);
	if (strcmp(product, "1B51") == 0)
	{
		granuleHandle.productID = TK_L1B_GV;
		status = radarPrep1B51(radar);
	}
	else if (strcmp(product, "1C51") == 0)
	{
		granuleHandle.productID = TK_L1C_GV;
		status = radarPrep1C51(radar);        /* Creates mask volumes. */
	}
	else  /* Unknown product. */
	{
		status = ABORT;
		if (radar_verbose_flag)
		  fprintf(stderr, "RSL_radar_to_hdf(): Unknown product requested: %s.\n",
							product);
	}
	if (status < 0) goto quit;

	/* Open the HDF file 'hdfFileName'. */
	status = hdfFileOpen(&granuleHandle, &vs, hdfFileName, radar);
	if (status < 0) goto quit;
	
	/* Build toolkit 'L1B_1C_GV' structure using data from radar structure.*/
	if (radar_verbose_flag)
	  fprintf(stderr, "\n******  Moving VOS from RSL structure --> toolkit structure ...\n");
	gvl1 = gvl1Build(radar, qcParm, &vs, (int)granuleHandle.productID);

	/* Write data from toolkit 'L1B_1C_GV' structure to HDF file. */
	if (radar_verbose_flag)
	  fprintf(stderr, "\n****** Writing VOS to HDF file: %s ...\n", hdfFileName);
	status = TKwriteL1GV(&granuleHandle, gvl1);
	if (status != TK_SUCCESS)
	{
		TKclose(&granuleHandle);
		status = ABORT;
		if (radar_verbose_flag)
		  fprintf(stderr, "RSL_radar_to_hdf(): *** TKwriteL1GV() error\n");
		goto free_memory_and_quit;
	}
	
	/* Close the HDF file */
	if (radar_verbose_flag)
	  fprintf(stderr, "\n****** Closing HDF file: %s ...\n\n", hdfFileName);
	status = TKclose(&granuleHandle);
	if (status == TK_SUCCESS)
	  status = OK;
	else
	{
		if (radar_verbose_flag)
		  fprintf(stderr, "RSL_radar_to_hdf(): *** TKclose() error\n");
	  status = ABORT;
	}
	
 free_memory_and_quit:
	/* Free memory allocated to the toolkit 'L1B_1C_GV' structure. */
	TKfreeGVL1(gvl1);
	/* If RSL mask volumes for 1C-51 were created above, free them. */
	if (radar->v[MZ_INDEX] != NULL)
	  RSL_free_volume(radar->v[MZ_INDEX]);
	if (radar->v[MD_INDEX] != NULL)
	  RSL_free_volume(radar->v[MD_INDEX]);
 quit:
	/* Restore the array of radar volume pointers in radar->v[]. */
	radarVolumesRestore(radar, v);
	return(status);
}
#endif
