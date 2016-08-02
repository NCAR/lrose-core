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

#include <cstdio>
#include <toolsa/umisc.h>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

class MetarFiles {

public:

  // Constructor
  MetarFiles(char *InDirPath, bool Debug, int MaxGap, int MaxFails,
	     long MinSize, char *OutDirPath, bool writeLdataFile);

  // Update
  int Update(bool Debug, long MinSize);

  // Destructor
  ~MetarFiles();


private:
protected:

  DsLdataInfo *_ldataInfo;
  FILE *ifp;
  char *_InDirPath; // Local copy of InDirPath
  char *_OutDirPath; // Local copy of OutDirPath
  char FileName[MAX_PATH_LEN];
  long Offset;
  date_time_t FileTime;
  int Max_Gap;
  int Max_Fails;
  int Fails;

  void get_filename(date_time_t t, char *Dir,
		    char *Name, int *Exists, long *Size);

  void process_grown_file(FILE **fp, long StartOffset,
			  long FinOffset, char *OutDir);

};






