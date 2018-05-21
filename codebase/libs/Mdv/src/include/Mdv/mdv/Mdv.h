/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/************************************************************************
 * Mdv.h : header file for Mdv object.  The source for this class is 
 *         divided into the following source files:
 *                      MdvPublic.cc
 *                      MdvRead.cc
 *                      MdvDump.cc
 *                      MdvPrivate.cc
 *
 *                      MdvFieldData.cc
 *                      MdvGrid.cc
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Mdv_H
#define Mdv_H

/*
 **************************** includes ***********************************
 */

#include <stdio.h>
#include <sys/time.h>

#include <Mdv/mdv/mdv_file.h>
#include <rapformats/kav_grid.h>
#include <toolsa/SimpleList.h>
#include <toolsa/globals.h>

/*
 ******************************** defines ********************************
 */


/*
 ******************************** types **********************************
 */

typedef enum
{
  MDV_DEBUG_OFF,
  MDV_DEBUG_ERRORS,
  MDV_DEBUG_MSGS,
  MDV_DEBUG_ROUTINES,
  MDV_DEBUG_ALL
} MdvDebugLevel;

typedef enum
{
  MDV_STATUS_SUCCESS,
  MDV_STATUS_FAILURE
} MdvStatus;


/*
 ************************* class definitions *****************************
 */

/*
 ************************* MdvGrid ***************************************
 */

class MdvGrid
{
  public :
    
    // Constructors

    MdvGrid(const double minx,
	    const double miny,
	    const double minz,
	    const double delta_x,
	    const double delta_y,
	    const double delta_z,
	    const int nx,
	    const int ny,
	    const int nz,
	    const int proj_type,
	    MdvDebugLevel debug_level = MDV_DEBUG_ERRORS);
  
    MdvGrid(const MdvGrid *grid_ptr);
  
    // Destructor

    virtual ~MdvGrid(void);
  
    // Update the grid values

    inline void updateOrigin(const double minx,
			     const double miny,
			     const double minz)
    {
      _minX = minx;
      _minY = miny;
      _minZ = minz;
    }
  
    inline void updateDeltas(const double delta_x,
			     const double delta_y,
			     const double delta_z)
    {
      _deltaX = delta_x;
      _deltaY = delta_y;
      _deltaZ = delta_z;
    }
  
    inline void updateSize(const int nx,
			   const int ny,
			   const int nz)
    {
      _nX = nx;
      _nY = ny;
      _nZ = nz;
    }
  
    inline void updateProjection(const int proj_type)
    {
      _projType = proj_type;
    }
  
    // Retrieve the fields in the grid object

    inline double getMinX(void)
    {
      return(_minX);
    }
  
    inline double getMinY(void)
    {
      return(_minY);
    }
  
    inline double getMinZ(void)
    {
      return(_minZ);
    }
  
    inline double getDeltaX(void)
    {
      return(_deltaX);
    }
  
    inline double getDeltaY(void)
    {
      return(_deltaY);
    }
  
    inline double getDeltaZ(void)
    {
      return(_deltaZ);
    }
  
    inline int getProjection(void)
    {
      return(_projType);
    }
  
    // Print the object information to the indicated stream.

    void print(char *filename);

    void print(FILE *stream);
  
    // Operators

    int operator==(const MdvGrid &grid);

    inline int operator==(const MdvGrid *grid)
    {
      if (this->_minX != grid->_minX) return -1;
      if (this->_minY != grid->_minY) return -1;
      if (this->_minZ != grid->_minZ) return -1;
      if (this->_deltaX != grid->_deltaX) return -1;
      if (this->_deltaY != grid->_deltaY) return -1;
      if (this->_deltaZ != grid->_deltaZ) return -1;
      if (this->_nX != grid->_nX) return -1;
      if (this->_nY != grid->_nY) return -1;
      if (this->_nZ != grid->_nZ) return -1;
      if (this->_projType != grid->_projType) return -1;
      return 0;
    }
  
    inline int operator!=(const MdvGrid &grid)
    {
      return(!(*this==grid));
    }
  

