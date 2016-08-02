/*
  rdr_scan.c
*/

#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rdr.h"
#include "palette.h"
#include "rdrscan.h"
#include "utils.h"
#include "rdrutils.h"
// #include <bstring.h>
#include "freelist.h"
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include "siteinfo.h"
#include "radlcnvt.h"
#include "log.h"
#include <stdlib.h>
#include <math.h>
#include <errno.h>

bool checkRadialMSSG = false;
bool useScanUserList = false;
bool dumpScanUsers = false;
bool useScanRegistry = false;
bool writeDetailedScanReport = false;
bool keepNullRadls = true;

char AnyStr[] = "Any";

int total_scans = 0;
int  RADLBUFFSIZE = 1024;

static char const cvsid[] = "$Id: rdrscan.C,v 1.7 2008/12/20 05:03:51 dixon Exp $";

rdr_scan *RdrScanAdminList = 0;	// linked list of all allocated rdr_scans

int skip_white(char* in_line, int max_len) {
  int line_pos = 0;
  while ((line_pos < max_len) && (in_line[line_pos] < 33)) 
    line_pos++;
  if (line_pos == max_len) line_pos = -1;
  return(line_pos);
}

char *scan_type_text[] = {
  "PPI","RHI","CompPPI","IMAGE","Volume", 
  "RHISet", "Merge", "ERROR SCAN", "CAPPI", "CartCAPPI", 
	
  ""  // insert new string corresponding to new e_scan_type before this
};

char vol_id_text[16] = "Volume";

char *volIDText(int vol_id)
{
  char id_str[16] = "";

  if (!((vol_id < 0) ||
	(vol_id > 9999)))
    sprintf(id_str, "%d", vol_id);
  strncpy(vol_id_text+6, id_str, 9);
  return vol_id_text;
}

char *scan_type_text_short[] = {
  "PPI","RHI","Comp","IMG","Vol", 
  "RHISet", "Mrg", "ERR", "CAPPI", "CartCAPPI", 
	
  ""  // insert new string corresponding to new e_scan_type before this
};

char *scanTypeText(rdr_scan *scan)
{
  if ((scan->scan_type != VOL) ||
      (scan->volume_id < 0))
    return scan_type_text[scan->scan_type];
  else
    return volIDText(scan->volume_id);
}     

/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int get_scan_type(char *scanstr) {
  return String2Int(scanstr, scan_type_text, e_st_max-1);
}

/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int decode_scantypestr(char *scanstr) {
  int scantype = PPI;
  if (isalpha(scanstr[0])) 
    scantype = get_scan_type(scanstr);	// NOT case sensitive
  else {
    if (isdigit(scanstr[0]) || (scanstr[0] == '-')) 
      sscanf(scanstr, "%d", &scantype);
    //	    scantype = atoi(scanstr);
  }
  return scantype;
}
	
char *get_scan_type_text(int scantype) {
  if ((scantype >= 0) && (scantype < e_st_max))
    return scan_type_text[scantype];
  else return AnyStr;
}

char *get_scan_type_short(int scantype) {
  if ((scantype >= 0) && (scantype < e_st_max))
    return scan_type_text_short[scantype];
  else return AnyStr;
}

char *data_fmt_text[] = {
  "Rapic 6 level","Rapic 16 level", "Raw 8 bit", "Rapic 8 bit", 
	
  ""  // insert strings corresponding to new e_data_fmt enum before this
};
char *data_fmt_text_short[] = {
  "Rapic6Lvl","Rapic16Lvl", "Raw8bit", "Rapic8bit",
	
  ""  // insert strings corresponding to new e_data_fmt enum before this
};

/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int get_format_type(char *fmtstr) {
  return String2Int(fmtstr, data_fmt_text_short, e_df_max-1);
}

/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int decode_formatstr(char *fmtstr) {
  int fmttype = -1;
  if (isalpha(fmtstr[0])) 
    fmttype = get_format_type(fmtstr);      // NOT case sensitive
  else {
    if (isdigit(fmtstr[0]) || (fmtstr[0] == '-')) 
      sscanf(fmtstr, "%d", &fmttype);
    switch (fmttype) {
    case RLE_6L_ASC :
    case RLE_16L_ASC :
    case RAW_8BIT :
    case RLE_8BIT :
    case 6 :    // interpret as RLE_6L_ASC
    case 7 :
    case 16 :    // interpret as RLE_16L_ASC
    case 32 :
    case 64 :
    case 128 :
    case 160 :
    case 256 :   // interpret as RAW8BIT
      break;    // valid enumerators, don't change
    default :    // interpret any other values as ANY
      fmttype = -1;
    }
  }
  return fmttype;
}
  	
char *get_data_fmt_text(int datafmt) {
  if (datafmt < 0)
    return AnyStr;
  if (datafmt < e_df_max)
    return data_fmt_text_short[datafmt];
  else if (datafmt == 6)
    return data_fmt_text_short[RLE_6L_ASC];
  else if ((datafmt == 16) || (datafmt == 32) || (datafmt == 64) || 
	   (datafmt == 128) || (datafmt == 160))
    return data_fmt_text_short[RLE_16L_ASC];
  else if (datafmt == 256)
    return data_fmt_text_short[RLE_8BIT];
  else 
    return AnyStr;
}

char *data_type_text[] = {
  "Refl","Vel","SpWdth", "DiffZ", "UnCorRefl", "RainAccum", 
  "FiltVel", "VIL", "3DTops", 
  "TerrainHt", "RainRate", "RainAccum",
  "RFFcstAccum", "RFProb",
  "PartID", "QPERate",
  "" // INSERT NEW STRINGS BEFORE THIS
};
/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int get_data_type(char *dtastr) {
  //int tempint;
  /* following converts string to bf_data_type
     tempint = String2Int(dtastr, data_type_text, e_dt_max-1);
     if (tempint < 0) return tempint;
     else return (1 << tempint);	    // convert enum to bit field
  */
  return String2Int(dtastr, data_type_text, e_dt_max-1);
}

/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int decode_datatypestr(char *dtastr) {
  int datatype = e_refl;
  if (isalpha(dtastr[0])) 
    datatype = get_data_type(dtastr);	// NOT case sensitive
  else {
    if (isdigit(dtastr[0]) || (dtastr[0] == '-')) 
      sscanf(dtastr, "%d", &datatype);
    //	    datatype = atoi(dtastr);
  }
  return datatype;
}
	
char *get_data_type_text(e_data_type datatype) {
  //int strtype = -1;
  //    if ((datatype >= 0) && (datatype < e_dt_max))
  //	return data_type_text[datatype];
  /* following converts bf_data_type to string
     switch (datatype) {
     case e_refl : strtype = 0;
     break;
     case e_vel : strtype = 1;
     break;
     case e_spectw : strtype = 2;
     break;
     case e_diffz : strtype = 3;
     break;
     case e_rawrefl : strtype = 4;
     break;
     default : strtype = -1;
     }
  */
  if ((datatype >= 0) &&
      (datatype < e_dt_max)) return data_type_text[datatype];
  else return AnyStr;
}

char *data_src_text[] = {
  "Comm","DB", "DBReloadRealtime", 
  "CommReq", "Prod_Accum", "Prod_Xlat", 
  "Replay", "Prod_FiltData", "Prod_CAPPI", 
  "Prod_VIL","Prod_Tops", 
  "Prod_Nexrad",
  ""  // insert new strings corresponding to e_data_source before this
};
/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int get_data_src(char *srcstr) {
  return String2Int(srcstr, data_src_text, e_ds_max-1);
}

/*
 * Can return -1 if "-1" or "ANY" string passed or no match 
 * It is up to the user of this utility to handle -1  
 */
int decode_datasrcstr(char *srcstr) {
  int datasrc = COMM;
  if (isalpha(srcstr[0])) 
    datasrc = get_data_src(srcstr);	// NOT case sensitive
  else {
    if (isdigit(srcstr[0]) || (srcstr[0] == '-')) 
      sscanf(srcstr, "%d", &datasrc);
    //	    datasrc = atoi(srcstr);
  }
  return datasrc;
}
	
char *get_data_src_text(int datasrc) {
  if ((datasrc >= 0) && (datasrc < e_ds_max))
    return data_src_text[datasrc];
  else return AnyStr;
}

int rdr_scan::StnID() {
  return (station & 0xff);
}
    
int rdr_scan::CountryID() {
  int temp = station >> 8;
  return (temp & 0xff);
}

/*
  Currently supported Country Codes
  Code         ID     Country/Region
  036          0      Australia
  458          1      Malaysia
*/

int rdr_scan::CountryCodeToID(int CountryCode) {	// default to 0 - Australia
  int id = 0;
  switch (CountryCode)
    {
    case 36:
      id = 0;
      break;
    case 458:
      id = 1;
      break;
    default:
      id = 0;
    }
  return id;
}

int rdr_scan::CountryIDToCode(int CountryID) {	// 36 - Australia
  int code = 36;
  switch (CountryID)
    {
    case 0:
      code = 36;
      break;
    case 1:
      code = 458;
      break;
    default:
      code = 36;
    }
  return code;
}
  
s_radl::s_radl(int BuffSize) {
  memset(this, 0, sizeof(s_radl));
  if (!BuffSize) buffsize = RADLBUFFSIZE;
  else buffsize = BuffSize;
  data = new unsigned char[buffsize];
}

s_radl::~s_radl() {
  if (Values) delete[] Values;
  if (data) delete[] data;
}

void s_radl::resize(int newsize, bool copy)
{
  if (newsize > buffsize)
    {
      if (data)
	{
	  unsigned char *newdata = new unsigned char[newsize];
	  memset(newdata, 0, newsize);
	  if (copy)
	    for (int x = 0; x < data_size; x++)
	      *(newdata+x) = *(data+x);
	  delete[] data;
	  data = newdata;
	}
      if (Values)
	{
	  float *newValues = new float[newsize];
	  memset(newValues, 0, newsize * sizeof(float));
	  if (copy)
	    for (int x = 0; x < data_size; x++)
	      *(newValues+x) = *(Values+x);
	  delete[] Values;
	  Values = newValues;
	}
      buffsize = newsize;
    }
}

void s_radl::Clear() {	// zero out this radial
  az = az1 = az2 = el = el1 = el1 = el2 = 0;
  startrng = undefinedrng = 0;
  data_size = 0;
  rngres = 0.0;
  scan_type = PPI;  // default to PPI type
  mode = INDEX;
  if (data) memset(data, 0, buffsize);
  if (Values) memset(Values, 0, buffsize * sizeof(float));
}
    		
int s_radl::IndexAtRange(float rng, float *idxrng) {
  float frng;
  int idx;
  if (!data_size) return -1;
  frng = int(rng * 1000);// convert rng to metres
  frng -= startrng;	// 
  if (frng < 0) return 0;	// if < 0,  return 0
  if (frng > (data_size * rngres)) {
    if (idxrng) {
      *idxrng = (startrng + ((data_size - 1) * rngres)) / 1000.;
      return data_size-1;	// if max,  return max
    }
    else return -1;
  }
  idx = int(frng / rngres);
  if (idxrng) *idxrng = (startrng + (idx * rngres)) / 1000.;
  if (idx < buffsize) return idx;
  else return -1;
}


float s_radl::RangeAtIndex(int idx) {
  if (!data_size) return -1;
  return (startrng + (idx * rngres)) / 1000.0;
}

uchar s_radl::DataAtRange(float rng) {
  int idx;
  if ((idx = IndexAtRange(rng)) != -1) return data[idx];
  else return 0;
}

float s_radl::ValueAtRange(float rng)    // return the Value at the given km rng
{
  uchar dataval = DataAtRange(rng);

  if (LvlTbl)
    return LvlTbl->val(dataval);
  else return 0;
}

float s_radl::ValueAtIndex(int idx)    // return the Value at the given km rng
{
  uchar dataval = data[idx]; 
  if (LvlTbl)
    return LvlTbl->val(dataval);
  else return 0;
}


int s_radl::CellsBtwnRngs(float rng1, float rng2)    // return the number of cells between the rngs
{
  int idx1 = IndexAtRange(rng1);
  int idx2 = IndexAtRange(rng2);

  if ((idx1 == -1) || (idx2 == -1))
    return 0;
  return abs(idx2 - idx1) + 1;
}

int s_radl::GetCellsBtwnRngs(float rng1, float rng2, uchar *buff, int maxcount)    // return the number of cells between the rngs
{
  int count = CellsBtwnRngs(rng1, rng2);
  if (count > maxcount)
    count = maxcount;
  int cellcount = count;
  if (count == 0)
    return count;
  int copy_inc = 1;
  uchar *inptr = &(data[IndexAtRange(rng1)]);

  if (rng1 > rng2) 
    {
      copy_inc = -1;    // need to step backwards in rng
    }
  uchar *outptr = buff;

  while (count-- > 0)
    {
      *outptr = *inptr;
      inptr += copy_inc;
      outptr++;
    }
  return cellcount;
}

int s_radl::GetCellsBtwnRngs(float rng1, float rng2,  vector<uchar> &buff, int buffpos, int maxcount)    // return the number of cells between the rngs
{
  int count = CellsBtwnRngs(rng1, rng2);
  if (count > maxcount)
    count = maxcount;
  int cellcount = count;
  if (count == 0)
    return count;
  int copy_inc = 1;

  uchar *inptr = &(data[IndexAtRange(rng1)]);

  if (rng1 > rng2) 
    {
      copy_inc = -1;    // need to step backwards in rng
    }

  while (count-- > 0)
    {
      buff[buffpos] = *inptr;
      inptr += copy_inc;
      buffpos++;
    }
  return cellcount;
}



/* 
 * Convert the radial to a tangent plane (0deg el) radial
 * Use a maxima fn where multiple src values map to one dest value 
 */
void s_radl::TanPlaneRadl(float *cosel) {
  float CosEl;
  int srcidx, destidx;
    
    
  if (!data_size) return;

  char  *tempdata = new char[data_size];
  float *tempval = 0;
  if (Values) tempval = new float[data_size];

  if (!cosel) CosEl = cos(el*DEG2RAD);
  else CosEl = *cosel;
  memset(tempdata, 0, data_size);
  if (Values) memset(tempval, 0, sizeof(float)*data_size);
  for (srcidx = 0; srcidx < data_size; srcidx++) {
    destidx = int(srcidx * CosEl);      
    if (Values) {
      if (Values[srcidx] > tempval[destidx]) {
	tempval[destidx] = Values[srcidx];
	tempdata[destidx] = data[srcidx];
      }
    }
    if (data[srcidx] > tempdata[destidx])
      tempdata[destidx] = data[srcidx];
  }
  data_size = destidx + 1;
  memcpy(data, tempdata, data_size);
  if (Values) memcpy(Values, tempval, data_size * sizeof(float));
  if (tempval) delete[] tempval;
  if (tempdata) delete[] tempdata;
}
	
int s_radl::PadRadl(int padsize) {  // pad radial to given range
  int padno = 0;
    
  if (padsize <= data_size) return data_size;
  if (padsize > buffsize) padsize = buffsize;
  padno = padsize - data_size;
  memset(&data[data_size], 0, padno);
  if (Values) 
    for (int x = data_size; x < padsize; x++)
      Values[x] = 0;
  data_size = padsize;
  return padsize;
}    

void s_radl::RngRes2000to1000() {
  unsigned char *tempbuff = new unsigned char[buffsize];
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = 0;
  if (int(rngres) != 2000) {
    printf("s_radl::RngRes2000to1000 ERROR Current rngres = %1.2f\n", rngres);
    return;
  }
  memcpy(tempbuff, data, data_size);	// copy 2000m data to tempbuff
  ipdata = tempbuff;	    // 2000m data
  opdata = data;	    // 1000m data
  while ((ipcount < data_size) && (opcount < buffsize)) {
    *opdata = *ipdata;  // copy 2000m data twice 
    opdata++;
    *opdata = *ipdata;
    opdata++;
    ipdata++;
    ipcount++;
    opcount += 2;
  }
  data_size = opcount;
  rngres = 1000;	
  delete[] tempbuff;
}

bool s_radl::RngResCanConvert(float op_res, float rng_res) // retrn true if able to convert to op_res
{                                           // 
  float factor;
  if (rng_res == 0) rng_res = rngres;
  if (rng_res == 0) return false;
  if (op_res == rng_res) // nothing to do
    return true;

  else if (op_res > rng_res)
    {
      factor = op_res / rng_res;
    }
  else    
    {
      factor = rng_res / op_res;
    }
  int ifactor = int(factor);
  if (ifactor == factor)
    return true;
  else
    return false;
}

bool s_radl::RngResConvert(float op_res, 
			   e_rngres_reduce_mode reduce_mode,          // convert to op_res using reduce/increase mode
			   e_rngres_increase_mode increase_mode)     // return false if op_res not eact multiple of current res
{
  if (rngres == 0) return false;
  if (!RngResCanConvert(op_res))
    return false;
  if (op_res == rngres) // nothing to do
    return true;
  else if (op_res < rngres)
    {
      float increase_factor =  rngres / op_res;
      RngResIncrease(int(increase_factor), increase_mode);
      return true;
    }
  else 
    {
      float reduce_factor = op_res / rngres;
      RngResReduce(int(reduce_factor), reduce_mode);
      return true;
    }
}      

void s_radl::RngResReduce(int reduce_factor, e_rngres_reduce_mode mode) {
  if (reduce_factor < 2) {
    return;
  }
  switch (mode) {
  case RRR_MAX :
    RngResReduceMax(reduce_factor);
    break;
  case RRR_MIN :
    RngResReduceMin(reduce_factor);
    break;
  case RRR_MED :
    RngResReduceMed(reduce_factor);
    break;
  case RRR_AVG :
    RngResReduceAvg(reduce_factor);
    break;
  case RRR_PWR :
    RngResReducePwrAvg(reduce_factor);
    break;
  default:
    RngResReduceMax(reduce_factor);    
  }
}

void s_radl::RngResReduceMax(int reduce_factor) {
  if (reduce_factor < 2) {
    return;
  }
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = data_size;
  int repeat_count;
  unsigned char maxval;

  ipdata = data;	    // rngres reduction so ipdata pointer will be beyond opdata pointer
  opdata = data;	    
  while (ipcount) {
    repeat_count = reduce_factor;
    maxval = 0;
    while (repeat_count && ipcount)
      {
	if (*ipdata > maxval) maxval = *ipdata;
	ipdata++;
	ipcount--;
	repeat_count--;
      }
    *opdata = maxval;  // maxval to output 
    opdata++;
    opcount++;
  }
  data_size = opcount;
  rngres = rngres*reduce_factor;   // reducing range res, i.e. bigger range res value	
}

void s_radl::RngResReduceMin(int reduce_factor) {
  if (reduce_factor < 2) {
    return;
  }
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = data_size;
  int repeat_count;
  unsigned char minval;

  ipdata = data;	    // rngres reduction so ipdata pointer will be beyond opdata pointer
  opdata = data;	    
  while (ipcount) {
    repeat_count = reduce_factor;
    minval = 255;
    while (repeat_count && ipcount)
      {
	if (data_type == e_vel)
	  {
	    if (*ipdata && (*ipdata < minval)) minval = *ipdata;	  // ignore 0(no echo)  value 
	  }
	else
	  {
	    if (*ipdata < minval) minval = *ipdata;
	  }
	ipdata++;
	ipcount--;
	repeat_count--;
      }
    if ((data_type == e_vel) && (minval == 255)) // no value defined for vel, set to 0
      minval = 0;
    *opdata = minval;  // maxval to output 
    opdata++;
    opcount++;
  }    
  data_size = opcount;
  rngres = rngres*reduce_factor;   // reducing range res, i.e. bigger range res value	
}

