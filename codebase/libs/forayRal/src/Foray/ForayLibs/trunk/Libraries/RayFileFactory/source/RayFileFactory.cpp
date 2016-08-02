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
    char basename[50];
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
