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
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <string.h>
#include <dirent.h>
#include <cstdlib>

#include "ProductInfo.h"
#include "HeaderAttribute.h"
#include "func_prototype.h"

using namespace std;   

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	Program:	MRMS_to_CFncdf
	
	Date:		July 2013
	
	Author:		Carrie Langston (CIMMS/NSSL)
	        	
	Purpose:	This program will read in a MRMS binary file,
	            write it to CF-compliant netCDF.  Various command-line
	            options allow the user to address little vs big endian
	            problems and choose between a FAA specific CF-netCDF.
		
	Input:		command-line arguments and options:
					1) input file name
					2) output path
					3) options
					   -swap: data is switching between little and big
					      endian systems.  Apply a byte swap when reading
					      input files
					   -faa: write data for FAA display, which requires a
					      time dimension be added to the netCDF file.  This
					      results in file dimensions like... [time][nx][ny],
					      where time's size is always 1
					   -gzip: compress the NetCDf file by gzip
					   -comp: compress the NetCDf file
					          by gzip if NETCDF_4_OUT is not defined OR
					          by using NetCDF native compression if NETCDF_4_OUT is defined
					     
	                  
	Output: 	CF-compliant netCDF
				
				
	To Compile:	Use make.  
	            Should result in executable called hmrg_to_CFncdf
	           	
	To Run:	mrms_to_CFncdf <input path/file> <output path> [opitons]
																											
	_____________________________________________________________			
	Modification History:

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


/************************************************/
/***  F U N C T I O N  P R O T O T Y P E (S)  ***/
/************************************************/

vector<ProductInfo> setupMRMS_ProductRefData( );
                   
string stripSpaces(string in);

int productKnown(const char *varname, const char *varunit,
                 vector<ProductInfo>& pInfo);

//also see func_prototype.h



/********************************/
/***  M A I N  P R O G R A M  ***/
/********************************/