    inline int operator!=(const MdvGrid *grid)
    {
      return(!(*this == *grid));
    }
  
  protected :

    double _minX;
    double _minY;
    double _minZ;
  
    double _deltaX;
    double _deltaY;
    double _deltaZ;
  
    int _nX;
    int _nY;
    int _nZ;
  
    int _projType;
  
    double _tolerance;
  
    MdvDebugLevel _debugLevel;
  
    char _errorMsg[1024];
  
    double _setTolerance(void);
  
    virtual const char * _className(void)
    {
      return("MdvGrid");
    }
  
};


// inline int operator==(const MdvGrid &grid1, const MdvGrid &grid2)
// {
//   return((grid1 == grid2));
// }
  
// inline int operator!=(const MdvGrid &grid1, const MdvGrid &grid2)
// {
//   return(!(grid1 == grid2));
// }
  
/*
 ************************* MdvFieldData **************************************
 */

class MdvFieldData
{
  public :
    
    // Constructors

    MdvFieldData(void);                       // default constructor
    MdvFieldData(const int field_number,
		 char *field_name_long,
		 char *field_name,
		 char *units,
		 char *transform,
		 int field_code = 0,
		 MdvDebugLevel debug_level = MDV_DEBUG_ERRORS);
    MdvFieldData(const int field_number,
		 FILE *mdv_file,
		 const int load_vlevel_hdr = FALSE,
		 const int vlevel_hdr_offset = 0,
		 MdvDebugLevel debug_level = MDV_DEBUG_ERRORS);
    MdvFieldData(const int field_number,
		 const MdvFieldData *field_obj);
  
    // Destructor

    virtual ~MdvFieldData(void);
  
    // Functions for updating field header values

    void updateName(char *field_name_long,
		    char *field_name,
		    char *units,
		    char *transform,
		    int field_code = 0);
  
    void updateGridParams(double minx,
			  double miny,
			  double minz,
			  double dx,
			  double dy,
			  double dz,
			  int nx,
			  int ny,
			  int nz);
  
    void updateProjectionParams(int type,
				double origin_lat,
				double origin_lon,
				double rotation,
				double param0 = 0.0,
				double param1 = 0.0,
				double param2 = 0.0,
				double param3 = 0.0,
				double param4 = 0.0,
				double param5 = 0.0,
				double param6 = 0.0,
				double param7 = 0.0);
  
    void updateDataParams(int encoding_type,
			  int data_element_nbytes,
			  double scale = 1.0,
			  double bias = 0.0,
			  double bad_data_value = -1.0,
			  double missing_data_value = 0.0);
  
    // Update the field data.

    void updateData(time_t forecast_time,
		    int forecast_delta,
		    void *data,
		    int data_size,
		    int encoding_type);
  
    // Retrieve information about the field

    inline int getNx(void)
    {
      return(_fieldHdr->nx);
    }
  
    inline int getNy(void)
    {
      return(_fieldHdr->ny);
    }
  
    inline int getNz(void)
    {
      return(_fieldHdr->nz);
    }
  
    inline time_t getForecastTime(void)
    {
      return((time_t)_fieldHdr->forecast_time);
    }
  
    inline double getScale(void)
    {
      return(_fieldHdr->scale);
    }
  
    inline double getBias(void)
    {
      return(_fieldHdr->bias);
    }
  
    inline double getMissingValue(void)
    {
      return(_fieldHdr->missing_data_value);
    }
  
    inline double getBadValue(void)
    {
      return(_fieldHdr->bad_data_value);
    }
  
    // Retrieve the field data

    void *getDataVolume(int *volume_size,
			int mdv_encoding_type = MDV_INT8);
  
    // Retrieve the grid object

    inline MdvGrid getGrid(void)
    {
      return(*_grid);
    }
  
    // Print the object information to the indicated stream.

    void print(char *filename,
	       int full_flag = FALSE,
	       int data_flag = FALSE);

    void print(FILE *stream,
	       int full_flag = FALSE,
	       int data_flag = FALSE);
  
