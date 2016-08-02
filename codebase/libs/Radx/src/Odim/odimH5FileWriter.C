/*

  odimH5FileWriter.C
  
  Implementation of odimH5FileWriter.h classes

*/


#include "odimH5FileWriter.h"
#include "rpEventTimer.h"
#include "rdrutils.h"
#include <hdf5.h>
#include <string>
#include <stdint.h>
#include <stdlib.h>

char* odimH5Conventions = "ODIM_H5/V2_0";
char* odim_data_type_text[] = {
  "DBZH","VRAD","WRAD", "ZDR", "TH", "ACRR", 
  "VRAD", "VIL", "HGHT", 
  "THGHT", "PHIDP", "ACRR", "Unused2",
  "RHOHV", "CLASS", "RATE", "QCFLAGS", "Unused5",
  "DBZH_CLEAN", "VRAD_CLEAN", "CLASS"
  "" // INSERT NEW STRINGS BEFORE THIS
};

char *odim_scan_type_text[] = {
  "SCAN","XSEC","UNDEF","UNDEF","PVOL", 
  "XSEC", "UNDEF", "UNDEF", "UNDEF", "IMAGE", 
	
  ""  // insert new string corresponding to new e_scan_type before this
};

class hid_handle
{
public:
  explicit hid_handle(hid_t id)
    : id_(id)
  { }

  ~hid_handle()
  {
    if (id_ > 0)
      H5Idec_ref(id_);
  }

  operator hid_t() const
  {
    return id_;
  }

  operator bool() const
  {
    return id_ > 0;
  }

private:
  // prevent copying of handles
  hid_handle(const hid_handle& rhs);
  hid_handle& operator=(const hid_handle& rhs);

private:
  hid_t id_;
};


// Need to use global odimh5Convert Lock because hdf5 library
// seems to be badly thread unsafe
spinlock *globalODIMh5ConvertLock = NULL;

char *get_odim_data_type_text(e_data_type datatype) {
  if ((datatype >= 0) &&
      (datatype < e_dt_max)) return odim_data_type_text[datatype];
  else return "BADQUANTITY";
}

e_data_type get_odim_data_type(const char *dtastr) {
  int tempint;
  // following converts string to bf_data_type
  return 
    e_data_type(String2Int((char*)dtastr, odim_data_type_text, e_dt_max-1));

  // if (tempint < 0) return tempint;
  // else return (1 << tempint);	    // convert enum to bit field
  // */
  // if (data_type_string_map.size() == 0)
  //   readDataTypeStrings();
  // data_type_string_map_lock->get_lock("get_data_type");
  // e_data_type type =  e_dt_max;
  // std::map<std::string, e_data_type>::iterator iter = 
  //   data_type_string_map.find(dtastr);
  // if (iter != data_type_string_map.end())
  //   type = iter->second;
  // else
  //   type = e_data_type(String2Int(dtastr, _data_type_text, e_dt_max-1, false));
  // data_type_string_map_lock->rel_lock();
  // if ((type >= 0) && (type < e_dt_max)) 
  //   return type;
  // else
  //   return e_dt_any;
}

e_scan_type get_odim_scan_type(const char *scantypestr) {
  return 
    e_scan_type(String2Int((char*)scantypestr, odim_scan_type_text, 
			   e_st_max-1));
}

odimH5FileWriter::odimH5FileWriter(char *rapicODIMH5_fname, 
				   rdr_scan *rapicscan)
{
  type = RFRW_OdimH5;
  setDefaults();
  if (rapicscan && rapicODIMH5_fname)
    writeFile(rapicODIMH5_fname, rapicscan);
}
  
odimH5FileWriter::odimH5FileWriter(char *rapicODIMH5_fname, 
				   rdr_scan *rapicscan,
				   bool writefloat)
{
  type = RFRW_OdimH5;
  setDefaults();
  writeFloat = writefloat;
  if (rapicscan && rapicODIMH5_fname)
    writeFile(rapicODIMH5_fname, rapicscan);
}

