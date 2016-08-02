#ifndef __odimH5FileWriter_h
#define __odimH5FileWriter_h

#include "radarFileWriter.h"
#include <hdf5.h>
#include <H5public.h>
#include "spinlock.h"

e_data_type get_odim_data_type(const char *dtastr);
e_scan_type get_odim_scan_type(const char *scantypestr);

class odimH5FileWriter : public radarFileWriter
{
public:
  odimH5FileWriter(char *rapicODIMH5_fname = NULL, rdr_scan *rapicscan = NULL);
  odimH5FileWriter(char *rapicODIMH5_fname, rdr_scan *rapicscan,
		   bool _writefloat = false);
  virtual ~odimH5FileWriter();
  void setDefaults();
  bool writeFile(char *rapicODIMH5_fname, rdr_scan *rapicscan);
  herr_t H5Acreatestring(hid_t root_id, char * name, char * s);
  herr_t H5Acreatedouble(hid_t root_id, const char* name, double val);
  herr_t H5Acreatelong(hid_t root_id, const char* name, long val);
  char * H5Areadstring(hid_t root_id, char * name);
  bool getLock(char *lockholder);
  void relLock();
  bool writeFloat;
  bool useSzip;
  int szipPixelsPerBlock;
  hsize_t compressChunkSize[2];
  int compressFactor;
private:
  void writeFile_impl(char *rapicODIMH5_fname, rdr_scan *rapicscan);
};

extern spinlock *globalODIMh5ConvertLock;

#endif
