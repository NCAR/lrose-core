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
// Interest_MDV.cc:  Sets up and writes interest MDV file
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#include <toolsa/str.h>

#include "Interest_MDV.h"

using namespace std;

//
// Constants
//
const int Interest_MDV::MAX_NAME_LEN = 80;

//////////////
// Constructor

Interest_MDV::Interest_MDV (const string &interest_url,
                            DsMdvx *input_data,
                            Refl_Interest *refl_interest,
                            Incr_Refl_Interest *incr_refl_interest,
			    vector< Template_Interest* > &template_interest,
                            Max_Interest *max_interest,
                            Texture_Interest *texture_interest,
                            Clump *clumps,
			    const bool debug_flag) :
  _debug(debug_flag),
  _interestUrl(interest_url)
{  
  static const string method_name = "Interest_MDV::Interest_MDV()";
  
   if (_debug)
   {
     cerr << "Creating Interest_MDV object" << endl;
   }

   // set up public data members

   DsMdvx output_file;
   
   _prepare_data(output_file,
		 input_data->getPathInUse(),
		 input_data->getMasterHeader(),
		 refl_interest,
		 incr_refl_interest,
		 template_interest,
		 max_interest, 
		 texture_interest,
		 clumps);

   if (output_file.writeToDir(_interestUrl.c_str()) != 0)
   {
     cerr << "ERROR: " << method_name << endl;
     cerr << "Error writing interest fields to URL: "
	  << _interestUrl << endl;
     cerr << output_file.getErrStr() << endl;
   }
   
}

//////////////////
// Destructor

Interest_MDV::~Interest_MDV()
   
{
  if (_debug)
  {
    cerr << "Deleting Interest_MDV object" << endl;
  }
  

}

/////////////////
// _prepare_data
//
// set up data and headers to be written to mdv file

void Interest_MDV::_prepare_data(DsMdvx &output_file,
				 const string &dataset_source,
				 const Mdvx::master_header_t input_master_hdr,
				 Refl_Interest *refl_interest,
                                 Incr_Refl_Interest *incr_refl_interest,
				 vector< Template_Interest* > &template_interest,
                                 Max_Interest *max_interest,
                                 Texture_Interest *texture_interest,
                                 Clump *clumps)
{
  // modify master header

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
   
  master_hdr.time_gen = (si32) time(0);
  master_hdr.time_begin = input_master_hdr.time_begin;
  master_hdr.time_end = input_master_hdr.time_end;
  master_hdr.time_centroid = input_master_hdr.time_centroid;
  master_hdr.time_expire = input_master_hdr.time_expire;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = input_master_hdr.vlevel_type;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  master_hdr.sensor_lon = input_master_hdr.sensor_lon;
  master_hdr.sensor_lat = input_master_hdr.sensor_lat;
  master_hdr.sensor_alt = input_master_hdr.sensor_alt;
  STRcopy(master_hdr.data_set_info, "BrightBand", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "BrightBand", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, dataset_source.c_str(), MDV_NAME_LEN);

  output_file.setMasterHeader(master_hdr);
   
  refl_interest->addInterestFields(output_file,
				   "MAX REFLECTIVITY",
				   "MAX REFL");

  incr_refl_interest->addInterestFields(output_file,
					"REFL INCREASING",
					"INCR REFL");
	
  vector< Template_Interest* >::iterator ti_iter;
  int template_num = 1;
  
  for (ti_iter = template_interest.begin();
       ti_iter != template_interest.end(); ++ti_iter, ++template_num)
  {
    char corr_long_name[MAX_NAME_LEN];
    char corr_short_name[MAX_NAME_LEN];
    char slope_long_name[MAX_NAME_LEN];
    char slope_short_name[MAX_NAME_LEN];
    char int_long_name[MAX_NAME_LEN];
    char int_short_name[MAX_NAME_LEN];
    
    sprintf(corr_long_name, "TEMP %d CORR", template_num);
    sprintf(corr_short_name, "CORR %d", template_num);
    sprintf(slope_long_name, "TEMP %d SLOPE", template_num);
    sprintf(slope_short_name, "SLOPE %d", template_num);
    sprintf(int_long_name, "TEMPLATE %d", template_num);
    sprintf(int_short_name, "TEMP %d", template_num);
    
    (*ti_iter)->addFields(output_file,
			  corr_long_name,
			  corr_short_name,
			  slope_long_name,
			  slope_short_name);
   
    (*ti_iter)->addInterestFields(output_file,
				  int_long_name,
				  int_short_name);

  }
  
  max_interest->addInterestFields(output_file,
				  "MAX OF TEMPLATES",
				  "MAX TEMP");

  texture_interest->addInterestFields(output_file,
				      "TEXTURE OF MAX",
				      "TEXTURE");
	
  clumps->addInterestFields(output_file,
			    "CLUMPED INTEREST",
			    "CLUMPS");
   
}
