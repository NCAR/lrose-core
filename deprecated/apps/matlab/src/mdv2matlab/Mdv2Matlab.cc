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
///////////////////////////////////////////////////////////////
// Mdv2Matlab.cc
//
// Mdv2Matlab object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
// MODIFICATION LOG
// John Williams, February, 1999
// Steve Carson, May, 2005
//
///////////////////////////////////////////////////////////////

#include <stdio.h>
#include <ctype.h>

#include <mat.h>

#include <mdv/mdv_handle.h>
#include <mdv/mdv_print.h>
#include <mdv/mdv_read.h>
#include <toolsa/os_config.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/globals.h>
#include <toolsa/str.h>
#include <rapformats/var_elev.h>

#include "Mdv2Matlab.h"
#include "Mdv2MatlabArgs.h"
#include "Mdv2MatlabParams.h"


const int Forever = TRUE;


/**************************************************************
 * Constructor
 */

Mdv2Matlab::Mdv2Matlab(int argc, char **argv)
{
  okay = TRUE;
  done = FALSE;

  // Set program name

  char *slash_pos = strrchr(argv[0], '/');
  
  if (slash_pos == (char *)NULL)
    _programName = STRdup(argv[0]);
  else
    _programName = STRdup(slash_pos + 1);
  
  // Get command line args

  _args = new Mdv2MatlabArgs(argc, argv, _programName);

  if (!_args->okay)
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with command line args\n");
    okay = FALSE;
    return;
  }

  if (_args->done)
  {
    done = TRUE;
    return;
  }

  // Get TDRP params

  _params = new Mdv2MatlabParams(_args->paramsFilePath,
				 &_args->override,
				 _programName,
				 _args->checkParams,
				 _args->printParams);
  
  if (!_params->okay)
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with TDRP parameters\n");

    okay = FALSE;
    return;
  }

  if (_params->done)
  {
    done = TRUE;
    return;
  }

  if (!okay)
    return;

  // Make sure the archive mode information is consistent

  if (_args->numInputFiles <= 0)
  {
    fprintf(stderr, "ERROR - %s\n", _programName);
    fprintf(stderr, "Must specify input file list\n");

    okay = FALSE;
    return;
  }

}


/**************************************************************
 * Destructor
 */

Mdv2Matlab::~Mdv2Matlab()
{
  // Free strings

  STRfree(_programName);
  
  // Call destructors

  delete _params;
  delete _args;

}


/**************************************************************
 * run()
 */

