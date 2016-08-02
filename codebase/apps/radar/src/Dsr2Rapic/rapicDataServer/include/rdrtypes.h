#ifndef __RDRTYPES_H__
#define __RDRTYPES_H__

/*
  PPI - single PPI scan
  RHI - single RHI scan	
  CompPPI - single composite PPI scan
  VOL - closely (time) related scan set.
*/
typedef char *strarr[];
enum	e_scan_type {PPI, RHI, CompPPI, IMAGE, VOL, RHISet, 
		     MERGE, SCANERROR, CAPPI, 
		     Cart_CAPPI,
		     e_st_max}; // insert new scan_types before ST_MAX
    
extern char *scan_type_text[];
extern char *scan_type_text_short[];

// the IMAGE type is used by the database to identify scans and images
/*
 * Following utilities can return -1 if "-1" or "ANY" string passed or 
 * no match 
 * It is up to the user of this utility to handle -1  
 */
int decode_scantypestr(char *scanstr);  // decodes either numeric or text
int get_scan_type(char *scanstr);
char *get_scan_type_text(int scantype);
char *get_scan_type_short(int scantype);


enum e_data_type {e_refl, e_vel, e_spectw, e_diffz, e_rawrefl, 
		  e_rainaccum, e_filtvel, e_vil, e_tops, 
		  e_terrainht, e_rf_rainrate, e_rf_rainaccum,
		  e_rf_fcstaccum, e_rf_probability, 
		  e_particle_id, e_qpe_rate,
		  e_dt_max}; // insert new data_type enums before e_dt_max
// IF A VALUE IS ADDED, data_type_text in rdrscan.C will need to be changed to include new strings

enum bf_data_type {bf_none = 0, bf_refl=1, bf_vel=2, 
		   bf_spectw=4, bf_diffz=8, bf_rawrefl=16, 
		   bf_rainaccum = 32, bf_filtvel};

extern char *data_type_text[];
int get_data_type(char *dtastr);
int decode_datatypestr(char *dtastr);
char *get_data_type_text(e_data_type datatype);

enum	e_data_fmt {RLE_6L_ASC,RLE_16L_ASC, RLE_8BIT, RAW_8BIT,
		    e_df_max};
extern char *data_fmt_text[];
int get_format_type(char *fmtstr);
int decode_formatstr(char *fmtstr); // decodes either numeric or text
char *get_data_fmt_text(int datafmt);


#endif
