// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

//////////////////////////////////////////////////////////
// mdv/MdvxField.hh
//
// Class for representing MDV fields.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
///////////////////////////////////////////////////////////
//
// A field object is maintained consistent at all times,
// except when the class functions are in progress.
//
// The main data members are (a) the field_header (b) the field data.
//
//   getFieldNum() returns the field number of this object.
//   getFieldHeader() returns a reference to the field header.
//   getVlevelHeader() returns a reference to the vlevel header.
//   getGrid() returns a reference to an mdv_grid_t struct. This struct
//     holds an alternative representation of the grid information to
//     be found in the field header. It is primarily included to make
//     it easier to convert old code to use this class.
//
// Data volume representation.
//
// Field data represents a 3-d volume. If the data is only 2-dimensional,
// nz will be 1.
//
// Data from a volume is stored in contiguous memory, accessed via
// the function getVol().
//
// You may also access each plane in the data, using getPlane(i).
// This returns a pointer to the start of plane i.
// getPlaneSize(i) and getPlaneOffset(i) return the size of the 
// plane data and the offset of the plane in the volume.
//
////////////////////////////////////////////////////////////////////////////

#ifndef MdvxField_hh
#define MdvxField_hh

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxVsectLut.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaFile.hh>
#include <vector>

#define MDV_FLAG_64 0x64646464U

using namespace std;

class DsMdvxMsg;
class Ncf2MdvTrans;
class Ncf2MdvField;

class MdvxField {
  
  friend class Mdvx;
  friend class DsMdvxMsg;
  friend class Ncf2MdvTrans;
  friend class Ncf2MdvField;
  friend class DsMdvServer;

public:

  // default constructor
  
  MdvxField();

  // copy constructor

  MdvxField(const MdvxField &rhs);

  // Constucts object from field header and data parts.
  //
  // If the data pointer is not NULL, space is allocated for it and the
  // data is copied in.
  //
  // If the data pointer is NULL, and alloc_mem is true,
  // memory in the volBuf member is allocated for data based on
  // the volume_size in the field header. In this case,
  // if init_with_missing is true, the array will be filled with
  // the missing_data_value in the header.
  //
  // NOTE: for this to work, the following items in the 
  //       header must be set:
  //
  //    nx, ny, nz, volume_size, data_element_nbytes,
  //    bad_data_value, missing_data_value
  //
  //  In addition, for INT8 and INT16, set:
  //    scale, bias
  
  MdvxField(const Mdvx::field_header_t &f_hdr,
	    const Mdvx::vlevel_header_t &v_hdr,
	    const void *vol_data = NULL,
	    bool init_with_missing = false,
	    bool compute_min_and_max = true,
	    bool alloc_mem = true);
  
  // Constructor for a single plane, given an object
  // with the entire data volume.
  // The data for the plane is copied from the volume.
  
  MdvxField(const MdvxField &rhs, int plane_num);

  // Constucts object for single plane from field header and data parts.
  //
  // If the plane data pointer is not NULL, space is allocated for it and the
  // data is copied in.
  //
  // If the plane data pointer is NULL, space is allocated for data based on
  // the volume_size in the field header.
  //
  // NOTE: for this to work, the following items in the 
  //       header must be set:
  //
  //    nx, ny, nz, volume_size, data_element_nbytes,
  //    bad_data_value, missing_data_value
  //
  //  In addition, for INT8 and INT16, set:
  //    scale, bias
  
  MdvxField(int plane_num,
	    int64_t plane_size,
	    const Mdvx::field_header_t &f_hdr,
	    const Mdvx::vlevel_header_t &v_hdr,
	    const void *plane_data = NULL);

  // destructor
  
  virtual ~MdvxField();

  // assignment
  
  MdvxField & operator=(const MdvxField &rhs);