int Mdv2Matlab::run()
{
  /*
   * process the files
   */

  for (int i = 0; i < _args->numInputFiles; i++)
    _processFile(_args->inputFileList[i]);
  
  return (0);
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/**************************************************************
 * _processFile()
 */

void Mdv2Matlab::_processFile(char *file_name)
{
  static char *routine_name = "_processFile()";
  
  if (_params->params.debug_level >= DEBUG_NORM)
    fprintf(stderr, "Processing file <%s>\n", file_name);
  
  MDV_handle_t mdv_handle;
  
  /*
   * Initialize the MDV handle.
   */

  if (MDV_init_handle(&mdv_handle) != 0)
  {
    fprintf(stderr,
       "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
       "Error initializing MDV handle when processing file <%s>\n",
	    file_name);
    
    return;
  }
  
  /*
   * Read in the MDV file.
   */

  if( _args->mdvInt16 )
     {
     if (MDV_read_all(&mdv_handle, file_name, MDV_INT16) != 0)
        {
        fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
        fprintf(stderr, "Error reading MDV_INT16 file <%s>.\n", file_name);
        return;
        }
     }
  else
     {
     if (MDV_read_all(&mdv_handle, file_name, MDV_INT8) != 0)
        {
        fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
        fprintf(stderr, "Error reading MDV_INT8 file <%s>.\n", file_name);
        return;
        }
     }
  
  /*
   * Make sure this data is in an order that we can currently process.
   */

  /******* Now allow any orientation and order of indices

  if (mdv_handle.master_hdr.grid_order_direction != MDV_ORIENT_SN_WE)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "MDV file has data in %s orientation.\n",
	    MDV_orient2string(mdv_handle.master_hdr.grid_order_direction));
    fprintf(stderr,
	    "Currently, we can only process data in a %s orientation.\n",
	    MDV_orient2string(MDV_ORIENT_SN_WE));
    
    MDV_free_handle(&mdv_handle);
    return;
  }

  if (mdv_handle.master_hdr.grid_order_indices != MDV_ORDER_XYZ)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "MDV file has data in %s ordering.\n",
	    MDV_order2string(mdv_handle.master_hdr.grid_order_indices));
    fprintf(stderr,
	    "Currently, we can only process data in a %s ordering.\n",
	    MDV_order2string(MDV_ORDER_XYZ));
    
    MDV_free_handle(&mdv_handle);
    return;
  } *******************************************************/

  /*
   * Determine the Matlab output filename.
   */

  char *mat_filename = _getMatlabFilename(_params->params.output_dir,
					  _params->params.output_file_ext,
                                          _args->makeOutputSubdir,
					  mdv_handle.master_hdr.time_centroid);
      
  /*
   * Open the Matlab output file.
   */

  MATFile *mat_fp = matOpen(mat_filename, "w");
  
  if (mat_fp == NULL)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error opening output file <%s>\n", mat_filename);
    
    MDV_free_handle(&mdv_handle);
    return;
  }

  /*
   * Get array of the mdv field names.  These are used as fields for
   * the field header, (optional) vlevel, and data structures.
   */

  char **mdv_data_fields = 
      (char **) umalloc( mdv_handle.master_hdr.n_fields * sizeof(char *) );

  for (int field = 0; field < mdv_handle.master_hdr.n_fields; field++)
  {
    /*
     * Get a pointer to the field header.
     */

    MDV_field_header_t *field_hdr = &mdv_handle.fld_hdrs[field];
    
    /*
     * Copy mdv field name into mdv_data_fields array.
     */

    mdv_data_fields[field] = 
        (char *)umalloc((strlen(field_hdr->field_name)+1) * sizeof(char));
    strcpy(mdv_data_fields[field],field_hdr->field_name);

    /*
     * Replace non-alphanumeric characters in field name by '_'.
     */

    for (unsigned int i = 0; i < strlen(mdv_data_fields[field]); i++)
      if(!isalnum(mdv_data_fields[field][i]))
        mdv_data_fields[field][i] = '_';
  }  /* endfor - field */

  /*
   * Create some useful variables for the Matlab conversion. 
   */

  double *pr;            // array pointer
  int ndim;              // matrix dimensions
  //int dims[3];           // matrix dimensions
  mwSize dims[3];
  char str[80];          // used to build strings
  mxArray *field_value;  // generic Matlab value
  int field_encoding_type; // MDV_INT16, MDV_INT8, etc.

  /*
   * Create the output Matlab struct, mdv.  See Mdv2MatlabArgs::_usage
   * for more information on its format.
   */

  const char *mdv_fields[] = {"src","hdr","fhdr","vlevel","data","chunk"}; 
  mxArray *mdv_ptr = mxCreateStructMatrix(1, 1, 6, mdv_fields);

  //mxSetName(mdv_ptr, "mdv");

  /*
   * Put string containing file name into 'src' field of mdv.
   */

  field_value = mxCreateString(file_name);
  mxSetField(mdv_ptr, 0, "src", field_value);


  /*
   * Build master header struct.  ****************************************
   */

  const char *hdr_fields[] = {"record_len1","struct_id","revision_number",
     "time_gen", "user_time","time_begin","time_end","time_centroid",
     "time_expire","num_data_times","index_number","data_dimension",
     "data_collection_type","user_data","native_vlevel_type","vlevel_type",
     "vlevel_included","grid_order_direction","grid_order_indices",
     "n_fields","max_nx","max_ny","max_nz","n_chunks","field_hdr_offset",
     "vlevel_hdr_offset","chunk_hdr_offset","field_grids_differ",
     "user_data_si32_0","user_data_si32_1","user_data_si32_2",
     "user_data_si32_3","user_data_si32_4","user_data_si32_5",
     "user_data_si32_6","user_data_si32_7","user_data_fl32_0",
     "user_data_fl32_1","user_data_fl32_2","user_data_fl32_3",
     "user_data_fl32_4","user_data_fl32_5","sensor_lon","sensor_lat",
     "sensor_alt","data_set_info","data_set_name","data_set_source",
     "record_len2"};

  mxArray *hdr_ptr = mxCreateStructMatrix(1, 1, 49, hdr_fields);

  /*
   * Fill master header struct with mdv master header data.
   */

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.record_len1;
  mxSetField(hdr_ptr, 0, "record_len1", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.struct_id;
  mxSetField(hdr_ptr, 0, "struct_id", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.revision_number;
  mxSetField(hdr_ptr, 0, "revision_number", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.time_gen;
  mxSetField(hdr_ptr, 0, "time_gen", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.user_time;
  mxSetField(hdr_ptr, 0, "user_time", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.time_begin;
  mxSetField(hdr_ptr, 0, "time_begin", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.time_end;
  mxSetField(hdr_ptr, 0, "time_end", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.time_centroid;
  mxSetField(hdr_ptr, 0, "time_centroid", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.time_expire;
  mxSetField(hdr_ptr, 0, "time_expire", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.num_data_times;
  mxSetField(hdr_ptr, 0, "num_data_times", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.index_number;
  mxSetField(hdr_ptr, 0, "index_number", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.data_dimension;
  mxSetField(hdr_ptr, 0, "data_dimension", field_value);

  field_value = mxCreateString(MDV_colltype2string(mdv_handle.master_hdr.data_collection_type));
  mxSetField(hdr_ptr, 0, "data_collection_type", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.user_data;
  mxSetField(hdr_ptr, 0, "user_data", field_value);

  field_value = mxCreateString(MDV_verttype2string(mdv_handle.master_hdr.native_vlevel_type));
  mxSetField(hdr_ptr, 0, "native_vlevel_type", field_value);

  field_value = mxCreateString(MDV_verttype2string(mdv_handle.master_hdr.vlevel_type));
  mxSetField(hdr_ptr, 0, "vlevel_type", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.vlevel_included;
  mxSetField(hdr_ptr, 0, "vlevel_included", field_value);

  field_value = mxCreateString(MDV_orient2string(mdv_handle.master_hdr.grid_order_direction));
  mxSetField(hdr_ptr, 0, "grid_order_direction", field_value);

  field_value = mxCreateString(MDV_order2string(mdv_handle.master_hdr.grid_order_indices));
  mxSetField(hdr_ptr, 0, "grid_order_indices", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.n_fields;
  mxSetField(hdr_ptr, 0, "n_fields", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.max_nx;
  mxSetField(hdr_ptr, 0, "max_nx", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.max_ny;
  mxSetField(hdr_ptr, 0, "max_ny", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.max_nz;
  mxSetField(hdr_ptr, 0, "max_nz", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.n_chunks;
  mxSetField(hdr_ptr, 0, "n_chunks", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.field_hdr_offset;
  mxSetField(hdr_ptr, 0, "field_hdr_offset", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.vlevel_hdr_offset;
  mxSetField(hdr_ptr, 0, "vlevel_hdr_offset", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.chunk_hdr_offset;
  mxSetField(hdr_ptr, 0, "chunk_hdr_offset", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.field_grids_differ;
  mxSetField(hdr_ptr, 0, "field_grids_differ", field_value);

  for (int i = 0; i < 8; i++)  
  {
    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) mdv_handle.master_hdr.user_data_si32[i];
    sprintf(str,"user_data_si32_%d",i);
    mxSetField(hdr_ptr, 0, str, field_value);
  }

  for (int i = 0; i < 6; i++)  
  {
    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) mdv_handle.master_hdr.user_data_fl32[i];
    sprintf(str,"user_data_fl32_%d",i);
    mxSetField(hdr_ptr, 0, str, field_value);
  }

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.sensor_lon;
  mxSetField(hdr_ptr, 0, "sensor_lon", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.sensor_lat;
  mxSetField(hdr_ptr, 0, "sensor_lat", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.sensor_alt;
  mxSetField(hdr_ptr, 0, "sensor_alt", field_value);

  field_value = mxCreateString(mdv_handle.master_hdr.data_set_info);
  mxSetField(hdr_ptr, 0, "data_set_info", field_value);

  field_value = mxCreateString(mdv_handle.master_hdr.data_set_name);
  mxSetField(hdr_ptr, 0, "data_set_name", field_value);

  field_value = mxCreateString(mdv_handle.master_hdr.data_set_source);
  mxSetField(hdr_ptr, 0, "data_set_source", field_value);

  field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
  pr = mxGetPr(field_value);
  pr[0] = (double) mdv_handle.master_hdr.record_len2;
  mxSetField(hdr_ptr, 0, "record_len2", field_value);

  /*
   * Put master header struct into 'hdr' field of mdv struct.
   */

  mxSetField(mdv_ptr, 0, "hdr", hdr_ptr);


  /*
   * Build field header struct.  ***************************************
   */

  mxArray *fhdr_ptr = mxCreateStructMatrix(1, 1, 
                                           mdv_handle.master_hdr.n_fields,
                                           (const char **)mdv_data_fields);

  const char *fhdr_fields[] = {"field_name_long","field_name","units",
    "transform","record_len1","struct_id","field_code","user_time1",
    "forecast_delta","user_time2","user_time3","forecast_time","user_time4",
    "nx","ny","nz","proj_type","encoding_type","data_element_nbytes",
    "field_data_offset","volume_size","user_data_si32_0","user_data_si32_1",
    "user_data_si32_2","user_data_si32_3","user_data_si32_4",
    "user_data_si32_5","user_data_si32_6","user_data_si32_7",
    "user_data_si32_8","user_data_si32_9","proj_origin_lon",
    "proj_origin_lat","proj_rotation","proj_param_0","proj_param_1",
    "proj_param_2","proj_param_3","proj_param_4","proj_param_5",
    "proj_param_6","proj_param_7","vert_reference","grid_dx","grid_dy",
    "grid_dz","grid_minx","grid_miny","grid_minz","scale","bias",
    "bad_data_value","missing_data_value","user_data_fl32_0",
    "user_data_fl32_1","user_data_fl32_2","user_data_fl32_3","record_len2"};

  for (int field = 0; field < mdv_handle.master_hdr.n_fields; field++)
  {
    /*
     * Get a pointer to this field's mdv header.
     */

    MDV_field_header_t *field_hdr = &mdv_handle.fld_hdrs[field];
    
    /*
     *  Create struct for this field's header.
     */

    mxArray *a_fhdr_ptr = mxCreateStructMatrix(1, 1, 58, fhdr_fields);

    /*
     *  Fill this field's header struct with this field's header info.
     */

    field_value = mxCreateString(field_hdr->field_name_long);
    mxSetField(a_fhdr_ptr, 0, "field_name_long", field_value);

    field_value = mxCreateString(field_hdr->field_name);
    mxSetField(a_fhdr_ptr, 0, "field_name", field_value);

    field_value = mxCreateString(field_hdr->units);
    mxSetField(a_fhdr_ptr, 0, "units", field_value);

    field_value = mxCreateString(field_hdr->transform);
    mxSetField(a_fhdr_ptr, 0, "transform", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->record_len1;
    mxSetField(a_fhdr_ptr, 0, "record_len1", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->struct_id;
    mxSetField(a_fhdr_ptr, 0, "struct_id", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->field_code;
    mxSetField(a_fhdr_ptr, 0, "field_code", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->user_time1;
    mxSetField(a_fhdr_ptr, 0, "user_time1", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->forecast_delta;
    mxSetField(a_fhdr_ptr, 0, "forecast_delta", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->user_time2;
    mxSetField(a_fhdr_ptr, 0, "user_time2", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->user_time3;
    mxSetField(a_fhdr_ptr, 0, "user_time3", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->forecast_time;
    mxSetField(a_fhdr_ptr, 0, "forecast_time", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->user_time4;
    mxSetField(a_fhdr_ptr, 0, "user_time4", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->nx;
    mxSetField(a_fhdr_ptr, 0, "nx", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->ny;
    mxSetField(a_fhdr_ptr, 0, "ny", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->nz;
    mxSetField(a_fhdr_ptr, 0, "nz", field_value);

    field_value = mxCreateString(MDV_proj2string(field_hdr->proj_type));
    mxSetField(a_fhdr_ptr, 0, "proj_type", field_value);

    field_value = mxCreateString(MDV_encode2string(field_hdr->encoding_type));
    mxSetField(a_fhdr_ptr, 0, "encoding_type", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->data_element_nbytes;
    mxSetField(a_fhdr_ptr, 0, "data_element_nbytes", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->field_data_offset;
    mxSetField(a_fhdr_ptr, 0, "field_data_offset", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->volume_size;
    mxSetField(a_fhdr_ptr, 0, "volume_size", field_value);

    for (int i = 0; i < 10; i++)  
    {
      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) field_hdr->user_data_si32[i];
      sprintf(str,"user_data_si32_%d",i);
      mxSetField(a_fhdr_ptr, 0, str, field_value);
    }

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->proj_origin_lon;
    mxSetField(a_fhdr_ptr, 0, "proj_origin_lon", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->proj_origin_lat;
    mxSetField(a_fhdr_ptr, 0, "proj_origin_lat", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->proj_rotation;
    mxSetField(a_fhdr_ptr, 0, "proj_rotation", field_value);

    for (int i = 0; i < 8; i++)  
    {
      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) field_hdr->proj_param[i];
      sprintf(str,"proj_param_%d",i);
      mxSetField(a_fhdr_ptr, 0, str, field_value);
    }

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->vert_reference;
    mxSetField(a_fhdr_ptr, 0, "vert_reference", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->grid_dx;
    mxSetField(a_fhdr_ptr, 0, "grid_dx", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->grid_dy;
    mxSetField(a_fhdr_ptr, 0, "grid_dy", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->grid_dz;
    mxSetField(a_fhdr_ptr, 0, "grid_dz", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->grid_minx;
    mxSetField(a_fhdr_ptr, 0, "grid_minx", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->grid_miny;
    mxSetField(a_fhdr_ptr, 0, "grid_miny", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->grid_minz;
    mxSetField(a_fhdr_ptr, 0, "grid_minz", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->scale;
    mxSetField(a_fhdr_ptr, 0, "scale", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->bias;
    mxSetField(a_fhdr_ptr, 0, "bias", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->bad_data_value;
    mxSetField(a_fhdr_ptr, 0, "bad_data_value", field_value);

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->missing_data_value;
    mxSetField(a_fhdr_ptr, 0, "missing_data_value", field_value);

    for (int i = 0; i < 4; i++)  
    {
      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) field_hdr->user_data_fl32[i];
      sprintf(str,"user_data_fl32_%d",i);
      mxSetField(a_fhdr_ptr, 0, str, field_value);
    }

    field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    pr = mxGetPr(field_value);
    pr[0] = (double) field_hdr->record_len2;
    mxSetField(a_fhdr_ptr, 0, "record_len2", field_value);

    /*
     * Put this field's header struct into field header struct.
     */

    mxSetField(fhdr_ptr, 0, mdv_data_fields[field], a_fhdr_ptr);

  }  /* endfor - field */

  /*
   * Put field header struct into 'fhdr' field of mdv struct.
   */

  mxSetField(mdv_ptr, 0, "fhdr", fhdr_ptr);


  /*
   * Build vlevel struct IF there are vlevels. **************************
   */

  if (mdv_handle.master_hdr.vlevel_included)
  {
    mxArray *vlevel_ptr = mxCreateStructMatrix(1, 1, 
                                               mdv_handle.master_hdr.n_fields, 
                                               (const char **)mdv_data_fields);

    const char *vlevel_fields[] = {"record_len1","struct_id","vlevel_type",
          "vlevel_params","record_len2"};

    for (int field = 0; field < mdv_handle.master_hdr.n_fields; field++)
    {
      /*
       * Get pointers to this field's mdv header and vlevel.
       */

      MDV_field_header_t *field_hdr = &mdv_handle.fld_hdrs[field];  
      MDV_vlevel_header_t *vlevel_hdr = &mdv_handle.vlv_hdrs[field];

      /*
       *  Create struct for this field's vlevel.
       */

      mxArray *a_vlevel_ptr = mxCreateStructMatrix(1, 1, 5, vlevel_fields);

      /*
       * Fill this field's vlevel struct with this field's vlevel info.
       */

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) vlevel_hdr->record_len1;
      mxSetField(a_vlevel_ptr, 0, "record_len1", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) vlevel_hdr->struct_id;
      mxSetField(a_vlevel_ptr, 0, "struct_id", field_value);

      mxArray *vlevel_type_ptr = mxCreateCellMatrix(field_hdr->nz,1);
      for (int i = 0; i < field_hdr->nz; i++)
      {
        field_value =
           mxCreateString(MDV_verttype2string(vlevel_hdr->vlevel_type[i]));

        mxSetCell(vlevel_type_ptr, i, field_value);
      }
      mxSetField(a_vlevel_ptr, 0, "vlevel_type", vlevel_type_ptr); 

      field_value = mxCreateDoubleMatrix(field_hdr->nz, 1, mxREAL);
      pr = mxGetPr(field_value);
      for (int i = 0; i < field_hdr->nz; i++)
        pr[i] = (double)vlevel_hdr->vlevel_params[i];
      mxSetField(a_vlevel_ptr, 0, "vlevel_params", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) vlevel_hdr->record_len2;
      mxSetField(a_vlevel_ptr, 0, "record_len2", field_value);

      /*
       * Put this field's vlevel struct into vlevel struct.
       */

      mxSetField(vlevel_ptr, 0, mdv_data_fields[field], a_vlevel_ptr);

    }  /* endfor - field */

    /*
     * Put vlevel struct into 'vlevel' field of mdv struct.
     */

    mxSetField(mdv_ptr, 0, "vlevel", vlevel_ptr);

  }  /* endif - there are vlevels */


  /*
   * Build data struct.  ************************************************
   */

  mxArray *data_ptr = mxCreateStructMatrix(1, 1, 
                                           mdv_handle.master_hdr.n_fields, 
                                           (const char **)mdv_data_fields);

  for (int field = 0; field < mdv_handle.master_hdr.n_fields; field++)
  {
    /*
     * Get a pointer to this field's mdv header.
     */

    MDV_field_header_t *field_hdr = &mdv_handle.fld_hdrs[field];

    /*
     * Determine field encoding type - MDV_INT8 or MDV_INT16
     */

    field_encoding_type = field_hdr->encoding_type;

//    if( field_encoding_type == MDV_INT8 )
//       {
//       printf("Mdv2Matlab: field '%s' is of type MDV_INT8\n",
//         mdv_data_fields[field]);
//       }
//    else if( field_encoding_type == MDV_INT16 )
//       {
//       printf("Mdv2Matlab: field '%s' is of type MDV_INT16\n",
//         mdv_data_fields[field]);
//       }
//    else
//       {
//       printf("Mdv2Matlab: field '%s' is of type UNKNOWN\n",
//         mdv_data_fields[field]);
//       }
 
    /*
     * Allocate space for the data array.
     */

    int
       array_size = field_hdr->nx * field_hdr->ny * field_hdr->nz;

    double         *data_double;
    unsigned char  *data_uchar;
    unsigned short *data_uint16;

    if (_args->mdvDataExpand)
      {
      data_double = (double *) umalloc(array_size * sizeof(double));
      }
    else
      {
      if( field_encoding_type == MDV_INT8 )
         {
         data_uchar = (unsigned char *) umalloc(array_size * sizeof(char));
         }
      else if( field_encoding_type == MDV_INT16 )
         {
         data_uint16 = (unsigned short *) umalloc(array_size * sizeof(short));
         }
      }

    /*
     * Read/convert 3-d field data and store it in the 1-d data array in
     * Matlab-compatible (i.e., Fortran) order.
     */

    for (int mdv_z = 0; mdv_z < field_hdr->nz; mdv_z++)
    {
      /*
       * Get a pointer to this plane of the data.
       */

      ui08 *mdv_data_ptr_ui8  = NULL;
      ui16 *mdv_data_ptr_ui16 = NULL;

      if( field_encoding_type == MDV_INT8 )
         {
         mdv_data_ptr_ui8 =
            (unsigned char *) mdv_handle.field_plane[field][mdv_z];
         }
      else if( field_encoding_type == MDV_INT16 )
         {
         mdv_data_ptr_ui16 =
            (unsigned short *) mdv_handle.field_plane[field][mdv_z];
         }
      
      /*
       * Extract the mdv data for this plane and put it into the data array.
       */

      for (int mdv_x = 0; mdv_x < field_hdr->nx; mdv_x++)
      {
	for (int mdv_y = 0; mdv_y < field_hdr->ny; mdv_y++)
	{
	  int
          mat_index =
             (field_hdr->nx *field_hdr->ny * mdv_z) +
             (field_hdr->ny * mdv_x) +
             mdv_y;
	  
	  if (mat_index >= array_size)
	  {
	    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
	    fprintf(stderr, "Invalid Matlab array index calculated.\n");
	    fprintf(stderr, "Index = %d, array_size = %d\n",
		    mat_index, array_size);    
	    exit(0);    
	  }
	  
          if (_args->mdvDataExpand) // expanding data to double
            {
            bool
               data_good = true;

            if( field_encoding_type == MDV_INT8 )
               {
               if (
                  *mdv_data_ptr_ui8 == field_hdr->bad_data_value ||
                  *mdv_data_ptr_ui8 == field_hdr->missing_data_value
                  )
                  data_good = false;
               }
            else if( field_encoding_type == MDV_INT16 )
               {
               if (
                  *mdv_data_ptr_ui16 == field_hdr->bad_data_value ||
                  *mdv_data_ptr_ui16 == field_hdr->missing_data_value
                  )
                  data_good = false;
               }

	    if ( !data_good )
              {
	      data_double[mat_index] = mxGetNaN();
              }
	    else
              {
              if( field_encoding_type == MDV_INT8 )
                 {
                 data_double[mat_index] =
                    ((double) *mdv_data_ptr_ui8 * field_hdr->scale) +
                    field_hdr->bias;
                 }
              else if( field_encoding_type == MDV_INT16 )
                 {
                 data_double[mat_index] =
                    ((double) *mdv_data_ptr_ui16 * field_hdr->scale) +
                    field_hdr->bias;
                 }
              }

            }
	  else // not expanding data to double
            {

            if( field_encoding_type == MDV_INT8 )
               {
               data_uchar[mat_index] = (unsigned char) *mdv_data_ptr_ui8;
               }
            else if( field_encoding_type == MDV_INT16 )
               {
               data_uint16[mat_index] = (unsigned short) *mdv_data_ptr_ui16;
               }
            }

          if( field_encoding_type == MDV_INT8 )
             {
             mdv_data_ptr_ui8++;
             }
          else if( field_encoding_type == MDV_INT16 )
             {
             mdv_data_ptr_ui16++;
             }
	  
	} /* endfor - mdv_x */
      } /* endfor - mdv_y */       
    } /* endfor - mdv_z */

    /*
     * Copy the data array into a 3-d Matlab array.
     */

    ndim = 3;
    dims[0] = field_hdr->nx;
    dims[1] = field_hdr->ny;
    dims[2] = field_hdr->nz;

    mxArray *mx_array;

    if (_args->mdvDataExpand)
      {
      mx_array = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, mxREAL);
      memcpy(mxGetPr(mx_array), data_double, array_size * sizeof(double));
      }
    else
      {
      if( field_encoding_type == MDV_INT8 )
         {
         mx_array = mxCreateNumericArray(ndim, dims, mxUINT8_CLASS, mxREAL);
         memcpy(mxGetPr(mx_array), data_uchar, array_size * sizeof(char));
         }
      else if( field_encoding_type == MDV_INT16 )
         {
         mx_array = mxCreateNumericArray(ndim, dims, mxUINT16_CLASS, mxREAL);
         memcpy(mxGetPr(mx_array), data_uint16, array_size * sizeof(short));
         }
      }

    /*
     * Put this field's 3-d Matlab array into the data struct.
     */

    mxSetField(data_ptr, 0, mdv_data_fields[field], mx_array);

    /*
     * Free the space allocated for the data array.
     */

    if (_args->mdvDataExpand)
      {
      ufree(data_double);
      }
    else
      {
      if( field_encoding_type == MDV_INT8 )
         {
         ufree(data_uchar);
         }
      else if( field_encoding_type == MDV_INT16 )
         {
         ufree(data_uint16);
         }
      }

  } /* endfor - field */
  
  /*
   * Put data struct into 'data' field of mdv struct.
   */

  mxSetField(mdv_ptr, 0, "data", data_ptr);

  /*
   * Build chunk structure IF there are chunks.  ***********************
   */

  if (mdv_handle.master_hdr.n_chunks > 0)
    {
    /*
     * Make the chunk cell array.
     */

    const char
       *chunk_fields[] = {"hdr","data"};

    const char
       *chunkhdr_fields[] =
          {
          "record_len1",
          "struct_id",
          "chunk_id",
          "chunk_data_offset",
          "size",
          "info",
          "record_len2"
          };

    mxArray
       *chunk_ptr = mxCreateCellMatrix(mdv_handle.master_hdr.n_chunks,1);

    for (int ichunk = 0; ichunk < mdv_handle.master_hdr.n_chunks; ichunk++)
      {
      /*
       * Get a pointer to this chunk's mdv header.
       */
    
      MDV_chunk_header_t
         *chunk_hdr = &mdv_handle.chunk_hdrs[ichunk];
    
      /*
       *  Create structs for this chunk and this chunk's header.
       */

      mxArray
         *a_chunk_ptr = mxCreateStructMatrix(1, 1, 2, chunk_fields);

      mxArray
         *a_chunkhdr_ptr = mxCreateStructMatrix(1, 1, 7, chunkhdr_fields);

      /*
       *  Fill this chunk's header struct with this chunk's header info
       */
    
      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) chunk_hdr->record_len1;
      mxSetField(a_chunkhdr_ptr, 0, "record_len1", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) chunk_hdr->struct_id;
      mxSetField(a_chunkhdr_ptr, 0, "struct_id", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) chunk_hdr->chunk_id;
      mxSetField(a_chunkhdr_ptr, 0, "chunk_id", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) chunk_hdr->chunk_data_offset;
      mxSetField(a_chunkhdr_ptr, 0, "chunk_data_offset", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) chunk_hdr->size;
      mxSetField(a_chunkhdr_ptr, 0, "size", field_value);
   
      field_value = mxCreateString(chunk_hdr->info);
      mxSetField(a_chunkhdr_ptr, 0, "info", field_value);

      field_value = mxCreateDoubleMatrix(1, 1, mxREAL);
      pr = mxGetPr(field_value);
      pr[0] = (double) chunk_hdr->record_len2;
      mxSetField(a_chunkhdr_ptr, 0, "record_len2", field_value);

      /*
       * Put this chunk's header struct into hdr field of this chunk's struct
       */

      mxSetField(a_chunk_ptr, 0, "hdr", a_chunkhdr_ptr);

      /*
       * Build chunk data struct IF chunk is of a supported type.
       % (Currently only variable elevations type is supported.)
       */

      if (chunk_hdr->chunk_id == MDV_CHUNK_VARIABLE_ELEV)
        {
        /*
         * Get a pointer to this chunk's mdv data.
         */
    
        void
           *chunk_data_ptr = (void *)mdv_handle.chunk_data[ichunk];

        /*
         * Get size of chunk and check for consistency.
         */

        int
           chunk_len = (chunk_hdr->size - sizeof(ui32))/sizeof(fl32);

        ui32
           *naz = (ui32 *)chunk_data_ptr;

        if(
          ((int)*naz != chunk_len) | 
          ((int)(chunk_len*sizeof(fl32)+sizeof(ui32)) != (int)chunk_hdr->size)
          )
	  {
          fprintf(stderr, "WARNING: %s::%s\n", _className(), routine_name);
          fprintf(stderr, "Chunk %d has improper or inconsistent size.\n",
             ichunk);
	  }

        chunk_len = (chunk_len < (int)*naz ? chunk_len : (int)*naz);

        /*
         * Allocate space for the chunk data array.
         */

        double
           *chunk_array = (double *)umalloc(chunk_len*sizeof(double));

        /*
         * Read/convert chunk data and put it into the chunk data array.
         */

        fl32
           *elev_ptr = (fl32*)((char *) chunk_data_ptr + sizeof(ui32));

        for (int i = 0; i < chunk_len; i++)
          {
          if (elev_ptr[i] == VAR_ELEV_BAD_ELEV | elev_ptr[i] == -99.0)
            chunk_array[i] = mxGetNaN();
          else
            chunk_array[i] = (double)elev_ptr[i];
          }

        /*
         * Copy the chunk data array into a Matlab array.
         */

        mxArray
           *a_chunkdata_ptr = mxCreateDoubleMatrix(chunk_len, 1, mxREAL);

        memcpy(
           mxGetPr(a_chunkdata_ptr),
           chunk_array,
           chunk_len * sizeof(double));

        /*
         * Put Matlab array into data field of this chunk's struct.
         */

        mxSetField(a_chunk_ptr, 0, "data", a_chunkdata_ptr);

        /*
         * Free the space allocated for the chunk data array.
         */

        ufree(chunk_array);

        }
      else if (chunk_hdr->chunk_id == MDV_CHUNK_TEXT)
        {

        // Get a pointer to this chunk's mdv data.

        void
           *chunk_data_ptr = (void *)mdv_handle.chunk_data[ichunk];

        // Get size of chunk

        int
           chunk_len = chunk_hdr->size;

        char
           *text_start = (char *) chunk_data_ptr,
           *text = text_start;

        text          = text_start;

        for(int i = 0; i < chunk_len; ++i)
           {
           if(*text == '\n')
              {
              *text = ',';
              //++str_count;
              //text_lines[str_count] = text + 1;
              }
           ++text;
           }

        // Create mxArray array to hold chunk strings

        int
           ndims = 2;

        mwSize
           dims[ 2 ];

        dims[0] = 1;
        dims[1] = chunk_len;

        mxArray
           *mx_char_array = mxCreateCharArray(ndims, dims);

        // Get pointer to data area of mxArray object
        // Matlab stores strings as type "mxChar" which is actually
        // an "unsigned short int" (16 bit unsigned int) in C terms.

        mxChar
           *mx_char_ptr = (mxChar *) mxGetData( mx_char_array );

        // Copy the chunk data to the mxArray object

        text = text_start;
        for(int i = 0; i < chunk_len; ++i)
           {
           *mx_char_ptr = (mxChar) *text;
           ++mx_char_ptr;
           ++text;
           }

        mxSetField(a_chunk_ptr, 0, "data", mx_char_array);

        }
      else  /* chunk is not of a currently supported type */
        {

        fprintf(stderr, "WARNING: %s::%s\n", _className(), routine_name);
        fprintf(stderr,
           "Chunk with id %d couldn't be converted and will be left empty.\n",
           chunk_hdr->chunk_id);
        }

      /*
       * Put this chunk's struct into this chunk's cell array position.
       */

      mxSetCell(chunk_ptr, ichunk, a_chunk_ptr);

      }  /* endfor - ichunks */

    /*
     * Put chunk cell array into chunk field of mdv struct.
     */

    mxSetField(mdv_ptr, 0, "chunk", chunk_ptr);

    }  /* endif - chunks */

  /*
   * Save the mdv struct to the Matlab output file.
   */

  //matPutArray(mat_fp,mdv_ptr);
  //matPutVariable(mfp, var_name, array_ptr);
  matPutVariable(mat_fp, "mdv", mdv_ptr);

  /*
   * Close the Matlab output file.
   */

  matClose(mat_fp);

  /*
   * Reclaim allocated memory and space used by the MDV data.
   */

  for (int field = 0; field < mdv_handle.master_hdr.n_fields; field++)
    {
    ufree(mdv_data_fields[field]);
    }

  ufree( mdv_data_fields );

  mxDestroyArray( mdv_ptr );

  MDV_free_handle( &mdv_handle );
  
  return;

}


/**************************************************************
 * _getMatlabFilename()
 */

char* Mdv2Matlab::_getMatlabFilename(char    *output_dir,
				     char    *output_ext,
                                     bool    makeOutputSubdir,
				     time_t  data_time)
{
  static char *routine_name = "_getMatlabFilename()";
  
  static char filename[MAX_PATH_LEN];
  char output_subdir[MAX_PATH_LEN];
  
  date_time_t *time_struct = udate_time(data_time);

  if( makeOutputSubdir )
    {
    sprintf(output_subdir, "%s%s%04d%02d%02d",
      output_dir, PATH_DELIM,
      time_struct->year, time_struct->month, time_struct->day);
  
    sprintf(filename, "%s%s%02d%02d%02d.%s",
      output_subdir, PATH_DELIM,
      time_struct->hour, time_struct->min, time_struct->sec,
      output_ext);
    }
  else
    {
    sprintf(filename, "%s%s%02d%02d%02d.%s",
      output_dir, PATH_DELIM,
      time_struct->hour, time_struct->min, time_struct->sec,
      output_ext);
    }
  
  /*
   * Make sure the subdirectory exists.
   */

  if( makeOutputSubdir )
    {
    if (makedir(output_subdir) != 0)
      {
      fprintf(stderr, "ERROR; %s::%s\n", _className(), routine_name);
      fprintf(stderr, "Error creating subdirectory <%s>\n", output_subdir);
    
      exit(0);
      }
    }

  return(filename);
  
}
