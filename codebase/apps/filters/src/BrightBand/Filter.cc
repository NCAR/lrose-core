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
// Filter.cc:  Data with brightband filtered out
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#include "Filter.h"
#include <dsserver/DsLdataInfo.hh>
using namespace std;


//////////////
// Constructor

Filter::Filter(const string &output_url,
	       DsMdvx *input_data,
               MdvxField *dbz_field,
               int num_clumps,
               fl32 *clump_values,
               int *template_num,
               int *hts,
               int band_base1,
               int band_top1,
               int band_base2,
               int band_top2,
               int band_base3,
               int band_top3,
	       const bool debug_flag,
               Mdvx::encoding_type_t output_encoding,
               Mdvx::compression_type_t output_compression) :
  _debug(debug_flag)
{
  static const string method_name = "Filter::Filter()";
  
   time_t current_time;
   double slope;

   // band_base_ht is the grid level at which the base of the band was applied
 
   int band_base_ht = 0;
   
   // band_top_ht is the grid level at which the top of the band was applied

   int band_top_ht = 0;

   if (_debug)
   {
     cerr << "Creating Filter object" << endl;
   }

   // set up public data members

   OK = TRUE;

   // set up private data members

   _input_data = input_data;
   _dbz_field = dbz_field;
   _dbzFieldHdr = _dbz_field->getFieldHeader();

   fl32 *dbz_array = (fl32 *)_dbz_field->getVol();

   // if at least one clump of brightband was found, filter the data

   if (num_clumps > 0)
   {

       // filter the data

     int plane_size = _dbzFieldHdr.nx * _dbzFieldHdr.ny;
     
       for (int iy = 0; iy < _dbzFieldHdr.ny; iy++)
       {
	 for (int ix = 0; ix < _dbzFieldHdr.nx; ix++)
	 {
	   int plane_index = ix+iy*_dbzFieldHdr.nx;
	   
	   if ((clump_values[plane_index] == 1) && 
               ((template_num[plane_index] == 1) ||
                (template_num[plane_index] == 2) ||
                (template_num[plane_index] == 3)))
	   {
	     band_base_ht = hts[plane_index];

	     if (template_num[plane_index] == 1)
	     {
		band_top_ht = band_base_ht + (band_top1 - band_base1);
	     }
	     else if (template_num[plane_index] == 2)
	     {
		band_top_ht = band_base_ht + (band_top2 - band_base2);
	     }
	     else if (template_num[plane_index] == 3)
	     {
		band_top_ht = band_base_ht + (band_top3 - band_base3);
	     }

	     int band_top_index =
	       ((band_top_ht+1) * plane_size) + plane_index;
	     int band_base_index =
	       ((band_base_ht-1) * plane_size) + plane_index;

	     if (dbz_array[band_top_index] != _dbzFieldHdr.bad_data_value &&
		 dbz_array[band_top_index] != _dbzFieldHdr.missing_data_value)
	     {

		   if (band_base_ht == 0 || 
		       dbz_array[band_base_index] == _dbzFieldHdr.bad_data_value ||
		       dbz_array[band_base_index] == _dbzFieldHdr.missing_data_value)
		   {
		      for (int iz = band_base_ht; iz <= band_top_ht; iz++)
		      {
			int vol_index = (iz * plane_size) + plane_index;
			
			 dbz_array[vol_index] = 
			    dbz_array[band_top_index];
		      }
		   }
		   else
		   {

		       slope = (dbz_array[band_top_index] - 
				dbz_array[band_base_index])/
			       ((band_top_ht - band_base_ht + 2) *
				_dbzFieldHdr.grid_dz);

		       for (int iz = band_base_ht; iz <= band_top_ht; iz++)
		       {
			 int vol_index = (iz * plane_size) + plane_index;
			
			 dbz_array[vol_index] = 
			  (fl32) (dbz_array[band_top_index] + 
			    slope * (iz*_dbzFieldHdr.grid_dz - (band_top_ht + 1)*_dbzFieldHdr.grid_dz) + 0.5);

		       }
		   }

	     }

	   }
	 }
       }
   }
   
   // modify master header

   Mdvx::master_header_t master_hdr = _input_data->getMasterHeader();
   master_hdr.time_gen = (si32) time(&current_time);
   _input_data->setMasterHeader(master_hdr);

   // convert output field

   _dbz_field->convertType(output_encoding, output_compression);

   // write the file
    
   if (OK)
   {
     if (_input_data->writeToDir(output_url.c_str()) != 0)
     {
       cerr << "ERROR: " << method_name << endl;
       cerr << "Error writing output file to URL: "
	    << output_url << endl;
       cerr << _input_data->getErrStr() << endl;
       OK=FALSE;
     }
     
   }
   
   
}


/////////////
// Destructor

Filter::~Filter()
{
   if (_debug)
     cerr << "Deleting Filter object" << endl;
}