  ///////////////////////////////////////////////////////////////////////////
  // Convert field data to an output encoding type, specifying
  // compression and scaling.
  //
  // The field object must contain a complete field header and data.
  //
  // Supported types are:
  //   Mdvx::ENCODING_MDV_ASIS  - no change
  //   Mdvx::ENCODING_INT8
  //   Mdvx::ENCODING_INT16
  //   Mdvx::ENCODING_FLOAT32
  //
  // Supported compression types are:
  //   Mdvx::COMPRESSION_ASIS - no change
  //   Mdvx::COMPRESSION_NONE - uncompressed
  //   Mdvx::COMPRESSION_RLE - see <toolsa/compress.h>
  //   Mdvx::COMPRESSION_LZO - see <toolsa/compress.h>
  //   Mdvx::COMPRESSION_ZLIB - see <toolsa/compress.h>
  //   Mdvx::COMPRESSION_BZIP - see <toolsa/compress.h>
  //   Mdvx::COMPRESSION_GZIP - see <toolsa/compress.h>
  //
  // Scaling types apply only to conversions to int types (INT8 and INT16)
  //
  // Supported scaling types are:
  //   Mdvx::SCALING_ROUNDED
  //   Mdvx::SCALING_INTEGRAL
  //   Mdvx::SCALING_DYNAMIC
  //   Mdvx::SCALING_SPECIFIED
  //
  // For Mdvx::SCALING_DYNAMIC, the scale and bias is determined from the
  // dynamic range of the data.
  //
  // For Mdvx::SCALING_INTEGRAL, the operation is similar to
  // Mdvx::SCALING_DYNAMIC, except that the scale and bias are
  // constrained to integral values.
  // 
  // For Mdvx::SCALING_SPECIFIED, the specified scale and bias are used.
  // You should set the scale and bias so that the data does not scale to
  // the integer values 0 and 1, because these are reserved for the
  // missing and bad values.
  //
  // Output scale and bias are ignored for conversions to float, and
  // for Mdvx::SCALING_DYNAMIC and Mdvx::SCALING_INTEGRAL.
  //
  // Returns 0 on success, -1 on failure.
  //
  // On success, the volume data is converted, and the header is adjusted
  // to reflect the changes.
  
  int convertType
  (Mdvx::encoding_type_t output_encoding = Mdvx::ENCODING_FLOAT32,
   Mdvx::compression_type_t output_compression = Mdvx::COMPRESSION_NONE,
   Mdvx::scaling_type_t output_scaling = Mdvx::SCALING_DYNAMIC,
   double output_scale = 1.0,
   double output_bias = 0.0);
  
  // This routine calls convertType(), with ROUNDED scaling.
  // See convertType().
  // 
  // Returns 0 on success, -1 on failure.

  int convertRounded(Mdvx::encoding_type_t output_encoding,
		     Mdvx::compression_type_t output_compression =
		     Mdvx::COMPRESSION_NONE);
     
  // This routine calls convertType(), with INTEGRAL scaling.
  // See convertType().
  // 
  // Returns 0 on success, -1 on failure.

  int convertIntegral(Mdvx::encoding_type_t output_encoding,
		      Mdvx::compression_type_t output_compression =
		      Mdvx::COMPRESSION_NONE);

  // This routine calls convertType(), with DYNAMIC scaling.
  // See convertType().
  // 
  // Returns 0 on success, -1 on failure.

  int convertDynamic(Mdvx::encoding_type_t output_encoding,
		     Mdvx::compression_type_t output_compression =
		     Mdvx::COMPRESSION_NONE);
  
  // Transform field data to natural log values.
  // The field object must contain a complete field header and data.
  //
  // Returns 0 on success, -1 on failure.
  //
  // On success, the volume data is converted, and the header is adjusted
  // to reflect the changes.

  int transform2Log();

  // Transform field data from natural log to linear values.
  // The field object must contain a complete field header and data.
  //
  // Returns 0 on success, -1 on failure.
  //
  // On success, the volume data is converted, and the header is adjusted
  // to reflect the changes.
  
  int transform2Linear();
     
  // Negate field.
  // Convert each byte value to its negative value.
  // The field object must contain a complete field header and data.
  //
  // This will also convert to linear if (a) the data is stored as
  // logs and (b) convert_to_linear is true.
  //
  // Returns 0 on success, -1 on failure.
  //
  // On success, the volume data is converted, and the header is adjusted
  // to reflect the changes.

