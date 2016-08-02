#ifdef __cplusplus                                                        
 extern "C" {                                                         
#endif
/*****************************************************************

 ** Copyright (c) 1993, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** Proprietary and Confidential to UCAR
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307, USA
 ** Licensed use only.
 ** Do not copy or distribute without authorization

	Header file defining the RDI radar data format. In this
	format the radar data measured 
	at a given time, elevation and
	azimuth are packed into a record (message). The record
	contains multiple gates (volume samples) of multiple 
	fields (DBZ, Doppler velocity, ...). The date set 
	containing an array of gates is sometimes called a ray
	or a beam data. The RDI format assumes arbitrary number
	of fields, arbitrary data word length, and arbitrary
	range spacing as well as sizes for the gates.

	File: r_data.h

*****************************************************************/


#define MAX_N_FLDS 16

/* Radar data header - optional static information */
struct radar_header {
    char radar_name[32]; /* radar name, project name... */
    long latitude;       /* in 1/100,000 degrees */
    long longitude;      /* in 1/100,000 degrees */
    long altitude;       /* in meters */
    long power;          /* in dbM */
    unsigned short pulse_width;    /* in naro-seconds */
    unsigned short beamwidth;      /* in 1/100 degrees */
    char notes[64];      /* any additional data or comments */
};
typedef struct radar_header Radar_header;

/* header for each data field */
struct field_header {
    char f_name[6];          /* field name */
    char f_unit[6];          /* unit name */
    short scale;             /* = 100*scale */
    short offset;            /* = 100*offset */
    unsigned short data_size;/* data size in number of bits. 
		8,16 or any other number */
    unsigned short range;    /* range of the center of the first
		gate in meters. Or, if g_size=0, the offset of a
		range-gatesize table. Refer to a later note */
    unsigned short g_size;   /* gate size in 1/16 meters. 
	        If 0, variable range or gate size is assumed */
    unsigned short n_gates;  /* number of gates */
};
typedef struct field_header Field_header;
	
/* field name used: DBZ, VEL, SNR, SPW, DBZ1, DBZ2, VEL1... */
/* unit examples: dbz, m/s, ... */

/* the mandatory radar data header */
struct ray_header {
    unsigned short length;  /* data length in bytes of this ray 
		including all headers */
    unsigned short freq;    /* in MHz or beamwidth in .01 degrees
		if the first bit is 1 */
    short prf;              /* Hz */
    unsigned char polar;    /* polarization 0,1,2,... */
    unsigned char bad_data; /* value used for bad data. This is for 
		byte data only */
    unsigned long time;     /* in unix time format */
    unsigned char r_id;     /* Lower 4 bits represent an id number
	        identifying the radar. e.g. MHR=1, CP2=2, ... 
		The high 4 bits represents scan mode: 
                0 - ppi, 1 - rhi, ...  */
    unsigned char f_cnt;    /* frame number in a volume */
    unsigned short v_cnt;   /* volume counter */
    unsigned short id;      /* 30303 a magic number for the format */
    unsigned short azi;     /* true azimuth. in 1/100 degrees */
    unsigned short t_ele;   /* target elevation. in 1/100 degrees */
    short ele;              /* true elevation. in 1/100 degrees */
    unsigned short n_fields;/* number of fields */
    unsigned short r_h_pt;  /* if non-zero, offset of the optional 
		radar header */
    unsigned short f_pt[MAX_N_FLDS]; /* offsets of the field data */
};
typedef struct ray_header Ray_header;

/* Each ray (beam) data starts with a Ray_header, followed by a number of
   data fields and, optionally, a Radar_header and/or a range-gatesize
   table. */

/* Each data field contains a field_header followed by data. The data 
   may need to be packed into 32 bit group if its length is not 8, 
   16 or 32 */

/* If the ray data has a variable gate spacing or gate sizes, a table 
   must be used to specify the range and size of each gate. This table 
   is an array of unsigned short integers specifying the range (in 16 
   meters) and gate size (in 1/16 meters) of each gate as follows:
   range of first gate, size of first gate, range of second gate, size
   of second gate, .... We assume all fields share the same range and 
   gate size table */

/* Note: The number of MAX_N_FLDS is not important in this format 
   definition in the sense that this number needs not to be known by
   a user of the data. However, if the user of the data uses a smaller
   MAX_N_FLDS than n_fields he can only access the first n_fields fields.

   In this file we use the offset to specify a subset data location,
   which means that the subset starts after the specified number of
   bytes from the beginning of the ray data */

#ifdef __cplusplus             
}
#endif
