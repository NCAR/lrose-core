#ifndef RAYFILEFACTORY_H
#define RAYFILEFACTORY_H



#include <string>

#include "RayFile.h"
#include "RaycTime.h"


class RayFileFactory 
{
 public:
    RayFileFactory(){};
    
    RayFile *createRayFile(std::string &file_type, std::string& directory,
			   ForayUtility::RaycTime& time,
			   std::string& platform, int scan_type, float fixed_angle,
                           int volume_num, int sweep_num) throw(ForayUtility::Fault);
    std::string basename() const {return basename_; }

    

 private:
    std::string basename_;
    
};
#endif // RAYFILEFACTORY_H