  int negate(bool convert_to_linear = false);

  // Compute the composite (max at multiple levels) given lower and
  // upper vlevel limits.
  //
  // Returns 0 on success, -1 on failure.
  
  int convert2Composite(double lower_vlevel,
			double upper_vlevel);

  // Compute the composite (max at multiple levels) given lower and
  // upper plane numbers.
  //
  // If lower_plane_num is -1, it is set to the lowest plane in the vol.
  // If upper_plane_num is -1, it is set to the highest plane in the vol.
  // 
  // Returns 0 on success, -1 on failure.
  
  int convert2Composite(int lower_plane_num = -1,
			int upper_plane_num = -1);
    
  // Convert vlevel type if possible
  //
  // Conversion between the following types are supported:
  //
  //   VERT_TYPE_Z
  //   VERT_TYPE_PRESSURE
  //   VERT_FLIGHT_LEVEL
  
  void convertVlevelType(Mdvx::vlevel_type_t req_vlevel_type);

  // Compute the vertical section for the data volume, given the 
  // waypts, nsamples and lookup table object. If the lookup table has not
  // been initialized it is computed. If the waypts, n_samples or
  // grid geometry has changed the lookup table is recomputed.
  //
  // If fill_missing is set and fewer than 20 % of the grid points in a
  // plane are missing, the missing vals are filled in by copying in
  // from adjacent points.
  //
  // Returns 0 on success, -1 on failure.
  
  int convert2Vsection(const Mdvx::master_header_t &mhdr,
                       const vector<Mdvx::vsect_waypt_t> &waypts,
		       int n_samples,
		       MdvxVsectLut &lut,
		       bool fill_missing = false,
		       bool interp = true,
		       bool specify_vlevel_type = false,
		       Mdvx::vlevel_type_t vlevel_type = Mdvx::VERT_TYPE_Z,
		       bool do_final_convert = true);

  // Compute to a single RHI.
  //
  // The RHI in the file is stored as: (x = range, y = elevation, z = azimuth)
  // The Single RHI stored as (x = range, y = az, z = elevation), in order
  // to match the vsection format.
  //
  // Returns 0 on success, -1 on failure.
  
  int convert2SingleRhi(const Mdvx::master_header_t &mhdr,
			int rhiIndex,
			const vector<Mdvx::vsect_waypt_t> &waypts,
			MdvxVsectLut &lut,
			bool do_final_convert = true);
  
  // Compute a vertical section from an RHI.
  //
  // The RHI is stored as: (x = range, y = elevation, z = azimuth)
  // The Vsection is stored as (x = range, y = 1, z = ht)
  //
  // Returns 0 on success, -1 on failure.
  
  int convertRhi2Vsect(const Mdvx::master_header_t &mhdr,
		       int rhiIndex,
		       const vector<Mdvx::vsect_waypt_t> &waypts,
		       MdvxVsectLut &lut,
		       bool do_final_convert = true);
  
  // Remap onto a LATLON projection.
  // This is a CYLINDRICAL EQUIDISTANT projection.
  //
  // If the lookup table has not been initialized it is computed.
  // If the projection geometry has changed the lookup table is recomputed.
  //
  // Returns 0 on success, -1 on failure.
  
  int remap2Latlon(MdvxRemapLut &lut,
		   int nx, int ny,
		   double minx, double miny,
		   double dx, double dy);

  
  // Remap onto a flat (Radar) projection.
  // This is a AZIMUTHAL EQUIDISTANT projection.
  //
  // If the lookup table has not been initialized it is computed.
  // If the projection geometry has changed the lookup table is recomputed.
  //
  // Returns 0 on success, -1 on failure.
  
  int remap2Flat(MdvxRemapLut &lut,
		 int nx, int ny,
		 double minx, double miny,
		 double dx, double dy,
		 double origin_lat, double origin_lon,
		 double rotation,
                 double false_northing = 0.0,
                 double false_easting = 0.0);

  // Remap onto a LAMBERT CONFORMAL projection.
  //
  // If the lookup table has not been initialized it is computed.
  // If the projection geometry has changed the lookup table is recomputed.
  //
  // Returns 0 on success, -1 on failure.

