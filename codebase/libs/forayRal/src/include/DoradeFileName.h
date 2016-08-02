//
//
//
//
//

#ifndef DORADEFILENAME_H
#define DORADEFILENAME_H

#include <string>

#include <Fault.h>

#include <KeyAccess.h>
#include <RaycTime.h>

#include <RayFile.h>

class DoradeFileName: public ForayUtility::KeyAccess {
public:
    DoradeFileName();
    ~DoradeFileName();


    std::string generate_sweep_name(const ForayUtility::RaycTime &time,
				    const std::string   radar,
				    const int           scan,
				    const double        fixedAngle, 
				    const int            volume)          throw(ForayUtility::Fault);

    std::string generate_sweep_name(RayFile &rayFile) throw(ForayUtility::Fault);


    std::string generate_swp_name(const ForayUtility::RaycTime &time,
				  const std::string   radar,
				  const int           scan,
				  const double        fixedAngle, 
				  const int            volume)          throw(ForayUtility::Fault);

    std::string generate_swp_name(RayFile &rayFile) throw(ForayUtility::Fault);

private:

};




#endif  // DORADEFILENAME_H