    // Dump the information in the object to the indicated file.
    // Returns the number of bytes of data written to the file
    // (in case the output encoding type was different from the
    // native encoding type) so that the next field's offset can
    // be calculated.

    int dump(FILE *out_file,
	     int field_position,
	     int field_data_offset,
	     int output_encoding_type);
  
  protected :

    int _fieldNum;
    MDV_field_header_t *_fieldHdr;
    MDV_vlevel_header_t *_vlevelHdr;
    void *_data;
    int _dataSize;
    int _dataAlloc;
    FILE *_inputFile;

    MdvGrid *_grid;

    MdvDebugLevel _debugLevel;
  
    char _errorMsg[1024];
  
    virtual const char * _className(void)
    {
      return("MdvFieldData");
    }
  
};


/*
 ************************* Mdv *******************************************
 */

class Mdv
{
  public :
    
    // Constructors

    Mdv(MdvDebugLevel debug_level = MDV_DEBUG_ERRORS);
    Mdv(const char *filename,
	MdvDebugLevel debug_level = MDV_DEBUG_ERRORS);
    Mdv(const Mdv *mdv_obj,
	const int copy_fields = TRUE);
  
    // Destructor

    virtual ~Mdv(void);
  
    // Initialize the dataset assuming the data is in a Kavouras 
    // mosaic grid.  The grid coordinates should be given to this
    // routine using the corner points of the grid square (like
    // Kavouras does it) rather than the center points of the grid
    // squares (like MDV does it).

    void initKavourasMosaic(double min_lat = KAV_MINLAT,
			    double min_lon = KAV_MINLON,
			    double lat_delta = KAV_DLAT,
			    double lon_delta = KAV_DLON,
			    long   num_lat = KAV_NLAT,
			    long   num_lon = KAV_NLON);

    // Update the dataset times
    void updateTimes(time_t gen_time,
		     time_t begin_time,
		     time_t centroid_time,
		     time_t end_time);
  
    // Update the dataset information

    void updateInfo(char *data_set_info,
		    char *data_set_name,
		    char *data_set_source);

    // Update some general data information

    void updateDataInfo(int data_dimension,
			int data_collection_type,
			int native_vlevel_type,
			int vlevel_type);
  
    // Update the sensor location

    void updateSensor(double sensor_lat,
		      double sensor_lon,
		      double sensor_alt);
  
    // Update the general dataset grid parameters

    void updateGridParams(double minx,
			  double miny,
			  double minz,
			  double dx,
			  double dy,
			  double dz,
			  int max_nx,
			  int max_ny,
			  int max_nz,
			  int grid_order_direction = MDV_ORIENT_SN_WE,
			  int grid_order_indices = MDV_ORDER_XYZ);
  
    // Functions for retrieving master header fields

    time_t getBeginTime(void)
    {
      return(_masterHdr->time_begin);
    }
  
    time_t getCentroidTime(void)
    {
      return(_masterHdr->time_centroid);
    }
  
    time_t getEndTime(void)
    {
      return(_masterHdr->time_end);
    }
  
    time_t getGenTime(void)
    {
      return(_masterHdr->time_gen);
    }
  
    // Add a field to the MDV dataset.  Returns a unique
    // field id for referencing the field in other calls.

    int addField(char *field_name_long,
		 char *field_name,
		 char *units,
		 char *transform,
		 int field_code = 0);
  
    int addField(MdvFieldData *field_obj);
  
    // Get the field id for the given field.  This is used to
    // identify a field in an MDV object that was initialized
    // by an input file.  Returns -1 if there is no field for
    // the given position.  Note that the first field in the
    // MDV file has field position 0.

    int getFieldId(const int field_position);
  
    // Get a pointer to the actual field object for the indicated
    // field.

    inline MdvFieldData *getField(int field_id)
    {
      return((*_fieldList)[field_id]);
    }
  
    // Functions for updating the parameter values for a given
    // field

    void updateFieldName(int field_id,
			 char *field_name_long,
			 char *field_name,
			 char *units,
			 char *transform,
			 int field_code = 0);
  
