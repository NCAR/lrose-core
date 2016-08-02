/*Class to create and write out mdv files
 */

#include <mdv/mdv_file.h>
#include <mdv/mdv_macros.h>

class mdv
{
  MDV_dataset_t dataset;
  float elevation[MDV_MAX_VLEVELS];
  int debug;
  
  void populate_field(int field_num,
		      rdr_scan *scan, 
		      e_scan_type tscan_type, 
		      e_data_type tdata_type);
  int mdv::count_scans(rdr_scan *scan, 
		       e_scan_type tscan_type, 
		       e_data_type tdata_type);
  void mdv::get_elevs(rdr_scan *scan, 
		       e_scan_type tscan_type, 
		       e_data_type tdata_type);
  int rapicToRadial(rdr_scan *scan, 
		    s_radl *radl,
		    unsigned char *radbyte, 
		    int nGates );
  //debug
  //int maxval;

  
 
public:
  mdv();
  mdv(rdr_scan *scan);
  ~mdv();
  int write(FILE *outfile);
  int scan_count;
  
};

