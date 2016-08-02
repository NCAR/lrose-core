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
//
//
//
//
//

#include <iostream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <errno.h>
extern int errno;

#include <string.h>
using namespace std;

#include <regex.h>

#include "Fault.h"
using namespace ForayUtility;

#include "FilePath.h"

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
FilePath::FilePath(){
    
    dir_            = "";
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
FilePath::~FilePath(){

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FilePath::file(const string file) throw (Fault){

    // Is this a real file ?
    
    try{
	if(!file_exist(file)){
	    char msg[2048];
	    sprintf(msg,"FilePath::file : %s does not exist \n",file.c_str());
	    throw Fault(msg); 
	}
    }catch (Fault re){
	char msg[2048];
	sprintf(msg,"FilePath::file caught Fault \n");
	re.add_msg(msg);
	throw re;
    }

    filenames_.clear();
    filenames_.push_back(file);
    filenameIterator_ = filenames_.begin();
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FilePath::directory(const string dirpath,const string pattern) throw (Fault){
    
    // default value of pattern is "*"

    filenames_.clear();

    regex_t rtbuf;
    int     err;

    if ((err = regcomp (&rtbuf, pattern.c_str(), 0)) != 0){
	char ebuf[1024];
	char msg[2048];
	regerror (err, &rtbuf, ebuf, sizeof(ebuf));
	sprintf(msg,"FilePath::directory : regcomp returned error for pattern %s : %s \n",
		pattern.c_str(),
		ebuf);
	throw Fault(msg);
    }

    DIR *dir_pointer = NULL;

    if((dir_pointer = opendir(dirpath.c_str())) == NULL){
	char msg[2048];
	sprintf(msg,"FilePath::directory:: opendir failed for %s : %s \n",
		dirpath.c_str(),
		strerror(errno));
	throw Fault(msg);
    }

    struct dirent *de;
    while((de = readdir(dir_pointer)) != NULL){

	string name(de->d_name);
	string fullname = dirpath + "/" + de->d_name;

	int returnValue;
	if ((returnValue = regexec(&rtbuf, de->d_name, 0, 0, 0)) == 0){
	    if(is_file(fullname)){
		filenames_.push_back(fullname);
	    }
	}
    }

    // Check for readdir error.  readdir returns NULL at end of entries
    // *and* when an error occurs.
    if(errno != 0){
	char msg[2048];
	sprintf(msg,"FilePath::directory:: readdir failed for %s : %s \n",
		dirpath.c_str(),
		strerror(errno));
	throw Fault(msg);
    }

    filenameIterator_ = filenames_.begin();
    
    closedir(dir_pointer);
    regfree(&rtbuf);

    dir_ = dirpath;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FilePath::sort_files(){
    filenames_.sort();
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int FilePath::file_count(){
    return filenames_.size();
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FilePath::first_file() throw (Fault){

    filenameIterator_ = filenames_.begin();
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool FilePath::next_file(){

    if(filenameIterator_ == filenames_.end()){
	return false;
    }


    filenameIterator_++;

    if(filenameIterator_ == filenames_.end()){
	return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string FilePath::get_full_name() throw (Fault){

    if(filenameIterator_ == filenames_.end()){
	char msg[2048];
	sprintf(msg,"FilePath::get_name() : end of file name list\n");
	throw Fault(msg);
    }

    return *filenameIterator_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string FilePath::get_name() throw (Fault){

    if(filenameIterator_ == filenames_.end()){
	char msg[2048];
	sprintf(msg,"FilePath::get_name() : end of file name list\n");
	throw Fault(msg);
    }

    string filename = *filenameIterator_;
    
    int lastSlash = filename.rfind("/");

    if(lastSlash == string::npos){
	return filename;
    }

    return filename.substr(lastSlash + 1);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string FilePath::get_directory() throw (Fault){

    if(filenameIterator_ == filenames_.end()){
	return dir_;
    }

    string filename = *filenameIterator_;
    
    int lastSlash = filename.rfind("/");

    if(lastSlash == string::npos){
	return string("");
    }

    return filename.substr(0,lastSlash);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool FilePath::is_file(const string filename) throw (Fault){

    char msg[2048];
    struct stat buf;

    if(stat(filename.c_str(),&buf) < 0){
	sprintf(msg,"FilePath::is_file:: stat failed for %s : %s \n",
		filename.c_str(),
		strerror(errno));
	throw Fault(msg);
    }

    return S_ISREG(buf.st_mode);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool FilePath::file_exist(const string filename) throw (Fault){

    if(access(filename.c_str(),F_OK) != 0){
	
	if(errno == ENOENT ){
	    return false;
	}else{
	    char msg[2048];
	    sprintf(msg,"FilePath::file_exist:: access failed for %s : %s \n",
		    filename.c_str(),
		    strerror(errno));
	    throw Fault(msg);
	}
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string FilePath::combine(const string dir, const string name) throw (Fault){

    string fullPath;

    int lastSlash = dir.rfind("/");
    
    if((lastSlash == string::npos) || (lastSlash != (dir.length() - 1))){
	fullPath = dir + "/" + name;
    }else{
	fullPath = dir + name;
    }

    return fullPath;
}



