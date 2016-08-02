//
//
//
//

#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

#include "Fault.h"
using namespace ForayUtility;

#include "DoradeFile.h"
#include "DoradeBlockRktb.h"

#include "Parameters.h"


void list_blocks(string filename,int nb ) throw (Fault &);
void dump_rktb  (string filename)         throw (Fault &);

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

    cout << "Hi\n";

    Parameters params;
    if(params.init(argc,argv) < 0){
	exit(0);
    };

    try {

	if(params.dumpRktb_){
	    dump_rktb(params.inputFile_);
	}else{
	    list_blocks(params.inputFile_, -1);
	}

    }catch(Fault &re){
	cout << "Caught Fault \n";
	cout << re.msg();
	return -1;
    }catch(...){
	cout << "Caught exception (Default) \n";
	return -1;
    }
	
    cout << "Bye\n";
	
    return 0;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void list_blocks(string filename, int numberBlocks) throw (Fault &){

    DoradeFile df;

    try {
	cout << endl;
	cout << "Blocks from : " << filename << endl;
	df.open_file(filename);

	int blockCount(0);
	DoradeBlock *block;

	while(block = df.read_next_block()){

	    char numberString[1024];
	    sprintf(numberString,"%4d:",blockCount);

	    cout << numberString << block->listEntry();

	    delete block;

	    blockCount++;
	    if(numberBlocks > 0){
		if(blockCount > numberBlocks){
		    break;
		}
	    }
	}

	df.close_file();

	cout << endl;

    }catch(Fault &re){
	char msg[2048];
	sprintf(msg,"list_blocks : Caught Fault while listing blocks from %s \n",
		filename.c_str());
	re.add_msg(msg);
	throw re;
    }catch(...){
	throw Fault("list_blocks : Caught exception (Default) \n");
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void dump_rktb(string filename) throw (Fault &){

    DoradeFile df;

    try {
	cout << endl;
	cout << "RKTB from : " << filename << endl;
	df.open_file(filename);

	int blockCount(0);
	DoradeBlock     *block;
	DoradeBlockRktb *rktb;

	while(block = df.read_next_block()){

	    if(rktb = block->castToDoradeBlockRktb()){
		
		cout << rktb->listEntry();
		cout << endl;

		int    numberIndexes   = rktb->get_integer("index_queue_size");
		double index_per_angle = rktb->get_double ("angle_to_index");

		for(int index = 0; index < numberIndexes; index++){
		    char  line[1024];
		    double angle = index / index_per_angle;
		    sprintf(line,"%3d (%6.2f) : %3d \n",
			    index,
			    angle,
			    rktb->get_integer("index_queue",index));
		    cout << line;
		}

		cout << endl;

		int numberOfRays = rktb->get_integer("number_of_rays");
		for(int index = 0; index < numberOfRays; index++){
		    char line[2048];
		    sprintf(line,"%3d : %6.2f  %8d  %6d \n",
			    index,
			    rktb->get_double ("rotation_angle",index),
			    rktb->get_integer("offset"        ,index),
			    rktb->get_integer("size"          ,index));
		    cout << line;
		}

	    }

	    delete block;
	}

	df.close_file();

	cout << endl;

    }catch(Fault &re){
	char msg[2048];
	sprintf(msg,"dump_rktb : Caught Fault while listing blocks from %s \n",
		filename.c_str());
	re.add_msg(msg);
	throw re;
    }catch(...){
	throw Fault("dump_rktb : Caught exception (Default) \n");
    }
    

}