void s_radl::RngResReduceMed(int reduce_factor) {
  if (reduce_factor < 2) {
    return;
  }
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = data_size;
  int repeat_count;
  unsigned char maxval, minval;
  

  ipdata = data;	    // rngres reduction so ipdata pointer will be beyond opdata pointer
  opdata = data;	    
  while (ipcount) {
    repeat_count = reduce_factor;
    maxval = 0;
    minval = 255;
    while (repeat_count && ipcount)
      {
	if (data_type == e_vel)
	  {
	    if (*ipdata && (*ipdata > maxval)) maxval = *ipdata; // ignore 0(no echo)  value
	    if (*ipdata && (*ipdata < minval)) minval = *ipdata;
	  }
	else
	  {
	    if (*ipdata > maxval) maxval = *ipdata;
	    if (*ipdata < minval) minval = *ipdata;
	  }
	ipdata++;
	ipcount--;
	repeat_count--;
      }
    if ((data_type == e_vel) && (minval == 255)) // no value defined for vel, set to 0
      *opdata = 0;                 // no values define, set op to 0
    else
      *opdata = (unsigned char)(int(maxval + minval)/2);  // maxval to output 
    opdata++;
    opcount++;
  }
  data_size = opcount;
  rngres = rngres*reduce_factor;   // reducing range res, i.e. bigger range res value	
}

void s_radl::RngResReduceAvg(int reduce_factor) {
  if (reduce_factor < 2) {
    return;
  }
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = data_size;
  int repeat_count;
  int sum, count;
  

  ipdata = data;	    // rngres reduction so ipdata pointer will be beyond opdata pointer
  opdata = data;	    
  while (ipcount) {
    repeat_count = reduce_factor;
    sum = count = 0;
    while (repeat_count && ipcount)
      {
	if (data_type == e_vel)
	  {
	    if (*ipdata)
	      {
		sum += *ipdata;
		count++;
	      }
	  }
	else
	  {
	    sum += *ipdata;
	    count++;
	  }
	ipcount--;
	ipdata++;
	repeat_count--;
      }
    if (count > 0)
      *opdata = (unsigned char)(sum/(count));  // maxval to output 
    else 
      *opdata = 0;
    opdata++;
    opcount++;
  }
  data_size = opcount;
  rngres = rngres*reduce_factor;   // reducing range res, i.e. bigger range res value	
}

void s_radl::RngResReducePwrAvg(int reduce_factor) {
  //like RngResReduceAvg but averages over the power, not the dBZ (for refl).  SD 20jun07
  if (reduce_factor < 2) {
    return;
  }
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = data_size;
  int repeat_count;
  int sum, count;
  

  ipdata = data;	    // rngres reduction so ipdata pointer will be beyond opdata pointer
  opdata = data;
  float dbz_value, pwr_value, sumf;
  
  
  while (ipcount) {
    repeat_count = reduce_factor;
    sum = count = 0;
    sumf = 0.0;
    
    while (repeat_count && ipcount)
      {
	if (data_type == e_vel)
	  {
	    if (*ipdata)
	      {
		sum += *ipdata;
		count++;
	      }
	  }
	else
	  {
	    if (*ipdata)  //ignore zeroes here too
	      {
		dbz_value = LvlTbl->val(*ipdata);
		pwr_value = exp(dbz_value);  //it doesnt matter if this is correct exponent, we convert avg back in a moment.
		sumf += pwr_value;
		count++;
	      }
	  }
	ipcount--;
	ipdata++;
	repeat_count--;
      }
    if (count > 0) {
      //*opdata = (unsigned char)(sum/(count));  // maxval to output 
      dbz_value = log(sumf/count);
      *opdata = LvlTbl->threshold(dbz_value);
    }
    else 
      *opdata = 0;
    opdata++;
    opcount++;
  }
  data_size = opcount;
  rngres = rngres*reduce_factor;   // reducing range res, i.e. bigger range res value	
}

void s_radl::RngResIncrease(int increase_factor, e_rngres_increase_mode mode) {
  if (increase_factor < 2) {
    return;
  }
  switch (mode) {
  case RRI_DUP :
  case RRI_INTERP :
  default :
    RngResIncreaseDup(increase_factor);
    break;
  }
}


// interp mode not implemented yet
void s_radl::RngResIncreaseInterp(int increase_factor) {
  RngResIncreaseDup(increase_factor);
}

void s_radl::RngResIncreaseDup(int increase_factor) {
  if (increase_factor < 2) {
    return;
  }
  unsigned char *tempbuff = new unsigned char[buffsize];
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = 0;
  int repeat_count;
  memcpy(tempbuff, data, data_size);	// copy ip data to tempbuff
  ipdata = tempbuff;	    // ip data
  opdata = data;	    // op data
  while ((ipcount < data_size) && (opcount < buffsize)) {
    repeat_count = increase_factor;
    while (repeat_count && (opcount < buffsize))
      {
	*opdata = *ipdata;  // copy ip data n times 
	opdata++;
	opcount++;
	repeat_count--;
      }
    ipdata++;
    ipcount++;
  }
  data_size = opcount;
  rngres = rngres/increase_factor;	
  delete[] tempbuff;
}

void s_radl::ThresholdFloat(float *floatarray, int size, 
			    LevelTable *thresh_table) {
  int	x = 0;
  float	*inval;
  unsigned char	*outval;

  if (size > buffsize) size = buffsize;    
  if (!thresh_table) thresh_table = LvlTbl;
  if (!thresh_table) return;
  if (floatarray)
    inval = floatarray;
  else {
    inval = Values;	// if floatarray not passed, assume Values should be used
  }
  if (!size)
    size = data_size;
  if (!inval) return;
  outval = data;
  for (x = 0; x < size; x++) {
    *outval = thresh_table->threshold(*inval);
    outval++;
    inval++;
  }
  outval = &data[size-1];  // point to last value
  while ((size > 0) && (*outval == 0)) {
    size--;		    
    outval--;
  }
  data_size = size;
}

void s_radl::ThresholdFloat(ushort *ushortarray, int size, 
			    LevelTable *thresh_table) {
  int	x = 0;
  ushort *inval;
  unsigned char	*outval;

  if (size > buffsize) size = buffsize;    
  if (!thresh_table) thresh_table = LvlTbl;
  if (!thresh_table) return;
  if (ushortarray)
    inval = ushortarray;
  if (!inval) return;
  if (!size)
    size = data_size;
  outval = data;
  for (x = 0; x < size; x++) {
    *outval = thresh_table->threshold(*inval);
    outval++;
    inval++;
  }
  outval = &data[size-1];  // point to last value
  while ((size > 0) && (*outval == 0)) {
    size--;		    
    outval--;
  }
  data_size = size;
}

void s_radl::indexToFloat(float *floatarray, int size, 
			  LevelTable *thresh_table) {
  int	x = 0;
  float	*outval;
  unsigned char	*inval;

  if (size > buffsize) size = buffsize;    
  if (!thresh_table) thresh_table = LvlTbl;
  if (!thresh_table) return;
  if (floatarray)
    outval = floatarray;
  else {
    if (!Values)
      Values = new float[buffsize];
    outval = Values;	// if floatarray not passed, assume Values should be used
    if (!size)
      size = data_size;
	
  }
  if (!outval) return;

  // minimise radial length to last non-zero
  inval = &data[size-1];  // point to last value
  while ((size > 0) && (*inval == 0)) {
    size--;		    
    inval--;
  }
  data_size = size;
  inval = data;
  for (x = 0; x < size; x++) {
    *outval = thresh_table->val(*inval);
    outval++;
    inval++;
  }
}

int s_radl::Encode16lvlAz(uchar *outstring, int maxopsize) {
  uchar *strpnt = outstring;
  int len = 0;
  TruncateData();
  sprintf((char *)strpnt, "%%%05.1f", float(az / 10.0));
  strpnt += (len = strlen((char *)strpnt));
  len += DeltaASCII(data, strpnt, data_size, maxopsize);
  return len;
}

int s_radl::Encode16lvlEl(uchar *outstring, int maxopsize) {
  uchar *strpnt = outstring;
  int len = 0;
  TruncateData();
  sprintf((char *)strpnt, "%%%04.1f", (float)el / 10.0);
  strpnt += (len = strlen((char *)strpnt));
  len += DeltaASCII(data, strpnt, data_size, maxopsize);
  return len;
}

int s_radl::EncodeBin(uchar *outstring, int maxopsize) {
  uchar *strpnt = outstring;
  int len = 0;

  TruncateData();
  sprintf((char *)strpnt, "@%05.1f,%05.1f,000=", 
	  (float)az / 10.0, (float)el / 10.0);
  strpnt += (len = strlen((char *)strpnt));
  len += EncodeBinaryRadl(data, (unsigned char *)strpnt, data_size, maxopsize);
  if (len < 21)
    fprintf(stderr, "s_radl::EncodeBin ERROR - SHORT RADL DETECTED - length = %d\n", len);
  return len;
}

int s_radl::Encode6lvlAz(uchar *outstring, int maxopsize) {
  uchar *strpnt = outstring;
  int len = 0;

  TruncateData();
  sprintf((char *)strpnt, "%%%03d", az / 10);
  strpnt += (len = strlen((char *)strpnt));
  len += SixLevelASCII(data, (char *)strpnt, data_size, maxopsize);
  return len;
}

int s_radl::Encode6lvlEl(uchar *outstring, int maxopsize) {
  uchar *strpnt = outstring;
  int len = 0;

  TruncateData();
  sprintf((char *)strpnt, "%%%04.1f", (float)el / 10.0);
  strpnt += (len = strlen((char *)strpnt));
  len += SixLevelASCII(data, (char *)strpnt, data_size, maxopsize);
  return len;
}

int s_radl::EncodeAz(uchar *outstring, int maxopsize) {
  if (numlevels == 6)
    return Encode6lvlAz(outstring, maxopsize);
  else if (numlevels <= 160)
    return Encode16lvlAz(outstring, maxopsize);
  else if (numlevels == 256)
    return EncodeBin(outstring, maxopsize);
  return 0;
}

int s_radl::EncodeEl(uchar *outstring, int maxopsize) {
  if (numlevels == 6)
    return Encode6lvlEl(outstring, maxopsize);
  else if (numlevels <= 160)
    return Encode16lvlEl(outstring, maxopsize);
  else if (numlevels == 256)
    return EncodeBin(outstring, maxopsize);
  return 0;
}

void s_radl::TruncateData()
{
  if (data_size == 0) return;
  unsigned char* lcldata = &data[data_size-1];
  while(data_size) {
    if(*lcldata--)
      break;
    data_size--;
  }
}

void s_radl::AddThisToHistogram(histogramClass *hist)
{
  if (!hist) return;
  hist->addHistogram(data, buffsize);
}

int s_radl::MaxValIdx(int *maxidx, 
		      float *maxrng, 
		      int *maxval, 
		      float *maxfval)
{
  unsigned char *p_data = data;
  unsigned char maxvalue = 0;
  int idx = 0, MaxIdx = -1;
    
  while (idx < data_size)
    {
      if (*p_data > maxvalue)
	{
	  maxvalue = *p_data;
	  MaxIdx = idx; 
	}
      idx++;
      p_data++;
    }
  if (maxidx)
    *maxidx = MaxIdx;
  if (maxrng)
    *maxrng = RangeAtIndex(MaxIdx);
  if (maxval)
    *maxval = maxvalue;
  if (maxfval && LvlTbl)
    *maxfval = LvlTbl->val(maxvalue);
  return MaxIdx;
}

int s_radl::MaxFValIdx(int *maxidx, 
		       float *maxrng, 
		       int *maxval, 
		       float *maxfval)
{
  if (!Values)
    return MaxValIdx(maxidx, maxrng, maxval, maxfval);

  float *p_Values = Values;
  float maxvalue = 0.0;
  int idx = 0, MaxIdx = -1;    

  while (idx < data_size)
    {
      if (*p_Values > maxvalue)
	{
	  maxvalue = *p_Values;
	  MaxIdx = idx; 
	}
      idx++;
      p_Values++;
    }
  if (maxidx)
    *maxidx = MaxIdx;
  if (maxrng)
    *maxrng = RangeAtIndex(MaxIdx);
  if (maxval && LvlTbl)
    *maxval = LvlTbl->threshold(maxvalue);
  if (maxfval)
    *maxfval = maxvalue;
  return MaxIdx;
}

s_radl_node::s_radl_node(int BuffSize)
{
  RadlData = new s_radl(BuffSize);
}

s_radl_node::~s_radl_node()
{
  if (RadlData)
    delete RadlData;
}

rdr_scan_linebuff::rdr_scan_linebuff(int buffsz)
{
  lb_max = buffsz-1;
  line_buff = new char[buffsz];
  reset();
  new_scan_ofs = 0;
  strcpy(logmssg, "");
}

rdr_scan_linebuff::~rdr_scan_linebuff()
{
  if (line_buff)
    delete[] line_buff;
}

void rdr_scan_linebuff::setLogMssg(char *_logmssg)
{
  if (_logmssg)
    strncpy(logmssg, _logmssg, 127);
  else
    strcpy(logmssg, "");
}  

void rdr_scan_linebuff::reset(char *_logmssg)
{
  lb_size = 0;
  isBinRadl = isRadl = isComment = EOL =
    termCh1 = termCh2 = Overflow = false;   
  lb_pnt = line_buff;
  setLogMssg(_logmssg);
}



void rdr_scan_linebuff::ensureTerminated()
{
  if ((line_buff[lb_size-1] != 0) &&	// if ln not null terminated
      (lb_size < (lb_max))) // check there is space for another char
    {
      line_buff[lb_size] = 0;	// null terminate
      lb_size++;
    }
}

bool rdr_scan_linebuff::shouldDoMssgCheck()
{
  return (strstr(line_buff, "DBZLVL:") ||
      strstr(line_buff, "DBZCALDLVL:") ||
	  strstr(line_buff, "DIGCALDLVL:"));
}

bool rdr_scan_linebuff::addchar_parsed(char c) // add char and test for EOL
{
  char *tempptr;
  char tempstr[256];


  if (IsFull()) return false;
  if (lb_size == 0)	// adding first char in line
    {
      if (isspace(c) || iscntrl(c))	// don't add leading white space
	//		if (!isalnum(c))// first must be alphanumeric
	return true;
      switch (c) {
      case '@' : isBinRadl = true; isRadl = true;
	break;
      case '%' : isRadl = true;
	break;
      case '/' : isComment = true;
	break;
      }
    }
  if (!isBinRadl)	// test for EOL for non BinRadl	line
    {
      switch (c) {
      case CR :			// CR,LF,CTRLZ,'#',0 all treated as line
      case LF :			// terminators
      case '#':
      case CTRLZ :
      case CTRLC :
      case EOT :
      case 0 :
	addc(0);		// use null as std line term
	EOL = true;
	if ((isRadl || shouldDoMssgCheck()) && checkRadialMSSG) // check that there isn't an embedded MSSG string 
	  {
	    tempptr = strstr(line_buff, "MSSG:");
	    if (!tempptr)
	      tempptr = strstr(line_buff, "END STATUS");  
	    if (tempptr)
	      {
		if (isRadl)
		  sprintf(tempstr, "rdr_scan_linebuff::addchar_parsed: %s FOUND MSSG: in radial - %-128s", 
			  logmssg, tempptr);
		else
		  sprintf(tempstr, "rdr_scan_linebuff::addchar_parsed: %s FOUND MSSG: in header - %-128s",
			  logmssg, tempptr);
		RapicLog(tempstr, LOG_ERR);
		//		RapicLog(line_buff, LOG_INFO);
		EOL = false;    // EOL came from MSSG: string, cancel it
		lb_size = (tempptr - line_buff); // set buffer pnt back to where MSSG started
		lb_pnt = tempptr;
	      }
	  }
	break;
      default:					// don't add leading white space
	addc(c);
	if (IsFull(1)) { 	// leave space for term null
	  addc(0);		// use null as std line term
	  printf("rdr_scan_linebuff::addchar_parsed: LINE BUFFER OVERFLOW");
	  EOL = Overflow = true;
	}
	break;
      }
    }
  else    // is a binradl, check for termination
    {
      addc(c);
      if ((c == 0) && (lb_size > 19))
	{
	  if (termCh1) termCh2 = true;    // already have first NULL, this is second
	  else termCh1 = true;	    // this is first null
	}
      else termCh1 = termCh2 = false;
      if (termCh2 || IsFull())
	{
	  EOL = true;
	  if (!termCh2 && IsFull())
	    {
	      printf("rdr_scan_linebuff::addchar_parsed: BINARY RADIAL LINE BUFFER OVERFLOW");
	      Overflow = true;    // buffer filled without true EOL
	    }
	}
    }
  return Overflow;
}

rdr_scan_array::rdr_scan_array(int _rngdim, int _azdim, 
			       int _tiltdim)
{
  data = NULL;
  rngDim = azDim = tiltDim = 0;
  rngRes = azRes = 0.0;
  startRng = endRng = 0.0;
  startAz  = 0.0;
  endAz = 360.0;
  fullCircle = true;
  dataType = e_refl;
  setDims(_rngdim, _azdim, _tiltdim);
}

rdr_scan_array::~rdr_scan_array()
{
  if (data)
    delete[] data;
}

void rdr_scan_array::setDims(int _rngdim, int _azdim, int _tiltdim)
{
  if (data &&
      ((rngDim * azDim * tiltDim) != (_rngdim * _azdim * _tiltdim)))
    {
      delete[] data;
      data = NULL;
    }
  if (!data)
    data = new uchar[_rngdim * _azdim * _tiltdim];
  rngDim = _rngdim;
  azDim = _azdim;
  tiltDim = _tiltdim;
  elTiltTable.resize(tiltDim);
}

void rdr_scan_array::clearData()
{
  if (data)
    memset(data, 0, rngDim * azDim * tiltDim);
}

rdr_scan::rdr_scan(void *creator, char *str, 
		   rdr_scan *RootScan, bool appendtoroot) {
  //_next = _prev = 0;				// new scan last in list
	
  //	debug = Debug;
  debug = false;
  if (!RootScan) {
    data_buff = new exp_buff(2048);
    rootscan = this;
    dbuff_rd.open(data_buff);	// set up buffer reader
    linebuff = new rdr_scan_linebuff;
    lockwaitmax = 1000;	// wait max 10secs for lock
    lock = new spinlock("rdr_scan->lock", lockwaitmax);
    if (ScanRegistry) ScanRegistry->add_new_scan(this, str);
    nextinset = previnset = 0;
    lastscan = 0;
  }
  else {
    rootscan = RootScan;
    data_buff = 0;
    dbuff_rd.open(rootscan->data_buff);	// set up buffer reader
    linebuff = 0;
    lock = rootscan->lock;
    lockwaitmax = rootscan->lockwaitmax;// wait time lock
    if (appendtoroot)
      {
	if (!rootscan->lastscan)
	  rootscan->lastscan = rootscan;
	nextinset = NULL;
	previnset = rootscan->lastscan;
	lastscan = 0;
	rootscan->lastscan->nextinset = this;
	rootscan->lastscan = this;
      }
    else
      {
	nextinset = previnset = 0;
	lastscan = 0;
      }
  }
  total_scans++;
  header_start = 0;	// start of data in image buffer
  data_start = 0;		// fix up later
  data_valid = header_valid = scan_complete = FALSE;
  faultno = 0;
  setlabel[0] = 0;
  // LvlTbl = 0;
  for (int x = e_refl; x < e_dt_max; x++)
    _LvlTbls[x] = 0;
  clearAirMode = false;
  NumLevels = 6;
  faultno = 0;
  UserCount = 0;
  if (debug)
    fprintf(stderr, "rdr_scan::rdr_scan - Created by %s\n", str);
  IncUserCount(creator, str);
  RadlPointers = 0;
  UseRadlPnt = TRUE;
  //  thisscan = this;				// default to single scan setup
  //  thisscanno = 0;
  data_source = DB;
  nullcount = 0;
  ShownToScanClients = FALSE;
  StoredInDB = FALSE;
  DBDuplicate = ScanMngDuplicate = FALSE;
  StartTm = EndTm = 0;
  headerAddedTime = 0;
  rxTimeStart = 0;
  rxTimeEnd = rxTimeSetEnd = 0; // default to current time
  scanCreator = creator;
  scanCreatorString = str;
  creatorFieldVersion = 0.0;
  set_dflt();			// set default values for scan params
  delstate = NO_DEL;
  productScanLock = NULL;
  write_error = 0;
  scanArray = NULL;
  _volScanArray = NULL;
}