int main(int argc,  char* argv[])
{
    cout<<"\n\n"<<endl;
    cout<<"      *************************************************"<<endl;
    cout<<"      *                                               *"<<endl;
    cout<<"      *       WELCOME TO MRMS->CFNCDF CONVERTER       *"<<endl;
    cout<<"      *               v1.0 07/25/2013                 *"<<endl;
    cout<<"      *                                               *"<<endl;
    cout<<"      *************************************************"<<endl;
    cout<<endl;
    


    /*---------------------------------------*/
    /*** 0. Process command-line arguments ***/ 
    /*---------------------------------------*/

    //Process command-line arguments
    if( argc < 3 )
    {
      cout<<"Usage:  mrms_to_CFncdf [input file] [output path] (options)"<<endl;
      cout<<"  [input file]: full path and filename of input file"<<endl;
      cout<<"  [output path]: top level output directory for netCDF"<<endl;
      cout<<"  (optional arguments)"<<endl;
      cout<<"    -swap: turns on byte swapping when reading input files.  This is "
          <<"needed when data is written on a system with a different endian than the "
          <<"system running this converter (little vs. big)."<<endl;
      cout<<"    -faa: write CF netCDF specifically for display by the FAA. This "
          <<"adds a time dimension to the netCDF file, resulting in file dimensions "
          <<"like... [time][nx][ny], where time's size is always 1"<<endl;
      cout<<"    -gzip: compress the NetCDf file."<<endl;
      cout<<"    -varname: append variable name to output path."<<endl;

      cout<<"Exiting from mrms_to_CFncdf"<<endl<<endl;
      exit(0);
    }
    
    int gzip_flag = 0; //off, 1 for on
    string input_file = argv[1];
    string output_path = argv[2];
    bool swapflag = false, faa_compliant = false;
    bool appendVariableName = false;
    if( argc > 3 ) {
      for (int index = 3; index < argc; index++) {
        string option = argv[index];
        if( (option == "-swap") || (option == "-SWAP") ) swapflag = true;
        if( (option == "-faa" ) || (option == "-FAA" ) ) faa_compliant = true;
        if( (option == "-gzip") || (option == "-GZIP") ) gzip_flag = 1;
        if( (option == "-comp") || (option == "-COMP") ) gzip_flag = 1;
        if( (option == "-varname") || (option == "-VARNAME") ) appendVariableName = true;
      }
    }
    
    cout<<"Swap flag for little vs. big endian is ";
    if(swapflag) cout<<"on"<<endl;
    else cout<<"off"<<endl;
    
    if(faa_compliant) 
      cout<<"Output will be FAA display compliant"<<endl;
    
    cout<<endl;
    
    
    
    /*---------------------------------------*/
    /*** 1. Read in product reference data ***/
    /*---------------------------------------*/

    vector<ProductInfo> productInfo;
    productInfo = setupMRMS_ProductRefData( );
    
    if(productInfo.size() < 1)
    {
      cout<<"+++ERROR: Failed to setup product reference information from. Exiting!"
          <<endl<<endl;
      exit(0);
    }
    
        

    /*----------------------------------------*/
    /*** 2. Read input file and error check ***/
    /*----------------------------------------*/
    
    //Declare some reusable variables
    char varname[20];
    char prev_varname[20];
    char varunit[6];
    int nradars;
    vector<string> radarnames;
    int var_scale, missing;
    float nw_lat, nw_lon;
    int nx, ny, nz;
    float dx, dy;
    float zhgt[50]; //assume there will be <50 levels <- BAD PRACTICE!
    long epoch_sec;
    short int* input_data_1D = 0;
    float* input_data_1D_FLOAT = 0;
        
    int pIndex = -1;
    strcpy(prev_varname, "");
    
    
    cout<<" Processing: "<<input_file<<endl;
      
      
    /*** 2A. Read file header and data ***/
      
    input_data_1D = mrms_binary_reader_cart3d(input_file.c_str(),                     
                           varname, varunit,
                           nradars, radarnames,
                           var_scale, missing,
                           nw_lon, nw_lat,
                           nx, ny, dx, dy,
                           zhgt, nz, epoch_sec, swapflag);
      
    //Error checking
    if(input_data_1D == 0)
    {
      cout<<"+++ERROR: Failed to read "<<input_file<<" Exiting!"<<endl;
      exit(0);

    }
    else if( (nx < 1) || (ny < 1) || (nz < 1) )
    {
      cout<<"+++ERROR: Dimensions bad for "<<input_file<<" Exiting!"<<endl;
      delete [] input_data_1D;
      input_data_1D = 0;
      exit(0);

    }
     
    cout<<" DONE reading file"<<endl;
      
      
    /*** 2B. Check if entry for data field exists in product ref data ***/
     
    //Remove any spaces in variable name. Replace with underscore
    string tmpstr = stripSpaces( (string)varname);
    strcpy(varname, tmpstr.c_str());      

    //Remove any spaces in variable unit. Replace with underscore
    //cout<<"VarUnit b4: >"<<varunit<<"<"<<endl;
    tmpstr = stripSpaces( (string)varunit);
    strcpy(varunit, tmpstr.c_str());      
    //cout<<"VarUnit aft: >"<<varunit<<"<"<<endl;

    //If previous and current file contain the same type of data
    //field, then no need to search for product info again.
    //If different, then search
    if(strcmp(varname, prev_varname) != 0)
    {
      pIndex = productKnown(varname, varunit, productInfo);
    }
      
    if(pIndex < 0)
    {
      cout<<"+++ERROR: Data field (name="<<varname<<", unit="<<varunit
          <<") not found in product reference info"<<endl;
      delete [] input_data_1D;
      input_data_1D = 0;
      exit(0);            
    }
      
      
    /*** 2C. Helpful print statement ***/
      
    //Print out header info.
    cout<<endl<<" Binary Header Info:"<<endl;
    cout<<"  variable name = "<<varname<<endl;
    cout<<"  variable unit = "<<varunit<<endl;
    cout<<"  number of radars = "<<nradars<<endl;
    
    cout<<"  Radars: ";
    for(size_t i = 0; i < radarnames.size(); i++)
      cout<<radarnames[i]<<" ";
    cout<<endl;
    
    cout<<"  variable scale = "<<var_scale<<endl;
    cout<<"  missing value = "<<missing<<endl;
    cout<<"  NW latitude = "<<nw_lat<<endl;
    cout<<"  NW longitude = "<<nw_lon<<endl;
    cout<<"  Number of columns = "<<nx<<endl;
    cout<<"  Number of rows = "<<ny<<endl;
    cout<<"  Number of levels = "<<nz<<endl;
    cout<<"  Grid cell size (degree lat.) = "<<dy<<endl;
    cout<<"  Grid cell size (degree lon.) = "<<dx<<endl;
    cout<<"  Number of vertical levels = "<<nz<<endl;
    
    cout<<"  Level heights = ";
    for(int i = 0; i < nz; i++) cout<<zhgt[i]<<" ";
    cout<<endl;
    
    char timestamp[20];
    char timestamp_ymd[20];
    strftime(timestamp, 20, "%m/%d/%Y %H%M", gmtime(&epoch_sec));
    cout<<"  Time = "<<timestamp<<" UTC  (or "<<epoch_sec
        <<" epoch seconds)"<<endl<<endl;
          
    //cout<<"  Will map missing values to "<<productInfo[pIndex].w2Missing<<endl;
    //cout<<"  Will map no coverage values to "<<productInfo[pIndex].w2NoCoverage<<endl;
      
            
      
    /*------------------------*/
    /*** 3. Write CF netCDF ***/
    /*------------------------*/

      
    /*** 3A. Prep for file output (header) ***/
    
    //bookkeeping
    int status = 0;
    
    //file names and system commands
    char output_path_name[500];
    char output_path_fname[500];
    char system_command[500];
    
    vector<HeaderAttribute> attrs; //keep empty
      
    long fcstTime = (long)productInfo[pIndex].fcstTime;
    string dataType = productInfo[pIndex].cfName;
    string longName = productInfo[pIndex].cfLongName;
    string varName = productInfo[pIndex].cfName;
    string varUnit = productInfo[pIndex].cfUnit;
    if(varUnit == "none") varUnit.clear();
      
    //Set time parameters for valid time. Take forecast products into account.
    string cf_time_string;
    long cf_fcst_length;
    
    if(fcstTime > 0)
    {
      //If product is a forecast
      char tmp_cf_time_string[50];
      strftime(tmp_cf_time_string, 50,
             "seconds since %Y-%m-%d %H:%M:%S", gmtime(&epoch_sec));
      cf_time_string = tmp_cf_time_string;
      cf_fcst_length = fcstTime;
    }
    else
    {
      //If product is NOT a forecast, then set time parameters 
      //for valid time
      cf_time_string = "seconds since 1970-1-1 0:0:0";
      cf_fcst_length = epoch_sec;  
    }

      
      
    //Data time info
    //strftime (timestamp, 20, "%Y%m%d-%H%M%S", gmtime(&epoch_sec));    
    strftime (timestamp, 20, "%Y%m%d_%H%M", gmtime(&epoch_sec));    
    strftime (timestamp_ymd, 20, "%Y%m%d", gmtime(&epoch_sec));
    float fractional_time = 0.0; //milliseconds
    
    
    //Check for special case where output path should include subdir
    //based on height of field (e.g., mrefl_levels)
    bool wrtSubDir = false;
    char sub_dir[20];

    if( (varName == "MREFL") && (nz == 1) )
    {
      cout<<"Looks like we're converting 2D slices of 3D reflectivity. "
          <<"Writing output to height-based subdirectories."<<endl;
      wrtSubDir = true;
    
      if(zhgt[0] < 1000)
        sprintf(sub_dir, "0%.2f", (zhgt[0]/1000.0));
      else if(zhgt[0] < 10000)
        sprintf(sub_dir, "0%.2f", (zhgt[0]/1000.0));
      else
        sprintf(sub_dir, "%.2f", (zhgt[0]/1000.0));    
    }
          
          
    //Build output file name and
    //Build mkdir command and execute (for output directory)
    // structure:  [top dir]/[product]
    //  subproduct = height level
    if(wrtSubDir)
    {
      if (appendVariableName)
      {
        sprintf(output_path_fname, "%s/%s/%s/%s.netcdf", output_path.c_str(),
                varName.c_str(), sub_dir, timestamp);
        sprintf(system_command, "mkdir -p %s/%s/%s", 
                output_path.c_str(), varName.c_str(), sub_dir);
      }
      else
      {
        sprintf(output_path_name, "%s/%s", output_path.c_str(),sub_dir);
        sprintf(output_path_fname, "%s/%s", output_path_name,timestamp);
        sprintf(system_command, "mkdir -p %s", output_path_name);
      }
    }
    else
    {
      if (appendVariableName)
      {
        sprintf(output_path_fname, "%s/%s/%s.netcdf", output_path.c_str(),
                varName.c_str(), timestamp);
        sprintf(system_command, "mkdir -p %s/%s", 
                output_path.c_str(), varName.c_str());    
      }
      else
      {
        sprintf(output_path_name, "%s/%s", output_path.c_str(), timestamp_ymd);
        sprintf(output_path_fname, "%s/%s", output_path_name,timestamp);
        sprintf(system_command, "mkdir -p %s", output_path_name);    
      }
    }

    //cout<<" Making: "<<system_command<<endl;
    system(system_command);
          
          
      
    float range_folded_value = missing -1;
      
    
      
    /*** 3B. Prep for file output (data) ***/
    
    int num = nx*ny*nz;
    input_data_1D_FLOAT = new float [num];
      
    //unscale
    for(int i = 0; i < num; i++) 
      input_data_1D_FLOAT[i] = (float)input_data_1D[i] / (float)var_scale;


      
    /*** 3C. Determine if 2D or 3D data.  Call correct output function ***/
    if(nz > 1)
    {
      if( faa_compliant )
      {
        cout<<" Writing 3D file (compliant with FAA display requirements)."<<endl;
        status = write_CF_netCDF_3d_FAA( (string)output_path_fname, 
                     dataType, longName, varName, varUnit,  
                     nx, ny, nz, dx, dy, nw_lat, nw_lon, zhgt,
                     epoch_sec, fractional_time,
                     missing, range_folded_value,
                     input_data_1D_FLOAT, gzip_flag);
      }
      else
      {
        cout<<" Writing 3D file."<<endl;
        status = write_CF_netCDF_3d( (string)output_path_fname, 
                     dataType, longName, varName, varUnit,  
                     nx, ny, nz, dx, dy, nw_lat, nw_lon, zhgt,
                     epoch_sec, fractional_time,
                     missing, range_folded_value,
                     input_data_1D_FLOAT, gzip_flag);
      }
      
    }
    else
    {
      if( faa_compliant )
      {
        cout<<" Writing 2D file (compliant with FAA display requirements)."<<endl;
        status = write_CF_netCDF_2d_FAA( (string)output_path_fname, 
                     dataType, longName, varName, varUnit,  
                     nx, ny, dx, dy, nw_lat, nw_lon, zhgt[0],
                     epoch_sec, fractional_time, cf_time_string, 
                     cf_fcst_length, attrs, missing, range_folded_value,
                     input_data_1D_FLOAT, gzip_flag);
               
      }
      else
      {
        cout<<" Writing 2D file."<<endl;
        status = write_CF_netCDF_2d( (string)output_path_fname, 
                     dataType, longName, varName, varUnit,  
                     nx, ny, dx, dy, nw_lat, nw_lon, zhgt[0],
                     epoch_sec, fractional_time, cf_time_string, 
                     cf_fcst_length, attrs, missing, range_folded_value,
                     input_data_1D_FLOAT, gzip_flag);
               
      }
        
    }//end if-blk (nz > 1)
      
      
    if(status > 0)
    {
      cout<<" Output is "<<output_path_fname;
      if(gzip_flag) {
#ifndef NETCDF_4_OUT
        cout<<".gz"<<endl;
#endif
      }
      else cout<<endl;
    }
      
    cout<<" DONE writing"<<endl<<endl;
          

    
    /*------------------------*/
    /*** 4. Free-up Memory, ***/
    /*------------------------*/

    //memory clean-up
    if(input_data_1D_FLOAT != 0) delete [] input_data_1D_FLOAT;
    if(input_data_1D != 0) delete [] input_data_1D;
                

    cout<<"CONVERTER DONE."<<endl<<endl;
    return 1;
    
}//end main function




