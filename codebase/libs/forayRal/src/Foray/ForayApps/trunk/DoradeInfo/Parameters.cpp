// $Id: Parameters.cpp,v 1.1 2008/10/23 05:06:18 dixon Exp $
//
//
//

#include <unistd.h>
extern char *optarg;

#include <iostream>
using namespace std;

#include "Parameters.h"


//////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////
Parameters::Parameters() {


}

//////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////
Parameters::~Parameters() {


}

//////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////
int Parameters::init(int argc, char *argv[]){

  set_defaults();
  if(read_command_line(argc,argv) < 0){
    return -1;
  }

  return 0;
}

//////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////
void Parameters::set_defaults(){
  
  inputFile_  = "none";
  dumpRktb_   = false;
}


//////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////
int Parameters::read_command_line(int argc, char *argv[]){

  char *opts = "f:th";
  int  arg;

  char *help_msg = "doradeinfo -f sweep_file [-t]\n";

  if(argc == 0){
    cerr << help_msg;
    return -1;
  }

  while((arg = getopt(argc,argv,opts)) != -1){

    switch(arg){
    case 'f':
	inputFile_  = optarg;
	singleFile_ = true;
	break;
	
    case 't':
	dumpRktb_ = true;
	break;

    case 'h':
	cout << help_msg;
	return -1;
      break;

    default:
      cerr << help_msg;
      return -1;
    }
  }

  if(inputFile_ == "none"){
      cerr << help_msg;
      return -1;
  }

  return 0;
}