  int remap2LambertConf(MdvxRemapLut &lut,
			int nx, int ny,
			double minx, double miny,
			double dx, double dy,
			double origin_lat, double origin_lon,
			double lat1, double lat2,
                        double false_northing = 0.0,
                        double false_easting = 0.0);
  
  int remap2Lc2(MdvxRemapLut &lut,
		int nx, int ny,
		double minx, double miny,
		double dx, double dy,
		double origin_lat, double origin_lon,
		double lat1, double lat2) {
    return remap2LambertConf(lut, nx, ny, minx, miny, dx, dy,
			     origin_lat, origin_lon, lat1, lat2);
  }
  
  // Remap onto a POLAR STEREOGRAPHIC projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2PolarStereo(MdvxRemapLut &lut,
			int nx, int ny,
			double minx, double miny,
			double dx, double dy,
			double origin_lat, double origin_lon,
			double tangent_lon, 
			Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH,
			double central_scale = 1.0,
                        double false_northing = 0.0,
                        double false_easting = 0.0,
			double lad = 90.0);

  // Remap onto an OBLIQUE STEREOGRAPHIC projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2ObliqueStereo(MdvxRemapLut &lut,
			  int nx, int ny,
			  double minx, double miny,
			  double dx, double dy,
			  double origin_lat, double origin_lon,
			  double tangent_lat, double tangent_lon,
                          double central_scale = 1.0,
                          double false_northing = 0.0,
                          double false_easting = 0.0);

  // Remap onto a MERCATOR projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2Mercator(MdvxRemapLut &lut,
		     int nx, int ny,
		     double minx, double miny,
		     double dx, double dy,
		     double origin_lat, double origin_lon,
                     double false_northing = 0.0,
                     double false_easting = 0.0);
  
  // Remap onto a TRANSVERSE MERCATOR projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2TransverseMercator(MdvxRemapLut &lut,
			       int nx, int ny,
			       double minx, double miny,
			       double dx, double dy,
			       double origin_lat, double origin_lon,
			       double central_scale,
                               double false_northing = 0.0,
                               double false_easting = 0.0);
  
  // Remap onto a ALBERS EQUAL AREA CONIC projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2Albers(MdvxRemapLut &lut,
		   int nx, int ny,
		   double minx, double miny,
		   double dx, double dy,
		   double origin_lat, double origin_lon,
		   double lat1, double lat2,
                   double false_northing = 0.0,
                   double false_easting = 0.0);

  // Remap onto a, LAMBERT AZIMUTHAL EQUAL AREA projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2LambertAzimuthal(MdvxRemapLut &lut,
			     int nx, int ny,
			     double minx, double miny,
			     double dx, double dy,
			     double origin_lat, double origin_lon,
                             double false_northing = 0.0,
                             double false_easting = 0.0);
  
  // Remap onto a VERT_PERSP (satellite view) projection.
  // Returns 0 on success, -1 on failure.
  
  int remap2VertPersp(MdvxRemapLut &lut,
                      int nx, int ny,
                      double minx, double miny,
                      double dx, double dy,
                      double origin_lat, double origin_lon,
                      double persp_radius,
                      double false_northing = 0.0,
                      double false_easting = 0.0);
  
  // Auto remap onto a latlon projection, without specifying
  // grid information.
  //
  // The function will determine grid information from the
  // projection information of the existing grid.
  //
  // If the lookup table has not been initialized it is computed.
  // If the projection geometry has changed the lookup table is recomputed.
  //
  // Returns 0 on success, -1 on failure.
  
  int autoRemap2Latlon(MdvxRemapLut &lut);

  // Remap onto specified projection.
  //
  // If the lookup table has not been initialized it is computed.
  // If the projection geometry has changed the lookup table is recomputed.
  //
  // Returns 0 on success, -1 on failure.
  
  int remap(MdvxRemapLut &lut,
	    MdvxProj &proj_target);

  // Remap to specified type, using the existing grid parameters to
  // estimate the best target grid.
  //
  // NOTE - NOT IMPLEMENTED YET.
  //
  // If the lookup table has not been initialized it is computed.
  // If the projection geometry has changed the lookup table is recomputed.
  //
  // Returns 0 on success, -1 on failure.
  
  int remap(MdvxRemapLut &lut,
	    Mdvx::projection_type_t proj_type);

  // constrain the data in the vertical, based on read limits set
  // the mdvx object

  void constrainVertical(const Mdvx &mdvx);

  // constrain the data in the horizontal, based on read limits set
  // the mdvx object

  void constrainHorizontal(const Mdvx &mdvx);

  // Decimate to max grid cell count
  // Returns 0 on success, -1 on failure.

  int decimate(int64_t max_nxy);
  
  // compute the plane number from the vlevel value

  int computePlaneNum(double vlevel);

  // compute the plane limits for a pair of vlevel values

  void computePlaneLimits(double vlevel1,
			  double vlevel2,
			  int &lower_plane,
			  int &upper_plane);
  
  // Request the compression type.
  // Compression will be deferred until the data is written
  // to a file, or to a message.
  
  void requestCompression(int compression_type = Mdvx::COMPRESSION_GZIP) const {
    _fhdr.requested_compression = compression_type;
  };

  int getRequestedCompression() const { return _fhdr.requested_compression; }
  
  // compress the data volume if compression has previously
  // been requested
  // Compressed buffer is stored in BE byte order.
  // returns 0 on success, -1 on failure
  
  int compressIfRequested() const;
  int compressIfRequested64() const;

  // compress the data volume - 32-bit version
  // The compressed buffer comprises the following in order:
  //   Array of nz ui32s: compressed plane offsets
  //   Array of nz ui32s: compressed plane sizes
  //   All of the compressed planes packed together.
  // Compressed buffer is stored in BE byte order.
  // returns 0 on success, -1 on failure
  
  int compress(int compression_type) const;

  // compress the data volume - 64-bit version
  // The compressed buffer comprises the following in order:
  //   Flags_64[2]: 2 x ui32: each with 0x64646464U - indicates 64 bit
  //   Array of nz ui64s: compressed plane offsets
  //   Array of nz ui64s: compressed plane sizes
  //   All of the compressed planes packed together.
  // Compressed buffer is stored in BE byte order.
  // returns 0 on success, -1 on failure
  
  int compress64(int compression_type) const;

  // decompress the field
  //
  // Decompresses the volume buffer if compressed
  //
  // returns 0 on success, -1 on failure

  int decompress() const;

  // Check if DZ is constant
  // Returns TRUE if dz constant, false otherwise.
  // 
  // Side effect: sets dz_constant in field header.
  
  bool isDzConstant();

  // Set dz, difference between vlevels, to be constant
  // If not already constant, data is remapped onto constant vlevels
  // by copying from closest available vlevel.
  // dz is computed as the smallest suitable delta z.
  
  void setDzConstant(int nzMax = 64);

  // Set dz, difference between vlevels, to be constant
  // If not already constant, data is remapped onto constant vlevels
  // by copying from closest available vlevel.
  
  void setDzConstant(double dz, int nzMax = 64);

  // remap vertical levels, using a constant dz.
  // If not already constant, data is remapped onto constant vlevels
  // using a nearest neighbor method
  
  void remapVlevels(int nz, double minz, double dz);

  // Print the contents of the MdvxField object headers
  //
  // print_file_headers: if true and the file headers exist, print them.
  
  void printHeaders(ostream &out,
		    bool print_file_headers = false) const;

  // Print the contents of the MdvxField object
  //
  // print_native: if true, type is preserved.
  //               if false, printed as floats.
  //
  // print_labels: if true, a label will be printed for each plane.
  //
  // pack_duplicates: if true, duplicates will be printed once with a 
  //                           repeat count.
  //                  if false, all values will be printed.
  //
  // print_file_headers: if true and the file headers exist, print them.
  //
  // n_lines_data: number of data lines to print. If -1, print all.
  
  void print(ostream &out,
             bool print_native = true,
             bool print_labels = true,
             bool pack_duplicates = true,
             bool print_file_headers = false,
             int n_lines_data = -1) const;

  // Print the volume data part of MdvxField object.
  //
  // print_native: if true, type is preserved.
  //               if false, printed as floats.
  //
  // print_labels: if true, a label will be printed for each plane.
  //
  // pack_duplicates: if true, duplicates will be printed once with a 
  //                           repeat count.
  //                  if false, all values will be printed.
  //
  // printCanonical: print in full resolution
  //
  // n_lines_data: number of data lines to print. If -1, print all.
  
  void printVoldata(ostream &out,
                    bool print_native = true,
                    bool print_labels = true,
                    bool pack_duplicates = true,
                    bool printCanonical = false,
                    int n_lines_data = -1) const;
     
  // Print time-height data
  
  void printTimeHeightData(ostream &out,
			   const vector<time_t> &times,
			   bool print_native = true);
     
  // isCompressed()
  // Returns true of field is compressed, false otherwise

  bool isCompressed() const;
  static bool isCompressed(const Mdvx::field_header_t &fhdr);

  // getting header references - these are const
  // If you want to alter the headers, make a local copy,
  // amend it and then call the set() functions.

  const Mdvx::field_header_t &getFieldHeader() const { return _fhdr; }
  const Mdvx::vlevel_header_t &getVlevelHeader() const { return _vhdr; }

  // Getting pointers to file headers for inspection.
  // Field and vlevel headers exactly as they appear in the file.
  // To get these on read, call setReadFieldFileHeaders() before
  // the read. They are NULL if not set.

  const Mdvx::field_header_t *getFieldHeaderFile() const
  { return _fhdrFile; }
  const Mdvx::vlevel_header_t *getVlevelHeaderFile() const
  { return _vhdrFile; }
  
  // setting the entire headers

  void setFieldHeader(const Mdvx::field_header_t &fhdr);
  void setVlevelHeader(const Mdvx::vlevel_header_t &vhdr);

  // setting the entire file headers

  void setFieldHeaderFile(const Mdvx::field_header_t &fhdr);
  void setVlevelHeaderFile(const Mdvx::vlevel_header_t &vhdr);

  // getting string parts of the header

  const char *getFieldName() const { return _fhdr.field_name; }
  const char *getFieldNameLong() const { return _fhdr.field_name_long; }
  const char *getUnits() const { return _fhdr.units; }
  const char *getTransform() const { return _fhdr.transform; }

  // setting string parts of the header

  void setFieldName(const string &name);
  void setFieldNameLong(const string &nameLong);
  void setUnits(const string &units);
  void setTransform(const string &transform);

  void setFieldName(const char *name);
  void setFieldNameLong(const char *nameLong);
  void setUnits(const char *units);
  void setTransform(const char *transform);

  static void setFieldName(const string &name,
                           Mdvx::field_header_t &fhdr);
  static void setFieldNameLong(const string &nameLong,
                               Mdvx::field_header_t &fhdr);
  static void setUnits(const string &units,
                       Mdvx::field_header_t &fhdr);
  static void setTransform(const string &transform, Mdvx::field_header_t &fhdr);