odimH5FileWriter::~odimH5FileWriter()
{
}

void odimH5FileWriter::setDefaults()
{
  writeFloat = false;
  compressChunkSize[0] = 45;
  compressChunkSize[1] = 80;
  compressFactor = 6;
  useSzip = false;
  szipPixelsPerBlock = 16;
}

bool odimH5FileWriter::getLock(char *lockholder)
{
  bool result = false;
  if (!globalODIMh5ConvertLock)
    globalODIMh5ConvertLock = new spinlock("odimH5FileWriter", 30.0);
  if (globalODIMh5ConvertLock)
     result = globalODIMh5ConvertLock->get_lock(lockholder);
  return result;
}

void odimH5FileWriter::relLock()
{
  if (globalODIMh5ConvertLock)
    globalODIMh5ConvertLock->rel_lock();
}

bool odimH5FileWriter::writeFile(char *rapicODIMH5_fname, rdr_scan *rapicscan)
{
  if (!rapicscan || !rapicODIMH5_fname)
    return false;

  rpEventTimer eventTimer;
  
  if (!getLock("odimH5FileWriter::writeFile"))
    return false;

  eventTimer.start();

  // we split the body of this function out to ensure that the HDF file is
  // closed (when the handle goes out of scope) before we call relLock()
  writeFile_impl(rapicODIMH5_fname, rapicscan);

  eventTimer.stop();
  fprintf(stdout, "odimH5FileWriter::writeFile - Elapsed time = %1.2fsecs\n",
          eventTimer.totalTime());
  
  relLock();

  return 1;
}