// rdr_scan* rdr_scan::reset_scan() {
//   rdr_scan *temp;
//   thisscan = rootscan;
//   if (thisscan && thisscan->scan_complete)
//     temp = thisscan;
//   else temp = 0;
//   thisscanno = 0;
//   return temp;
// }

// rdr_scan* rdr_scan::this_scan() {
//   rdr_scan *temp;
//   if (thisscan && thisscan->scan_complete)
//     temp = thisscan;
//   else temp = 0;
//   return temp;
// }

// rdr_scan* rdr_scan::next_scan() {
//   rdr_scan *temp;
//   if (thisscan) {
//     thisscan = thisscan->nextinset;
//     thisscanno++;
//   }
//   if (thisscan && thisscan->scan_complete)
//     temp = thisscan;
//   else temp = 0;
//   return temp;
// }

rdr_scan* rdr_scan::gotoScan(int n) 
{
  if (n >= num_scans)
    return NULL;

  bool	lock_ok = get_lock();
  if (!lock_ok)
    fprintf(stderr,"rdr_scan::gotoScan - get_lock FAILED\n");	
  rdr_scan *temp = rootscan;
  while (temp && n)
    {
      temp = temp->NextScan(temp);
      n--;
    }
  if (lock_ok) rel_lock();
  return temp;
}

// rdr_scan* rdr_scan::goto_scan(int n) {
//   rdr_scan *temp;
//   int	lock_ok = 0;

//   if (this != rootscan) {
//     fprintf(stderr,"rdr_scan::goto_scan - Called for CHILD, calling rootscan->goto_scan\n");
//     if (rootscan)
//       return rootscan->goto_scan(n);
//     else 
//       return 0;
//   }
//   if (!(lock_ok = get_lock()))
//     fprintf(stderr,"rdr_scan::goto_scan - get_lock FAILED\n");	
//   if (!n) reset_scan();
//   if (n != thisscanno) {
//     if (n > thisscanno) {
//       while (thisscan && (thisscanno < n))
// 	next_scan();
//     }
//     else {
//       reset_scan();
//       while (thisscan && (thisscanno < n))
// 	next_scan();
//     }
//   }
//   if (thisscan && thisscan->scan_complete)
//     temp = thisscan;
//   else temp = 0;
//   if (lock_ok) rel_lock();
//   return temp;
// }

// rdr_scan* rdr_scan::setRootScan() {
//   thisscan = rootscan;
//   return rootscan;
// }

// rdr_scan* rdr_scan::setNextInSet() {
//   rdr_scan* _next = thisscan;
//   if (thisscan) _next = thisscan->nextinset;
//   thisscan = _next;
//   return thisscan;
// }

// rdr_scan* rdr_scan::nextInSet() {
//   rdr_scan* _next = thisscan;
//   if (thisscan) _next = thisscan->nextinset;
//   return _next;
// }

// rdr_scan* rdr_scan::setLastScan() {
//   if (lastscan)
//     thisscan = lastscan;
//   else
//     thisscan = this;
//   return thisscan;
// }

rdr_scan* rdr_scan::rootScan() {
  return rootscan;
}

rdr_scan* rdr_scan::NextScan(rdr_scan *scan) {
  if (scan) return scan->nextinset;
  else return NULL;
}

rdr_scan* rdr_scan::lastScan() {
  if (lastscan)
    return lastscan;
  else
    return this;
}

bool rdr_scan::load_scan(int scan_ofs) {
  header_start = scan_ofs;	// start of data in image buffer
  data_start = scan_ofs;		// fix up later
  check_data();			// find data offset, count radls etc.
  return data_valid;
}

rdr_scan::~rdr_scan() {
  rdr_scan *delscan,*nextscan;
  int	lock_ok = 0;
  char	tempstr[256];

  if (delstate != NO_DEL)
    {
      switch (delstate)
	{
	case DELETING:
	  sprintf(tempstr,"rdr_scan::~rdr_scan() ERROR ON ENTRY delstate is DELETING");
	  RapicLog(tempstr, LOG_CRIT);
	  break;
	case DELETED:
	  sprintf(tempstr,"rdr_scan::~rdr_scan()  ERROR ON ENTRY delstate is DELETED");
	  RapicLog(tempstr, LOG_CRIT);
	  break;
	default:
	  break;
	}
    }
  delstate = DELETING;
  
  if (ScanRegistry) ScanRegistry->remove_scan(this);

  if (useScanUserList)
    ClearUserList(true);	// if users remain on list print them out
  if (this == rootscan) {	// if rootscan, delete ALL scans in set
    if (lock && !(lock_ok = get_lock()))
      {
	RapicLog("rdr_scan::~rdrscan - get_lock FAILED,  RETRYING", LOG_ERR);	
	if (!(lock_ok = get_lock()))
	  RapicLog("rdr_scan::~rdrscan - get_lock RETRY FAILED***", LOG_CRIT);	
      }
    /*
      if (_next) _next->_prev = _prev;	// take out of root scan list
      if (_prev) _prev->_next = _next;
    */
    if (UserCount != 0) {
      sprintf(tempstr,"rdr_scan::~rdr_scan()  ROOT - SERIOUS ERROR: USERCOUNT=%d", UserCount);
      RapicLog(tempstr, LOG_CRIT);
    }
    delscan = nextinset;
    while (delscan) {
      nextscan = delscan->nextinset;
      if (delscan->ShouldDelete(scanCreator, "rdr_scan::~rdr_scan"))	
	delete delscan;
      delscan = nextscan;
    }
    delAllProductScans(this, "rdr_scan::~rdr_scan");
    if (productScanLock)
      {
	delete productScanLock;
	productScanLock = 0;
      }
    for (int x = e_refl; x < e_dt_max; x++)
      if (_LvlTbls && _LvlTbls[x]) {
	if (!_LvlTbls[x]->getGlobal())
	  delete _LvlTbls[x];
	_LvlTbls[x] = 0;
      }
    if (linebuff) {
      delete linebuff;
      linebuff = 0;
    }
    if (data_buff) {
      delete data_buff;
      data_buff = 0;
    }
    if (RadlPointers) {
      if (FreeListMng) 
	FreeListMng->RadlPntFreeList->StoreRadlPnt(RadlPointers);
      else 
	delete RadlPointers;
      RadlPointers = 0;
    }

    /*
      if (filteredScanSet && filteredScanSet->ShouldDelete(this, "rdr_scan::~rdr_scan"))
      delete filteredScanSet;
      if (filter) {
      delete filter;
      filter = NULL;
      }
      if (filterlock) {
      delete filterlock;
      filterlock = NULL;
      }
    */

    if (_volScanArray)
      delete _volScanArray;
    if (lock_ok) rel_lock();
    del_lock();
  }
  else {		// not root scan, just take this scan out of scan set list
    if (UserCount != 0) {
      sprintf(tempstr,"rdr_scan::~rdr_scan()  CHILD - SERIOUS ERROR: USERCOUNT NOT ZERO (%d)", UserCount);
      RapicLog(tempstr, LOG_CRIT);
    }
    if (nextinset) nextinset->previnset = previnset;
    if (previnset) previnset->nextinset = nextinset;
    for (int x = e_refl; x < e_dt_max; x++)
      if (_LvlTbls &&_LvlTbls[x]) {
	if (!_LvlTbls[x]->getGlobal())
	  delete _LvlTbls[x];
	_LvlTbls[x] = 0;
      }
    if (linebuff) {
      delete linebuff;
      linebuff = 0;
    }
    if (RadlPointers) {
      if (FreeListMng) 
	FreeListMng->RadlPntFreeList->StoreRadlPnt(RadlPointers);
      else 
	delete RadlPointers;
      RadlPointers = 0;
    }
  }
  total_scans--;
  data_valid = false;
  //	if (adminlist) delete adminlist;
  if (delstate != DELETING)
    {
      switch (delstate)
	{
	case DELETED:
	  sprintf(tempstr,"rdr_scan::~rdr_scan()  ERROR ON EXIT delstate already set to DELETED");
	  RapicLog(tempstr, LOG_CRIT);
	  break;
	default:
	  break;
	}
    }
  if (scanArray)
    delete scanArray;
  delstate = DELETED;
}

void rdr_scan::AddUserToList(void *user, char *str)
{
  if (!useScanUserList) return;
  char tempstr[512];
  if (FindUserNode(user))
    {
      sprintf(tempstr, "rdr_scan::AddUserToList PROBABLE ERROR, User already in list - %s - Not adding again", str);
      RapicLog(tempstr, LOG_CRIT);
      return;
    }
  get_lock();
  ScanUserList[user] = str;
  rel_lock();
}


bool rdr_scan::RemoveUserFromList(void *user, char *str)
{
  if (!useScanUserList) return true;
  char tempstr[512];
  bool result;
  if (!(result = FindUserNode(user, true)))  // find and delete
    {
      sprintf(tempstr, "rdr_scan::RemoveUserFromList SERIOUS ERROR, User not in list - %s\n"
	      "     Users still on list = %d\n", str, int(ScanUserList.size()));
      RapicLog(tempstr, LOG_INFO);
      DumpUserList(NULL, "    ");
    }   
  return result;
}

bool rdr_scan::FindUserNode(void *user, bool remove)
{
  if (!useScanUserList) return true;
  get_lock();
  map<void*, string>::iterator node = ScanUserList.find(user);
  if (node != ScanUserList.end())   // must have found user match
    {
      if (remove)
	{
	  ScanUserList.erase(node);
	}
      rel_lock();
      return true;
    }	    
  else
    {  
      rel_lock();
    return false;
}
}

void rdr_scan::ClearUserList(bool report)
{
  if (!useScanUserList) return;
  char tempstr[512], tempstr2[256];
  get_lock();
  map<void*, string>::iterator first = ScanUserList.begin();
  map<void*, string>::iterator last = ScanUserList.end();
  int sz = ScanUserList.size();
  int count = 1;

  if (report && sz) 
    {
      if (first != last)
	{
	  sprintf(tempstr, "rdr_scan::ClearUserList - Clearing user list for %s\n", ScanString2(tempstr2));
	  RapicLog(tempstr, LOG_INFO);
	  sprintf(tempstr, "    Created by %s\n", scanCreatorString.c_str());
	  RapicLog(tempstr, LOG_INFO);
	}
      while (first != last)
	{
	  if (first->second.c_str())
	    sprintf(tempstr, "     Clearing User %2d - %s\n", count, first->second.c_str());
	  else
	    sprintf(tempstr, "     User String %2d - NO STRING\n", count);
	  RapicLog(tempstr, LOG_INFO);
      first++;
	  count++;
	}
    }   
  ScanUserList.clear();
  rel_lock();
}

void rdr_scan::DumpUserList(FILE *outfile, char *prefix)
{
  if (!useScanUserList) return;
  if (!outfile)
    outfile = stdout;
  char local_prefix[128];
  if (prefix)
    strcpy(local_prefix, prefix);
  else
    strcpy(local_prefix, "");
  get_lock();
  map<void*, string>::iterator first = ScanUserList.begin();
  map<void*, string>::iterator last = ScanUserList.end();
  int count = 1;
  while (first != last)
    {
      if (first->second.c_str())
	fprintf(outfile, "%srdr_scan::DumpUserList - Node String %2d = %s\n", local_prefix, count, first->second.c_str());
      else
	fprintf(outfile, "%srdr_scan::DumpUserList - Node String %2d - NO STRING\n", local_prefix, count);
      first++;
      count++;
    }
  rel_lock();
}

bool rdr_scan::get_lock() {
  // output debug info about locks
  if (debug && lock && lock->hasTimedOut())
    fprintf(stderr, "rdr_scan::get_lock - lock has timed out, timeouts = %d\n", lock->timedOutCount());

  // bool debug = false;
  if (this != rootscan) {
    if (debug)
      fprintf(stderr,"rdr_scan::get_lock - Called for CHILD\n");
    return FALSE;
  }
  if (lock) return lock->get_lock();
  else return TRUE;	
}

void rdr_scan::rel_lock() {
  // bool debug = false;
  if (this != rootscan) {
    if (debug)
      fprintf(stderr,"rdr_scan::rel_lock - Called for CHILD\n");
    return;
  }
  if (lock) lock->rel_lock();
  return;
}

void rdr_scan::del_lock() {	// if scan not subject to further mods, remove lock
				
  if (this != rootscan) {
    fprintf(stderr,"rdr_scan::del_lock - Called for CHILD scan\n");
    return;
  }
  if (!lock) return;
  delete lock;
  lock = 0;
}

rdr_scan* rdr_scan::getTilt(int tiltnum, e_data_type datatype) 
{
  rdr_scan *temp = rootscan;
  rdr_angle tiltangle= rootscan->set_angle;
  int _tiltnum = 0;

  if (!rootscan)
    return NULL;

  while (temp && 
	 ((_tiltnum != tiltnum) ||
	  (datatype != temp->data_type)))
    {
      temp = temp->nextinset;
      if (temp)
	{
	  if (tiltangle != temp->set_angle)
	    {
	      _tiltnum++;
	      tiltangle = temp->set_angle;
	    }
	}
    }
  if (temp && temp->scan_complete)
    return temp;
  else return NULL;
}

rdr_scan* rdr_scan::check_scan_type(e_scan_type tscan_type, e_data_type tdata_type, rdr_angle elev, rdr_angle elev_tol) 
{ 
  //loop through rdr_scan,  return true if scan is found which matches the arguments,
  //esp.  if elevation is found within +- elevation tolerance

  rdr_scan *temp;

  temp = rootscan;
  while (temp)
    {
      if (temp->header_valid &&
	  temp->scan_type == tscan_type &&
	  temp->data_type == tdata_type &&
	  temp->set_angle >= elev - elev_tol &&
	  temp->set_angle <= elev + elev_tol)
	return temp;
      temp = temp->nextinset;
    }
  return (rdr_scan *)NULL;
}

void rdr_scan::set_dflt() {		// set defaults for rdr_scan
  station = 0;
  FirstTm = LastTm = LastScanTm = scan_time_t = time(0);// use current system time as default
  if (rootScan())
    {
      FirstTm = rootScan()->FirstTime();
      LastTm = rootScan()->LastTime();
    }
  UnixTime2DateTime(scan_time_t,year,month,day,hour,min,sec);
  memset(&rdr_params, 0, sizeof(rdr_params));
  strcpy(rdr_params.name,"Undefined");	// distinctive default
  scan_type = PPI;			// default PPI image
  data_type = e_refl;			// default e_reflectivity
  bfdata_type = bf_none;
  radltype = SIMPLE;
  data_fmt = RLE_6L_ASC;		// default ASCII 6 level format
  NumLevels = 6 ;
  angle_res = 10;				// default 1deg res.
  frequency = 0;
  rng_res = 2000;				// default 2km data
  start_rng = 
    undefined_rng = 4000;
  max_rng = 512;			// default to 512km max range
  num_radls = 0;				// none yet
  radl_pos = 0;
  data_size = 0;
  vol_scans = vol_scan_no = 0;
  vol_tilts = vol_tilt_no = 0;
  num_scans = 0;
  volume_id = -1;      // id=-1, no explicit id defined
  set_angle = 0;
  sectorScan = false;
  sectorStartAngle = 
    sectorEndAngle = 0.0;
  sectorAngleIncreasing = true;
  prf = 250;
  completed_tilts = 0;
  previous_tilt_angle = -9999;
  memset(&comp_params, 0, sizeof(comp_params));
  ScanSetComplete = FALSE;
  ScanFinished = FALSE;
  nyquist = 10;
  ShownToScanClients = FALSE;
  //	isFilteredScan = false;
  // zmap = dBZTbl16L;
}

void rdr_scan::set_dflt(rdr_scan *basis_scan) {	// set defaults for rdr_scan based on passed rdr_scan
  if (!basis_scan)
    {
    set_dflt();    
      return;
    }
  //  rootscan = basis_scan->rootscan;
  station = basis_scan->station;
  FirstTm = basis_scan->rootScan()->FirstTm;
  LastTm = basis_scan->rootScan()->LastTm;
  LastScanTm = basis_scan->LastScanTm;
  scan_time_t = basis_scan->scan_time_t;
  UnixTime2DateTime(scan_time_t,year,month,day,hour,min,sec);
  rdr_params = basis_scan->rdr_params;
  scan_type = basis_scan->scan_type;
  data_type = basis_scan->data_type;
  bfdata_type = basis_scan->bfdata_type;
  radltype = basis_scan->radltype;
  data_fmt = basis_scan->data_fmt; 
  angle_res = basis_scan->angle_res;				// default 1deg res.
  if (!angle_res) angle_res = 10;
  NumLevels = basis_scan->NumLevels;
  nyquist = basis_scan->nyquist;
  frequency = basis_scan->frequency;
  rng_res = basis_scan->rng_res;	
  start_rng = basis_scan->start_rng;
  undefined_rng = basis_scan->undefined_rng;
  max_rng = basis_scan->max_rng;	
  num_radls = 0;	
  num_bad_radls = 0;	
  radl_pos = 0;
  data_size = 0;
  prf = basis_scan->prf;
  vol_scans = basis_scan->vol_scans;
  vol_scan_no = basis_scan->vol_scan_no;
  vol_tilts = basis_scan->vol_tilts;
  vol_tilt_no = basis_scan->vol_tilt_no;
  num_scans = basis_scan->num_scans;
  set_angle = basis_scan->set_angle;
  completed_tilts = basis_scan->completed_tilts;
  previous_tilt_angle = -9999;
  comp_params = basis_scan->comp_params;
  ScanSetComplete = basis_scan->ScanSetComplete;
  ScanFinished = basis_scan->ScanFinished;
  ShownToScanClients = FALSE;
  data_source = basis_scan->data_source;
  for (int x = e_refl; x < e_dt_max; x++)
    if (basis_scan->_LvlTbls && basis_scan->_LvlTbls[x])
      {
	if (_LvlTbls[x])
	  {
	    delete _LvlTbls[x];
	    _LvlTbls[x] = 0;
	  }
	_LvlTbls[x] = new LevelTable(basis_scan->_LvlTbls[x]);
      }
  clearAirMode = basis_scan->clearAirMode;
  //    isFilteredScan = basis_scan->isFilteredScan;
  // zmap = d1795BZTbl16L;
}

bool rdr_scan::end_img_str(char* instr) {
  char  c;
  instr += skip_white(instr, 256);
  if (instr[0] != 'E')
    return FALSE;
  if (sscanf(instr," END RADAR IMAG%c",&c) == 1)
      return(TRUE);
#ifdef CHECK_RECSIZE_BUG
  else 
    if (sscanf(instr," END R/IMAG%c",&c) == 1) 
      return(TRUE); // work around for short image copy bug
#endif
  else return(FALSE);
}

bool rdr_scan::end_img_str(rdr_scan_linebuff *lbuff) {
  return end_img_str(lbuff->line_buff);
}

bool rdr_scan::end_scanset_str(char* instr) {
  char  c;
  instr += skip_white(instr, 256);
  if (instr[0] != 'E')
    return FALSE;
  if ((sscanf(instr," END RADAR VOL%c",&c) == 1) ||
      strstr(instr,"END SCAN SET")) {
    fprintf(stdout,"rdr_scan::end_img_str detected END SCAN SET for %s\n",
	    ScanString());
    return(TRUE);
  }
  else if (sscanf(instr," END INCOMPLETE VOL%c",&c) == 1) {
    return(TRUE);
  }
  else return(FALSE);
}

bool rdr_scan::end_scanset_str(rdr_scan_linebuff *lbuff) {
  return end_scanset_str(lbuff->line_buff);
}

