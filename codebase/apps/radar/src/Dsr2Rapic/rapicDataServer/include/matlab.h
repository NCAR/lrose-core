/*Class to create and write out matlab readable files (comma delimited)
 */


class matlab
{
  int debug;
  
  void populate_field(rdr_scan *scan, FILE *outfile);
   int rapicToRadial(rdr_scan *scan, s_radl *radl, FILE *outfile);
  
public:
  matlab();
  void write_matlab(rdr_scan *scan, FILE *outfile);
  ~matlab();
  int max_ny;
  float max_nx;
  float rng_res;
  char data_set_info[20];
  float start_rng;
  float nyquist;
  
};