void odimH5FileWriter::writeFile_impl(char *rapicODIMH5_fname, 
				      rdr_scan *rapicscan)
{
  herr_t status;

  /* Create a new file using default properties. */
  hid_handle file_id(H5Fcreate(rapicODIMH5_fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT));

  hid_handle root_id(H5Gopen(file_id, "/", H5P_DEFAULT));
  status = H5Acreatestring(root_id, "Conventions", odimH5Conventions);

  {
    hid_handle group_id(H5Gcreate(root_id, "what", 0, 0, 0));
    status = H5Acreatestring(group_id, "object", "PVOL");
    status = H5Acreatestring(group_id, "version", "H5rad 2.0");

    char str[100];
    time_t scantime = rapicscan->ScanTime();
    struct tm * local = gmtime(&scantime);
    sprintf(str, "%04d%02d%02d", local->tm_year+1900, local->tm_mon+1, local->tm_mday);
    status = H5Acreatestring(group_id, "date", str);
    sprintf(str, "%02d%02d%02d", local->tm_hour, local->tm_min, local->tm_sec);
    status = H5Acreatestring(group_id, "time", str);

    sprintf(str, "RAD:%s%02d,PLC:%s", rapicscan->countryStr(), rapicscan->StnID(),
            rapicscan->stnName());
    status = H5Acreatestring(group_id, "source", str);
  }

  {
    hid_handle group_id(H5Gcreate(root_id, "where", 0, 0, 0));
    status = H5Acreatedouble(group_id, "lat", rapicscan->latitude());
    status = H5Acreatedouble(group_id, "lon", rapicscan->longitude());
    status = H5Acreatedouble(group_id, "height", rapicscan->height() * 1000.0);
  }

  {
    hid_handle group_id(H5Gcreate(root_id, "how", 0, 0, 0));
    if (rapicscan->rdr_params.wavelength)
      status = H5Acreatedouble(group_id, "wavelength", rapicscan->rdr_params.wavelength / 10.0);
    if (rapicscan->rdr_params.az_beamwidth)
      status = H5Acreatedouble(group_id, "beamwH", rapicscan->rdr_params.az_beamwidth / 10.0);
    if (rapicscan->rdr_params.el_beamwidth)
      status = H5Acreatedouble(group_id, "beamwV", rapicscan->rdr_params.el_beamwidth / 10.0);
  }

  rdr_scan *thisscan = rapicscan->rootScan();
//   int thistiltnum = thisscan->vol_tilt_no;
  int thistiltnum = thisscan->getThisTiltNum(thisscan);
  
  while (thisscan) {
    char str[20];
    int maxTiltBins = thisscan->getTiltMaxBins();

//      thistiltnum = thisscan->vol_tilt_no;
    thistiltnum = thisscan->getThisTiltNum(thisscan);
    fprintf(stdout, "Writing tilt %d at el=%f\n", thistiltnum, thisscan->setAngle());
    sprintf(str, "dataset%d", thistiltnum);

    hid_handle group_id(H5Gcreate(root_id, str, 0, 0, 0));

    {
      hid_handle g_id(H5Gcreate(group_id, "what", 0, 0, 0));
      status = H5Acreatestring(g_id, "product", "SCAN");

      char str[100];
      time_t scantime = thisscan->ScanTime();
      struct tm * local = gmtime(&scantime);
      sprintf(str, "%04d%02d%02d", local->tm_year+1900, local->tm_mon+1, local->tm_mday);
      status = H5Acreatestring(g_id, "startdate", str);
      sprintf(str, "%02d%02d%02d", local->tm_hour, local->tm_min, local->tm_sec);
      status = H5Acreatestring(g_id, "starttime", str);
      // we don't keep scan end time, use next tilt time - 1 if avial
      // else use start+20 for typical 3RPM
      if (thisscan->nextTilt())
        scantime = thisscan->nextTilt()->ScanTime()-1;
      else
        scantime += 20;
      local = gmtime(&scantime);
      sprintf(str, "%04d%02d%02d", local->tm_year+1900, local->tm_mon+1, local->tm_mday);
      status = H5Acreatestring(g_id, "enddate", str);
      sprintf(str, "%02d%02d%02d", local->tm_hour, local->tm_min, local->tm_sec);
      status = H5Acreatestring(g_id, "endtime", str);
    }

    {
      hid_handle g_id(H5Gcreate(group_id, "where", 0, 0, 0));
      status = H5Acreatedouble(g_id, "elangle", thisscan->setAngle());
      status = H5Acreatelong(g_id, "nbins", maxTiltBins);
      status = H5Acreatedouble(g_id, "rstart", thisscan->startRngKM());
      status = H5Acreatedouble(g_id, "rscale", thisscan->rng_res);
      status = H5Acreatelong(g_id, "nrays", thisscan->numRadls());
      // just use 0 as 1st azimuth radiated in the scan
      status = H5Acreatelong(g_id, "a1gate", 0);
    }

    {
      hid_handle g_id(H5Gcreate(group_id, "how", 0, 0, 0));
      // ODIM_H5 expects gate 0 to span 0-1 degrees, rapic expects gate 0 to span -0.5 - 0.5 degrees.
      // We insert this attribute to tell ODIM_H5 readers to offset the gate locations 0.5 degrees CCW.
      status = H5Acreatedouble(g_id, "gate_start", -0.5);
    }

    int j = 1;
    while (thisscan &&
           (thistiltnum == thisscan->getThisTiltNum(thisscan)))
//         (thistiltnum == thisscan->vol_tilt_no))
    {
      char str[20];

      fprintf(stdout, "  Writing field %s\n",
              get_data_type_text(thisscan->data_type));

      double gain = 1.0, offset = 0.0, nodata = 0.0;
      if (!writeFloat)  // float arrays use default unity gain, no offset
      {
        switch (thisscan->data_type) {
          case e_refl:
          case e_rawrefl:
            // encode all refl as -32 to 95.5 dBZ 0.5dbZ steps
            gain = 0.5;
            offset = -32.0;
            nodata = 0.0;
            break;
          case e_vel:
            // For typical 160 levels, 0=no echo, 80=0vel
            // 1 = -nyquist, 159 = +nyquist
            gain = (thisscan->nyquist * 2) / (thisscan->NumLevels - 2);
            offset = -thisscan->nyquist - gain;
            nodata = 0.0;
            break;
          case e_spectw:
            // For typical 160 levels, 0=no echo, 80=0vel
            // 1 = -nyquist, 159 = +nyquist
            gain = (thisscan->nyquist) / (thisscan->NumLevels - 2);
            offset = 0.0;
            nodata = 0.0;
            break;
          case e_phidp:
            // assume range is -90.0 - 90.0 degs
            gain = 180.0/(thisscan->NumLevels - 2);
            offset = -90.0 - gain;
            nodata = 0.0;
            break;
          case e_diffz:
            gain = 18.0/(thisscan->NumLevels - 2);
            offset = -9.0 - gain;
            nodata = 0.0;
            break;
          case e_rhohv:
            gain = 1.14/(thisscan->NumLevels - 2);
            offset = 0.01;
            nodata = 0.0;
            break;
          default:
            // encode all others no offset, unity gain for now
            offset = 0.0;
            gain = 1.0;
            nodata = 0.0;
            break;
        }
      }
      else
      {
        switch (thisscan->data_type) {
          case e_refl:
          case e_rawrefl:
            offset = 0.0;
            gain = 1.0;
            nodata = -32.0;
            break;
          case e_vel:
            // For typical 160 levels, 0=no echo, 80=0vel
            // 1 = -nyquist, 159 = +nyquist
            offset = 0.0;
            gain = 1.0;
            nodata = 0.0;
            break;
          default:
            // encode all others no offset, unity gain for now
            offset = 0.0;
            gain = 1.0;
            nodata = 0.0;
            break;
        }
      }

      sprintf(str, "data%d", j);      
      hid_handle data_id(H5Gcreate(group_id, str, 0, 0, 0));

      {
        hid_handle g_id(H5Gcreate(data_id, "what", 0, 0, 0));
        status = H5Acreatestring(g_id, "quantity", 
				 get_odim_data_type_text(thisscan->data_type));
        status = H5Acreatedouble(g_id, "gain", gain);
        status = H5Acreatedouble(g_id, "offset", offset);
        status = H5Acreatedouble(g_id, "nodata", nodata);
        status = H5Acreatedouble(g_id, "undetect", nodata);
      }

      {
        hid_handle g_id(H5Gcreate(data_id, "how", 0, 0, 0));
        if ((thisscan->data_type == e_vel)||
	  (thisscan->data_type == e_spectw))
          status = H5Acreatedouble(g_id, "NI", thisscan->nyquist);
      }

      rdr_scan_array* scan_array = NULL;
      if (writeFloat)
      {
        scan_array = thisscan->CreateScanArray(NULL, writeFloat, nodata,
                                               maxTiltBins);
      }
      else if ((thisscan->data_type == e_refl) || 
               (thisscan->data_type == e_rawrefl))
        // force conversion to StdBinary scaling
        scan_array = 
          thisscan->CreateScanArray(GetStdBinaryReflLevelTbl(),
                                    writeFloat, nodata, maxTiltBins);
      else
        scan_array = thisscan->CreateScanArray(NULL, writeFloat,
                                               nodata, maxTiltBins);

      // TODO - need to convert refl data to -32/0.5dbZ std here
      if (scan_array)
      {
        hsize_t dims[2];
        dims[0] = scan_array->azDim; 
        dims[1] = scan_array->rngDim; 
        hid_handle dataspace_id(H5Screate_simple(2, dims, NULL));
        hid_handle plist(H5Pcreate(H5P_DATASET_CREATE));
        status = H5Pset_chunk(plist, 2, compressChunkSize);
        if (!useSzip)
          status = H5Pset_deflate(plist, compressFactor); 
        else
        {
          unsigned int szip_options_mask=H5_SZIP_NN_OPTION_MASK;
          H5Pset_szip (plist, szip_options_mask, szipPixelsPerBlock);
        }

        hid_handle dataset_id(H5Dcreate(data_id, "data", 
            writeFloat ? H5T_IEEE_F32LE : H5T_NATIVE_UCHAR,
            dataspace_id, H5P_DEFAULT, plist, H5P_DEFAULT));
        if (!dataset_id)
          H5Eprint(H5E_DEFAULT, stdout);
        else
        {
          if (writeFloat)
          {
            status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, 
                H5S_ALL, H5P_DEFAULT, 
                scan_array->float_data);
          }
          else
          {
            status = H5Dwrite(dataset_id, H5T_NATIVE_UCHAR, H5S_ALL, 
                H5S_ALL, H5P_DEFAULT, 
                scan_array->uchar_data);
          }
          if (status < 0)
            H5Eprint(H5E_DEFAULT, stdout);

          status = H5Acreatestring(dataset_id, "CLASS", "IMAGE");
          status = H5Acreatestring(dataset_id, "IMAGE_VERSION", "1.2");
        }
      }
      thisscan->deAllocScanArray();
      thisscan = thisscan->NextScan(thisscan);
      j++;
    }
  }
}
  