/**************************/
/*** F U N C T I O N S  ***/
/**************************/

//also see mrms_binary_reader.cc, write_CF_netCDF_2d.cc, write_CF_netCDF_2d_FAA.cc,
// write_CF_netCDF_3d.cc, write_CF_netCDF_3d_FAA.cc

string stripSpaces(string in)
{
    string out = in;
    size_t length;
  
    while(out.find(" ") != string::npos)
    {
      length = out.length();
      
      if( (out.find(" ") == 0) && (length > 1) )
      {
        out = out.substr(1);
      }
      else if( (out.find(" ") == (length-1)) && 
               (length > 1) )
      {
        out = out.substr(0, (length-1));
      }
      else if(length <= 1)
      {
        out.clear();
      }
      else
      {  
        out[out.find(" ")] = '_';
      }
       
    }//end while-loop
    
    //the loop above may result in extra leading or ending underscores.
    //e.g., when there are more than one leading or ending spaces
    //undo the damage here. 
    while(out.find("_") != string::npos)
    {      
      if(out.rfind("_") == 0)
        out = out.substr(1);  
      else break;      
    }

    while(out.rfind("_") != string::npos)
    {
      length = out.length();
      
      if(out.rfind("_") == (length-1))
        out = out.substr(0, (length-1));
      else break;
    }
           
    return out;
  
}//end function stripSpaces



int productKnown(const char *varname, const char *varunit, 
                 vector<ProductInfo>& pInfo)
{
    string pName = varname;
    string pUnit = varunit;
    
    if(pName.empty()) return -1;
    //if(pUnit.empty()) return -1;
    
    
    for(size_t p = 0; p < pInfo.size(); p++)
    {
      if( pInfo[p].isMatch( pName, pUnit ) )  return p;
      
    }//end p-loop
    
    return -1;

}//end functio productKnown
                 