void rdr_scan::data_finished() {
  if (this != rootscan) {
    fprintf(stderr,"rdr_scan::data_finished - called for CHILD scan");
    fprintf(stderr," - Calling rootscan->data_finished\n");
    rootscan->data_finished();
    return;
  }
  ScanFinished = TRUE;
  if (!vol_scans) // number of scans not defined, mark as complete
    {
      ScanSetComplete = TRUE;
      if (lastScan())
	vol_scans = lastScan()->vol_scan_no; // set vol_scans to same as last
      fprintf(stderr,"rdr_scan::data_finished - Undefined vol_scans value - setting Complete - scans=%d\n",
	      vol_scans);
    }
  else if (vol_scans == lastScan()->vol_scan_no)
    ScanSetComplete = TRUE;    
  if (!RxTimeSetEnd())
    setRxTimeSetEnd(time(0));
  /*
   * LOCK NECESSARY FOR ADDING DATA AND SAFE USE OF USERCOUNT
   * ***KEEP LOCK UNTIL SCAN DESTROYED***
   */
  //	if (lock) lock->ReleaseOSLock();	// locks only necessary while data changing
  if (data_buff) 
    data_buff->ReleaseOSLock();	    // remove when data frozen
}

void rdr_scan::fault_no(int fault, char *_faultstr) {
  faultno = fault;
  if (faultno != 0)
    {
      if (_faultstr && strlen(_faultstr))
	faultstr = _faultstr;
      else
	if (faultstr.size() == 0)  // only write this if faultstr empty
	  faultstr = "Undefined Fault";
    }
}

int rdr_scan::get_fault_no() {
  return faultno;
}

void rdr_scan::add_data(char* inbuff, int dcount) {
  int		inpos=0;
  char	c;
  bool debug=FALSE;

  if (rootscan != this) {
    fprintf(stderr,"rdr_scan::add_data - Attempted to write data to child scan, Calling rootscan->add_data\n");
    rootscan->add_data(inbuff,dcount);
    return;
  }
  if (!linebuff) {
    fprintf(stderr,"rdr_scan::add_data - linebuff = 0\n");
    return;
  }
  if (debug) printf("ADD DATA-%s\n",inbuff);
  while (inpos < dcount) {
    c = inbuff[inpos];
    linebuff->addchar_parsed(c);
    if (linebuff->IsEOL())
      {
	add_line();	// add a line of data to data_buff
	linebuff->reset();// if END RADAR IMAGE pass to scan
      }
    inpos++;
  }
}

void rdr_scan::add_line(char *ln, int lnsz, 
			bool check, bool allowchildadd) {
  bool	debug=FALSE;
  int	lock_ok = 0;
  bool addingToChildScan = false;
  exp_buff *_data_buff = data_buff;

  if (rootscan != this) {
    if (allowchildadd)
      {
	addingToChildScan = true;
	_data_buff = rootscan->data_buff;
      }
    else
      {
	fprintf(stderr,"rdr_scan::add_line - Attempted to write data to child scan\n");
	return;
      }
  }
  if (!ln) {
    ln = linebuff->line_buff;
    lnsz = linebuff->lb_size;
  }
  else {
    if (lnsz < 0) lnsz = strlen(ln) + 1;
  }
  if (!lnsz) return;			// null line
  if (rootscan->lock && !(lock_ok = rootscan->get_lock()))
    fprintf(stderr,"rdr_scan::add_line - get_lock FAILED\n");	
  if (ln[lnsz-1] != 0) {			// if last char not 0
    ln[lnsz] = 0;			// add term 0
    lnsz++;
  }
  if ((ln[0] == '%') || (ln[0] == '@'))
    {
      if (!HeaderValid() && !num_radls)   // first radial and header not valid
	check_data();	// **** decode header data
      if (ln[0] == '@') // some binary data may
	{
	  if (NumLevels != 256)
	    {
	      NumLevels = 256; // in case incorrectly been labelled as 16lvl
	      data_fmt = RLE_8BIT;
	    }
	}
      if ((data_fmt == RLE_8BIT) ||    // add all RLE_8BIT radials
	  keepNullRadls ||             // if flag set, don't filter out null radials
	  (!null_radl(ln)) || (nullcount == 9)) { 	// do not add null radl
	if (debug) printf("ADD DATA-%s\n",ln);
	_data_buff->append_data(ln,lnsz);// radials
	//	num_radls++;	// increment radl count.
	nullcount = 0;				// allow every 10th null radial
      }
      else nullcount++;   // write every tenth null radl
    }
  else	// add all non-radials
    {
      if (debug) printf("rdr_scan::add_line - ADD HEAD-%s\n",ln);
      _data_buff->append_data(ln,lnsz);
      if (!end_img_str(ln) && !headerAddedTime)
	headerAddedTime = time(0);
      // could check data here to pick up time etc?????
    }
  if ((ln[0] == 'E') && 		// new scan available
      end_img_str(ln)) {
    if (!HeaderValid()) 
      check_data();	// **** decode header data
    if (!addingToChildScan)
      {
	if (!load_new_scan())// for root scan, load scan into root
	  data_finished();   // if root loaded, append new scan and load 
      }
    headerAddedTime = 0;
  }
  if (check && !HeaderValid()) 
    check_data();	// **** decode header data
  if (lock_ok) rootscan->rel_lock();
  if (ScanSetComplete) data_finished();
}

void rdr_scan::add_line(rdr_scan_linebuff *lbuff, bool check, 
			bool allowchildadd)
{
  add_line(lbuff->line_buff, lbuff->lb_size, check, allowchildadd);
}

// for root scan, load scan into root
// if root loaded, append new scan instance and load from expbuff data
// a bad_scan (complete and not valid) should never occur, 
// it will be culled on creation
bool rdr_scan::load_new_scan() {
  rdr_scan*	temp_scan;
  char errstr[256];
  //    bool debug = FALSE;
    
  if (rootscan != this) {
    fprintf(stderr,"rdr_scan::new_scan - Called for child scan\n");
    return false;
  }
  if (!scan_complete) {		// root scan not complete, use this data in root
    load_scan(linebuff->new_scan_ofs);
    if (headerAddedTime)
//       setRxTimeStart(headerAddedTime, "rdr_scan::new_scan - headerAddedTime");
      setRxTimeStart(headerAddedTime);
    else 
      setRxTimeStart(time(0));
    if (!(scan_complete = data_valid)) {
      fprintf(stderr,"rdr_scan::new_scan Invalid Scan - not added as root\n");
      linebuff->new_scan_ofs = data_buff->get_size();	// offset of next scan
      return true;
    }
    temp_scan = this;	// for use with following list manipulations
    FirstTm = LastTm = scan_time_t;	// time of first scan
  } 
  else {				// root scan complete, construct new scan
    temp_scan = new rdr_scan(scanCreator, "rdr_scan::new_scan - child", 
			     rootscan);	// if not valid disregard this data
    temp_scan->load_scan(linebuff->new_scan_ofs);
    //    if (headerAddedTime)
    //       temp_scan->setRxTimeStart(headerAddedTime, "rdr_scan::new_scan - headerAddedTime");
    // temp_scan->setRxTimeStart(headerAddedTime);
    //    else 
    // temp_scan->setRxTimeStart(time(0));
    if (!(temp_scan->scan_complete = temp_scan->data_valid)) {
      if (temp_scan->ShouldDelete(scanCreator, "rdr_scan::new_scan"))
	delete temp_scan;
      linebuff->new_scan_ofs = data_buff->get_size();	// offset of next scan
      fprintf(stderr,"rdr_scan::new_scan - Invalid Scan - not added as child to %s\n", ScanString());
      return false;
    }
    if ((temp_scan->station != station) ||
	(temp_scan->scan_type != scan_type)) {
      sprintf(errstr,"rdr_scan::new_scan - Scan mismatch - %s not added as child to %s\n", temp_scan->ScanString(), ScanString());
      RapicLog(errstr, LOG_ERR);	
      if (temp_scan->ShouldDelete(scanCreator, "rdr_scan::new_scan"))
	delete temp_scan;
      linebuff->new_scan_ofs = data_buff->get_size();	// offset of next scan
      return false;
    }
  }
  temp_scan->nextinset = 0;
  temp_scan->previnset = rootscan->lastscan;
  if (lastscan && !lastscan->RxTimeEnd()) // previous end not set yet, do it now
    lastscan->setRxTimeEnd(time(0));
//     lastscan->setRxTimeEnd(time(0), "rdr_scan::new_scan");
  if (lastscan != 0) lastscan->nextinset = temp_scan;
  lastscan = temp_scan;
  linebuff->new_scan_ofs = data_buff->get_size();	// offset of next scan
  num_scans++;
  if (set_angle != previous_tilt_angle)
    {
      completed_tilts++;
      previous_tilt_angle = set_angle;
    }
  /*
    if (debug)
    printf("first=%d last=%d this=%d\n",rootscan,lastscan,
    thisscan);
  */
  if (temp_scan->scan_time_t < rootscan->FirstTm)
    rootscan->FirstTm = temp_scan->scan_time_t;
  if (temp_scan->scan_time_t > rootscan->LastTm)
    rootscan->LastTm = temp_scan->scan_time_t;
  LastScanTm = temp_scan->scan_time_t;	// allow clients to detect scan set change
  vol_scans = temp_scan->vol_scans;
  //	vol_scan_no = temp_scan->vol_scan_no;
  if (vol_scans == temp_scan->vol_scan_no) { //check for complete volume
    // this check wiil work for single scan sets (PPI RHI) as
    // vol_scans and vol_scan_no will both be 0
    //		fprintf(stderr,"LAST SCAN IN PRIMARY VOLUME ADDED\n");
    ScanSetComplete = TRUE;
  }
  return true;
}

bool rdr_scan::Finished() {
  return ScanFinished;
}

bool rdr_scan::Complete() {
  return ScanSetComplete;
}

bool rdr_scan::HeaderValid() {
  return header_valid;
}

bool rdr_scan::Faulty() {
  bool temp;
  temp = !((faultno == 0) || (faultno == 3) || 
	   (faultno == 4)); // allow az and el setting fault images
  temp |= ScanFinished && (!data_valid || (!station));
  return temp;
}


time_t rdr_scan::FirstTime() {
  return FirstTm;
}

time_t rdr_scan::LastTime() {
  return LastTm;
}

time_t rdr_scan::ScanTime() {
  return scan_time_t;
}

int rdr_scan::get_date(char* LineIn,int StartPos) {    // dddyy format
  int	tempint;
  bool debug = FALSE;
    
  if (debug) printf("%s\n",LineIn);
  if (sscanf(LineIn+StartPos,"DATE: %d",&tempint) == 1) {
    rpdate2ymd(tempint, year, month, day);
    //	if (debug) printf("Year=%d DoY=%d %d/%d/%d\n",yr,DoY,year,month,day);
    scan_time_t = DateTime2UnixTime(year,month,day,hour,min,sec);
    return 1;
  }
  else return 0;
}

int rdr_scan::get_time(char* LineIn,int StartPos) {    // dddyy format
  if (sscanf(LineIn+StartPos,"TIME: %d.%d",&hour,&min) == 2) {
    sec = 0;
    scan_time_t = DateTime2UnixTime(year,month,day,hour,min,sec);
    return 1;
  }
  else return 0;
}

int rdr_scan::get_timestamp(char* LineIn,int StartPos) {    // dddyy format
  if (sscanf(LineIn+StartPos,"TIMESTAMP: %4d%2d%2d%2d%2d%2d",
	     &year, &month, &day, &hour, &min, &sec) == 6) {
    scan_time_t = DateTime2UnixTime(year,month,day,hour,min,sec);
    return 1;
  }
  else return 0;
}

int rdr_scan::get_datetime(char* LineIn,int StartPos, time_t *outtime) {    // dddyy format
  int tempint = 0, year, month, day, hour, min, sec = 0;
  char *strpos = strstr(LineIn+StartPos, ":");
  *outtime = 0;
  if (strpos) {
    strpos++;	// skip past ":"
    if (sscanf(strpos," %d %d.%d",&tempint, &hour, &min) == 3) {
      rpdate2ymd(tempint, year, month, day);
      *outtime = DateTime2UnixTime(year,month,day,hour,min,sec);
      return 1;
    }
    else return 0;
  }
  else return 0;
}

void rdr_scan::get_threshtbl(char *LineIn, float *threshtbl, int &numthresh) {
  char *charpos;
  float	thisval;

  charpos = LineIn;
  while (*charpos && (*charpos != ':'))   // skip to ':' of header
    charpos++;    
  if (*charpos) charpos++;	// skip past ':'
  //    charpos = LineIn + 7;	// skip the DBMLVL: or DBZLVL: prefix
  numthresh = 1;
  threshtbl[0] = -32.0;	// 1st level is currently assumed to be -32dbz ????
  while (*charpos && isspace(*charpos)) charpos++;	// skip and white space
  while ((numthresh < MAXDBZRES) && 
	 *charpos && 
	 (sscanf(charpos,"%f",&thisval) == 1)) {
    threshtbl[numthresh++] = thisval;
    while (*charpos && !isspace(*charpos)) charpos++;	// skip to next white space
    while (*charpos && isspace(*charpos)) charpos++;	// skip white space to next value
  }
}

int rdr_scan::read_line(char *linebuff, int maxchars, exp_buff_rd *dbuffrd)   // read a line of data from the data_buff
{
  bool isBinRadl = false;
  char *nextchar = 0; 
    
  if (!dbuffrd)   // use rdr_scan's dbuff_rd object
    dbuffrd = &dbuff_rd;
  dbuffrd->skip_white();
  nextchar = dbuffrd->peek_nextchar();
  isBinRadl = nextchar && (*nextchar == '@');
    
  if (isBinRadl)
    return dbuffrd->read_term2(linebuff,maxchars-1, 0, 0, 19);
  else
    return dbuffrd->read_term(linebuff,maxchars-1);
}