herr_t odimH5FileWriter::H5Acreatestring(hid_t root_id, char * name, char * s)
{
  hid_handle type_id(H5Tcopy(H5T_C_S1));
  if (!type_id)
    return -1;
  /* the length */
  if ((H5Tset_size(type_id, strlen(s)+1)) < 0)
    return -1;

  hid_handle space_id(H5Screate(H5S_SCALAR));
  hid_handle attr_id(H5Acreate(root_id, name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT));
  return H5Awrite(attr_id, type_id, s);
}

herr_t odimH5FileWriter::H5Acreatedouble(hid_t root_id, const char* name, double val)
{
  hid_handle space_id(H5Screate(H5S_SCALAR));
  hid_handle attr_id(H5Acreate(root_id, name, H5T_IEEE_F64LE, space_id, H5P_DEFAULT, H5P_DEFAULT));
  return H5Awrite(attr_id, H5T_NATIVE_DOUBLE, &val);
}

herr_t odimH5FileWriter::H5Acreatelong(hid_t root_id, const char* name, long val)
{
  hid_handle space_id(H5Screate(H5S_SCALAR));
  hid_handle attr_id(H5Acreate(root_id, name, H5T_STD_I64LE, space_id, H5P_DEFAULT, H5P_DEFAULT));
  return H5Awrite(attr_id, H5T_NATIVE_LONG, &val);
}

