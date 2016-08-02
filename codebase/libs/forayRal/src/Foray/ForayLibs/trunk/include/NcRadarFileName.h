//
//
//
//
//


#ifndef NCRADARFILENAME
#define NCRADARFILENAME

#include <string>

#include <Fault.h>

#include <KeyAccess.h>
#include <RaycTime.h>

#include <RayFile.h>

class  NcRadarFileName: public ForayUtility::KeyAccess {
public:
    NcRadarFileName();
    ~NcRadarFileName();

    std::string generate_sweep_name(const ForayUtility::RaycTime &time,
				    const std::string            radar,
				    const int                    scanType,
				    const double                 fixedAngle, 
				    const int                    volumeNumber,
				    const int                    sweepNumber)     throw(ForayUtility::Fault);


    std::string generate_sweep_name(RayFile &rayFile) throw(ForayUtility::Fault);

private:

};






#endif // NCRADARFILENAME