void rdr_scan::check_data() {
  int		line_len,line_pos,last_rd_ofs;
  char	line_buff[2048];
  bool	n_found = FALSE, h_found = FALSE, c_found = FALSE;
  bool	d_found = FALSE;
  bool done = FALSE;
  float	ftemp;
  // char	ctemp;
  int		itemp,itemp2;
  int angle_idx;
  float	fangle;
  char	stemp[64],stemp2[64];
  bool	debug = FALSE;
  int		Date_Valid = FALSE, Time_Valid = FALSE, Stn_Valid = FALSE;
  float	dbzcor = 0;
  float	threshtbl[MAXDBZRES];
  int		numthresh = 0,lvl = 0;
  exp_buff_rd *dbuffrd = new exp_buff_rd(rootscan->data_buff);
    
  num_radls = 0;		// clear the radl count
  num_bad_radls = 0;		// clear the radl count
  data_valid = FALSE;
  if (dbuffrd->set_read_ofs(header_start) < 0)
    {
      if (dbuffrd) delete dbuffrd;
      return;
    }
  data_size = 0;
  nyquist = 0;
  max_rng = 0;
  scan_type = PPI;	// return to default before start
  vol_scans = vol_scan_no = 1;
  while (!done) {
    last_rd_ofs = dbuffrd->get_read_ofs();
    if ((line_len = read_line(line_buff, 2047, dbuffrd)) < 0)
      break;
    data_size += line_len;			// keep track of scan dataq size
    line_buff[line_len] = 0;		// for the benefit of sscanf
    //if (debug) printf("CHECKING %s\n",line_buff);
    if ((line_pos = skip_white(line_buff,line_len)) >= 0)
      switch (line_buff[line_pos]) {
      case 'A' :
	if (sscanf(line_buff+line_pos,"ANGRES:%f",&ftemp) == 1) {
	  angle_res = int(ftemp*10);
	  if (debug) printf("ANGRES=%d\n",angle_res);
	}
	else if (sscanf(line_buff+line_pos,"AZIM:%f",&ftemp) == 1) {
	  set_angle = rdr_angle(ftemp * 10);
	  if (scan_type != RHISet) scan_type = RHI;
	  if (debug) printf("SETANGLE=%d\n",set_angle);
	}
	break;
      case 'C' :
	if (sscanf(line_buff+line_pos,"CAPPIHT:%d",&itemp) == 1) {
	  set_angle = itemp;
	  if (scan_type == PPI) scan_type = CAPPI ;
	  if (debug) printf("CAPPIHT=%d\n",set_angle);
	}
	else if (sscanf(line_buff+line_pos,"COUNTRY:%d",&itemp) == 1) {
	  if (!h_found) {		// NAME or COUNTRY field is first in header
	    if (debug)
	      printf("NAME=%s HEAD_OFS=%d\n",rdr_params.name,
		     last_rd_ofs);
	    header_start = last_rd_ofs;
	    h_found = TRUE;
	    c_found = TRUE;
	  }
	  else {
	    if (c_found && d_found) 
	      done = TRUE;	// must be header of next scan
	  }
	  itemp = CountryCodeToID(itemp);
	  station &= 0xff;
	  station |= itemp << 8;
	  //	printf("rdr_scan::check_data COUNTRY STNID=%d\n",itemp);
	}
	else if (sscanf(line_buff+line_pos,"CREATOR:%s",stemp) == 1) {
	  creatorFieldString = stemp;
	}
	break;
      case 'D' :
	if (!Date_Valid && strstr(line_buff+line_pos,"DATE:"))
	  Date_Valid = get_date(line_buff,line_pos);
	else if (sscanf(line_buff+line_pos,"DBZCOR: %f",&dbzcor) == 1);
	else if (strstr(line_buff+line_pos,"DBMLVL:"))
	  get_threshtbl(line_buff+line_pos,threshtbl,numthresh);
	else if (strstr(line_buff+line_pos,"DBZLVL:")) {
	  get_threshtbl(line_buff+line_pos,threshtbl,numthresh);
	  dbzcor = 0;
	}
	break;
      case 'E' :
	if (sscanf(line_buff+line_pos,"ELEV:%f",&ftemp) == 1) {
	  set_angle = rdr_angle(roundf((ftemp * 10)));
	  //			if (scan_type != VOL) scan_type = PPI; // default is PPI, leave alone
	  if (debug) printf("SETANGLE=%d\n",set_angle);
	}
	else if (end_img_str(line_buff+line_pos)) {
	  if (debug) printf("END OF IMAGE FOUND\n");
	  done = TRUE;
	  if (!header_valid)
	    CheckForValidHeader();

	  //	  data_valid = (faultno == 0) || (faultno == 3) || (faultno == 4);
	  //	  if (scan_type == SCANERROR)  // ERRORSCAN ALWAYS INVALID
	  //	    data_valid = FALSE;
	  //	  data_valid = data_valid && header_valid;
	  data_valid = header_valid;
	}
	else if (sscanf(line_buff+line_pos,"ENDRNG: %d", &itemp) == 1)
	  max_rng = itemp/1000.;
	else if (strstr(line_buff+line_pos,"ENDTIME:"))
	  get_datetime(line_buff,line_pos, &EndTm);
	break;
      case 'F' :
	//SD added 31/10/1
	if (sscanf(line_buff+line_pos,
		   "FREQUENCY:%d",&itemp) == 1) {
	  frequency = itemp;
	}
	else if (strstr(line_buff+line_pos,"CLEARAIR: ON"))
	  clearAirMode = true;
	break;
      case 'I' :
	if (sscanf(line_buff+line_pos,"IMGFMT:%s",stemp) == 1) {
	  UpperStr(stemp);
	  if (strstr(stemp,"COMPPPI")) {
	    if (debug) printf("IMG TYPE = COMPPPI\n");
	    scan_type = CompPPI;
	  }
	  else if (strstr(stemp,"CAPPI")) {    // must put this in before PPI (rjp 12 Jul 2001)
	    if (debug) printf("IMG TYPE = CAPPI\n");
	    scan_type = CAPPI;
	  }
	  else if (strstr(stemp,"PPI")) {
	    if (debug) printf("IMG TYPE = PPI\n");
	    if ((scan_type != VOL) &&	// if still default, set it
		(scan_type != SCANERROR))
	      scan_type = PPI;
	    // default is PPI, don't change, may already be set to VOL or ERROR
	  }
	  else if (strstr(stemp,"RHI")) {
	    if (debug) printf("IMG TYPE = RHI\n");
	    if (scan_type != RHISet)
	      scan_type = RHI;
	  }
	  else if (strstr(stemp,"RHISet")) {
	    if (debug) printf("IMG TYPE = RHISet\n");
	    scan_type = RHISet;
	  }
	}
	break;
      case 'N' :
	if (sscanf(line_buff+line_pos,"NAME: %7s", rdr_params.name) == 1) {
	  if (!h_found) {		// NAME or COUNTRY field is first in header
	    if (debug)
	      printf("NAME=%s HEAD_OFS=%d\n",rdr_params.name,
		     last_rd_ofs);
	    header_start = last_rd_ofs;
	    h_found = TRUE;
	    n_found = TRUE;
	  }
	  else {
	    if (n_found && d_found) 
	      done = TRUE;	// must be header of next scan
	  }
	}
	else if (sscanf(line_buff+line_pos,"NYQUIST:%f", &nyquist) == 1)
	  {
// 	    if (nyquist < 0)
// 	      fprintf(stdout, "rdr_scan::check_data - NOTE: Negative nyquist, swapping twds/away\n");
// 	  nyquist = fabsf(nyquist);
	  }
	break;
      case 'P' :
	if (debug) printf("%s\n",line_buff+line_pos);
	int _args;
	if ((_args = sscanf(line_buff+line_pos,"PASS:%d of %d",&itemp,&itemp2)) >= 1) {
	  if (itemp == 0) {
	    printf("ERROR IN SCAN - PASS: 00 detected\n");
	    data_valid = FALSE;
	    if (dbuffrd) delete dbuffrd;
	    return;
	  }
	  else if ((_args == 2) && (itemp2 > 0)) {  // PASS: x of y properly defined - use it
	    if (itemp2 >= itemp)   // should be >= scan_no 
	      vol_scans = itemp2;
	    else                   // if not, probably vol_scans is 0, i.e. unknown no of scans
	      vol_scans = 0;
	    vol_scan_no = itemp;
	  }
	  else {                  // PASS: x of y - x is defined, not y, set vol_scans to 0
	    vol_scan_no = itemp;
	    vol_scans = 0;
	  }
	}
	else if (sscanf(line_buff+line_pos,"PRF:%d",&prf) == 1) 
	  {
	    if (debug) printf("PRF=%d\n",prf);
	  }
	else 
	  switch (sscanf(line_buff+line_pos,"PRODUCT: %s %s",stemp,stemp2)) {
	  case 1:
	    if (strstr(stemp,"ERROR")) {
	      if (debug) printf("IMG TYPE = ERROR\n");
	      //  fprintf(stderr, "rdr_scan::check_data - IMG TYPE = ERROR\n");
	      scan_type = SCANERROR;
	    }
	    break;
	  case 2:
	    if (strstr(stemp,"VOLUMETRIC")) {
	      strncpy(setlabel,stemp2,32);
	      scan_type = VOL;
	    }
	    else if (strstr(stemp,"RHISet")) {
	      strncpy(setlabel,stemp2,32);
	      scan_type = RHISet;
	    }
	    if (strstr(line_buff+line_pos,"SECTOR")) {
	      sectorScan = true;
	    }
	    break;
	  }
	/*check_P(line_buff,line_pos);*/
	break;
      case 'R' :
	if (sscanf(line_buff+line_pos,
		   "RNGRES:%f",&rng_res) == 1) {
	  if (debug) printf("RNGRES=%1.2f\n",rng_res);
	}
	break;
      case 'S' :
	if (sscanf(line_buff+line_pos,"STNID:%d",&itemp) == 1) {
	  station &= 0xff00;
	  station |= itemp;
	  //			if (debug) printf("STNID=%d\n",station);
	  Stn_Valid = 1;
	}
	else if (sscanf(line_buff+line_pos,"STARTRNG:%f",&start_rng) == 1) {
	  if (debug) printf("STARTRNG=%1.2f\n",start_rng);
	}
	else if (strstr(line_buff+line_pos,"STARTTIME:"))
	  get_datetime(line_buff,line_pos, &StartTm);
	break;
      case 'T' : 
	if (debug) printf("%s\n",line_buff+line_pos);
	itemp2 = 0;
	if (sscanf(line_buff+line_pos,"TILT:%d of %d",&itemp,&itemp2) >= 1) {
	  if (itemp == 0) {
	    printf("ERROR IN SCAN - TILT: 00 detected\n");
	    data_valid = FALSE;
	    if (dbuffrd) delete dbuffrd;
	    return;
	  }
	  else if ((_args == 2) && (itemp2 > 0)) {  // TILT: x of y properly defined - use it
            if (itemp2 >= itemp)   // should be >= tilt_no 
              vol_tilts = itemp2;
            else                   // if not, probably vol_tilts is 0, i.e. unknown no of tilts
              vol_tilts = 0;
	    vol_tilt_no = itemp;
	  }
          else {                  // TILT: x of y - x is defined, not y, set vol_tilts to 0
            vol_tilt_no = itemp;
            vol_tilts = 0;
          }
	}
	else if (!Time_Valid && strstr(line_buff+line_pos,"TIME:"))
	  Time_Valid = get_time(line_buff,line_pos);
	else if (strstr(line_buff+line_pos,"TIMESTAMP:"))
	  Time_Valid = Date_Valid = get_timestamp(line_buff,line_pos);
	else if (strstr(line_buff+line_pos,"THRESHTBL:")) {
	  get_threshtbl(line_buff+line_pos,threshtbl,numthresh);
	  dbzcor = 0;
	}
	break;
      case 'U' :
	if (sscanf(line_buff+line_pos,"UNDEFINEDRNG:%f",
		   &undefined_rng) == 1) 
	  {
	    if (debug) printf("UNDEFINEDRNG=%1.2f\n",undefined_rng);
	  }
	break;
      case 'V' :
	if (sscanf(line_buff+line_pos,"VERS:%f",&ftemp) == 1) {
	  if (debug) printf("VERS=%f\n",ftemp);
	  creatorFieldVersion = ftemp;
	}
	else if (sscanf(line_buff+line_pos,
			"VIDRES:%d",&itemp) == 1) {
	  if ((itemp >= 16) && (itemp < 256))
	    data_fmt = RLE_16L_ASC;
	  if (itemp == 256)
	    data_fmt = RLE_8BIT;
	  if (debug)
	    printf("VIDRES=%d DATA_FMT=%d\n",itemp,data_fmt);
	  NumLevels = itemp;
	}
	else if (sscanf(line_buff+line_pos,
			"VIDEO:%s", stemp) == 1) {
	  /*
	    if (strstr(stemp, data_type_text[e_rawrefl]))
	    data_type = e_rawrefl;
	    else if (strstr(stemp, data_type_text[e_rawrefl]))
	    data_type = e_refl;
	    else if (strstr(stemp, data_type_text[e_rawrefl]))
	    data_type = e_vel;
	    else if (strstr(stemp, data_type_text[e_rawrefl]))
	    data_type = e_rainaccum;
	    else if (strstr(stemp, data_type_text[e_rawrefl]))
	    data_type = e_rainaccum;
	    else if (strstr(stemp, data_type_text[e_rawrefl]))
	    data_type = e_rainaccum;
	  */
	  data_type = e_data_type(decode_datatypestr(stemp));
	  if ((data_type < e_refl) || (data_type >= e_dt_max))
	    data_type = e_refl;
	}
	else if (sscanf(line_buff+line_pos,"VELLVL:%f", &nyquist) == 1)
	  nyquist = fabsf(nyquist);
	else if (sscanf(line_buff+line_pos,"VOLUMEID:%d", &itemp) == 1)
	  volume_id = itemp;
	break;
	/*
	  case 'F' :
	  if (sscanf(line_buff+line_pos,"FILTERTYPE:%s",stemp) == 1) {
	  isFilteredScan = true;
	  }
	  break;
	*/
      case '/' :
	tm rxtimestart, rxtimeend;
	if (strstr(line_buff+line_pos, "/RXTIME:"))
	  if (sscanf(line_buff+line_pos,
		     "/RXTIME: %04d%02d%02d%02d%02d%02d %04d%02d%02d%02d%02d%02d",
		     &rxtimestart.tm_year,&rxtimestart.tm_mon,&rxtimestart.tm_mday,
		     &rxtimestart.tm_hour,&rxtimestart.tm_min, &rxtimestart.tm_sec,
		     &rxtimeend.tm_year,&rxtimeend.tm_mon,&rxtimeend.tm_mday,
		     &rxtimeend.tm_hour,&rxtimeend.tm_min, &rxtimeend.tm_sec) == 12) 
	    {
	      if (rxtimestart.tm_year > 1900)
		rxtimestart.tm_year-=1900; // convert actual year back to struct tm year since 1900
	      if (rxtimeend.tm_year > 1900)
		rxtimeend.tm_year-=1900;
	      setRxTimeStart(DateTime2UnixTime(rxtimestart.tm_year, rxtimestart.tm_mon,
					      rxtimestart.tm_mday, rxtimestart.tm_hour,
					      rxtimestart.tm_min, rxtimestart.tm_sec));
	      setRxTimeSetEnd(DateTime2UnixTime(rxtimeend.tm_year, rxtimeend.tm_mon,
						rxtimeend.tm_mday, rxtimeend.tm_hour,
						rxtimeend.tm_min, rxtimeend.tm_sec));
// 			      "rdr_scan::check_data");
	    }
	break;
      case '%' :
      case '@' :
	if (line_buff[line_pos] == '@') // some binary data may
	  {
	    if (NumLevels != 256)
	      {
		NumLevels = 256;    // have incorrectly been labelled as 16 level
		data_fmt = RLE_8BIT;
	      }
	  }
	if (!d_found) {
	  if ((!RadlPointers) && UseRadlPnt) {
	    if (FreeListMng && (angle_res > 0)) 
	      RadlPointers = FreeListMng->RadlPntFreeList->GetRadlPnt(3600 / angle_res);
	    else if (angle_res > 0)
		  RadlPointers = new radl_pnt(3600 / angle_res);
	  }
	  data_start = last_rd_ofs;
	  if (debug) printf("DATA START=%d\n",last_rd_ofs);
	}
	if (RadlPointers) {
// 	  int temp1, temp2, temp3;
	  int degs = 0,
	    tenths = 0,
	    hundths = 0;
	  sscanf(line_buff+line_pos,"%*c%d.%1d%1d",&degs, &tenths, &hundths);
	  fangle = degs + (float(tenths)/10.0) + (float(hundths)/100.0);
// 	  sscanf(line_buff+line_pos,"%*c%f",&fangle);
	  angle_idx = (int(int(fangle * 10) + (angle_res/2))%3600) / angle_res;
// 	  if ((abs((int((angle_idx * angle_res)%3600) - 
// 		    (int(fangle * 10.0)%3600))) > angle_res))
// 	    {
// 	      temp1 = int(fangle * 10);
// 	      temp2 = int(temp1 + (angle_res/2))%3600;
// 	      temp3 = temp2 / angle_res;
// 	    }
	  if ((angle_idx >= 0) && (angle_idx < RadlPointers->numradials))
	    RadlPointers->PntTbl[angle_idx] = last_rd_ofs; 
	}
	d_found = TRUE;
	num_radls++;			// inc radial counter
	break;
      }
  }
  header_valid = Date_Valid && Time_Valid && Stn_Valid;
  bool	badlevel = FALSE;
  float badlevelval = 0,val;
  if (dbzcor) {
    for (lvl = 1; lvl < numthresh; lvl++) {	// apply dbm to dbz correction
      val = (threshtbl[lvl] += dbzcor);
      if (!gte_lte(val,MINDBZ,MAXDBZ)) {
	badlevel = TRUE;
	badlevelval = threshtbl[lvl];
	threshtbl[lvl] = 0;
      }
    }
  }
  if (badlevel)
    fprintf(stderr,"rdr_scan::check_data - Out of range level (%fdBz) detected from %s\n",badlevelval,StnRec[station].Name);
  if (data_type == e_vel)
    {
      if (!rootscan->_LvlTbls[data_type] ||
	  (rootscan->_LvlTbls[data_type] && (rootscan->nyquist != nyquist)))
	{
	  rootscan->nyquist = nyquist;
	  if (!rootscan->_LvlTbls[data_type])
	    rootscan->_LvlTbls[data_type] = new LevelTable(NumLevels, 0.0, -nyquist, nyquist);
	}
    }
	
  if ((header_valid && !rootscan->_LvlTbls[data_type]) || // rootscan is the only scan with level table
      (data_valid && rootscan->_LvlTbls[data_type] && 
       (rootscan->_LvlTbls[data_type]->numLevels() != NumLevels)))
    switch (data_fmt) {
    case RLE_6L_ASC:
      if (rootscan->_LvlTbls[data_type] && 
	  (rootscan->_LvlTbls[data_type]->numLevels() != 8)) {
	if (!rootscan->_LvlTbls[data_type]->getGlobal())
	  delete rootscan->_LvlTbls[data_type];
	rootscan->_LvlTbls[data_type] = 0;
      }
      if (!badlevel && ((numthresh == 7) || (numthresh == 8))) {
	if (numthresh == 7) threshtbl[7] = threshtbl[6]; 	// use last level to fill table
	if (!rootscan->_LvlTbls[data_type] || rootscan->_LvlTbls[data_type]->getGlobal()) 
	  rootscan->_LvlTbls[data_type] = new LevelTable(8,threshtbl);
	else
	  {
	    if ((scan_type == rootscan->scan_type) &&
		(station == rootscan->station))
	  rootscan->_LvlTbls[data_type]->SetLevels(8,threshtbl);
	    else
	      fprintf(stderr, "rdr_scan::check_data - Root scan mismatch - this=%d-%s, root=%d-%s\n",
		      station, get_scan_type_text(scan_type),
		      rootscan->station, get_scan_type_text(rootscan->scan_type));
	  }
      }
      else {
	if (!rootscan->_LvlTbls[data_type] || 
	    rootscan->_LvlTbls[data_type]->getGlobal())
	  rootscan->_LvlTbls[data_type] = new LevelTable(8,Dflt_dBZThresh6Lvl);		// use default 6 level table
	else
	  rootscan->_LvlTbls[data_type]->SetLevels(8,Dflt_dBZThresh6Lvl);
      }
      break;
    case RLE_16L_ASC:
      if ((numthresh == NumLevels) && !badlevel) {
	if (!rootscan->_LvlTbls[data_type] || 
	    rootscan->_LvlTbls[data_type]->getGlobal())
	  rootscan->_LvlTbls[data_type] = new LevelTable(NumLevels,threshtbl);
	else
	  rootscan->_LvlTbls[data_type]->SetLevels(NumLevels,threshtbl);
      }
      else {
	if (!rootscan->_LvlTbls[data_type] || 
	    rootscan->_LvlTbls[data_type]->getGlobal())
	  rootscan->_LvlTbls[data_type] = 
	    new LevelTable(NumLevels,Dflt_dBZThresh16Lvl);// use default 16 level table
	else
	  rootscan->_LvlTbls[data_type]->SetLevels(NumLevels,
						   Dflt_dBZThresh16Lvl);
      }
      break;
    case RLE_8BIT:
      if (rootscan->_LvlTbls[data_type] && 
	  (rootscan->_LvlTbls[data_type]->numLevels() != NumLevels)) {
	if (!rootscan->_LvlTbls[data_type]->getGlobal())
	  delete rootscan->_LvlTbls[data_type];
	rootscan->_LvlTbls[data_type] = 0;
      }
      rootscan->_LvlTbls[data_type] = GetStdBinaryReflLevelTbl();
      break;
    default:
      break;
    }
  if (!max_rng) {
    if (rng_res > 1000) max_rng = 512;
    else max_rng = 256;
  }
  if (station && (station < StnDefCount))
    {
      if (!rdr_params.az_beamwidth)
	rdr_params.az_beamwidth = RdrRec[StnRec[station].Rdr].AzBeamWidth;
      if (!rdr_params.el_beamwidth)
	rdr_params.el_beamwidth = RdrRec[StnRec[station].Rdr].ElBeamWidth;
      if (!rdr_params.wavelength)
	rdr_params.wavelength = RdrRec[StnRec[station].Rdr].WaveLength;
    }
  if (dbuffrd) delete dbuffrd;
  if (debug) printf("NUMRADLS=%d\n",num_radls);
}

bool	rdr_scan::CheckForValidHeader() {	// true if adequate valid header data present
  header_valid = 
    (station > 0) && (scan_time_t > 0);
  return header_valid;
}
    
void rdr_scan::reset_radl(exp_buff_rd *dbuffrd, 
			  short *radlpos) {
    
  if (!dbuffrd)   // use rdr_scan's dbuff_rd object
    dbuffrd = &dbuff_rd;
  if (!radlpos)   // use rdr_scan's radl_pos object
    radlpos = &radl_pos;
  dbuffrd->set_read_ofs(data_start);
  *radlpos = 0;
}

int rdr_scan::get_next_radl(s_radl* NextRadl, exp_buff_rd *dbuffrd, 
			    short *radlpos, e_data_type datatype) {
    
  int		b_size=0;
  char	buff[2048];
  //  char	tempstr[512];
  int		result;
  bool binaryradl = false;

  if (!NextRadl || !data_valid) return -1;
  if (datatype);
  if (!dbuffrd)   // use rdr_scan's dbuff_rd object
    dbuffrd = &dbuff_rd;
  if (!radlpos)   // use rdr_scan's radl_pos object
    radlpos = &radl_pos;
  NextRadl->data_size = 0;
  NextRadl->rngres = rng_res;
  NextRadl->startrng = start_rng;
  NextRadl->undefinedrng = undefined_rng; // used by CAPPI type radials
  NextRadl->mode = INDEX;
  NextRadl->data_type = data_type;
  NextRadl->scan_type = scan_type;
  NextRadl->numlevels = NumLevels;
  if (rootscan->_LvlTbls[data_type] && (data_type == rootscan->_LvlTbls[data_type]->getMoment()))
    NextRadl->LvlTbl = rootscan->_LvlTbls[data_type];
  else 
    NextRadl->LvlTbl = 0;
  if (NextRadl->Values) {
    delete[] NextRadl->Values;
    NextRadl->Values = 0;
  }
  NextRadl->resize(rngBins());  // expand buffers if reqd.
  if (*radlpos >= num_radls) return(-1);
  b_size = read_line(buff,2048, dbuffrd);
  if ((b_size > 0) && 
      ((buff[0] == '%') || (buff[0] == '@'))) {
    if (buff[0] == '@') // this is a binary radial
      {
	binaryradl = true;
	if ((result = DecodeBinaryRadl((unsigned char *)buff, b_size, NextRadl)) < 0)
	  {
	    /* bad idea, this can really clutter console with repeated errors
	    sprintf(tempstr,"rdr_scan::get_next_radl - BAD RADIAL in scan=%s Pass=%d\n",
		    ScanString(), vol_scan_no);
	    RapicLog(tempstr, LOG_CRIT);
	    */
	    if (result == -1)   // -1 is fatal erro, -2 is recoverable error
	      return(-1);	    // errno -2 will be logged but will continue
	  }
      }
    else
      {
	switch (data_fmt) { // can't rely on data_fmt to detect binary format
	case RLE_6L_ASC:
	  // zmap = dBZTbl6L;
	  if (RLE_6L_radl(buff, b_size,NextRadl) < 0)
	    return(-1);
	  break;
	case RLE_16L_ASC:
	  // zmap = dBZTbl16L;
	  if (RLE_16L_radl(buff, b_size,NextRadl, NumLevels-1) < 0)
	    return(-1);
	  break;
	default:
	  break;
	}
      }
    (*radlpos)++;
    switch (scan_type) {		// radl cnvts return angle in az only
    case RHI:				// angle for RHI is actually elev
    case RHISet:			// angle for RHI is actually elev
      if (!binaryradl)
	NextRadl->el = NextRadl->az;    // move ASCII fmt radl angle to el
      if ((NextRadl->el > 900) || (NextRadl->el < -100))
	return -1;
      NextRadl->el1 = NextRadl->el-(angle_res/2);
      NextRadl->el2 = NextRadl->el1+angle_res;
      NextRadl->az = NextRadl->az1 = NextRadl->az2 = set_angle;
      break;
    case VOL:
    case PPI:
    case CAPPI:
    case CompPPI:
      if ((NextRadl->az > 3600) || (NextRadl->az < 0))
	return -1;
      NextRadl->az_hr = NextRadl->az;	    // retain "actual az"
      NextRadl->az = int((NextRadl->az+(angle_res/2)) / angle_res) 
	* angle_res;	    // round to nearest mult of angle_res
      if (NextRadl->az >= 3600)
	NextRadl->az -= 3600;
      NextRadl->az2 = NextRadl->az+(angle_res/2);
      NextRadl->az1 = NextRadl->az2-angle_res;
      NextRadl->el = NextRadl->el1 = NextRadl->el2 = set_angle;
      break;
    case IMAGE:
    default: 
      fprintf(stderr,"rdr_scan::get_next_radl ERROR Bad scan type\n");
      return(-1);
    }
    return(0);
  }
  else return(-1);
}