char * odimH5FileWriter::H5Areadstring(hid_t root_id, char * name)
{
  hid_t strtype, attr_id;
  herr_t status;
  char * s;
  size_t size;

  if ((strtype=H5Tcopy(H5T_C_S1))<0) {
    return 0;
  }
  /* the length */
  if ((H5Tset_size(strtype,H5T_VARIABLE) < 0)) {
    return 0;
  }
  attr_id = H5Aopen_name(root_id, name);

  {
    hid_t type, ftype;
    //    H5T_class_t type_class;
    htri_t size_var;

    ftype = H5Aget_type(attr_id);

    // type_class = H5Tget_class (ftype);   

    /*if (type_class == H5T_STRING) printf ("File datatype has class H5T_STRING\n");*/
    size = H5Tget_size(ftype);

    /*printf(" Size is of the file datatype returned by H5Tget_size %d \n This is a size of char pointer\n Use H5Tis_variable_str call instead \n", size);*/

    if((size_var = H5Tis_variable_str(ftype)) == 1)
        printf(" to find if string has variable size \n");

    type = H5Tget_native_type(ftype, H5T_DIR_ASCEND);

    s = (char *)malloc(size+1);
    status = H5Aread(attr_id, type, s);
    s[size] = 0;
    status = H5Tclose(ftype);
    status = H5Tclose(type);
    /*fprintf(stderr, "%.*s\n", size, s);*/
  }

  status = H5Aclose(attr_id);
  if (!size) {
    free(s);
    return 0;
  }
  return s;
  
}

