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
// Interest.cc:  Interest derived from template
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include <math.h>

#include <toolsa/str.h>

#include "Template_Interest.h"

using namespace std;

//
// Constants
//
const double Template_Interest::_SLOPE_INTEREST_WEIGHT = 0.30;
const double Template_Interest::_BAD_CORR_VALUE  = -2.0;
const double Template_Interest::_BAD_SLOPE_VALUE = 0.0;


//////////////
// Constructor

Template_Interest::Template_Interest(vector< double > input_template_points,
				     long band_base_idex,
				     long band_top_idex,
				     double min_refl_in_band,
				     double max_refl_in_band,
				     bool compute,
				     const double max_down,
				     const double max_up,
				     const bool debug_flag) :
  Interest(debug_flag),
  _computeInterest(compute),
  _templateValues(input_template_points),
  _corr_values(0),
  _slope_values(0),
  _band_base(band_base_idex),
  _band_top(band_top_idex),
  _maxDownParam(max_down),
  _maxUpParam(max_up),
  _min_refl_in_band(min_refl_in_band),
  _max_refl_in_band(max_refl_in_band)
{
  static const string method_name = "Template_Interest::Template_Interest()";
  
  if (_debug)
  {
    cerr << "Creating Template_Interest object" << endl;
  }
}

 
/////////////
// Destructor

Template_Interest::~Template_Interest ()
  
{
   if (_debug)
     cerr << "Deleting Template_Interest object" << endl;
   
   delete [] _corr_values;
   delete [] _slope_values;
  
}

/////////////
// _compute_interest
//
// computes interest based on template

int Template_Interest::_compute_interest(fl32 *max_refl_interest,
                                         fl32 *incr_refl_interest,
					 const double max_down,
					 const double max_up)
   
{
  static const string method_name = "Template_Interest::_compute_interest()";
  
  // make local copy of data

  fl32 *dbz_array = (fl32 *)_dbzField->getVol();

   // variables used in looping through data

   int iz, start_plane = 1;
   int refl_out_of_range = FALSE;
   int dbz_missing = FALSE;

   // sum of product, sum of data points and sum of template points
   // used for computing correlation between data points and 
   // template points

   double sum_of_prod;
   double sum_of_dp, sum_of_dp_sq;
   double sum_of_tp = 0;
   double sum_of_tp_sq = 0;

   double slope, corr;
   double interest = 0.0;

   for (size_t it = 0; it < _templateValues.size(); it++)
   {
      sum_of_tp += _templateValues[it];
      sum_of_tp_sq += _templateValues[it]*_templateValues[it];
   }

   // make sure that template is not constant

   if ((_templateValues.size()*sum_of_tp_sq - sum_of_tp*sum_of_tp) == 0)
   {
     cerr << "ERROR: " << method_name << endl;
     cerr << "Template cannot be constant" << endl;
     return(-1);
   }

   // find place to start comparing band of template with data

   int plane_size = _dbzFieldHdr.nx * _dbzFieldHdr.ny;
   
   while (((start_plane+_band_base) * (_vlevelHdr.level[start_plane] - _vlevelHdr.level[start_plane-1]) + _dbzFieldHdr.grid_minz) < max_down)   
   {
      start_plane++;
   }

   for (int iy = 0; iy < _dbzFieldHdr.ny; iy++)
   {
      for (int ix = 0; ix < _dbzFieldHdr.nx; ix++)
      {
	int plane_index = ix+iy*_dbzFieldHdr.nx;
	
	 if ((max_refl_interest[plane_index] == 1.0) &&
             (incr_refl_interest[plane_index] == 1.0))
	 {
	     iz = start_plane;
	    
	     while (((iz + _band_top) * (_vlevelHdr.level[iz] - _vlevelHdr.level[iz-1]) + _dbzFieldHdr.grid_minz) <= max_up)
	     {
		sum_of_prod = 0;
		sum_of_dp = 0;
		sum_of_dp_sq = 0;
                slope = _BAD_SLOPE_VALUE;
		corr = _BAD_CORR_VALUE;

		for (size_t it = 0; it < _templateValues.size(); it++)
		{
		  int volume_index = ((iz+it) * plane_size) + plane_index;
		  
		   // check to see if refl is missing

		   if (dbz_array[volume_index] == _dbzFieldHdr.bad_data_value ||
		       dbz_array[volume_index] == _dbzFieldHdr.missing_data_value)
		   {
		      dbz_missing = TRUE;
		      break;
		   }

		   // check band to see if reflectivity values are
		   // too low or too high

		   if ((int)it >= _band_base && (int)it <= _band_top)
		   {
		      if (dbz_array[volume_index] < _min_refl_in_band ||
			  dbz_array[volume_index] > _max_refl_in_band)
		      {
			 refl_out_of_range = TRUE;
			 break;
		      }
		   }

		   sum_of_prod += _templateValues[it] * 
		      dbz_array[volume_index];
		   sum_of_dp += dbz_array[volume_index];
		   sum_of_dp_sq += dbz_array[volume_index] *
		      dbz_array[volume_index];
		   
		} /* endfor - it */


		if (((_templateValues.size()*sum_of_dp_sq - sum_of_dp*sum_of_dp) != 0) &&
		    !refl_out_of_range && !dbz_missing)
		{
		   slope= (_templateValues.size()*sum_of_prod - sum_of_dp*sum_of_tp)/
			  (_templateValues.size()*sum_of_tp_sq - sum_of_tp*sum_of_tp);

		   corr= (_templateValues.size()*sum_of_prod - sum_of_dp*sum_of_tp)/ 
			  sqrt((_templateValues.size()*sum_of_tp_sq - sum_of_tp*sum_of_tp) *
			       (_templateValues.size()*sum_of_dp_sq - sum_of_dp*sum_of_dp));
		}

		if ((slope < 0.1) || (slope > 10.0) || (corr == _BAD_CORR_VALUE))
		{
		   interest = 0;
		}
		else
		{
		   if (slope <= 1.0)
		   {
		      interest = ((slope-0.1)/0.9)*_SLOPE_INTEREST_WEIGHT +
			 corr * (1.0 - _SLOPE_INTEREST_WEIGHT);
		   }
		   else
		   {
		      interest = (((1/slope)-0.1)/9)* _SLOPE_INTEREST_WEIGHT +
			 corr * (1.0 - _SLOPE_INTEREST_WEIGHT);
		   }

		}

		if (interest > interest_values[plane_index])
		{
		   interest_values[plane_index] = interest;
		   ht[plane_index] = iz + _band_base;
		   _slope_values[plane_index] = slope;
		   _corr_values[plane_index] = corr;
		}

		refl_out_of_range = FALSE;
		dbz_missing = FALSE;
		iz++;

	     }
	 }
	 else
	 {
	    // If max refl interest field is zero, the max refl is
	    // less than the minimum allowed in the band.  In this
	    // case, it is not necessary to compute the template
	    // interest - it should automatically be zero

	    interest_values[plane_index] = 0.0;
	    ht[plane_index] = 0;
	 }
      }
   }

   return(0);
      
   
   
}	       