// setting the volume data
  //
  // The data must be uncompressed.
  // Scaling type, scale and bias only apply to INT8 and INT16 encoding.

  void setVolData(const void *vol_data,
		  int64_t volume_size,
		  Mdvx::encoding_type_t encoding_type,
		  Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
		  double scale = 1.0,
		  double bias = 0.0);
  
  void setVolData(const void *vol_data,
                  int nz, int ny, int nx,
                  Mdvx::encoding_type_t encoding_type,
                  Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
                  double scale = 1.0,
                  double bias = 0.0);

  // Set object state from field header and data parts.
  //
  // If the data pointer is not NULL, space is allocated for it and the
  // data is copied in.
  //
  // If the data pointer is NULL, and alloc_mem is true,
  // memory in the volBuf member is allocated for data based on
  // the volume_size in the field header. In this case,
  // if init_with_missing is true, the array will be filled with
  // the missing_data_value in the header.
  //
  // NOTE: for this to work, the following items in the 
  //       header must be set:
  //
  //    nx, ny, nz, volume_size, data_element_nbytes,
  //    bad_data_value, missing_data_value
  //
  //  In addition, for INT8 and INT16, set:
  //    scale, bias
  
  void setHdrsAndVolData(const Mdvx::field_header_t &f_hdr,
                         const Mdvx::vlevel_header_t &v_hdr,
                         const void *vol_data = NULL,
                         bool init_with_missing = false,
                         bool compute_min_and_max = true,
                         bool alloc_mem = true);
  
  // Set object state for single plane from field header and data parts.
  //
  // If the plane data pointer is not NULL, space is allocated for it and the
  // data is copied in.
  //
  // If the plane data pointer is NULL, space is allocated for data based on
  // the volume_size in the field header.
  //
  // NOTE: for this to work, the following items in the 
  //       header must be set:
  //
  //    nx, ny, nz, volume_size, data_element_nbytes,
  //    bad_data_value, missing_data_value
  //
  //  In addition, for INT8 and INT16, set:
  //    scale, bias
  
  void setHdrsAndPlaneData(int plane_num,
                           int64_t plane_size,
                           const Mdvx::field_header_t &f_hdr,
                           const Mdvx::vlevel_header_t &v_hdr,
                           const void *plane_data = NULL);

  // clearing the volume data
  //
  // The data must be uncompressed.
  // Sets all of the data values to missing.

  void clearVolData();

  // compute the the min and max in a data volume
  // Note: this is automatically done in convert(), so if you call convert()
  // you need not make this call separately

  int computeMinAndMax(bool force = false) const;

  // access to the data as a volume
  
  const void *getVol() const { return _volBuf.getPtr(); }
  int64_t getVolLen() const { return _volBuf.getLen(); }
  int64_t getVolNumValues() const {
    return (_volBuf.getLen() / _fhdr.data_element_nbytes);
  }

  // access to the data as a plane
  //
  // Note: you must call setPlanePtrs() just prior to
  //       using these functions.
  
  const void *getPlane(int i) const { return _planeData[i]; }
  int64_t getPlaneSize(int i) const { return _planeSizes[i]; }
  int64_t getPlaneOffset(int i) const { return _planeOffsets[i]; }
  
  // Set the plane pointers into the data
  //
  // This must be called before using the following plane functions:
  //    getPlane();
  //    getPlaneSize()
  //    getPlaneOffset()
  
  void setPlanePtrs() const;

  // Convert data buffer to/from BE byte ordering

  static void buffer_to_BE(void *buf, int64_t buflen, int encoding_type);
  static void buffer_from_BE(void *buf, int64_t buflen, int encoding_type);
  
  // Get the Error String. This has contents when an error is returned.
  
  string getErrStr() const { return _errStr; }

  // clear the error string
  
  void clearErrStr() const { _errStr = ""; }

