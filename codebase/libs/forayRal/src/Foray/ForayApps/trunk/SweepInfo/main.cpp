/*
 *  Foray show header application.
 *
 */


#include "SweepInfo.h"

#include "Fault.h"
using namespace ForayUtility;


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

    SweepInfo sweepInfo;
    
    try{
	sweepInfo.init(argc,argv);
    }catch(Fault &fault){
	cout << fault.msg();
	exit(0);
    }

    if(sweepInfo.is_help_required()){
	sweepInfo.cout_help_message();
	exit(0);
    }

    try{
	if(sweepInfo.is_sweep_header_required()){
	    sweepInfo.cout_sweep_info();
	}
    }catch(Fault &fault){
	cout << fault.msg();
	exit(0);
    }

    exit(0);
}