/*
 * get the radial at the given angle,  if available
 */
int rdr_scan::get_radl_angl(s_radl *Radl, rdr_angle Angl, exp_buff_rd *dbuffrd, 
			    short *radlpos, e_data_type datatype) {

  int	RadlOK = FALSE;
  int radlofs;
  int angle_idx;

  if (!Radl || !data_valid)
    return -1;
  if (!dbuffrd)   // use rdr_scan's dbuff_rd object
    dbuffrd = &dbuff_rd;
  if (!radlpos)   // use rdr_scan's radl_pos object
    radlpos = &radl_pos;

  Angl = Angl % 3600;
  if (Angl < 0) 
    Angl += 3600;  // handle most common az index error
  if (Angl >= 3600) 
    Angl -= 3600;
  
  angle_idx = (Angl+(angle_res/2)) / angle_res;

  Radl->resize(rngBins());  // expand buffers if reqd.

  if (RadlPointers && 
      (angle_idx >= 0) &&
      (angle_idx < RadlPointers->numradials)) {
    if ((radlofs = RadlPointers->PntTbl[angle_idx]) != -1) {
      dbuffrd->set_read_ofs(radlofs);
      *radlpos = 0;	//
      RadlOK = get_next_radl(Radl, dbuffrd, radlpos) != -1;
    }
  }	
  if ((RadlOK != -1) && (abs(Radl->az-Angl)<=((angle_res+1)/2))) 
    // returned az must be within angle_res/2 
    return(0);		// successful, use this
  else {  // either no RadlPointers or RadlPointers failed
    reset_radl(dbuffrd, radlpos);
    // printf("GETTING RADL NO %d ",Angl/10);
    while (((RadlOK = get_next_radl(Radl, dbuffrd, radlpos)) != -1) && 
	   (abs(Radl->az-Angl)>((angle_res+1)/2)));
  }
  if ((RadlOK != -1) && (abs(Radl->az-Angl)<=((angle_res+1)/2))) 
    // returned az must be within angle_res/2 
    return(0);		// successful, use this
  // printf("NOT FOUND - RETURNING NULL\n");
  Radl->data_size = 0;								// return null radl
  Radl->az = Angl;
  Radl->az2 = Radl->az+(angle_res/2);
  Radl->az1 = Radl->az2-angle_res;
  Radl->el = Radl->el1 = Radl->el2 = set_angle;
  Radl->scan_type = scan_type;
  Radl->rngres = rng_res;
  Radl->startrng = start_rng;
  Radl->undefinedrng = undefined_rng;
  Radl->LvlTbl = LvlTbls(data_type);
  Radl->numlevels = rootscan->NumLevels;
  if (Radl->Values) {
    delete[] Radl->Values;
    Radl->Values = 0;
  }
  return -1;
}
	
/*
 * get the data value at the given angle and range (km),  if available
 * return true if value defined, false otherwise
 */
bool rdr_scan::get_data_angl_rng(uchar &retval, rdr_angle Angl, float rng, exp_buff_rd *dbuffrd, 
				  short *radlpos, e_data_type datatype,
				  bool tanplanerng) {
  s_radl tempradl;

  if (!data_valid) return 0;
  if (get_radl_angl(&tempradl, Angl, dbuffrd, radlpos, datatype) < 0) return 0;
  if (tanplanerng && 
      ((scan_type == PPI) ||(scan_type == VOL)))
    {
      float CosEl = cos(tempradl.el/10.0*DEG2RAD);
      if (CosEl)
	{
	  retval = tempradl.DataAtRange(rng/CosEl);
	  return retval != 0;
	}
      else
	{
	  retval = 0;
	  return false;
	}
    }
  else
    {
      retval = tempradl.DataAtRange(rng);
      return retval != 0;
    }
  retval = 0;
  return false; 
}    

// return true if value defined, false otherwise
bool rdr_scan::get_val_angl_rng(float &retval, float az, float rng, exp_buff_rd *dbuffrd, 
				 short *radlpos, e_data_type datatype,
				  bool tanplanerng)
{
  rdr_angle localaz = int(az * 10.);
  
  if (!data_valid) return 0;
  uchar dataval;
  bool val_defined = get_data_angl_rng(dataval, localaz, rng, dbuffrd, radlpos, datatype, tanplanerng);
  
  if (LvlTbls(datatype) && val_defined)
    retval = LvlTbls(datatype)->val(dataval);
  else
    retval = 0;
  return val_defined;
}
      
// return true if value defined, false otherwise
bool rdr_scan::get_val_angl_rng(float &retval, int &idx, float az, float rng, exp_buff_rd *dbuffrd, 
				 short *radlpos, e_data_type datatype,
				  bool tanplanerng)
{
  rdr_angle localaz = int(az * 10.);
  
  if (!data_valid) return 0;
  uchar dataval;
  bool val_defined = get_data_angl_rng(dataval, localaz, rng, dbuffrd, radlpos, datatype, tanplanerng);
  
  if (LvlTbls(datatype) && val_defined)
    {
      retval = LvlTbls(datatype)->val(dataval);
      idx = dataval;
    }
  else
    {
      retval = 0;
      idx = 0;
    }
  return val_defined;
}
      

/*
 * get the nth radial in this scan,  if available
 */
int rdr_scan::get_radl_no(s_radl* Radl, int radlno, exp_buff_rd *dbuffrd, 
			  short *radlpos, e_data_type datatype) {
  return get_radl_angl(Radl, radlno * angle_res, dbuffrd, radlpos, datatype);
}	


int rdr_scan::write_scan(char *fname, 
			 bool reflonly) {
  int tempfd;
  int numwr = 0;
  if (!fname) {
    fprintf(stderr, "rdr_scan::write_scan FAILED - fname not defined\n");
    return numwr;
  }
  tempfd = open(fname, O_RDWR+O_CREAT, 0664);
  if (tempfd < 0) {
    fprintf(stderr, "rdr_scan::write_scan FAILED opening %s\n", fname);
    perror(0);
    return numwr;
  }
  write_scan(tempfd, reflonly);
  close(tempfd);
  return numwr;
}

int rdr_scan::write_scan(int fd, 
			 bool reflonly) {

#define RSWSBUFFSZ 4096 
  char	tempbuff[RSWSBUFFSZ];
  exp_buff_rd *temp_rd;
  int	numrd = 0, numwr = 0;
  int	rdsz,tempr,tempw;
  int data_read_size = data_size;

  if (fd < 0) {
    fprintf(stderr,"rdr_scan::write_scan ERROR fd < 0");
    return 0;
  }
  write_error = 0;
  temp_rd = new exp_buff_rd;
  temp_rd->open(rootscan->data_buff);
  if (!reflonly || 
      (vol_scans == completedTilts())) // one scan per tilt, assume refl anyway
    {
      if (temp_rd->set_read_ofs(header_start) < 0)
	return 0;
      data_read_size = data_size;
    }
  else
    { 
      int hdrsize = 0;
      int temp_vol_scan_no = vol_scan_no;
      int temp_vol_scans = vol_scans;
      vol_scan_no = thisTilt();
      vol_scans = completedTilts();
      Write16lvlHeader(tempbuff, hdrsize, RSWSBUFFSZ);
      vol_scan_no = temp_vol_scan_no;
      vol_scans = temp_vol_scans;
      tempw = write(fd,tempbuff,hdrsize);
      if (tempw < 0) {
	perror("rdr_scan::write_scan ERROR ");
	delete temp_rd;
	write_error = errno;	
	return numwr;
      }
      numwr += tempw;
      // don't write header
      if (temp_rd->set_read_ofs(data_start) < 0)
	return 0;
      data_read_size = data_size - 
	(data_start - header_start);
    }
  while (numrd < data_read_size) {
    rdsz = data_read_size - numrd;
    if (rdsz > RSWSBUFFSZ) rdsz = RSWSBUFFSZ;
    tempr = temp_rd->read_blk(tempbuff,rdsz);
    numrd += tempr;
    tempw = write(fd,tempbuff,tempr);
    if (tempw < 0) {
      perror("rdr_scan::write_scan ERROR ");
      delete temp_rd;
      write_error = errno;
      return numwr;
    }
    numwr += tempw;
    if (tempr != rdsz) {
      //			fprintf(stderr,"rdr_scan::write_scan ERROR reading from exp_buff,  req=%d returned=%d\n", rdsz, tempr);
      delete temp_rd;
      return numwr;
    }
  }
  delete temp_rd;
  return numwr;
}

int rdr_scan::write_scan_set(char *fname,
			     bool writeReflOnly) {
  int tempfd;
  int size;
  int numwr = 0;
  if (!fname) {
    fprintf(stderr, "rdr_scan::write_scan FAILED - fname not defined\n");
    return numwr;
  }
  tempfd = open(fname, O_RDWR+O_CREAT, 0664);
  if (tempfd < 0) {
    fprintf(stderr, "rdr_scan::write_scan FAILED opening %s\n", fname);
    perror(0);
    return numwr;
  }
  numwr = write_scan_set(tempfd, 0, &size, writeReflOnly);
  close(tempfd);
  return numwr;
}

int rdr_scan::append_scan(int fd,off_t *startofs,int *size,
			  bool writeReflOnly) {
  if (fd < 0) {
    fprintf(stderr,"rdr_scan::append_scan ERROR fd < 0");
    return 0;
  }
  *startofs = lseek(fd,0,SEEK_END);
  *size = write_scan(fd, writeReflOnly);
  return *size;
}

/*
  write scan set to the given file at the current position
  Return no of bytes written
  An image header is written which details no. of scans, total size,
  and individual scan offsets and sizes

  THE TERM IMAGE SHOULD MORE CORRECTLY BE SCAN-SET.
  LEAVE AS IMAGE FOR BACKWARDS COMPATIBILITY

  Header format:
  /IMAGE: stnid yymmddhhmm
  /IMGSCANS: 23               no. of scans in img
  /IMGSIZE: 123456            total size of img including header
  /SCAN1: .....ofs ......sz   scan details incl offset and size of scans (SET SIZE)
  /SCAN2: ........
  ...
  ...
  /IMGHEADER END
  STNID: .....                normal scan data
  ....
  ....
  /IMAGEEND: stnid yymmddhhmm



  Scan offsets and sizes are initially left undefined (-1) and
  filled in after data is written.
  These are strictly 10 digit values which are filled in after each scan
  is written. A separate offset must be maintained to allow return to
  fill in these blanks
*/

int rdr_scan::write_scan_set(int fd,off_t *startofs,int *size,
			     bool writeReflOnly) {
  int wrtno = 0;
  int tempno, x ;
  off_t dataofs, scanheadofs, scandataofs, imgszofs;
  char  temp[128];
  tm rxtimestart, rxtimeend;
  rdr_scan *lcl_thisscan;

  if (this != rootscan) {
    fprintf(stderr,"rdr_scan::write_scan_set ERROR - called for CHILD SCAN\n");
    return 0;
  }
  if (startofs)
    *startofs = 0;
  if (size)
    *size = 0;
  if (fd < 0) {
    fprintf(stderr,"rdr_scan::write_scan_set ERROR fd < 0");
    return 0;
  }
  if (startofs)
    *startofs = lseek(fd,0,SEEK_CUR);   // points to start of image
  sprintf(temp,"/IMAGE: %04d %02d%02d%02d%02d%02d\n",station,year%100,month,day,hour,min);
  write_error = 0;
  if ((tempno = write(fd,temp,strlen(temp))) < 0) {
    perror("rdr_scan::write_scan_set ERROR ");
    if (size)
      *size = wrtno;
    write_error = errno;
    return wrtno;
  }
  wrtno += tempno;
  if (RxTimeStart())
    {
      time_t temptime = RxTimeStart();
      gmtime_r(&temptime, &rxtimestart);
      time_t temptime2 = RxTimeSetEnd();
      gmtime_r(&temptime2, &rxtimeend);
      rxtimestart.tm_year += 1900; // convert struct tm type ys since 1900 to actual year
      rxtimeend.tm_year += 1900; 
      sprintf(temp,"/RXTIME: %04d%02d%02d%02d%02d%02d %04d%02d%02d%02d%02d%02d\n",
	      rxtimestart.tm_year,rxtimestart.tm_mon+1,rxtimestart.tm_mday,
	      rxtimestart.tm_hour,rxtimestart.tm_min, rxtimestart.tm_sec,
	      rxtimeend.tm_year,rxtimeend.tm_mon+1,rxtimeend.tm_mday,
	      rxtimeend.tm_hour,rxtimeend.tm_min, rxtimeend.tm_sec);
      if ((tempno = write(fd,temp,strlen(temp))) < 0) {
	perror("rdr_scan::write_scan_set ERROR ");
	if (size)
	  *size = wrtno;
	write_error = errno;
	return wrtno;
      }
    }
  wrtno += tempno;
  if (writeReflOnly)
    sprintf(temp,"/IMAGESCANS: %d\n",completedTilts());
  else
    sprintf(temp,"/IMAGESCANS: %d\n",num_scans);
  if ((tempno = write(fd,temp,strlen(temp))) < 0) {
    perror("rdr_scan::write_scan_set ERROR ");
    if (size)
      *size = wrtno;
    write_error = errno;
    return wrtno;
  }
  wrtno += tempno;
  imgszofs = lseek(fd,0,SEEK_CUR);    // points to IMAGESIZE info
  sprintf(temp,"/IMAGESIZE: %8d\n",-1);
  if ((tempno = write(fd,temp,strlen(temp))) < 0) {
    perror("rdr_scan::write_scan_set ERROR ");
    if (size)
      *size = wrtno;
    write_error = errno;
    return wrtno;
  }
  wrtno += tempno;
  scanheadofs = lseek(fd,0,SEEK_CUR);   // points to SCAN1 info
  lcl_thisscan = rootScan();
  x = 1;
  while (lcl_thisscan) {  
    if (!writeReflOnly || 
	(lcl_thisscan->data_type == e_refl))
      {
	sprintf(temp,"/SCAN %4d: %4d %02d%02d%02d%02d%02d %2d %5.1f %2d %2d %10d %10d\n",x,0,0,0,0,0,0,0,0.,0,0,-1,-1);
	if ((tempno = write(fd,temp,strlen(temp))) < 0) {
	  perror("rdr_scan::write_scan_set ERROR ");
	  if (size)
	    *size = wrtno;
	  write_error = errno;
	  return wrtno;
	}
	x++;
      }
    wrtno += tempno;
    lcl_thisscan = lcl_thisscan->nextinset;
  }
  sprintf(temp,"/IMAGEHEADER END:\n");
  if ((tempno = write(fd,temp,strlen(temp))) < 0) {
    perror("rdr_scan::write_scan_set ERROR ");
    if (size)
      *size = wrtno;
    write_error = errno;
    return wrtno;
  }
  wrtno += tempno;
  dataofs = lseek(fd,0,SEEK_CUR);
  lcl_thisscan = rootScan();
  x = 1;
  while (lcl_thisscan) {
    if (!writeReflOnly || 
	(lcl_thisscan->data_type == e_refl))
      {
	scandataofs = dataofs;              // start of this scan
	if ((tempno = lcl_thisscan->write_scan(fd, writeReflOnly)) < 0) {
	  fprintf(stderr,"rdr_scan::write_scan_set ERROR WRITING SCAN");
	  if (size)
	    *size = wrtno;
	  return wrtno;
	}
	wrtno += tempno;
	dataofs = lseek(fd,0,SEEK_CUR); // save dataofs
	lseek(fd,scanheadofs,SEEK_SET); // return to write scan offset, etc
	sprintf(temp,
		"/SCAN %4d: %4d %02d%02d%02d%02d%02d "
		"%2d %5.1f %2d %2d %10ld %10d\n",
		x,lcl_thisscan->station,
		lcl_thisscan->year%100,lcl_thisscan->month,
		lcl_thisscan->day,lcl_thisscan->hour,lcl_thisscan->min,
		lcl_thisscan->scan_type,lcl_thisscan->set_angle/10.,
		lcl_thisscan->data_type,lcl_thisscan->data_fmt,
		(long int)(scandataofs),tempno);
	if ((tempno = write(fd,temp,strlen(temp))) < 0) {
	  perror("rdr_scan::write_scan_set ERROR ");
	  if (size)
	    *size = wrtno;
	  return wrtno;
	}                                   // DO NOT += tempno to wrtno.
	scanheadofs = lseek(fd,0,SEEK_CUR);   // points to next scan info
	lseek(fd,dataofs,SEEK_SET);   // reposition to append next data
	x++;
      }
    lcl_thisscan = lcl_thisscan->nextinset;
  }
  sprintf(temp,"\n/IMAGEEND: %04d %02d%02d%02d%02d%02d\n",
	  station,year%100,month,day,hour,min);
  if ((tempno = write(fd,temp,strlen(temp))) < 0) {
    perror("rdr_scan::write_scan_set ERROR ");
    if (size)
      *size = wrtno;
    write_error = errno;
    return wrtno;
  }
  wrtno += tempno;
  dataofs = lseek(fd,0,SEEK_CUR);
  lseek(fd,imgszofs,SEEK_SET);
  sprintf(temp,"/IMAGESIZE: %8d\n",wrtno);
  if ((tempno = write(fd,temp,strlen(temp))) < 0) {
    perror("rdr_scan::write_scan_set ERROR ");
    if (size)
      *size = wrtno;
    write_error = errno;
    return wrtno;
  }
  lseek(fd,dataofs,SEEK_SET);       // leave file offset to append after img
  if (size)
    *size = wrtno;
  return wrtno;
}

int rdr_scan::append_scan_set(int fd,off_t *startofs,int *size,
			      bool writeReflOnly) {
  off_t CurrentOfs;
  if (fd < 0) {
    fprintf(stderr,"rdr_scan::append_scan_set ERROR fd < 0");
    return 0;
  }
  CurrentOfs = lseek(fd,0,SEEK_END);
  return write_scan_set(fd,startofs,size, writeReflOnly);
};

bool rdr_scan::readRapicFile(char *rapicfname)
{

  if (!rapicfname) return false;
  int rapicfile = open(rapicfname, O_RDONLY);
  if (rapicfile < 0) return false;
  
  unsigned long ofs = 0, endofs = 0;
  int sz = 0;
  bool result = readRapicFile(rapicfile, ofs, endofs, sz);
  close(rapicfile);
  return result;
}

bool rdr_scan::readRapicFile(int rapicfd, 
			     unsigned long &offset, 
			     unsigned long &endoffset, int &sz)
{
  uchar c;
  //  bool debug = FALSE;
  bool done = FALSE;
  int	  buff_size = 2048;
  char  *buff = 0, *buff_pos;	// data buffer, & buff pos pointer
  int   b_size = 0, imgsize = 0;
  int   b_pos = 0;		// buff pos count
  rdr_scan_linebuff *linebuff = 0;
  int		pass1count = 0;
  //	bool	headerok = FALSE;


  int   total_rd = 0;

  if (rapicfd < 0) return false;

  buff = new char[buff_size];
  linebuff = new rdr_scan_linebuff();
  // if offset < 0, use current pos
  if (offset >= 0) lseek(rapicfd,offset,SEEK_SET);
  else offset = long(lseek(rapicfd,0,SEEK_CUR));
  endoffset = offset;
  while ((!done) && ((b_size = read(rapicfd,buff,buff_size)) > 0)) {
    done |= b_size == 0;
    total_rd += b_size;
    b_pos = 0;
    buff_pos = buff;
    while ((b_pos < b_size) && (!done)) {
      c = *buff_pos;
      imgsize++;		// imgsize is file size of this scan
      endoffset++;
      linebuff->addchar_parsed(c);
      if (linebuff->IsEOL())
	{
	  AddScanLine(linebuff->line_buff, linebuff->lb_size);
	  done |= Complete();
	  done |= HeaderValid() &&   // only done if a valid header has been read
	    (sscanf(linebuff->line_buff,"/IMAGEEND%c",&c) == 1);
	  if (sscanf(linebuff->line_buff," PASS: 01 of%c",&c) == 1) {
	    pass1count++;
	    done |= pass1count > 1;
	  }
	  linebuff->reset();
	}
      b_pos++;	    // buffer pos count
      buff_pos++;	    // buffer pos pointer
    }
    if (sz >= 0) done |= total_rd >= sz;
  }
  if (sz == -1) sz = imgsize;
  data_finished();
  if (linebuff) delete linebuff;
  if (buff) delete[] buff;
  if (!Finished())
    return false;	    // scan not finished or not valid
  else return true;
}  