    void updateFieldGridParams(int field_id,
			       double minx,
			       double miny,
			       double minz,
			       double dx,
			       double dy,
			       double dz,
			       int nx,
			       int ny,
			       int nz);
  
    void updateFieldProjectionParams(int field_id,
				     int type,
				     double origin_lat,
				     double origin_lon,
				     double rotation,
				     double param0 = 0.0,
				     double param1 = 0.0,
				     double param2 = 0.0,
				     double param3 = 0.0,
				     double param4 = 0.0,
				     double param5 = 0.0,
				     double param6 = 0.0,
				     double param7 = 0.0);
  
    void updateFieldDataParams(int field_id,
			       int encoding_type,
			       int data_element_nbytes,
			       double scale = 1.0,
			       double bias = 0.0,
			       double bad_data_value = -1.0,
			       double missing_data_value = 0.0);
  
    // Initialize the field data volume to the given value using the
    // given data format (encoding type).  If there is existing field
    // data, it is freed.  Note that currently only MDV_INT8 format is
    // supported by this routine.

    void initFieldDataVolume(int field_id,
			     int data_value,
			     int encoding_type);
  
    // Update the field data.

    void updateFieldData(int field_id,
			 time_t forecast_time,
			 void *data,
			 int data_size,
			 int encoding_type);
  
    // Retrieve the field information for the given field.

    int getFieldNx(int field_id);
    int getFieldNy(int field_id);
    int getFieldNz(int field_id);
  
    time_t getFieldForecastTime(int field_id);

    double getFieldScale(int field_id);
    double getFieldBias(int field_id);
  
    double getFieldMissingValue(int field_id);
    double getFieldBadValue(int field_id);
  
    // Retrieve the field data for the given field.

    void *getFieldDataVolume(int field_id,
			     int *volume_size,
			     int mdv_encoding_type = MDV_INT8);
  
    // Retrieve the grid object for the given field

    MdvGrid getFieldGrid(int field_id);
  
    // Retrieve the grid object for the dataset

    inline MdvGrid getGrid(void)
    {
      return(*_grid);
    }
  
    // Dump the information in the object to the indicated file.

    void dump(char *filename,
	      int output_encoding_type,
	      char *curr_file_index_dir = NULL);
  
    void dump(FILE *out_file,
	      int output_encoding_type);
  
    void dump(char *output_dir,
	      char *output_filename,
	      int output_encoding_type,
	      char *curr_file_index_dir = NULL);
  
    void dump(char *output_host,
	      char *output_dir,
	      char *output_filename,
	      char *local_tmp_dir,
	      int output_encoding_type,
	      char *curr_file_index_dir = NULL);
  
    void dump(char *output_host,
	      char *output_dir,
	      time_t data_time,
	      char *output_extension,
	      char *local_tmp_dir,
	      int output_encoding_type,
	      char *curr_file_index_dir = NULL);
  
    // Print the object information to the indicated stream.

    void print(FILE *stream, int full_flag = FALSE);
  
    void print(char *filename, int full_flag = FALSE);
  
  protected :

    MDV_master_header_t *_masterHdr;
    SimpleList<MdvFieldData *> *_fieldList;
  
    MdvGrid *_grid;
  
    char *_inputFilename;
    FILE *_inputFile;
  
    MdvDebugLevel _debugLevel;
  
    char _errorMsg[1024];
  
    virtual const char * _className(void)
    {
      return("Mdv");
    }
  
    /**************************************************************
     * Routines in MdvRead.cc
     */

    void _readFile(void);
  
    /**************************************************************
     * Routines in MdvDump.cc
     */

    void _dumpBinary(FILE *out_file,
		     int output_encoding_type);
  
    void _dumpAscii(FILE *stream,
		    int full_flag = FALSE);
  
    char *_writeCurrentIndex(char *output_dir,
			     char *output_ext,
			     time_t data_time);
  
			      
    /**************************************************************
     * Routines in MdvPrivate.cc
     */

    MDV_master_header_t *_createInitialMasterHdr(void);
  
};


#endif