//////////////////////
// addFields
//
// Add correlation, slope and shape measure arrays to the MDV file

bool Template_Interest::addFields(DsMdvx &output_file,
				  char *long_corr_name,
				  char *short_corr_name,
				  char *long_slope_name,
				  char *short_slope_name)
{
  static const string method_name = "Template_Interest::addFields()";
  
  // correlation field - initialize field header

  Mdvx::field_header_t field_hdr = _dbzFieldHdr;

  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _BAD_CORR_VALUE;
  field_hdr.missing_data_value = _BAD_CORR_VALUE;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.units, "none", MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "none", MDV_TRANSFORM_LEN);
  STRcopy(field_hdr.field_name_long, long_corr_name, MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, short_corr_name, MDV_SHORT_FIELD_LEN);

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_Z;
  vlevel_hdr.level[0] = field_hdr.grid_minz;
  
  MdvxField *corr_field = new MdvxField(field_hdr, vlevel_hdr,
					(void *)_corr_values);
  
  if (corr_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating MDV correlation field" << endl;
    
    return false;
  }
  
  corr_field->convertType(Mdvx::ENCODING_INT16,
                          Mdvx::COMPRESSION_GZIP);
  
  output_file.addField(corr_field);
  
  // slope field - initialize field header

  field_hdr = _dbzFieldHdr;

  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _BAD_SLOPE_VALUE;
  field_hdr.missing_data_value = _BAD_SLOPE_VALUE;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.units, "none", MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "none", MDV_TRANSFORM_LEN);
  STRcopy(field_hdr.field_name_long, long_slope_name, MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, short_slope_name, MDV_SHORT_FIELD_LEN);

  for (int i = 0; i < field_hdr.nx * field_hdr.ny; ++i)
  {
    if (_slope_values[i] < 0.1 || _slope_values[i] > 10.0)
      _slope_values[i] = _BAD_SLOPE_VALUE;
  } /* endfor - i */

  MdvxField *slope_field = new MdvxField(field_hdr, vlevel_hdr,
					 (void *)_slope_values);
  
  if (slope_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating MDV slope field" << endl;
    
    return false;
  }
  
  slope_field->convertType(Mdvx::ENCODING_INT16,
			   Mdvx::COMPRESSION_GZIP);
  
  output_file.addField(slope_field);
  
  return true;
}


//////////////
// calcInterestFields()

bool Template_Interest::calcInterestFields(MdvxField *dbz_field,
					   fl32 *max_refl_interest,
					   fl32 *incr_refl_interest)
{
  static const string method_name = "Template_Interest::calcInterestFields()";
  
  // Initialize the interest fields

  int old_npoints = npoints;
  
  _initInterestFields(dbz_field);
  
  // set up correlation and slope arrays

  if (npoints != old_npoints)
  {
    delete [] _corr_values;
    delete [] _slope_values;
    
    _corr_values = new fl32[npoints];
    _slope_values = new fl32[npoints];
  }
  
  for (int i = 0; i < npoints; i++)
  {
    _corr_values[i] = _BAD_CORR_VALUE;
    _slope_values[i] = _BAD_SLOPE_VALUE;
  }

  // Make sure band base and top lie within data bounds

  if (_band_base < 0 || _band_base > _dbzFieldHdr.nz - 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "The base of the band is either too large or too small" << endl;

    return false;
  }

  if (_band_top < 0 || _band_top > _dbzFieldHdr.nz - 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "The top of the band is either too large or too small" << endl;

    return false;
  }
  
  // set max_up and max_down  
  double max_up = MIN(_maxUpParam,_vlevelHdr.level[_dbzFieldHdr.nz-1]);
  double max_down = MAX(_maxDownParam, _dbzFieldHdr.grid_minz);

  if (max_up <= max_down)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "max_up is less than max_down" << endl;
     
    return false;
  }

  // compute the interest
    
  if (_computeInterest)
  {
    if (_compute_interest(max_refl_interest, incr_refl_interest,
			  max_down, max_up) == -1)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Interest not computed" << endl;
      
      return false;
    }
  }
   
  return true;
}