void rdr_scan::AddScanLine(char *ln_buff, int lb_size) {    
  if (lb_size == 0) return;
  if (strstr(ln_buff, "/RXTIMES:"))    // pick up rxtimes from file read
    add_line(ln_buff,lb_size); 
  if (ln_buff[0] != '/')		// remove header lines inserted by db
    add_line(ln_buff,lb_size);
}
	
void rdr_scan::IncUserCount(void *user, char *str) {
  get_lock();
  UserCount++;
  rel_lock();
  if (useScanUserList)
  AddUserToList(user, str);
  if (debug)
    fprintf(stderr, "rdr_scan::IncUserCount - Called by %s, User count = %d\n", str, UserCount);
}

int rdr_scan::GetUserCount() {
  return UserCount;
}

bool rdr_scan::ShouldDelete(void *user, char *str) {
  char tempstr[256];
  if (useScanUserList)
    RemoveUserFromList(user, str);
  get_lock();
  UserCount--;
  if ( UserCount < 0) {
    sprintf(tempstr,"rdr_scan::ShouldDelete() - ERROR: UserCount < 0 (%d)", UserCount);
    RapicLog(tempstr, LOG_CRIT);
  }
  rel_lock();
  if (debug)
    fprintf(stderr,  "rdr_scan::ShouldDelete - Called by %s, User count = %d\n", str, UserCount);
  return (UserCount == 0);
}

bool rdr_scan::HasOSLock() {
  return (lock && lock->HasOSLock());
}

bool rdr_scan::ScanSame(rdr_scan *samescan) {
  return ((station == samescan->station) &&
	  (scan_time_t == samescan->scan_time_t) &&
	  (scan_type == samescan->scan_type) &&
	  (data_type == samescan->data_type) &&
	  (data_fmt == samescan->data_fmt));
}

char	*rdr_scan::ScanString(char *scanstring) {
  char *localstr = scanStr;

  if (scanstring) localstr = scanstring;
  if (header_valid)
  sprintf(localstr, "%s %02d:%02d %02d/%02d/%04d", stn_name(station), hour, min, 
	  day, month, year);
  else
    sprintf(localstr, "Header not valid");
  return localstr;
}

char	*rdr_scan::ScanString2(char *scanstring) {
  char *localstr = scanstring;

  if (!scanstring) return ScanString(); // do something safe
  if (header_valid)
    if (data_type == e_rainaccum)
      sprintf(localstr, "%s %02d:%02d %02d/%02d/%04d %s %s %6d", 
	      stn_name(station), hour, min, day, month, year, 
	      scan_type_text[scan_type], data_type_text[data_type],
	      int(Period()));
    else
      sprintf(localstr, "%s %02d:%02d %02d/%02d/%04d %s %s", 
	      stn_name(station), hour, min, day, month, year, 
	      scan_type_text[scan_type], data_type_text[data_type]);
  else
    sprintf(localstr, "Header not valid");
  return localstr;
}

time_t rdrScanKeyTimet(const char *scankey)
{
  if (!scankey) return 0;
  int year, month, day, hour, min, sec;
  if (sscanf(scankey, "%04d%02d%02d%02d%02d%02d",
	     &year, &month, &day, &hour, &min, &sec) == 6)
    return DateTime2UnixTime(year, month, day, hour, min, sec);
  else
    return 0;
}

char	*rdr_scan::ScanKey(char *scanstring) {
  char *localstr = scanKey;

  if (scanstring) localstr = scanstring;
  if (header_valid)
    {
      sprintf(localstr, "%04d%02d%02d%02d%02d%02d_%05d%02d%02d%02d%06d%06d",
	      year, month, day, hour, min, sec, station, int(scan_type), 
	      int(data_fmt),int(data_type), int(set_angle), int(Period()));
    }
  else
    sprintf(localstr, "0000- Header not valid");
  return localstr;
}

void rdr_scan::Write16lvlHeader(exp_buff *outbuff) {
  char *tempbuff;
  int buffsize = 0;

  tempbuff = new char[4096];
    
  Write16lvlHeader(tempbuff, buffsize, 4096);
  if (buffsize == 0) {
    fprintf(stderr, "rdr_scan::Write16lvlHeader FAILED, BUFFER TOO SMALL\n");
    delete[] tempbuff;
    return;
  }
  if (!outbuff) outbuff = rootscan->data_buff;
  if (outbuff) 
    {
      header_start = outbuff->get_size();
      outbuff->append_data(tempbuff,buffsize);
      data_start = outbuff->get_size();
    }
  delete[] tempbuff;
}    

/*
 * if unable to write header (buffer too small)
 * return with buffsize = 0
 * if buffsize > 0,  write was successful
 */

void rdr_scan::Write16lvlHeader(char *buffer, int &hdrsize, int maxsize) {
  int totallen = 0, linelen = 0;
  char line[4096], templn[2048];
  tm	TmStruct;
    
    
  hdrsize = 0;
    
  sprintf(line, "COUNTRY: %03d", CountryIDToCode(CountryID()));
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "NAME: %s", stn_name(StnID()));
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "STNID: %02d", StnID());
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  if (stnIDOK(StnID()))
    {
      sprintf(line, "LATITUDE: %4.3f", StnRec[StnID()].Lat());
      linelen = strlen(line) + 1;
      totallen += linelen;
      if (totallen < maxsize) memcpy(buffer, line, linelen);
      else return;
      buffer += linelen;
      
      sprintf(line, "LONGITUDE: %4.3f", StnRec[StnID()].Lng());
      linelen = strlen(line) + 1;
      totallen += linelen;
      if (totallen < maxsize) memcpy(buffer, line, linelen);
      else return;
      buffer += linelen;
      
      sprintf(line, "HEIGHT: %d", int(StnRec[StnID()].Ht() * 1000.0));
      linelen = strlen(line) + 1;
      totallen += linelen;
      if (totallen < maxsize) memcpy(buffer, line, linelen);
      else return;
      buffer += linelen;
    }
  
  UnixTime2DateTime(scan_time_t,year,month,day,hour,min,sec);

  sprintf(line, "DATE: %03d%02d", DoY(year, month, day), year % 100);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "TIME: %02d.%02d", hour, min);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "TIMESTAMP: %04d%02d%02d%02d%02d%02d", 
	  year, month, day, hour, min, sec);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  if (StartTm) {
    gmtime_r(&StartTm, &TmStruct);
    sprintf(line, "STARTTIME: %03d%02d %02d.%02d", 
	    DoY(TmStruct.tm_year + 1900, TmStruct.tm_mon + 1, 
		TmStruct.tm_mday), 
	    TmStruct.tm_year % 100, 
	    TmStruct.tm_hour, TmStruct.tm_min);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }

  if (EndTm) {
    gmtime_r(&EndTm, &TmStruct);
    sprintf(line, "ENDTIME: %03d%02d %02d.%02d", 
	    DoY(TmStruct.tm_year + 1900, TmStruct.tm_mon + 1, 
		TmStruct.tm_mday), 
	    TmStruct.tm_year % 100, 
	    TmStruct.tm_hour, TmStruct.tm_min);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }

  //    sprintf(line, "VERS: %5.2f", (float)CURRENTVERSION/100.0);

  sprintf(line, "RNGRES: %1.2f", rng_res);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "ANGRES: %3.1f", angle_res / 10.0);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "VIDRES: %d", NumLevels);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "STARTRNG: %1.2f", start_rng);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "ENDRNG: %d", int(max_rng * 1000.));
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  sprintf(line, "UNDEFINEDRNG: %1.2f", undefined_rng);
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  char sectorstr[128] = "";
  if (sectorScan)
    sprintf(sectorstr, " SECTOR ANGLE1=%1.1f ANGLE2=%1.1f ANGLEINCREASING=%d",
	    sectorStartAngle, sectorEndAngle, sectorAngleIncreasing);
    
  if ((scan_type != VOL) &&
      (scan_type != RHISet))
    sprintf(line, "PRODUCT: %s", get_scan_type_text(scan_type));
  else if (scan_type == VOL) {
    //sprintf(line, "PRODUCT: %s", get_scan_type_text(scan_type));
    //SD change 2mar05: make this header item legible by rapic
    sprintf(line, "PRODUCT: VOLUMETRIC [%02d%02d%03d%02d]%s",
	    rootScan()->hour, rootScan()->min,
	    DoY(rootScan()->year, rootScan()->month, rootScan()->day), 
	    rootScan()->year % 100, sectorstr);
    /*
      total += copytext("VOLUMETRIC [", destination);
      total += copynum(Chnl->PicInfo.basehour[Chnl->CurrentBuffer], 2, destination);
      total += copynum(Chnl->PicInfo.basemin[Chnl->CurrentBuffer], 2, destination);
      total += copynum(Int2BCD(Chnl->PicInfo.baseJDN[Chnl->CurrentBuffer]), 3, destination);
      total += copynum(Int2BCD(Chnl->PicInfo.baseyear[Chnl->CurrentBuffer]), 2, destination);
      total += copytext("]", destination);
      total += termline(destination, Chnl->AustPac);
      total += copytext("PASS: ", destination);
      total += copynum(Int2BCD(Chnl->PicInfo.PassNo), 2, destination);
      total += copytext(" of ", destination);
      total += copynum(Int2BCD(Chnl->PicInfo.TotalPasses), 2, destination);
    */
  }
  else if (scan_type == RHISet) {
    //sprintf(line, "PRODUCT: %s", get_scan_type_text(scan_type));
    //SD change 2mar05: make this header item legible by rapic
    sprintf(line, "PRODUCT: RHISet [%02d%02d%03d%02d]%s",
	    rootScan()->hour, rootScan()->min,
	    DoY(rootScan()->year, rootScan()->month, rootScan()->day), 
	    rootScan()->year % 100, sectorstr);
  }
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;

  //SD change 2mar05,  add pass to make this volume legible by rapic
  if ((scan_type == VOL) ||
      (scan_type == RHISet)) {
    sprintf(line, "PASS: %02d of %02d",vol_scan_no ,vol_scans);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }

  if ((scan_type == VOL) ||
      (scan_type == RHISet)) {
    if (!vol_tilt_no)
      vol_tilt_no = thisTilt();
    if (!vol_tilts)
      vol_tilts = completedTilts();
    sprintf(line, "TILT: %02d of %02d", vol_tilt_no, vol_tilts);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }


  //SD change 2mar05,  add creator for information only
  if (!scanCreatorString.empty()) {
    sprintf(line, "CREATOR: %s", scanCreatorString.c_str());
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }

  if (creatorFieldVersion > 0.0) {
    sprintf(line, "VERS: %4.2f", creatorFieldVersion);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }

  /* Information Specific to RHI images */
  if ((scan_type == RHI) || 
      (scan_type == RHISet)) {
    sprintf(line, "IMGFMT: RHI");
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
    sprintf(line, "AZIM: %5.1f", set_angle / 10.0);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }
  else if (scan_type == CAPPI) {
    sprintf(line, "IMGFMT: CAPPI");
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
    sprintf(line, "CAPPIHT: %d", int(set_angle));
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }
    
  /* Information Specific to PPI images */
  else {
    if(scan_type == VOL) 
      sprintf(line, "IMGFMT: PPI");
    else if(scan_type == CompPPI) 
      sprintf(line, "IMGFMT: CompPPI");
    else
      sprintf(line, "IMGFMT: PPI");
    sprintf(line, "ELEV: %5.1f", set_angle / 10.0);
    linelen = strlen(line) + 1;
    totallen += linelen;
    if (totallen < maxsize) memcpy(buffer, line, linelen);
    else return;
    buffer += linelen;
  }
//   linelen = strlen(line) + 1;
//   totallen += linelen;
//   if (totallen < maxsize) memcpy(buffer, line, linelen);
//   else return;
//   buffer += linelen;
//   sprintf(line, "ELEV: %5.1f", set_angle / 10.0);
//   linelen = strlen(line) + 1;
//   totallen += linelen;
//   if (totallen < maxsize) memcpy(buffer, line, linelen);
//   else return;
//   buffer += linelen;
  if ((data_type >= e_refl) && (data_type < e_dt_max))
    {
      sprintf(line, "VIDEO: %s", get_data_type_text(data_type));
      if ((data_type == e_vel) || 
	  (data_type == e_spectw)) {
	linelen = strlen(line) + 1; // add VIDEO line now
	totallen += linelen;
	if (totallen < maxsize) memcpy(buffer, line, linelen);
	else return;
	buffer += linelen;
	sprintf(line, "NYQUIST:%.1f", nyquist);
      }
    }
  else
    sprintf(line, "VIDEO: Undefined");
  linelen = strlen(line) + 1;
  totallen += linelen;
  if (totallen < maxsize) memcpy(buffer, line, linelen);
  else return;
  buffer += linelen;
  
  if (LvlTbls(data_type)) {
    if (LvlTbls(data_type)->getMode() != LTM_enum) {
      if (NumLevels < 256) { // Don't do table for binary data	
	strcpy(line, "DBZLVL:");
	if (NumLevels == 6) NumLevels = 7;  // it SHOULD REALLY BE 7
	for(int loop = 1; loop < NumLevels; loop++) {
	  sprintf(templn, " %1.1f", LvlTbls(data_type)->val(loop));
	  strcat(line, templn);
	}
	if (NumLevels == 7) NumLevels = 6;  // it SHOULD BE REALLY 7
	linelen = strlen(line) + 1;
	totallen += linelen;
	if (totallen < maxsize) memcpy(buffer, line, linelen);
	else return;
	buffer += linelen;
      }
    }
    else {    // LTM_enum, write out enum type strings
       strcpy(line, "TYPESTRINGS:");
      for(int loop = 0; loop < NumLevels; loop++) {
	if (loop < int(LvlTbls(data_type)->enumTypeStrings.size())) {
	  sprintf(templn, " %s", 
		  LvlTbls(data_type)->enumTypeStrings[loop].c_str());
	  strcat(line, templn);
	}
      }
      linelen = strlen(line) + 1;
      totallen += linelen;
      if (totallen < maxsize) memcpy(buffer, line, linelen);
      else return;
      buffer += linelen;
    }
  }
  hdrsize = totallen;
  return;
}
    
unsigned char* rdr_scan::Create2DPolarArray(int *rngdim, int *azdim,
					    uchar *destbuffer)
{
  unsigned char *pol_array = 0, *array_pnt = 0;
  s_radl *temp_radl = new s_radl(rngBins());
  short	    radlpos = 0;
  exp_buff_rd *dbuffrd = new exp_buff_rd(rootscan->data_buff);

  if (!data_valid) return pol_array;
  if (!*rngdim) *rngdim = rngBins();
  *azdim = 3600/angle_res;
  if (destbuffer)
    pol_array = destbuffer;
  else
    pol_array = new unsigned char[*rngdim * *azdim];
  memset(pol_array, 0, *rngdim * *azdim);
  array_pnt = pol_array;
  reset_radl(dbuffrd, &radlpos);
  while (get_next_radl(temp_radl, dbuffrd, &radlpos) != -1)
    {
      int azidx = temp_radl->az / angle_res;
      if (azidx < *azdim)
	{
	  array_pnt = pol_array + (azidx * *rngdim);
	  {
	    if (temp_radl->data_size <= *rngdim)
	      memcpy(array_pnt, temp_radl->data, temp_radl->data_size);
	    else 
	      memcpy(array_pnt, temp_radl->data, *rngdim);
	  }
	}
      
    }
  delete temp_radl;
  return pol_array;
}
    
rdr_scan_array *rdr_scan::CreateScanArray()
{
  int rngdim = rngBins();
  int azdim = 3600/angle_res;
  if (scanArray)
    deAllocScanArray();
  scanArray = new rdr_scan_array(rngdim, azdim, 1);
  Create2DPolarArray(&rngdim, &azdim,
		     scanArray->data);
  scanArray->rngRes = rngResKM();  // metres to km
  scanArray->azRes = angle_res_f();
  scanArray->startRng = startRngKM();; // metres to km 
  scanArray->endRng = endRngKM();
  scanArray->startAz = 0;
  scanArray->endAz = 360 - scanArray->azRes;
  scanArray->elTiltTable[0] = set_angle / 10.0;
  scanArray->dataType = data_type;
  return scanArray;
}

void rdr_scan::deAllocScanArray()
{
  if (scanArray)
    delete scanArray;
  scanArray = NULL;
}

rdr_scan_array* rdr_scan::CreateVolScanArray(e_data_type datafield)
{
  // dealloc volScanArray if it exists
  deAllocVolScanArray();
  rdr_scan *pscan = rootscan;
  int rngdim = rootscan->volMaxRngBins();
  int azdim = 3600 / rootscan->angle_res;
  int tiltdim = vol_tilts;
  allocVolScanArray(rngdim, azdim, tiltdim);
  volScanArray()->clearData();
  volScanArray()->rngRes = rng_res;
  volScanArray()->azRes = angle_res / 10.0;
  volScanArray()->startAz = 0;
  volScanArray()->endAz = 360 - volScanArray()->azRes;
  volScanArray()->dataType = datafield;
  uchar *pdata = NULL;
  while (pscan)
    {
      if (pscan->data_type == datafield)
	{
	  if (pscan->vol_tilt_no < tiltdim)
	    {
	      pdata = volScanArray()->data + (rngdim * tiltdim * vol_tilt_no);
	      pscan->Create2DPolarArray(&rngdim, &azdim, pdata);
	    }
	  volScanArray()->elTiltTable[pscan->vol_tilt_no] = 
	    pscan->set_angle / 10.0;
	}
      pscan = pscan->nextinset;
    }
  return volScanArray();
}
    
rdr_scan* rdr_scan::EncodeDataFrom2DPolarArray(unsigned char *dest_array, 
					       int dest_xdim, int dest_ydim, 
					       rdr_scan *srcscan)
{
  int az = 0, linelen = 0;
  s_radl *temp_radl = new s_radl(dest_xdim);
//   s_radl *temp_radl = new s_radl(rngBins());
  uchar tempstr[2048];
  uchar *c = 0;
  bool uselinebuff = false;
    
  if (!srcscan)
    srcscan = this;
    
  delete [] temp_radl->data;  // point data to data in array directly
  temp_radl->buffsize = dest_xdim;
  temp_radl->data_size = dest_xdim;
  temp_radl->numlevels = srcscan->NumLevels;
  srcscan->Write16lvlHeader(this->data_buff); // write header to data_buff
  if (!dest_array)
    {
      delete temp_radl;
      return 0;
    }
  temp_radl->el = srcscan->set_angle;
  for (az = 0; az < 360; az++) 
    {
      temp_radl->data = dest_array + (az * dest_xdim);
      temp_radl->data_size = dest_xdim;
      temp_radl->az = az * 10;
      linelen = temp_radl->EncodeAz(tempstr, 2048);
      if ((temp_radl->numlevels <= 160) && 
	  (linelen != int(strlen((char *)tempstr)+1)))
	{
	  fprintf(stderr, "rdr_scan::EncodeDataFrom2DPolarArray - ERROR"
		  "\nlinelen(%d)!=strlen(%d) az=%d\n", 
		  linelen, int(strlen((char *)tempstr)+1), temp_radl->az / 10);
	  uselinebuff = true; // be more careful parsing line
	}
      else
	uselinebuff = false;
      if (uselinebuff)
	{
	  linebuff->reset();// if END RADAR IMAGE pass to scan
	  c = tempstr;
	  while (!linebuff->IsEOL() && !linebuff->IsFull()) {
	    linebuff->addchar_parsed(*c);
	    c++;
	  }
	  if (linebuff->IsEOL())
	    add_line(linebuff);
	}
      else
	{
	  add_line((char *)tempstr, linelen);
	}
    }
  add_line("END RADAR IMAGE");
  temp_radl->data = NULL;    // clear data pointer
  delete temp_radl;
  return lastScan();
}

