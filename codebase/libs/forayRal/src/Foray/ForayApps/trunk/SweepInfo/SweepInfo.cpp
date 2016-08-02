//
//
//
//
//

#include <unistd.h>
extern char *optarg;

#include <iostream>
#include <string>
using namespace std;

#include <ForayVersion.h>
using namespace ForayUtility;

#include <DoradeFile.h>
#include <NcRadarFile.h>


#include "SweepInfo.h"
//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
SweepInfo::SweepInfo(){


}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
SweepInfo::~SweepInfo(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void SweepInfo::init(int argc, char *argv[]) throw(Fault &){

    help_required_         = false;
    sweep_header_required_ = false;
    ray_header_required_   = false;

    fileType_ = NOTSET;

    int arg;
    char *opts = "hsrd:n:";

    while((arg = getopt(argc,argv,opts)) != -1){

	switch(arg){
	case 'h':
	    help_required_ = true;
	    return;

	case 's':
	    sweep_header_required_ = true;
	    break;   

	case 'r':
	    ray_header_required_ = true;
	    break;   

	case 'd':
	    fileType_    = DORADE;
	    rayFileName_ = string(optarg);
	    break;   

	case 'n':
	    fileType_    = NCRADAR;
	    rayFileName_ = string(optarg);
	    break;   

	default:
	    char message[2048];
	    sprintf(message,
		    "SweepInfo::init: Undefined command line option.\n"
		    "Use -h command line option for help message\n");
	    throw Fault(message);
	}
    }

    if(fileType_ == NOTSET){
	char message[2048];
	sprintf(message,
		"SweepInfo::init: No input file given.\n"
		"Use -h command line option for help message\n");
	throw Fault(message);

    }else if(fileType_ == DORADE){
	rayFile_ = new DoradeFile();

    }else if(fileType_ == NCRADAR){
	rayFile_ = new NcRadarFile();

    }

    try{
	rayFile_->open_file(rayFileName_,false);
    }catch(Fault &fault){
	char message[512];
	sprintf(message,"SweepInfo::init: Fault was caught while opening %s\n",
		rayFileName_.c_str());
	fault.add_msg(message);
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool SweepInfo::is_help_required(){

    return help_required_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool SweepInfo::is_sweep_header_required(){

    return sweep_header_required_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool SweepInfo::is_ray_header_required(){

    return ray_header_required_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void SweepInfo::init_help_message_(){

    string svnNumber = ForayVersion::get_svn_revision_string();
    string svnDate   = ForayVersion::get_svn_date_string();
    
    sprintf(help_message_,
            "\n"
	    " sweepinfo\n"
	    " Foray revision number: %s\n"
	    " Foray revsisoin date : %s\n"
	    "\n"
	    "  -h        :  this message \n"
	    "  -s        :  display sweep headers\n"
	    "  -r        :  display ray headers\n"
	    "  -d [File] :  input dorade file\n"
            "  -n [File] :  input NcRadar file\n\n",
	    svnNumber.c_str(),
	    svnDate.c_str());
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void SweepInfo::cout_help_message(){

    init_help_message_();
    cout << help_message_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void SweepInfo::cout_sweep_info() throw (Fault &){

    cout << "Sweep header data for " << rayFileName_ << endl << endl;

    try {

	rayFile_->read_headers();

	cout << "sweep_number  : " << rayFile_->get_integer("sweep_number")  << endl;
	cout << "volume_number : " << rayFile_->get_integer("volume_number") << endl;

    }catch (Fault &fault){
	fault.add_msg("SweepInfo::cout_sweep_info: caught Fault\n");
	throw fault;
    }

}