protected:

  // field and vlevel headers

  Mdvx::field_header_t _fhdr;
  Mdvx::vlevel_header_t _vhdr;

  // File headers for inspection.
  // Field and vlevel headers exactly as they appear in the file.
  // To get these on read, call setReadFieldFileHeaders() before
  // the read. They are NULL if not set.

  Mdvx::field_header_t *_fhdrFile;
  Mdvx::vlevel_header_t *_vhdrFile;

  // memory buffer is used for volume data
  
  mutable MemBuf _volBuf;

  // Data can be represented as an array of planes.
  // The function setPlanePtrs() sets up these vectors.
  
  mutable vector<void *> _planeData;
  mutable vector<int64_t> _planeSizes;
  mutable vector<int64_t> _planeOffsets;

   // error string

  mutable string _errStr;

  //////////////////////
  // protected functions

  // copy

  MdvxField & _copy(const MdvxField &rhs);

  // conversions

  void _int8_to_int16(int output_scaling,
		      double output_scale,
		      double output_bias);

  void _int16_to_int8(int output_scaling,
		      double output_scale,
		      double output_bias);
     
  void _int8_to_float32();
     
  void _int16_to_float32();

  void _float32_to_int8(int output_scaling);

  void _float32_to_int8(double output_scale,
			double output_bias);

  void _float32_to_int16(int output_scaling);

  void _float32_to_int16(double output_scale,
			 double output_bias);

  // is volbuf valid in size?

  bool _volbufSizeValid() const;

  // compression

  int _compressGzipVol() const;
  int _decompressGzipVol() const;
  int _decompress64() const;

  // constraining the domain in the horizontal and vertical dimensions

  int _constrain_radar_horiz(const Mdvx &mdvx);
  int _decimate_radar_horiz(int64_t max_nxy);
  int _decimate_rgba(int64_t max_nxy);
  void _check_lon_domain(double read_min_lon,
			 double read_max_lon);
  
  // min/max/rounding
  
  void _set_data_element_nbytes();
  double _round_up(double z);
  
  // Reading / writing data volumes
  
  int _read_volume(TaFile &infile,
		   Mdvx &mdvx,
		   bool fill_missing,
		   bool do_decimate,
		   bool do_final_convert,
		   MdvxRemapLut &remapLut,
		   bool is_vsection,
		   double vsection_min_lon,
		   double vsection_max_lon);

  int _apply_read_constraints(const Mdvx &mdvx,
                              bool fill_missing,
                              bool do_decimate,
                              bool do_final_convert,
                              MdvxRemapLut &remapLut,
                              bool is_vsection,
                              double vsection_min_lon,
                              double vsection_max_lon);
  
  int _write_volume(TaFile &outfile,
		    int64_t this_offset,
		    int64_t &next_offset) const;
  
  // Convert data buffer to/from BE byte ordering
  
  static void _data_from_BE(const Mdvx::field_header_t &fhdr,
                            void *buf,
                            int64_t buflen);

  // vert sections

  void _computeVsection(MdvxVsectLut &lut,
			bool interp,
			MemBuf &workBuf);

  void _computeVsectionRGBA(MdvxVsectLut &lut,
			    MemBuf &workBuf);

  void _computeVsectionPolarRadar(const MdvxVsectLut &lut,
				  const MdvxProj &proj,
				  MemBuf &workBuf);

  void _vsectionElev2Ht(const Mdvx::master_header_t &mhdr,
                        const MdvxProj &proj,
			const MdvxVsectLut &lut,
			bool interp,
			MemBuf &workBuf);

  // printing
  
  void _print_voldata_verbose(ostream &out, bool print_labels);
  
  void _print_voldata_packed(ostream &out, bool print_labels,
			     bool printCanonical = false,
                             int n_lines_print = -1);
  
  void _print_time_height(ostream &out,
			  const vector<time_t> &times);
  
  void _print_int8_packed(ostream &out, int count,
			  ui08 val, ui08 bad,
			  ui08 missing,
			  bool printCanonical = false);
  
  void _print_int16_packed(ostream &out, int count,
			   ui16 val, ui16 bad,
			   ui16 missing,
			   bool printCanonical = false);
  
  void _print_float32_packed(ostream &out, int count,
			     fl32 val, fl32 bad,
			     fl32 missing,
			     bool printCanonical = false);
  
  void _print_rgba32_packed(ostream &out, int count,
			    ui32 val, ui32 bad,
			    ui32 missing,
			    bool printCanonical = false);
  
  int _planes_fill_missing();
  
  void _plane_fill_missing(int encoding_type,
			   fl32 missing_data_value,
			   void *array, int nx, int ny);
  
  void _vsection_fill_missing(int encoding_type,
                              fl32 missing_data_value,
                              void *array, int nx, int nz);
  
  void _convert_vlevels(Mdvx::vlevel_type_t file_vlevel_type,
                        Mdvx::vlevel_type_t req_vlevel_type,
                        Mdvx::field_header_t &fhdr,
                        Mdvx::vlevel_header_t &vhdr);
  
  double _round_dz(double dz);

  void _check_finite(const void *vol_data) const;

  // Clearing data

  void _clearVolDataInt8();
  
  void _clearVolDataInt16();
  
  void _clearVolDataFloat32();
  
  void _clearVolDataRgba32();
  
  
private:

};

#endif