void rdr_scan::WriteDataFrom2DPolarArray(int fd, unsigned char *dest_array, 
					 int dest_xdim, int dest_ydim)
{
  //SD write out data to file 24feb05
  //should calc max_rng here from dest_xdim and set in this.
  int az = 0, linelen = 0;
  s_radl *temp_radl = new s_radl(rngBins());
  char tempstr[2048];
  //char passstr[100];
  
  char *c = 0;
  bool uselinebuff = false;

  max_rng = startRngKM() + dest_xdim*rngResKM();  //try this here 25/2
    
  delete[] temp_radl->data;  // point data to data in array directly. //SD 29/3/5 add [] due to valgrind Mismatched free() / delete / delete []
  temp_radl->buffsize = dest_xdim;
  temp_radl->data_size = dest_xdim;
  temp_radl->numlevels = this->NumLevels;
  char *tempbuff;
  int buffsize = 0;

  tempbuff = new char[4096];
    
  Write16lvlHeader(tempbuff, buffsize, 4096);  //modified this to suit!
  //now write out header to file, using buffsize 
  if (!write(fd,tempbuff,buffsize)) 
    fprintf(stderr, "rdr_scan::WriteDataFrom2DPolarArray - ERROR writing header\n");
  //sprintf(passstr,"PASS: %02d of %02d",vol_scan_no ,vol_scans);
  //if (!write(fd,passstr,strlen((char *)passstr)+1)) 
  //fprintf(stderr, "rdr_scan::WriteDataFrom2DPolarArray - ERROR writing PASS stt\n");
  //SD debug
  //printf("PASSSTR IS :%s\n",passstr);
  
  if (!dest_array)
    {
      delete temp_radl;
      return ;
    }
  temp_radl->el = set_angle;
  for (az = 0; az < 360; az++) 
    {
      temp_radl->data = dest_array + (az * dest_xdim);
      temp_radl->data_size = dest_xdim;
      temp_radl->az = az * 10;
      linelen = temp_radl->EncodeAz((uchar*)tempstr, 2048);
      if ((temp_radl->numlevels <= 160) && 
	  (linelen != int(strlen((char *)tempstr)+1)))
	{
	  fprintf(stderr, "rdr_scan::WriteDataFrom2DPolarArray - ERROR"
		  "\nlinelen(%d)!=strlen(%d) az=%d\n", 
		  linelen, int(strlen((char *)tempstr)+1), temp_radl->az / 10);
	  uselinebuff = true; // be more careful parsing line
	}
      else
	uselinebuff = false;
      if (uselinebuff)
	{
	  linebuff->reset();// if END RADAR IMAGE pass to scan
	  c = tempstr;
	  while (!linebuff->IsEOL() && !linebuff->IsFull()) {
	    linebuff->addchar_parsed(*c);
	    c++;
	  }
	  if (linebuff->IsEOL())
	    //write it to file directly, use strlen for char count
	    //add_line(linebuff);
	    if (!write(fd,linebuff->line_buff, linebuff->lb_size)) 
	      fprintf(stderr, "rdr_scan::WriteDataFrom2DPolarArray - ERROR writing linebuff\n");
	    
	}
      else
	{
	    //write it to file directly, use strlen for char count
	  //add_line((char *)tempstr, linelen);
	  if (!write(fd,tempstr, linelen)) 
	    fprintf(stderr, "rdr_scan::WriteDataFrom2DPolarArray - ERROR writing tempstr\n");
	}
      
    }
					    
  //write it to file directly, use strlen for char count
  //add_line("END RADAR IMAGE");
  if (!write(fd,"END RADAR IMAGE\0",16)) 
    fprintf(stderr, "rdr_scan::WriteDataFrom2DPolarArray - ERROR writing END RADAR IMAGE\n");
  temp_radl->data = 0;    // clear data pointer
  delete temp_radl;
  return;
}

/*
  rdr_scan* rdr_scan::getFilteredScan(rdrfilter *scanfilter)
  {
  if (!filterlock)
  filterlock = new spinlock("rdr_scan->filterlock", 2000);    // 20 secs
  if (filterlock) filterlock->get_lock();
  if (!filteredScan)
  MakeFilteredScan(this, scanfilter);
  if (filterlock) filterlock->rel_lock();
  return filteredScan;
  }

  void rdr_scan::MakeFilteredScan(rdr_scan *unfilteredScan,
  rdrfilter *scanfilter)
  {
  rdrfilter *localfilter = 0;

  if (!unfilteredScan || filteredScan)	// we already have a filtered scan for this
  return;

  // The whole filter part could be expanded to provide various filters
  // Currently it is confined to a median type of speckle filter primarily for vel filtering


  if (!scanfilter)
  {
  localfilter = new rdrfilter;
  scanfilter = localfilter;
  }
  filteredScan = scanfilter->MakeFilteredScan(unfilteredScan, rootscan->filteredScanSet);
  if (filteredScan)
  filteredScan->isFilteredScan = true;
  if (!rootscan->filteredScanSet)	// this was first(root) in set
  rootscan->filteredScanSet = filteredScan; // assign root scan
  filteredScanSet = rootscan->filteredScanSet;
  if (localfilter)
  {
  delete localfilter;
  localfilter = NULL;
  }
  // give this scan a reference to the filtered scanset root
  }
*/

void rdr_scan::setCreator(void *creator)
{
  scanCreator = creator;
}

void* rdr_scan::getCreator()
{
  return scanCreator;
}

void rdr_scan::addProductScan(rdr_scan* newProdScan)	// add product scan to this rdr_scan
{
  if (!productScanLock)
    productScanLock = new spinlock("rdrscan->productScanLock", 2000);	// 20 secs
  if (productScanLock) productScanLock->get_lock();
  productScans.push_back(newProdScan);
  if (productScanLock) productScanLock->rel_lock();
}

bool rdr_scan::delAllProductScans(void *creator, char *str)  // del all product scans
{
  bool changed = false;
  list<rdr_scan*>::iterator first = productScans.begin();
  list<rdr_scan*>::iterator listthis;
  list<rdr_scan*>::iterator last = productScans.end();
  rdr_scan* thisscan = NULL;

  if (!productScanLock)
    productScanLock = new spinlock("rdrscan->productScanLock", 2000);	// 20 secs
  if (productScanLock) productScanLock->get_lock();
  while (first != last)
    {
      thisscan = (*first);
      if (thisscan && thisscan->ShouldDelete(creator, str))
	{
	  changed = true;
	  delete thisscan;
	}
      listthis = first;
      first++;
      productScans.erase(listthis);
    }
  if (productScanLock) productScanLock->rel_lock();
  return changed;
}

bool rdr_scan::delProductScansByCreator(void *creator, char *str)	// del all product scans from this creator
{
  bool changed = false;
  list<rdr_scan*>::iterator first = productScans.begin();
  list<rdr_scan*>::iterator listthis;
  list<rdr_scan*>::iterator last = productScans.end();
  rdr_scan* thisscan = NULL;

  if (!productScanLock)
    productScanLock = new spinlock("rdrscan->productScanLock", 2000);	// 20 secs
  if (productScanLock) productScanLock->get_lock();
  while (first != last)
    {
      thisscan = (*first);
      if (thisscan->getCreator() == creator)
	{
	  if (thisscan->ShouldDelete(creator, str))
	    {
	      changed = true;
	      delete thisscan;
	    }
	  listthis = first;
	  first++;
	  productScans.erase(listthis);
	}
      else
	first++;
    }

  if (productScanLock) productScanLock->rel_lock();
  return changed;
}

rdr_scan* rdr_scan::getProductScanByCreator(void *creator)  // return first product scan from this creator
{
  list<rdr_scan*>::iterator first = productScans.begin();
  list<rdr_scan*>::iterator last = productScans.end();
  rdr_scan* thisscan = NULL;
  bool found = false;

  if (!productScanLock)
    productScanLock = new spinlock("rdrscan->productScanLock", 2000);	// 20 secs
  if (productScanLock) productScanLock->get_lock();
  while (!found && (first != last))
    {
      thisscan = (*first);
      found = thisscan->getCreator() == creator;
      first++;
    }
  if (productScanLock) productScanLock->rel_lock();
  if (found)
    return thisscan;
  else
    return NULL;
}

rdr_scan* rdr_scan::getProductScanByScanCreator(scanProductCreator *creator)  // return first product scan from this creator
{
  rdr_scan* thisscan = NULL;

  thisscan = getProductScanByCreator(creator);
  if (thisscan &&
      ((thisscan->num_scans == -1) ||      // if num_scans == -1, don't recalc it on vol change
       (thisscan->num_scans == num_scans)))  // scan product found, and vol hasn't changed
    return thisscan;                                    // return it
  if (thisscan)                            // existing product doesn't match vol scans any more
    delProductScansByCreator(creator, "rdr_scan::getProductScanByScanCreator");    // remove it and recalculate it
  thisscan = creator->createScanProduct(this);
  if (thisscan)
    addProductScan(thisscan);
  return thisscan;
}

rdr_scan_node* rdr_scan::getProductScanNodeByCreator(void *creator)  // return first product scan from this creator
{
  rdr_scan* thisscan = NULL;
  rdr_scan_node *thisscannode = NULL;

  thisscan = getProductScanByCreator(creator);
  if (thisscan)
    thisscannode = new rdr_scan_node(creator, "rdr_scan::getProductScanByCreator", thisscan);
  return thisscannode;
}

int rdr_scan::scanSetCount()
{
  return num_scans;
}

int rdr_scan::scanSetSize() {
  if (rootscan->data_buff)
    return rootscan->data_buff->get_size();
  else
    return 0;
}

int rdr_scan::thisScanSize() {
  
  if (nextinset)
    return header_start - nextinset->header_start;
  else
    return scanSetSize()-header_start;
}

void rdr_scan::setRxTimeStart(time_t rxstart, bool force, char *debugstr)
{
  if ((!rxTimeStart) || force)  // only set if not already set or force
    {
      rxTimeStart = rxstart;
      char tempstr[128];
      if (debugstr)
	fprintf(stdout, "rdr_scan::setRxTimeStart to %s - %s\n",
		ShortTimeString(rxstart, tempstr), debugstr);
//       else
// 	fprintf(stdout, "rdr_scan::setRxTimeStart to %s\n",
// 		ShortTimeString(rxstart, tempstr));
    }
}

void rdr_scan::setRxTimeEnd(time_t rxend, bool force, char *debugstr)
{
  if (!rxTimeEnd || force)  // only set if not already set or force
    {
      rxTimeEnd = rxend;
      char tempstr[128];
      if (debugstr)
	fprintf(stdout, "rdr_scan::setRxTimeEnd to %s - %s\n",
		ShortTimeString(rxend, tempstr), debugstr);
//       else
// 	fprintf(stdout, "rdr_scan::setRxTimeEnd to %s\n",
// 		ShortTimeString(rxend, tempstr));
    }
}

void rdr_scan::setRxTimeSetEnd(time_t rxsetend, bool force, char *debugstr)
{
  if (!rxTimeSetEnd || force)  // only set if not already set or force
    {
      rxTimeSetEnd = rxsetend;
      char tempstr[128];
      if (debugstr)
	fprintf(stdout, "rdr_scan::setRxTimeSetEnd to %s - %s\n",
		ShortTimeString(rxsetend, tempstr), debugstr);
//       else
// 	fprintf(stdout, "rdr_scan::setRxTimeSetEnd to %s\n",
// 		ShortTimeString(rxsetend, tempstr));
    }
}

int rdr_scan::rxTimeSecs()             // secs btwn end&start - return at least 1
{
  if (rxTimeEnd != rxTimeStart)
    return rxTimeEnd - rxTimeStart;
  else
    return 1;  // return > 0, to avoid divide by zero issues
}

int rdr_scan::productScanCount()
{
  return productScans.size();
}

int rdr_scan::productScansSize() {
  list<rdr_scan*>::iterator first = productScans.begin();
  list<rdr_scan*>::iterator last = productScans.end();
  rdr_scan* thisscan = NULL;
  int sz = 0;
  int prodCount = productScans.size();

  if (!prodCount)
    return sz;
  if (!productScanLock)
    productScanLock = new spinlock("rdrscan->productScanLock", 2000);	// 20 secs
  if (productScanLock) productScanLock->get_lock();
  while (first != last)
    {
      thisscan = (*first);
      sz += thisscan->scanSetSize();
      first++;
    }

  if (productScanLock) productScanLock->rel_lock();
  return sz;
}

int rdr_scan::completedTilts() 
{
  rdr_scan* thisscan = rootscan;
  rdr_angle lastel = thisscan->set_angle;
  int tiltcount = 1;

  while (thisscan)
    {
      if (thisscan->scan_complete &&
	  (lastel != thisscan->set_angle))
	{
	  tiltcount++;
	  lastel = thisscan->set_angle;
	}
      thisscan = thisscan->nextinset;
    }
  return (completed_tilts = tiltcount);
} 

int rdr_scan::thisTilt(rdr_scan *tiltscan) 
{
  if (!tiltscan)
    tiltscan = this;
  rdr_scan* thisscan = rootscan;
  rdr_angle lastel = thisscan->set_angle;
  int tiltcount = 1;
  bool done = false;

  while (!done && thisscan)
    {
      if (thisscan->scan_complete &&
	  (lastel != thisscan->set_angle))
	{
	  tiltcount++;
	  lastel = thisscan->set_angle;
	}
      done =  thisscan == tiltscan;
      thisscan = thisscan->nextinset;
    }
  return (tiltcount);
} 

int rdr_scan::completedScans() 
{
  rdr_scan* thisscan = rootscan;
  int scancount = 0;

  while (thisscan)
    {
      scancount++;
      thisscan = thisscan->nextinset;
    }
  return scancount;
} 

rdr_scan_node::rdr_scan_node(void *user, char *str, rdr_scan *ThisScan) {
  next = prev = 0;
  scansinset = 0;
  scan = 0;
  GPFlag = FALSE;
  userpnt = 0;
  strcpy(userstr, "");
  attachscan(user, str, ThisScan);
}

rdr_scan_node::~rdr_scan_node() {
  remove_from_list();
  /* SCANFREELIST
     if (scan->ShouldDelete()) {
     if (FreeListMng && FreeListMng->RdrScanFreeList) 
     RdrScanFreeList->StoreRdrScan(scan);
     else delete scan;
     }
  */
  if (scan && scan->ShouldDelete(userpnt, userstr)) 
    delete scan;
}

void rdr_scan_node::remove_from_list() {		// remove from linked list, move prev/next links
  if (next)
    next->prev = prev;
  if (prev)
    prev->next = next;
  next = 0;
  prev = 0;
}

void rdr_scan_node::attachscan(void *user, char *str, rdr_scan *ThisScan) {
  if (scan) {
    fprintf(stderr,"rdr_scan_node::attachscan - ERROR Scan already attached\n");
    return;
  }
  userpnt = user;
  scan = ThisScan;
  if (str)
    {
      strncpy(userstr, str, 58);
      strcat(userstr, "-node");
    }
  else
    strcpy(userstr, "rdr_scan_node");
  if (scan) 
    scan->IncUserCount(userpnt, userstr);
  reset_scan();
}    

void rdr_scan_node::detachscan() {
  if (!scan) {
    fprintf(stderr,"rdr_scan_node::detachscan - ERROR Scan not defined\n");
    return;
  }
  /* SCANFREELIST
     if (scan->ShouldDelete()) {
     if (FreeListMng && FreeListMng->RdrScanFreeList) 
     RdrScanFreeList->StoreRdrScan(scan);
     else delete scan;
     }
  */
  if (scan->ShouldDelete(userpnt, userstr)) delete scan;
  scan = 0;
}    

int rdr_scan_node::scanSetCount() {
  
  if (scan)
    return scan->scanSetCount();
  else return 0;
}

int rdr_scan_node::scanSetSize()
{
  if (scan)
    return scan->scanSetSize();
  else return 0;
}

// return number of all scans (including children) from this node in img linked list
int rdr_scan_node::ListCount() {
   int count = 0;
   rdr_scan_node *temp = this;
  
   while (temp) {
     count += temp->scan->scanSetCount();
     temp = temp->next;
   }
   return count;
 }
  
// return number of root scans from this node in img linked list
int rdr_scan_node::ListRootCount() {
   int count = 0;
   rdr_scan_node *temp = this;
  
   while (temp) {
     count++;
     temp = temp->next;
   }
   return count;
 }
  
int rdr_scan_node::ListSize() {
   int size = 0;
   rdr_scan_node *temp = this;
  
   while (temp) {
     if (temp->scan)
       size += temp->scan->scanSetSize();
     temp = temp->next;
   }
   return size;
 }

int rdr_scan_node::ListSizeVol() {
  int size = 0;
  rdr_scan_node *temp = this;
  
  while (temp) {
    if ((temp->scan) &&
	(temp->scan->scan_type == VOL))
      size += temp->scan->scanSetSize();
    temp = temp->next;
  }
  return size;
}
  
int rdr_scan_node::ListSizePPI() {
   int size = 0;
   rdr_scan_node *temp = this;
  
   while (temp) {
     if ((temp->scan) &&
	 ((temp->scan->scan_type == CompPPI) ||
	  (temp->scan->scan_type == PPI)))
       size += temp->scan->scanSetSize();
     temp = temp->next;
   }
   return size;
}
 
int rdr_scan_node::productScanCount() {
  
  if (scan)
    return scan->productScanCount();
  else return 0;
}

int rdr_scan_node::productScansSize()
{
  if (scan)
    return scan->productScansSize();
  else return 0;
}

// return number of scans from this node in img linked list
int rdr_scan_node::productListCount() {
   int count = 0;
   rdr_scan_node *temp = this;
  
   while (temp) {
     if (temp->scan)
       count += temp->scan->productScanCount();
     temp = temp->next;
   }
   return count;
 }
  
int rdr_scan_node::productListSize() {
   int size = 0;
   rdr_scan_node *temp = this;
  
   while (temp) {
     if (temp->scan)
       size += temp->scan->productScansSize();
     temp = temp->next;
   }
   return size;
 }
  
 
 rdr_scan* rdr_scan_node::goto_scan(int n) {
  this_scan_no = n;
  if (scan) return (thisscan = scan->gotoScan(n));
  else {
    fprintf(stderr,"rdr_scan_node::goto_scan - ERROR scan = 0\n");
    return 0;
  }
}

rdr_scan* rdr_scan_node::reset_scan() {
  this_scan_no = 0;
  if (scan) return (thisscan = scan->gotoScan(0));
  else {
    fprintf(stderr,"rdr_scan_node::goto_scan - ERROR scan = 0\n");
    return 0;
  }
}

rdr_scan* rdr_scan_node::next_scan() {
  this_scan_no++;
  if (scan) return (thisscan = scan->gotoScan(this_scan_no));
  else {
    fprintf(stderr,"rdr_scan_node::goto_scan - ERROR scan = 0\n");
    return 0;
  }
}

rdr_scan* rdr_scan_node::prev_scan() {
  if (this_scan_no > 0) this_scan_no--;
  if (scan) return (thisscan = scan->gotoScan(this_scan_no));
  else {
    fprintf(stderr,"rdr_scan_node::goto_scan - ERROR scan = 0\n");
    return 0;
  }
}

rdr_scan* rdr_scan_node::this_scan() {
  return thisscan;
}

scanProductCreator::scanProductCreator()
{
    
}


scanProductCreator::~scanProductCreator()
{
    
}

rdr_scan* scanProductCreator::createScanProduct(rdr_scan *src_scan, rdr_scan *scansetroot)
{
  fprintf(stderr, "scanProductCreator::createScanProduct - MUST BE DEFINED IN CHILD CLASS\n");
  return NULL;
}


