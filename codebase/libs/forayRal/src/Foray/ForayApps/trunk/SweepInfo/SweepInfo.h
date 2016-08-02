//
//
//
//
//

#include <iostream>
using namespace std;

#include <stdio.h>

#include <Fault.h>
#include <ForayVersion.h>
using namespace ForayUtility;

#include "RayFile.h"


class SweepInfo {

public:
    SweepInfo();
    ~SweepInfo();
    void init(int argc, char *argv[])  throw(ForayUtility::Fault &);
    
    bool is_help_required();
    bool is_sweep_header_required();
    bool is_ray_header_required();

    void cout_help_message();
    void cout_sweep_info()             throw(ForayUtility::Fault &);

private:

    void init_help_message_();

    enum FILETYPES {DORADE,NCRADAR,NOTSET} fileType_;
    
    bool help_required_;
    bool sweep_header_required_;
    bool ray_header_required_;

    char help_message_[4096];

    std::string rayFileName_;


    RayFile *rayFile_;
    
};
