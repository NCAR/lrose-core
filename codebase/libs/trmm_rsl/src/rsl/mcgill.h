/**********************************************************************
 *    Structure and parameter definitions for Mcgill format
 * radar data processing.
 *
 *   Kolander
 *
 **********************************************************************/

#include <stdio.h>

#define MCG_RECORD     2048   /* Mcgill record size (bytes) */
#define MCG_CSP       12288   /* Mcgill CSP block size (bytes) */
#define MCG_MAX_SEG_NUM 107   /* 107 segments per Mcgill logical record */

/* Mcgill method return codes */
#define MCG_EOD             1 /* End_Of_Data flag */
#define MCG_OK              0 /* Successful return */
#define MCG_OPEN_FILE_ERR  -1 /* Couldn't open Mcgill data file */
#define MCG_CLOSE_FILE_ERR -2 /* Couldn't close Mcgill data file */
#define MCG_EOF            -3 /* Reached end of Mcgill data file */
#define MCG_READ_ERR       -4 /* Error occurred reading data file */
#define MCG_FORMAT_ERR     -5 /* Unidentified radar site or format error*/
#define SYS_NO_SPACE       -6 /* Memory allocation problem */

/* Mcgill segment identifiers */
#define MCG_UNDEFINED_SEG 0
#define MCG_DATA_SEG      1
#define MCG_ELEV_SEG      2
#define MCG_EOD_SEG       3

/* Radar site/format codes */
#define MCG_PAFB_3_4 0      /* Patrick Air Force Base, format 3 or 4 */
#define MCG_PAFB_1_2 1      /* Patrick Air Force Base, format 1 or 2 */
#define MCG_SAOP     2      /* Sao Paulo */

typedef unsigned short word;
typedef unsigned char byte;
typedef int mcgSegmentID;

typedef struct
   {
   word unused[40];
   /* Word 41 of Mcgill header record. A word is 2 (unsigned) bytes. */
   word hour;         /* 0 to 23 */
   word min;          /* 0 to 59 */
   word sec;          /* 0 to 59 */
   word day;          /* 1 to 31 */
   word month;        /* 1 to 12 */
   word year;         /* 0 to 99 */
   word num_records;  /* No. logical records in volume scan */
   word unused1[3];
   /* Word 51 */
   word vol_scan_format; /*  1: 24 sweeps, normal
                             2: 24 sweeps, compressed
                             3: 12 sweeps, normal
                             4: 12 sweeps, compressed */
   word unused2[4];
   /* Word 56 */
   word csp_rec;   /* 0: No CSP block     1: CSP block */
   }
mcgHeader_t;

typedef struct
   {
   FILE *fp;            /* Pointer to Mcgill file */
   int site;
   int *num_bins;       /* Points to array of 24 bin_counts,
						   one bin_count per sweep. */
   mcgHeader_t head;
   }
mcgFile_t;

/* Structure to contain a Mcgill logical record (2048 bytes) */
typedef struct
   {
   word record_num;     /* No. of this logical record in volume scan */
   byte last_record_flag; /* 1 if last record in vol_scan, 0 otherwise */
   byte start_elev_num;  /* Starting elev no. of data in this record */
   byte end_elev_num;    /* End elev no. of data in this record */
   byte unused[9];
   byte segment[107][19]; /* 107 elevation and data segments, 
                             19 bytes each. */
   byte unused1;
   }
mcgRecord_t;

/* Structure to contain Mcgill reflectivity data from 1 ray */
typedef struct
   {
   float elev;
   float azm;
   int sweep_num;
   int num_bins;   /* no. of bins for this ray */
   char data[240]; /* reflectivity data value for each bin */
   }
mcgRay_t;


/*************** Function Prototypes **********************/
/* Grouped by object operated on and/or returned. */
mcgFile_t *mcgFileOpen(int *code, char *filename);
int mcgFileClose(mcgFile_t *file);

int mcgRecordRead(mcgRecord_t *record, mcgFile_t *file);

mcgSegmentID mcgSegmentKeyIdentify(int key);

int mcgRayBuild(mcgRay_t *ray, mcgFile_t *file);

