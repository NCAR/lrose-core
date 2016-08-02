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

  Memory management functions used for the dynamic allocation/
  deallocation of TSDIS Toolkit GV_Level_1 structures, created when
	reading/writing to an HDF file.

	Functions herein are called by RSL_radar_to_hdf() and RSL_hdf_to_radar().

*******************************************************************/

/* TSDIS toolkit function and structure definitions. */
#include "IO.h"
#include "IO_GV.h"

/* ------ Functions defined in this file. ------- */
/*
void TKfreeGVL1(L1B_1C_GV *gvl1);
int8 ***TKnewParmData1byte(int nsweep, int nray, int ncell);
int16 ***TKnewParmData2byte(int nsweep, int nray, int ncell);
PARAMETER *TKnewGVL1parm(void);
L1B_1C_GV *TKnewGVL1(void);
*/

/*************************************************************/
/*                                                           */
/*                       TKfreeGVL1                          */
/*                                                           */
/*************************************************************/
void TKfreeGVL1(L1B_1C_GV *gvl1)
{
/* Frees all memory which was (dynamically) allocated to the toolkit
	 'L1B_1C_GV' structure, and associated substructures. 
*/
	int pindex;
	
	for (pindex=0; pindex<MAX_PARM; pindex++)
	{
	  if (gvl1->sensor.parm[pindex] != NULL)
		{
			if (gvl1->sensor.parm[pindex]->parmData1byte != NULL)
			{
				free(gvl1->sensor.parm[pindex]->parmData1byte[0][0]);
				free(gvl1->sensor.parm[pindex]->parmData1byte[0]);
				free(gvl1->sensor.parm[pindex]->parmData1byte);
			}  /* end if (gvl1->sensor.parm[pindex]->parmData1byte != NULL) */
			else if (gvl1->sensor.parm[pindex]->parmData2byte != NULL)
			{
				free(gvl1->sensor.parm[pindex]->parmData2byte[0][0]);
				free(gvl1->sensor.parm[pindex]->parmData2byte[0]);
				free(gvl1->sensor.parm[pindex]->parmData2byte);
			}  /* end if (gvl1->sensor.parm[pindex]->parmData1byte != NULL) */
			free(gvl1->sensor.parm[pindex]);
		}  /* end if (gvl1->sensor.parm[pindex] != NULL)*/
	}  /* end for (pindex=0;*/

	free(gvl1);
}

/*************************************************************/
/*                                                           */
/*                       TKnewParmData1byte                  */
/*                                                           */
/*************************************************************/
int8 ***TKnewParmData1byte(int nsweep, int nray, int ncell)
{
/* Allocates memory for indexed storage of a 3_dimensional array 
	 of 1_byte mask values associated with one parameter of the
	 toolkit 'L1B_1C_GV' structure.
*/
	int8 ***parmData1byte;
	int8 **ray;
	int8 *data;
	int sindex, rindex;
	
	/* Vector for 'nsweep' sweep pointers. */
	parmData1byte = (int8 ***)calloc(nsweep, sizeof(int8 **));
	/* 2D array for 'nsweep*nray' ray pointers. */
	ray = (int8 **)calloc(nsweep*nray, sizeof(int8 *));
	/* 3D array for data. */
	data = (int8 *)calloc(nsweep*nray*ncell, sizeof(int8));

	/* Fill all the sweep and ray pointer slots created above. */
  for (sindex=0; sindex<nsweep; sindex++)
	{
		parmData1byte[sindex] = (int8 **)(ray + sindex*nray);
		for (rindex=0; rindex<nray; rindex++)
			parmData1byte[sindex][rindex] = (int8 *)(data + sindex*nray*ncell +
																							 rindex*ncell);
  }

	return(parmData1byte);
}

/*************************************************************/
/*                                                           */
/*                       TKnewParmData2byte                  */
/*                                                           */
/*************************************************************/
int16 ***TKnewParmData2byte(int nsweep, int nray, int ncell)
{
/* Allocates memory for storage of a 3_dimensional array of 2_byte
	 data values associated with one parameter of the toolkit 
	 'L1B_1C_GV' structure.
*/
	int16 ***parmData2byte;
	int16 **ray;
	int16 *data;	
	int sindex, rindex;
	
	/* Vector for 'nsweep' sweep pointers. */
	parmData2byte = (int16 ***)calloc(nsweep, sizeof(int16 **));
	/* 2D array for 'nsweep*nray' ray pointers. */
	ray = (int16 **)calloc(nsweep*nray, sizeof(int16 *));
	/* 3D array for data. */
	data = (int16 *)calloc(nsweep*nray*ncell, sizeof(int16));

	/* Fill all the sweep and ray pointer slots created above. */
  for (sindex=0; sindex<nsweep; sindex++)
	{
		parmData2byte[sindex] = (int16 **)(ray + sindex*nray);
		for (rindex=0; rindex<nray; rindex++)
			parmData2byte[sindex][rindex] = (int16 *)(data + sindex*nray*ncell +
																								rindex*ncell);
  }

	return(parmData2byte);
}

/*************************************************************/
/*                                                           */
/*                       TKnewGVL1parm                       */
/*                                                           */
/*************************************************************/
PARAMETER *TKnewGVL1parm(void)
{
/* Allocates memory for a 'PARAMETER' structure. */
	PARAMETER *parm;

	parm = (PARAMETER *)calloc(1, sizeof(PARAMETER));
	parm->parmData1byte = (int8 ***)NULL;
	parm->parmData2byte = (int16 ***)NULL;
	return(parm);
}

/*************************************************************/
/*                                                           */
/*                          TKnewGVL1                        */
/*                                                           */
/*************************************************************/
L1B_1C_GV *TKnewGVL1(void)
{
/* Allocates memory for a 'L1B_1C_GV' structure. */
	L1B_1C_GV *gvl1;
	int pindex;
	
	gvl1 = (L1B_1C_GV *)calloc(1, sizeof(L1B_1C_GV));
	for (pindex=0; pindex<MAX_PARM; pindex++)
	  gvl1->sensor.parm[pindex] = (PARAMETER *)NULL;

	return(gvl1);
}

#endif
