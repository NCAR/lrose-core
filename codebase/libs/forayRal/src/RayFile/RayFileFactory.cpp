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
#include "RayFileFactory.h"

#include "DoradeFile.h"
#include "DoradeFileName.h"

#include "NcRadarFile.h"
#include "NcRadarFileName.h"
#include "NcREAL_File.h"

using namespace std;
using namespace ForayUtility;

RayFile *RayFileFactory::createRayFile(std::string &file_type,
                                       std::string& directory, ForayUtility::RaycTime& time,
				       std::string& platform, int scan_type, float fixed_angle,
                                       int volume_num, int sweep_num) throw (Fault)
{
  // char basename[50];
    RayFile *rayfile = NULL;

    if (file_type == "dorade"){
        rayfile = new DoradeFile();
        DoradeFileName fileNameClass;
        basename_ = fileNameClass.generate_swp_name(time, platform, scan_type, fixed_angle, volume_num);
    } else if (file_type == "netcdf") {
        rayfile = new NcRadarFile();
        NcRadarFileName fileNameClass;

        basename_ = fileNameClass.generate_sweep_name(time, platform, scan_type, fixed_angle, volume_num, sweep_num);
        
    } else if (file_type == "nc_real") {
        rayfile = new NcREAL_File();

        NcRadarFileName fileNameClass;
        basename_ = fileNameClass.generate_sweep_name(time, platform, scan_type, fixed_angle, volume_num, sweep_num);

    } else {
        throw Fault("unsupported output type");
    }
   
        
    //    basename_ =basename;
    string fullname = directory + "/" +  basename_;
    rayfile->open_file(fullname, true);
    return rayfile;
} 
