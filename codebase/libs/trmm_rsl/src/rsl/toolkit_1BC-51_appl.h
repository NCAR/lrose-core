/*
   Parameter definitions for 1B-51 and 1C-51 HDF
	 file handling applications using the TSDIS toolkit. 

	 Mike Kolander
*/

#define SCALE_FACTOR     100.0   /* Most data values placed into HDF file
																	  are scaled by this factor. */
#define X 200.0                  /* Used to create 1C-51 reflectivity
																		values: 
																		1C-51_val = dBz_val - X * MaskValue,
																		where MaskValue = 0 or 1 .
																 */

/* For missing HDF header values. From TSDIS specs. */
#define NOVAL_INT16 -9999
#define NOVAL_INT32 -9999
#define NOVAL_FLOAT -9999.9

/* HDF cell value flags. */
#define NO_VALUE -32767      /* No value recorded in bin by radar site. */
#define RNG_AMBIG_VALUE -32766    /* Range-ambiguous flag */
#define NOECHO_VALUE -32765       /* Value too low for SNR */
#define AP_VALUE -32764           /* Anomalous propagation flag */


#define MAX_RANGE_1B51   230.0   /* Max range (km) for 1B-51. */
#define MAX_RANGE_1C51   200.0   /* Max range (km) for 1C-51. */
#define TK_MAX_FILENAME  256     /* 256 bytes for filename storage. */

/* ----------- Function return codes --------------
	 See RSL files: radar_to_hdf_1.c, radar_to_hdf_2.c, hdf_to_radar.c
	 See application files: level_1.c, hdf_to_uf.c   */
#define OK        0  /* Nominal termination with product produced. */
#define ABORT    -1  /* PANIC exit -> No product produced. */
#define INTER    -2  /* Abort execution. Received signal INT, KILL, or STOP */
#define QUIT     -3  /* Anomalous_condition exit -> No product produced. */
#define SHUTDOWN -4  /* Shut down all processing, including scripts. */

/* TSDIS toolkit parameter definitions. */
#include "IO_GV.h"


/* VOS dimensions w.r.t toolkit. The toolkit L1 structure is logically
   organized as a sequence of physical sweeps. */
typedef struct
{
int nparm;  /* No. of toolkit parameters (ie, volumes) for this VOS. */
int nsweep;
int nray[MAX_SWEEP];
int ncell[MAX_SWEEP][MAX_PARM];  /* ncell[tk_sindex][pindex] */
} tkVosSize;


/* VOS dimensions w.r.t RSL. RSL is logically organized as a sequence
	 of "volumes".*/
typedef struct
{
int maxNsweep;
int maxNray;    /* Max(nray[sweep0] ... nray[nsweep-1]) */
int nsweep[MAX_PARM];
int nray[MAX_PARM][MAX_SWEEP];
int ncell[MAX_PARM][MAX_SWEEP];
/* Arrays of pointers mapping toolkit objects back to the
	 corresponding rsl objects. */
Volume *v[MAX_PARM];     /* Maps each tk volume to a rsl volume. */
Sweep *sweep[MAX_SWEEP]; /* Maps each tk sweep to a rsl sweep. */
} rslVosSize;


typedef struct
{
int vos_num;  /* 0...MAX_VOS-1 : Position no. of this VOS in HDF file. */
rslVosSize rsl;  /* Dimensions of rsl radar structure. */
tkVosSize tk;    /* Dimensions of toolkit structure. */
} VosSize;

#define NUMBER_QC_PARAMS 10
enum QC_parameters
{
	HTHRESH1, HTHRESH2, HTHRESH3,
	ZTHRESH0, ZTHRESH1, ZTHRESH2, ZTHRESH3, 
	HFREEZE, DBZNOISE, ZCAL
};

/*************************************************************/
/*                                                           */
/*                Function Prototypes                        */
/*                                                           */
/*************************************************************/
/* Toolkit memory management functions in file: toolkit_memory_mgt.c */
void TKfreeGVL1(L1B_1C_GV *gvl1);
int8 ***TKnewParmData1byte(int nsweep, int nray, int ncell);
int16 ***TKnewParmData2byte(int nsweep, int nray, int ncell);
PARAMETER *TKnewGVL1parm(void);
L1B_1C_GV *TKnewGVL1(void);
