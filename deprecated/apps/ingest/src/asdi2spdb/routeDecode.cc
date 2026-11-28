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

/************************************************************************
 * routeDecode.cc: routeDecode program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2005
 *
 * Jason Craig
 *
 ************************************************************************/

#include <cstdio>
#include <math.h>
#include "routeDecode.hh"

float _mod(float a, float b) {
  return a - b*floor(a/b);
}

routeDecode::routeDecode(Params *tdrpParams)
{ 
  _isOK = false;
  char APTpath[strlen(tdrpParams->FaaRouteDir)+9];
  char ARPpath[strlen(tdrpParams->FaaRouteDir)+9];
  char NAVpath[strlen(tdrpParams->FaaRouteDir)+9]; 
  char FIXpath[strlen(tdrpParams->FaaRouteDir)+9]; 
  char AWYpath[strlen(tdrpParams->FaaRouteDir)+9]; 
  char SSDpath[strlen(tdrpParams->FaaRouteDir)+9]; 

  float altf;                     // Altitude as float (arp file)
  int i = 0, j = 0, a = 0;        // Loop Indexs
  int numRead;                    // Number of bytes read using fread
  int alt;                        // Altitude as int  (apt file)
  int offset;                     // Offset into buffer
  int sscan;                      // Return int from sscanf
  vector<char *> *nameVec;        // Pointer to name Vector
  vector<float> *latVec;          // Pointer to lat Vector
  vector<float> *lonVec;          // Pointer to lon Vector
  char *name;                     // Character pointer for name
  char buffer[_bufferSize];       // Line buffer [MAX_LINE_SIZE]
  char latc[_internalStringLenSmall];  // Character array to read in lat before atof
  char lonc[_internalStringLenSmall];  // Character array to read in lon before atof

  _params = tdrpParams;

  _destName = new char[_internalStringLenSmall];
  _deptName = new char[_internalStringLenSmall];

  strcpy(APTpath,tdrpParams->FaaRouteDir);
  strcat(APTpath,"/APT.loc");
  strcpy(ARPpath,tdrpParams->FaaRouteDir);
  strcat(ARPpath,"/ARP.loc"); 
  strcpy(NAVpath,tdrpParams->FaaRouteDir);
  strcat(NAVpath,"/NAV.loc");
  strcpy(FIXpath,tdrpParams->FaaRouteDir);
  strcat(FIXpath,"/FIX.loc");
  strcpy(AWYpath,tdrpParams->FaaRouteDir);
  strcat(AWYpath,"/AWY.loc");
  strcpy(SSDpath,tdrpParams->FaaRouteDir);
  strcat(SSDpath,"/SSD.loc");


// *************************
// Read in APT file
// *************************
  if (_params->debug)
    cout << "Opening " << APTpath << endl;
  FILE *APTfp = fopen(APTpath,"rb");
  if(APTfp == NULL) {
    cerr << "ERROR: Cannot open file " << APTpath << endl;
    return;
  }
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, APTfp);
    i++;
    if(numRead != 1)
      buffer[i-1] = EOL;
  } while(buffer[i-1] != EOL && i < _bufferSize);
  if(buffer[i-1] == EOL)
    buffer[i-1] = NUL;
  else {
    cerr << "ERROR: reading size of APT file" << endl;
    return;
  }
  _APTsize = atoi(buffer);

  _APTname = new char *[_APTsize]; 
  for(a = 0; a < _APTsize; a++) {
    _APTname[a] = new char[_internalStringLenSmall];
  }
  _APTlat = new float[_APTsize];
  _APTlon = new float[_APTsize];

  i = 0;
  j = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, APTfp);
    if (numRead == 1) {
      if (buffer[i] == EOL) {
	buffer[i]=NUL;
	i=0;
	if(4 != sscanf(buffer, "%s %f %f %d", _APTname[j], &_APTlat[j], &_APTlon[j], &alt)) {
	  cerr << "WARNING: Bad line in APT, " << buffer << endl;
	  return;
	} else {
	  j++;
	}
      } else {
	if ((int)buffer[i] > 0 && i < _bufferSize)
	  i++;
      }
    }
  } while (numRead == 1 && j < _APTsize);
  fclose(APTfp);
  _APTsize = j;
  if (_params->debug)
    cout <<"Read "<<_APTsize<<" lines from "<<APTpath<<endl;

// *************************
// Read in ARP file
// *************************
  if (_params->debug)
    cout << "Opening " << ARPpath << endl;
  FILE *ARPfp = fopen(ARPpath,"rb");
  if(ARPfp == NULL) {
    cerr << "ERROR: Cannot open file " << ARPpath << endl;
    return;
  }
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, ARPfp);
    i++;
    if(numRead != 1)
      buffer[i-1] = EOL;
  } while(buffer[i-1] != EOL && i < _bufferSize);
  if(buffer[i-1] == EOL)
    buffer[i-1] = NUL;
  else {
    cerr << "ERROR: reading size of ARP file" << endl;
    return;
  }
  _ARPsize = atoi(buffer);

  _ARPname = new char *[_ARPsize];
  for(a = 0; a < _ARPsize; a++) {
    _ARPname[a] = new char[_internalStringLenSmall];
  }
  _ARPlat = new float[_ARPsize];
  _ARPlon = new float[_ARPsize];

  i = 0;
  j = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, ARPfp);
    if (numRead == 1) {
      if (buffer[i] == EOL) {
	buffer[i]=NUL;
	i=0;
	if(4 != sscanf(buffer, "%s %f %f %f", _ARPname[j], &_ARPlat[j], &_ARPlon[j], &altf)) {
	  cerr << "WARNING: Bad line in ARP, " << buffer << endl;
	  return;
	} else {
	  j++;
	}
      } else {
	if ((int)buffer[i] > 0 && i < _bufferSize)
	  i++;
      }
    }
  } while (numRead == 1 && j < _ARPsize);
  fclose(ARPfp);
  _ARPsize = j;
  if (_params->debug)
    cout <<"Read "<<_ARPsize<<" lines from "<<ARPpath<<endl;

// *************************
// Read in NAV file
// *************************
  if (_params->debug)
    cout << "Opening " << NAVpath << endl;
  FILE *NAVfp = fopen(NAVpath,"rb");
  if(NAVfp == NULL) {
    cerr << "ERROR: Cannot open file " << NAVpath << endl;
    return;
  }
  i = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, NAVfp);
    i++;
    if(numRead != 1)
      buffer[i-1] = EOL;
  } while(buffer[i-1] != EOL && i < _bufferSize);
  if(buffer[i-1] == EOL)
    buffer[i-1] = NUL;
  else {
    cerr << "ERROR: reading size of NAV file" << endl;
    return;
  }
  _NAVsize = atoi(buffer);

  _NAVname = new char *[_NAVsize]; 
  for(a = 0; a < _NAVsize; a++) {
    _NAVname[a] = new char[_internalStringLenSmall];
  }
  _NAVlat = new float[_NAVsize];
  _NAVlon = new float[_NAVsize];

  i = 0;
  j = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, NAVfp);
    if (numRead == 1) {
      if (buffer[i] == EOL) {
	buffer[i]=NUL;
	i=0;
	if(3 != sscanf(buffer, "%s %f %f", _NAVname[j], &_NAVlat[j], &_NAVlon[j])) {
	  cerr << "WARNING: Bad line in NAV, " << buffer << endl;
	  return;
	} else {
	  j++;
	}
      } else {
	if ((int)buffer[i] > 0 && i < _bufferSize-1)
	  i++;
      }
    }
  } while (numRead == 1 && j < _NAVsize);
  fclose(NAVfp);
  _NAVsize = j;
  if (_params->debug)
    cout <<"Read "<<_NAVsize<<" lines from "<<NAVpath<<endl;

// *************************
// Read in FIX file
// *************************
  if (_params->debug)
    cout << "Opening " << FIXpath << endl;
  FILE *FIXfp = fopen(FIXpath,"rb");
  if(FIXfp == NULL) {
    cerr << "ERROR: Cannot open file " << FIXpath << endl;
    return;
  }
  i = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, FIXfp);
    i++;
    if(numRead != 1)
      buffer[i-1] = EOL;
  } while(buffer[i-1] != EOL && i < _bufferSize);
  if(buffer[i-1] == EOL)
    buffer[i-1] = NUL;
  else {
    cerr << "ERROR: reading size of FIX file" << endl;
    return;
  }
  _FIXsize = atoi(buffer);

  _FIXname = new char *[_FIXsize];
  for(a = 0; a < _FIXsize; a++) {
    _FIXname[a] = new char[_internalStringLenSmall];
  }
  _FIXlat = new float[_FIXsize];
  _FIXlon = new float[_FIXsize];

  i = 0;
  j = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, FIXfp);
    if (numRead == 1) {
      if (buffer[i] == EOL) {
	buffer[i]=NUL;
	i=0;
	if(3 != sscanf(buffer, "%s %f %f", _FIXname[j], &_FIXlat[j], &_FIXlon[j])) {
	  cerr << "WARNING: Bad line in FIX, " << buffer << endl;
	  return;
	} else {
	  j++;
	}
      } else {
	if ((int)buffer[i] > 0 && i < _bufferSize-1)
	  i++;
      }
    }
  } while (numRead == 1 && j < _FIXsize);
  fclose(FIXfp);
  _FIXsize = j;
  if (_params->debug)
    cout <<"Read "<<_FIXsize<<" lines from "<<FIXpath<<endl;

// *************************
// Read in AWY file
// *************************
  if (_params->debug)
    cout << "Opening " << AWYpath << endl;
  FILE *AWYfp = fopen(AWYpath,"rb");
  if(AWYfp == NULL) {
    cerr << "ERROR: Cannot open file " << AWYpath << endl;
    return;
  }
  i = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, AWYfp);
    i++;
    if(numRead != 1)
      buffer[i-1] = EOL;
  } while(buffer[i-1] != EOL && i < _bufferSize);
  if(buffer[i-1] == EOL)
    buffer[i-1] = NUL;
  else {
    cerr << "ERROR: reading size of AWY file" << endl;
    return;
  }
  _AWYsize = atoi(buffer);

  _AWYname = new char *[_AWYsize]; 
  for(a = 0; a < _AWYsize; a++) {
    _AWYname[a] = new char[_internalStringLenSmall];
  }
  _AWYnameArrayVec = new vector<char *> *[_AWYsize];
  _AWYlatArrayVec = new vector<float> *[_AWYsize];
  _AWYlonArrayVec = new vector<float> *[_AWYsize];

  i = 0;
  j = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, AWYfp);
    if (numRead == 1) {
      if (buffer[i] == EOL) {
	buffer[i]=NUL;
	i=0;
	if(1 != sscanf(buffer, "%s ", _AWYname[j])) {
	  cerr << "WARNING: Bad line in AWY, " << buffer << endl;
	  return;
	} else {
	  offset = strlen(_AWYname[j])+1;
	  nameVec = new vector<char *>;
	  latVec = new vector<float>;
	  lonVec = new vector<float>;
	  do {
	    name = new char[_internalStringLenSmall];
	    sscan = sscanf(buffer+offset, "%s %s %s", name, latc, lonc);
	    if(sscan == 3){
	      nameVec->push_back(name);
	      latVec->push_back(atof(latc));
	      lonVec->push_back(atof(lonc));
	      offset += strlen(name)+strlen(latc)+strlen(lonc)+3;
	    } else {
	      delete [] name;
	    }
	  } while(sscan == 3 && offset < strlen(buffer));
	  _AWYnameArrayVec[j] = nameVec;
	  _AWYlatArrayVec[j] = latVec;
	  _AWYlonArrayVec[j] = lonVec;
	  j++;
	}
      } else {
	if ((int)buffer[i] > 0 && i < _bufferSize-1)
	  i++;
      }
    }
  } while (numRead == 1 && j < _AWYsize);
  fclose(AWYfp);
  _AWYsize = j;
  if (_params->debug)
    cout <<"Read "<<_AWYsize<<" lines from "<<AWYpath<<endl;

// *************************
// Read in SSD file
// *************************
  if (_params->debug)
    cout << "Opening " << SSDpath << endl;
  FILE *SSDfp = fopen(SSDpath,"rb");
  if(SSDfp == NULL) {
    cerr << "ERROR: Cannot open file " << SSDpath << endl;
    return;
  }
  i = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, SSDfp);
    i++;
    if(numRead != 1)
      buffer[i-1] = EOL;
  } while(buffer[i-1] != EOL && i < _bufferSize);
  if(buffer[i-1] == EOL)
    buffer[i-1] = NUL;
  else {
    cerr << "ERROR: reading size of SSD file" << endl;
    return;
  }
  _SSDsize = atoi(buffer);

  _SSDname = new char *[_SSDsize]; 
  for(a = 0; a < _SSDsize; a++) {
    _SSDname[a] = new char[_internalStringLenSmall]; 
  }
  _SSDnameArrayVec = new vector<char *> *[_SSDsize];
  _SSDlatArrayVec  = new vector<float> *[_SSDsize];
  _SSDlonArrayVec  = new vector<float> *[_SSDsize]; 

  i = 0;
  j = 0;
  do {
    numRead = fread(&buffer[i], sizeof(unsigned char), 1, SSDfp);
    if (numRead == 1) {
      if (buffer[i] == EOL) {
	buffer[i]=NUL;
	i=0;
	if(1 != sscanf(buffer, "%s ", _SSDname[j])) {
	  cerr << "WARNING: Bad line in SSD, " << buffer << endl;
	  return;
	} else {
	  offset = strlen(_SSDname[j])+1;
	  nameVec = new vector<char *>;
	  latVec = new vector<float>;
	  lonVec = new vector<float>;
	  do {
	    name = new char[_internalStringLenSmall];
	    sscan = sscanf(buffer+offset, "%s %s %s", name, latc, lonc);
	    if(sscan == 3){
	      nameVec->push_back(name);
	      latVec->push_back(atof(latc));
	      lonVec->push_back(atof(lonc));
	      offset += strlen(name)+strlen(latc)+strlen(lonc)+3;
	    } else {
	      delete [] name;
	    }
	  } while(sscan == 3 && offset < strlen(buffer));
	  _SSDnameArrayVec[j] = nameVec;
	  _SSDlatArrayVec[j] = latVec;
	  _SSDlonArrayVec[j] = lonVec;
	  j++;
	}
      } else {
	if ((int)buffer[i] > 0 && i < _bufferSize-1)
	  i++;
      }
    }
  } while (numRead == 1 && j < _SSDsize);
  fclose(SSDfp);
  _SSDsize = j;
  if (_params->debug)
    cout <<"Read "<<_SSDsize<<" lines from "<<SSDpath<<endl;
  _isOK = true;
}

routeDecode::~routeDecode()
{
  if(!_isOK)
    return;
  _clearRoute();

  delete [] _destName;
  delete [] _deptName;

  int a;
  vector<char *>::iterator nameIter;
  for(a = 0; a < _APTsize; a++) {
    delete [] _APTname[a];
  }
  delete [] _APTname;
  delete [] _APTlat;
  delete [] _APTlon;

  for(a = 0; a < _ARPsize; a++) {
    delete [] _ARPname[a];
  }
  delete [] _ARPname;
  delete [] _ARPlat;
  delete [] _ARPlon;

  for(a = 0; a < _NAVsize; a++) {
    delete [] _NAVname[a];
  }
  delete [] _NAVname;
  delete [] _NAVlat;
  delete [] _NAVlon;

  for(a = 0; a < _FIXsize; a++) {
    delete [] _FIXname[a];
  }
  delete [] _FIXname;
  delete [] _FIXlat;
  delete [] _FIXlon;

  for(a = 0; a < _AWYsize; a++) {
    delete [] _AWYname[a];
  }
  delete [] _AWYname;
  for(a = 0; a < _AWYsize; a++) {
    nameIter = _AWYnameArrayVec[a]->begin();
    while(nameIter != _AWYnameArrayVec[a]->end()) {
      delete [] *(nameIter);
      nameIter++;
    }
    delete _AWYnameArrayVec[a];
    delete _AWYlatArrayVec[a];
    delete _AWYlonArrayVec[a];
  }
  delete [] _AWYnameArrayVec;
  delete [] _AWYlatArrayVec;
  delete [] _AWYlonArrayVec;

  for(a = 0; a < _SSDsize; a++) {
    delete [] _SSDname[a];
  }
  delete [] _SSDname;
  for(a = 0; a < _SSDsize; a++) {
    nameIter = _SSDnameArrayVec[a]->begin();
    while(nameIter != _SSDnameArrayVec[a]->end()) {
      delete [] *(nameIter);
      nameIter++;
    }
    delete _SSDnameArrayVec[a];
    delete _SSDlatArrayVec[a];
    delete _SSDlonArrayVec[a];
  }
  delete [] _SSDnameArrayVec;
  delete [] _SSDlatArrayVec;
  delete [] _SSDlonArrayVec;

}

bool routeDecode::isOK()
{
  return _isOK;
}

void routeDecode::_clearRoute()
{
  for(int a = 0; a < _routeNameVec.size(); a++) {
    delete [] _routeNameVec[a];
  }
  _routeNameVec.clear();
  _routeLatVec.clear();
  _routeLonVec.clear();
  _estArrival = 0;
}

ac_route_posn_t *routeDecode::getRouteArray(int *arraySize, int *estArrival, 
					    char *destStr, char *deptStr,
					    int stringSize)
{
  *estArrival = _estArrival;
  strncpy(destStr, _destName, stringSize);
  strncpy(deptStr, _deptName, stringSize);

  return getRouteArray(arraySize);
}

ac_route_posn_t *routeDecode::getRouteArray(int *arraySize)
{
  if(_isOK) {
    *arraySize = _routeNameVec.size();
    if(*arraySize == 0)
      return NULL;
    ac_route_posn_t *posn = new ac_route_posn_t[*arraySize];
    vector<char *>::iterator nameIter = _routeNameVec.begin();
    vector<float>::iterator latIter = _routeLatVec.begin();
    vector<float>::iterator lonIter = _routeLonVec.begin();
    int i = 0;

    while( nameIter != _routeNameVec.end() ) {
      strncpy(posn[i].name, *(nameIter), AC_ROUTE_N_POSNAME);
      posn[i].lat = *(latIter);
      posn[i].lon = *(lonIter);
      i++;
      nameIter++;
      latIter++;
      lonIter++;
    }
    return posn;
  } else {
    return NULL;
  }
}

int routeDecode::getEstArrival() {
  return _estArrival;
}


void routeDecode::print_route()
{
    cout << "Route = ";
    vector<char *>::iterator iter1 = _routeNameVec.begin();
    while( iter1 != _routeNameVec.end() ) {
      cout << *iter1 << "'";
      iter1++;
    }
    cout <<endl;
    
    cout << "RouteLats = ";
    vector<float>::iterator iter2 = _routeLatVec.begin();
    while( iter2 != _routeLatVec.end() ) {
      cout << *iter2 << "'";
      iter2++;
    }
    cout <<endl;
    
    cout << "RouteLons = ";
    iter2 = _routeLonVec.begin();
    while( iter2 != _routeLonVec.end() ) {
      cout << *iter2 << "'";
      iter2++;
    }
    cout <<endl;
}

int routeDecode::decoder(char *message)
{

  int A_day, A_hour, A_min, A_sec;

  if (4 != sscanf(message + strlen("513D"),
		  "%2d%2d%2d%2d",
		  &A_day, &A_hour, &A_min, &A_sec)){
    return 1;
  }

  if (strncmp(message + strlen("513D20180453KZKC"), "FZ", 2)){
    return 1;
  }

  char fltID[_internalStringLenSmall];
  unsigned offset = strlen("E00720182123KNCTTZ ");
  if (offset >= strlen(message)) return 1;
  char *p = message + offset;
  _extractString(p, fltID);

  char typeStr[_internalStringLenSmall];
   offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 1;
  if (offset >= strlen(message)) return 1;
  p = message + offset;
  _extractString(p, typeStr);

  char speedStr[_internalStringLenSmall];
   offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(typeStr) + 2;
  if (offset >= strlen(message)) return 1;
  p = message + offset;
  _extractString(p, speedStr);

  char boundryStr[_internalStringLenSmall];
   offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(typeStr) + strlen(speedStr) + 3;
  if (offset >= strlen(message)) return 1;
  p = message + offset;
  _extractString(p, boundryStr);

  char boundryTimeStr[_internalStringLenSmall];
   offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(typeStr) + strlen(speedStr) +strlen(boundryStr) + 4;
  if (offset >= strlen(message)) return 1;
  p = message + offset;
  _extractString(p, boundryTimeStr);

  char altStr[_internalStringLenSmall];
   offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(typeStr)+strlen(speedStr)+strlen(boundryStr)+strlen(boundryTimeStr) + 5;
  if (offset >= strlen(message)) return 1;
  p = message + offset;
  _extractString(p, altStr);

  char routeStr[_internalStringLenSmall];
   offset = strlen("E00720182123KNCTTZ ") + strlen(fltID) + 
    strlen(typeStr)+strlen(speedStr)+strlen(boundryStr)+strlen(boundryTimeStr) +
    strlen(altStr) + 6;
  if (offset >= strlen(message)) return 1;
  p = message + offset;
  _extractString(p, routeStr);

  if (_params->debug)
    cout<<fltID<<"'"<<typeStr<<"'"<<speedStr<<"'"<<boundryStr<<"'"<<boundryTimeStr<<
      "'"<<altStr<<"'"<<routeStr<<endl;

  return decodeRoute(routeStr);

}

int routeDecode::decodeRoute(char *route)
{
  _clearRoute();

  //
  // Remove trailing space, check for spaces
  if(route[strlen(route)-1] == ' ')
    route[strlen(route)-1] = NUL;
  if(strchr(route, ' ') != NULL) {
    cerr << "WARNING: Route cannot have spaces in it, ignoring."<<endl;
    return -1;
  }

  // Look for trailing Estimated Arrival, save it
  // "/MMSS"
  if(route[strlen(route)-5] == '/') {
    char *arrival = route + strlen(route)-4;
    _estArrival = atoi(arrival);
    route[strlen(route)-5] = char(0);
  } else
    _estArrival = AC_ROUTE_MISSING_INT;

  char *dotSlash = strstr(route,"./.");
  vector<char *>::iterator iter1;

  //
  // Split the route at every ".."
  // Also check for a single "./." and split
  if (dotSlash == NULL) {
    inputRouteVec = _splitString(route, "..", _internalStringLenLarge);
  } else {
    char *departureStr = new char[_internalStringLenSmall];
    _extractString(route, dotSlash, departureStr);
    inputRouteVec = _splitString(dotSlash+3, "..", _internalStringLenLarge);
    iter1 = inputRouteVec.begin();
    inputRouteVec.insert(iter1,1,departureStr);
  }

  //
  // Decode each peice individually
  // The first peice should be the departure  
  // The last peice should be the destination 
  // If we only have one peice it is both     
  // Anything else is a regular peice         
  char *routePeice;
  for(inputRoutePos = 0; inputRoutePos < inputRouteVec.size(); inputRoutePos++) {
    routePeice = inputRouteVec[inputRoutePos];
    if(inputRoutePos == 0 && inputRoutePos == inputRouteVec.size()-1)
      _decodePeice(routePeice, ONLY_PEICE);
    else if(inputRoutePos == 0)
      _decodePeice(routePeice, FIRST_PEICE);
    else if(inputRoutePos == inputRouteVec.size()-1) 
      _decodePeice(routePeice, LAST_PEICE);
    else 
      _decodePeice(routePeice, REGULAR_PEICE);
  }
  for(int i = 0; i < inputRouteVec.size(); i++) {
    delete [] inputRouteVec[i];
  }
  inputRouteVec.clear();
  inputRoutePos = -1;

  //
  // This loop removes duplicate waypoints (based on name)
  // Also finds matching destination Airports  ex: ORD and KORD
  if(_routeNameVec.size() != 0) {
    iter1 = _routeNameVec.begin()+1;
    vector<float>::iterator iter2 = _routeLatVec.begin()+1;
    vector<float>::iterator iter3 = _routeLonVec.begin()+1;
    while( iter1 != _routeNameVec.end() ) {
      if(strcmp(*(iter1), *(iter1-1)) == 0 || 
	 (iter1 == _routeNameVec.end()-1 &&
	  (*(iter1))[0] == 'K' && 
	  strcmp((*(iter1))+1, *(iter1-1)) == 0 )) {
	delete [] *(iter1-1);
	iter1 = _routeNameVec.erase(iter1-1);
	iter2 = _routeLatVec.erase(iter2-1);
	iter3 = _routeLonVec.erase(iter3-1);
      } 
      iter1++;
      iter2++;
      iter3++;
    }
  }

  return 0;
}

/* _decodePeice, function that decodes a subset of the route
 * subsets are seperated by "./." or ".."
 * Inputs:
 *    peice: The subset of the route to decode
 *    type : The Type of subset, used to find departure and
 *           destination airports, which should always be present.
 *
 *    Types: First peice, must contain departure airport
 *           Regular peice
 *           Last peice, must contain destination airport
 *           First, Last and only peice, must contain 
 *             departure and destination airport
 * Output:
 *     Returns 0 on success (not implemented yet)
 */
int routeDecode::_decodePeice(char *peice, peice_type_t type)
{

  peiceVec = _splitString(peice, ".", _internalStringLenSmall);
  for(peiceVecPos = 0; peiceVecPos < peiceVec.size(); peiceVecPos++) {
    char *part = peiceVec[peiceVecPos];
    if((type == FIRST_PEICE || type == ONLY_PEICE) && peiceVecPos == 0) {
      strncpy(_deptName, part, _internalStringLenSmall);
      if(_isAPT(part)) {
	if(!_findAPT(part))
	  cerr << "Cant find Apt " << part << endl;
      } else if(_isARP(part)) {
	if(!_findARP(part))
	  cerr << "Cant find Arp " << part << endl;
      } else {
	cerr << "Unknown Departure Type: " << part << endl;
      }
    } else if((type == LAST_PEICE || type == ONLY_PEICE) && peiceVecPos == peiceVec.size()-1) {
      strncpy(_destName, part, _internalStringLenSmall);
      if(_isAPT(part)) {
	if(!_findAPT(part))
	  cerr << "Cant find Apt " << part << endl;
      } else if(_isAPT2(part)) {
	char apt[4] = {part[0],part[1],part[2],NUL};
	char arrive[7] = {part[4],part[5],part[6],part[7],NUL};
	_estArrival = atoi(arrive);
	if(!_findAPT(apt))
	  cerr << "Cant find Apt " << apt << endl;
      } else if(_isARP(part)) {
	if(!_findARP(part))
	  cerr << "Cant find Arp " << part << endl;
      } else if(_isARP2(part)) {
	char apt[5] = {part[0],part[1],part[2],part[3],NUL};
	char arrive[7] = {part[5],part[6],part[7],part[8],NUL};
	_estArrival = atoi(arrive);
	if(!_findARP(apt))
	  cerr << "Cant find Arp " << apt << endl;
      } else {
	cerr << "Unknown Destination Type: " << part << endl;
      }
    } else { 
      if(_isLatLon(part)) {
	
      } else if(_isLatLon2(part)) {

      } else if(_isRadialFix(part)) {
	char fix[4] = {part[0],part[1],part[2],NUL};
	char radial[7] = {part[3],part[4],part[5],part[6],part[7],part[8],NUL};
	if(_findNAV(fix) || _findAPT(fix)) {
	  if(!_findRadial(radial))
	    cerr << "Cant find Radial "<<fix<<" "<<radial<<endl;
	} else 
	  cerr << "Cant find Nav " << fix << endl;
      } else if(_isRadialIntersection(part)) {
	char fix[6] = {part[0],part[1],part[2],part[3],part[4],NUL};
	char radial[7] = {part[5],part[6],part[7],part[8],part[9],part[10],NUL};
	if(_findFIX(fix)) {
	  if(!_findRadial(radial))
	    cerr << "Cant find Radial "<<fix<<" "<<radial<<endl;
	} else 
	  cerr << "Cant find Fix " << fix << endl;
      } else if(_isNAV(part)) {
	if(!_findNAV(part))
	  cerr << "Cant find Nav " << part << endl;
      } else if(_isFIX(part)) {
	if(!_findFIX(part))
	  cerr << "Cant find Fix " << part << endl;
      } else if(_isAWY(part)) {
	if(peiceVecPos == 0 || peiceVecPos == peiceVec.size()-1) {
	  cerr << "AWY not prefixed and followed by fix, " << part << endl;
	} else {
	  char *partPre = peiceVec[peiceVecPos-1];
	  char *partNex = peiceVec[peiceVecPos+1];
	  if(!_findAWY(part, partPre, partNex))
	    cerr << "Cant find Awy " << part << endl;
	}
      } else if(_isSSD(part)) {
	if(type == FIRST_PEICE) { /* SID Departure route */
	  if(peiceVecPos == peiceVec.size()-1) 
	    cerr << "SSD not followed by fix " << part << endl;
	  else {
	    char partFix[strlen(part)+strlen(peiceVec[peiceVecPos+1])+2];
	    strcpy(partFix,part);
	    strcat(partFix,".");
	    strcat(partFix,peiceVec[peiceVecPos+1]);
	    if(!_findSSD(partFix, peiceVec[peiceVecPos+1]))
	      cerr << "Cant find Ssd " << partFix << endl;
	  }
	} else if(type == LAST_PEICE) { /* STAR Arrival route */
	  if(peiceVecPos == 0)
	    cerr << "SSD not preceded by fix " << part << endl;
	  else {
	    char partFix[strlen(part)+strlen(peiceVec[peiceVecPos-1])+2];
	    strcpy(partFix,peiceVec[peiceVecPos-1]);
	    strcat(partFix,".");
	    strcat(partFix,part);
	    if(peiceVecPos == peiceVec.size()-1)
	      cerr << "SSD not followed by exit fix " << part << endl;
	    else
	      if(!_findSSD(partFix, peiceVec[peiceVecPos+1])) {
		if(peiceVecPos >= 3) {
		  strcpy(partFix,peiceVec[peiceVecPos-3]);
		  strcat(partFix,".");
		  strcat(partFix,part);
		  if(!_findSSD(peiceVec[peiceVecPos-1], partFix, peiceVec[peiceVecPos+1])) {
		    cerr << "Cant find Ssd " << partFix << " or " <<
		      peiceVec[peiceVecPos-1] << "." << part << endl;
		  }
		} else 
		  cerr << "Cant find Ssd " << partFix << endl;
	      }
	  }
	} else if(type == ONLY_PEICE) {
	  if(peiceVec.size() == 3) {
	    /* We shouldn't ever have a route that is only three connected parts 
	    * it would indicate the SID departure is also a STAR arrival.
	    * Trys both the SID and STAR anyways. */
	    cerr << "SID departure can't also be a STAR arrival." << endl;
	    char partFix[strlen(part)+strlen(peiceVec[peiceVecPos-1])+2];
	    strcpy(partFix,peiceVec[peiceVecPos-1]);
	    strcat(partFix,".");
	    strcat(partFix,part);
	    if(!_findSSD(partFix))
	      cerr << "Cant find Ssd " << partFix << endl;
	    char partFix2[strlen(part)+strlen(peiceVec[peiceVecPos+1])+2];
	    strcpy(partFix2,part);
	    strcat(partFix2,".");
	    strcat(partFix2,peiceVec[peiceVecPos+1]);
	    if(!_findSSD(partFix2))
	      cerr << "Cant find Ssd " << partFix2 << endl;
	  } else if(peiceVecPos == 1) { /* SID Departure route */
	    char partFix[strlen(part)+strlen(peiceVec[peiceVecPos+1])+2];
	    strcpy(partFix,part);
	    strcat(partFix,".");
	    strcat(partFix,peiceVec[peiceVecPos+1]);
	    if(!_findSSD(partFix, peiceVec[peiceVecPos+1]))
	      cerr << "Cant find Ssd " << partFix << endl;
	  } else if(peiceVecPos == peiceVec.size()-2) { /* STAR Arrival route */
	    char partFix[strlen(part)+strlen(peiceVec[peiceVecPos-1])+2];
	    strcpy(partFix,peiceVec[peiceVecPos-1]);
	    strcat(partFix,".");
	    strcat(partFix,part);
	    if(!_findSSD(peiceVec[peiceVecPos-1], partFix, peiceVec[peiceVecPos+1]))
	      cerr << "Cant find Ssd " << partFix << endl;
	  } else {
	    cerr << "SSD type 3 at peiceVecPos = " << peiceVecPos << endl;
	  }
	} else {
	  /* Special Case
	   * Poorly defined STAR Arrival route */
	  if(inputRoutePos == inputRouteVec.size()-2) { 
	    char partFix[strlen(part)+strlen(inputRouteVec[inputRoutePos-1])+2];
	    strcpy(partFix,inputRouteVec[inputRoutePos-1]);
	    strcat(partFix,".");
	    strcat(partFix,part);
	    if(!_findSSD(inputRouteVec[inputRoutePos-1], partFix, inputRouteVec[inputRoutePos+1]))
	      cerr << "Cant find Ssd " << partFix << endl;
	  } else
	    cerr << "An SSD with regular type is undefined " << part << endl;
	}
      } else {
	cerr << "Unknown Type: " << part << endl;
      }
    }
  }
  for(int i = 0; i < peiceVec.size(); i++) {
    delete [] peiceVec[i];
  }
  peiceVec.clear();
  peiceVecPos = -1;
}

/* Function to check if the char array could be a APT fix
 * returns true if part = "LLL"
 * where L is any upercase letter
 */
bool routeDecode::_isAPT(char *part)
{
  if(strlen(part) == 3 && _isLetter(part[0]) && 
     _isLetter(part[1]) && _isLetter(part[2]))
    return true;
  return false;
} 
/* Function to check if the char array could be a APT fix
 * returns true if part = "LLL/nnnn"
 * where L is any upercase letter, n is any digit
 */
bool routeDecode::_isAPT2(char *part)
{
  if(strlen(part) == 8 && _isLetter(part[0]) && _isLetter(part[1]) && 
     _isLetter(part[2]) && part[3] == '/' && _isDigit(part[4]) && 
     _isDigit(part[5]) && _isDigit(part[6]) && _isDigit(part[7]))
    return true;
  return false;
} 
/* Function to check if the char array could be a ARP fix
 * returns true if part = "LLLL"
 * where L is any upercase letter
 */
bool routeDecode::_isARP(char *part)
{
  if(strlen(part) == 4 && _isLetter(part[0]) && _isLetter(part[1]) && 
     _isLetter(part[2]) && _isLetter(part[3]))
    return true;
  return false;
} 
/* Function to check if the char array could be a ARP fix
 * returns true if part = "LLLL/nnnn"
 * where L is any upercase letter, n is any digit
 */
bool routeDecode::_isARP2(char *part)
{
  if(strlen(part) == 9 && _isLetter(part[0]) && _isLetter(part[1]) && 
     _isLetter(part[2]) && _isLetter(part[3]) && part[4] == '/' &&
     _isDigit(part[5]) && _isDigit(part[6]) && _isDigit(part[7]) &&
     _isDigit(part[8]))
    return true;
  return false;
} 
/* Function to check if the char array could be a lat lon fix
 * if true parses the fix and pushes in onto the route
 * returns true if part = "nnnn/nnnnn" or "nnnn/nnnn"
 * where n is any digit
 */
bool routeDecode::_isLatLon(char *part)
{
  if(strlen(part) == 10 && _isDigit(part[0]) && _isDigit(part[1]) && _isDigit(part[2]) && 
      _isDigit(part[3]) && part[4] == '/' && _isDigit(part[5]) && _isDigit(part[6]) && 
      _isDigit(part[7]) && _isDigit(part[8]) && _isDigit(part[9]))
  {
    char *name = new char[_internalStringLenSmall];
    strcpy(name, part);
    char deglatStr[3] = {part[0],part[1],NUL};
    float deglat = atof(deglatStr);
    char minlatStr[3] = {part[2],part[3],NUL};
    float minlat = atof(minlatStr);
    char deglonStr[4] = {part[5],part[6],part[7],NUL};
    float deglon = atof(deglonStr);
    char minlonStr[3] = {part[8],part[9],NUL};
    float minlon = atof(minlonStr);
    float lat = deglat + (minlat / 60.0);
    float lon = (deglon + (minlon / 60.0)) * -1;  // assumed negative lon
    _routeNameVec.push_back(name);
    _routeLatVec.push_back(lat);
    _routeLonVec.push_back(lon);
    return true;
  }
  if(strlen(part) == 9 && _isDigit(part[0]) && _isDigit(part[1]) && _isDigit(part[2]) && 
     _isDigit(part[3]) && part[4] == '/' && _isDigit(part[5]) && _isDigit(part[6]) && 
     _isDigit(part[7]) && _isDigit(part[8]))
  {
    char *name = new char[_internalStringLenSmall];
    strcpy(name, part);
    char deglatStr[3] = {part[0],part[1],NUL};
    float deglat = atof(deglatStr);
    char minlatStr[3] = {part[2],part[3],NUL};
    float minlat = atof(minlatStr);
    char deglonStr[4] = {part[5],part[6],NUL};
    float deglon = atof(deglonStr);
    char minlonStr[3] = {part[7],part[8],NUL};
    float minlon = atof(minlonStr);
    float lat = deglat + (minlat / 60.0);
    float lon = (deglon + (minlon / 60.0)) * -1;  // assumed negative lon
    _routeNameVec.push_back(name);
    _routeLatVec.push_back(lat);
    _routeLonVec.push_back(lon);
    return true;
  }
  return false;
}
/* Function to check if the char array could be a lat lon fix
 * if true parses the fix and pushes in onto the route
 * returns true if part = "nnnnN/nnnnnE" or "nnnnN/nnnnE"
 * where N is letter N or S, E is letter E or W, n is any digit
 */
bool routeDecode::_isLatLon2(char *part)
{
  if(strlen(part) == 12 && _isDigit(part[0]) && _isDigit(part[1]) && _isDigit(part[2]) && 
     _isDigit(part[3]) && (part[4] == 'N' || part[4] == 'S') && part[5] == '/' &&
     _isDigit(part[6]) && _isDigit(part[7]) && _isDigit(part[8]) && _isDigit(part[9]) && 
     _isDigit(part[10]) && (part[11] == 'E' || part[11] == 'W'))
  {
    char *name = new char[_internalStringLenSmall];
    strcpy(name, part);
    char deglatStr[3] = {part[0],part[1],NUL};
    float deglat = atof(deglatStr);
    char minlatStr[3] = {part[2],part[3],NUL};
    float minlat = atof(minlatStr);
    char deglonStr[4] = {part[6],part[7],part[8],NUL};
    float deglon = atof(deglonStr);
    char minlonStr[3] = {part[9],part[10],NUL};
    float minlon = atof(minlonStr);
    float lat = deglat + (minlat / 60.0);
    if(part[4] == 'S') lat = lat * -1;
    float lon = deglon + (minlon / 60.0);
    if(part[11] == 'W') lon = lon * -1;

    _routeNameVec.push_back(name);
    _routeLatVec.push_back(lat);
    _routeLonVec.push_back(lon);

    return true;
  }
  if(strlen(part) == 11 && _isDigit(part[0]) && _isDigit(part[1]) && _isDigit(part[2]) && 
     _isDigit(part[3]) && (part[4] == 'N' || part[4] == 'S') && part[5] == '/' &&
     _isDigit(part[6]) && _isDigit(part[7]) && _isDigit(part[8]) && _isDigit(part[9]) && 
     (part[10] == 'E' || part[10] == 'W'))
  {
    char *name = new char[_internalStringLenSmall];
    strcpy(name, part);
    char deglatStr[3] = {part[0],part[1],NUL};
    float deglat = atof(deglatStr);
    char minlatStr[3] = {part[2],part[3],NUL};
    float minlat = atof(minlatStr);
    char deglonStr[4] = {part[6],part[7],NUL};
    float deglon = atof(deglonStr);
    char minlonStr[3] = {part[8],part[9],NUL};
    float minlon = atof(minlonStr);
    float lat = deglat + (minlat / 60.0);
    if(part[4] == 'S') lat = lat * -1;
    float lon = deglon + (minlon / 60.0);
    if(part[10] == 'W') lon = lon * -1;

    _routeNameVec.push_back(name);
    _routeLatVec.push_back(lat);
    _routeLonVec.push_back(lon);

    return true;
  }
  return false;
}
/* Function to check if the char array could be a radial fix
 * returns true if part = "LLLnnnnnn"
 * where L is any upercase letter, n is any digit
 */
bool routeDecode::_isRadialFix(char *part)
{
  if(strlen(part) == 9 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]) && 
     _isDigit(part[3]) && _isDigit(part[4]) && _isDigit(part[5])&& _isDigit(part[6]) && 
     _isDigit(part[7]) && _isDigit(part[8]))
    return true;
  return false;
}
/* Function to check if the char array could be a radial intersection fix
 * returns true if part = "LLLLLnnnnnn"
 * where L is any upercase letter, n is any digit
 */
bool routeDecode::_isRadialIntersection(char *part)
{
  if(strlen(part) == 11 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]) && 
     _isLetter(part[3]) && _isLetter(part[4]) && _isDigit(part[5]) && _isDigit(part[6]) && 
     _isDigit(part[7])&& _isDigit(part[8]) && _isDigit(part[9]) && _isDigit(part[10]))
    return true;
  return false;
}
/* Function to check if the char array could be a NavAid fix
 * returns true if part = "LLL"
 * where L is any upercase letter
 */
bool routeDecode::_isNAV(char *part)
{
  if(strlen(part) == 3 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]))
    return true;
  return false;
}
/* Function to check if the char array could be a intersection fix
 * returns true if part = "LLLLL"
 * where L is any upercase letter
 */
bool routeDecode::_isFIX(char *part)
{
  if(strlen(part) == 5 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]) &&
     _isLetter(part[3]) && _isLetter(part[4]))
    return true;
  return false;
}
/* Function to check if the char array could be a Jet or Victor Route
 * returns true if part = "Ln" or "Lnn" or "Lnnn"
 * where L is any upercase letter, n is any digit
 */
bool routeDecode::_isAWY(char *part)
{
  if(strlen(part) == 2 && _isLetter(part[0]) && _isDigit(part[1]))
    return true;
  if(strlen(part) == 3 && _isLetter(part[0]) && _isDigit(part[1]) && _isDigit(part[2]))
    return true;
  if(strlen(part) == 4 && _isLetter(part[0]) && _isDigit(part[1]) && _isDigit(part[2]) && _isDigit(part[3]))
    return true;
  return false;
}
/* Function to check if the char array could be a Sid or STAR Route
 * returns true if part = "LLLn" or "LLLLn" or "LLLLLn"
 * where L is any upercase letter, n is any digit
 */
bool routeDecode::_isSSD(char *part)
{
  if(strlen(part) == 4 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]) && _isDigit(part[3]))
    return true;
  if(strlen(part) == 5 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]) && 
     _isLetter(part[3]) && _isDigit(part[4]))
    return true;
  if(strlen(part) == 6 && _isLetter(part[0]) && _isLetter(part[1]) && _isLetter(part[2]) && 
     _isLetter(part[3]) && _isLetter(part[4]) && _isDigit(part[5]))
    return true;
  return false;
}

/* Function to find a given APT in the FAA database
 * returns true if found and pushes it on the route
 */
bool routeDecode::_findAPT(char *part)
{
  for(int i = 0; i < _APTsize; i++) {
    if(strcmp(_APTname[i], part) == 0) {
      char *thisstring = new char[strlen(part)+1];
      strcpy(thisstring, part);
      _routeNameVec.push_back(thisstring);
      _routeLatVec.push_back(_APTlat[i]);
      _routeLonVec.push_back(_APTlon[i]);
      return true;
    }
  }
  return false;
}

/* External function to find a given APT in the FAA database
 * returns true if found.
 */
bool routeDecode::lookupAirport(char *APT, float *lat, float *lon)
{
  if(_isARP(APT)) {
    for(int i = 0; i < _ARPsize; i++) {
      if(strcmp(_ARPname[i], APT) == 0) {
	*lat = _ARPlat[i];
	*lon = _ARPlon[i];
	return true;
      }
    }
  } 
  if(_isAPT(APT)) {
    for(int i = 0; i < _APTsize; i++) {
      if(strcmp(_APTname[i], APT) == 0) {
	*lat = _APTlat[i];
	*lon = _APTlon[i];
	return true;
      }
    }
  }
  return false;
}

/* Function to find a given ARP in the FAA database
 * returns true if found and pushes it on the route
 */
bool routeDecode::_findARP(char *part)
{
  for(int i = 0; i < _ARPsize; i++) {
    if(strcmp(_ARPname[i], part) == 0) {
      char *thisstring = new char[strlen(part)+1];
      strcpy(thisstring, part);
      _routeNameVec.push_back(thisstring);
      _routeLatVec.push_back(_ARPlat[i]);
      _routeLonVec.push_back(_ARPlon[i]);
      return true;
    }
  }
  return false;
}
/* Function to find a given NAV in the FAA database
 * returns true if found and pushes it on the route
 */
bool routeDecode::_findNAV(char *part)
{
  for(int i = 0; i < _NAVsize; i++) {
    if(strcmp(_NAVname[i], part) == 0) {
      char *thisstring = new char[strlen(part)+1];
      strcpy(thisstring, part);
      _routeNameVec.push_back(thisstring);
      _routeLatVec.push_back(_NAVlat[i]);
      _routeLonVec.push_back(_NAVlon[i]);
      return true;
    }
  }
  return false;
}
/* Function to find a given FIX in the FAA database
 * returns true if found and pushes it on the route
 */
bool routeDecode::_findFIX(char *part)
{
  for(int i = 0; i < _FIXsize; i++) {
    if(strcmp(_FIXname[i], part) == 0) {
      char *thisstring = new char[strlen(part)+1];
      strcpy(thisstring, part);
      _routeNameVec.push_back(thisstring);
      _routeLatVec.push_back(_FIXlat[i]);
      _routeLonVec.push_back(_FIXlon[i]);
      return true;
    }
  }
  return false;
}
/* Function to find a given AWY in the FAA database
 * An awy must be preceded by a fix and followed by a fix
 *  These define where to get on the awy and where to get off.
 *
 * returns true if found and pushes the defined route 
 *  onto the output route
 */
bool routeDecode::_findAWY(char *part, char *prevPart, char *nextPart)
{
  for(int i = 0; i < _AWYsize; i++) {
    if(strcmp(_AWYname[i], part) == 0) {
      vector<char *>::iterator nameIter = _AWYnameArrayVec[i]->begin();
      vector<float>::iterator latIter = _AWYlatArrayVec[i]->begin();
      vector<float>::iterator lonIter = _AWYlonArrayVec[i]->begin();
      vector<char *> backNameVec;
      vector<float> backLatVec;
      vector<float > backLonVec;
      char *thisstring;
      bool forwards = false, backwards = false, found = false;;
      while(nameIter != _AWYnameArrayVec[i]->end() && !found) {
	if(strcmp(*(nameIter), prevPart) == 0) {
	  if(backwards) {
	    thisstring = new char[strlen(*(nameIter))+1];
	    strcpy(thisstring, *(nameIter));
	    backNameVec.push_back(thisstring);
	    backLatVec.push_back(*(latIter));
	    backLonVec.push_back(*(lonIter));
	    found = true;
	  } else {
	    forwards = true;
	    thisstring = new char[strlen(*(nameIter))+1];
	    strcpy(thisstring, *(nameIter));
	    _routeNameVec.push_back(thisstring);
	    _routeLatVec.push_back(*(latIter));
	    _routeLonVec.push_back(*(lonIter));
	  }
	} else if(strcmp(*(nameIter), nextPart) == 0) {
	  if(forwards) {
	    thisstring = new char[strlen(*(nameIter))+1];
	    strcpy(thisstring, *(nameIter));
	    _routeNameVec.push_back(thisstring);
	    _routeLatVec.push_back(*(latIter));
	    _routeLonVec.push_back(*(lonIter));
	    found = true;
	  } else {
	    backwards = true;
	    thisstring = new char[strlen(*(nameIter))+1];
	    strcpy(thisstring, *(nameIter));
	    backNameVec.push_back(thisstring);
	    backLatVec.push_back(*(latIter));
	    backLonVec.push_back(*(lonIter));
	  }
	} else if(nameIter+1 == _AWYnameArrayVec[i]->end()) {
	  cerr<< "2nd Fix not found on awy, "<<part << endl;
	  return true;
	} else if(forwards) {
	  thisstring = new char[strlen(*(nameIter))+1];
	  strcpy(thisstring, *(nameIter));
	  _routeNameVec.push_back(thisstring);
	  _routeLatVec.push_back(*(latIter));
	  _routeLonVec.push_back(*(lonIter));
	} else if(backwards) {
	  thisstring = new char[strlen(*(nameIter))+1];
	  strcpy(thisstring, *(nameIter));
	  backNameVec.push_back(thisstring);
	  backLatVec.push_back(*(latIter));
	  backLonVec.push_back(*(lonIter));
	}
	nameIter++;
	latIter++;
	lonIter++;
      }
      if(backwards) {
	vector<char *>::reverse_iterator nameRiter = backNameVec.rbegin();
	vector<float>::reverse_iterator latRiter = backLatVec.rbegin();
	vector<float>::reverse_iterator lonRiter = backLonVec.rbegin();
	do {
	  _routeNameVec.push_back(*(nameRiter));
	  _routeLatVec.push_back(*(latRiter));
	  _routeLonVec.push_back(*(lonRiter));
	  nameRiter++;
	  latRiter++;
	  lonRiter++;
	} while(nameRiter != backNameVec.rend());
	backNameVec.clear();
	backLatVec.clear();
	backLonVec.clear();
      } else if(!forwards) {
	cerr<< "Fixes not found on awy, "<<part << endl;
	return true;
      }
      return true;
    }
  }
  return false;
}
/* Function to find a given SSD in the FAA database
 * An ssd is defined by a ssd name followed by a . followed 
 * by a exit fix for a SID route or a entrance fix for a STAR route.
 *
 * returns true if found and pushes the defined route 
 *  onto the output route
 */
bool routeDecode::_findSSD(char *part)
{
  for(int i = 0; i < _SSDsize; i++) {
    if(strcmp(_SSDname[i], part) == 0) {
      vector<char *>::iterator nameIter = _SSDnameArrayVec[i]->begin();
      vector<float>::iterator latIter = _SSDlatArrayVec[i]->begin();
      vector<float>::iterator lonIter = _SSDlonArrayVec[i]->begin();
      int j = 0;
      while(nameIter != _SSDnameArrayVec[i]->end()) {
	char *thisstring = new char[strlen(*(nameIter))+1];
	strcpy(thisstring, *(nameIter));
	_routeNameVec.push_back(thisstring);
	_routeLatVec.push_back(*(latIter));
	_routeLonVec.push_back(*(lonIter));
	nameIter++;
	latIter++;
	lonIter++;
	j++;
      }
      return true;
    }
  }
  return false;
}
/* Function to find a given SSD in the FAA database
 * An ssd is defined by a ssd name followed by a . followed 
 * by a exit fix for a SID route or a entrance fix for a STAR route.
 *
 * This function takes an exit fix for getting off the SSDroute.
 * returns true if found and pushes the defined route 
 *  onto the output route
 */
bool routeDecode::_findSSD(char *part, char *exit)
{
  for(int i = 0; i < _SSDsize; i++) {
    if(strcmp(_SSDname[i], part) == 0) {
      vector<char *>::iterator nameIter = _SSDnameArrayVec[i]->begin();
      vector<float>::iterator latIter = _SSDlatArrayVec[i]->begin();
      vector<float>::iterator lonIter = _SSDlonArrayVec[i]->begin();
      int j = 0;
      while(nameIter != _SSDnameArrayVec[i]->end() && strcmp(*(nameIter), exit) != 0) {
	char *thisstring = new char[strlen(*(nameIter))+1];
	strcpy(thisstring, *(nameIter));
	_routeNameVec.push_back(thisstring);
	_routeLatVec.push_back(*(latIter));
	_routeLonVec.push_back(*(lonIter));
	nameIter++;
	latIter++;
	lonIter++;
	j++;
      }
      return true;
    }
  }
  return false;
}
/* Function to find a given SSD in the FAA database
 * An ssd is defined by a ssd name followed by a . followed 
 * by a exit fix for a SID route or a entrance fix for a STAR route.
 *
 * This function takes an exit fix for getting off the SSDroute and
 *  takes a fix for getting on the SSDroute
 * returns true if found and pushes the defined route 
 *  onto the output route
 */
bool routeDecode::_findSSD(char *entrance, char *part, char *exit)
{

  for(int i = 0; i < _SSDsize; i++) {
    if(strcmp(_SSDname[i], part) == 0) {
      vector<char *>::iterator nameIter = _SSDnameArrayVec[i]->begin();
      vector<float>::iterator latIter = _SSDlatArrayVec[i]->begin();
      vector<float>::iterator lonIter = _SSDlonArrayVec[i]->begin();
      int j = 0;
      while(nameIter != _SSDnameArrayVec[i]->end() && strcmp(*(nameIter), entrance) != 0) {
	nameIter++;
	latIter++;
	lonIter++;
	j++;
      }
      if(nameIter == _SSDnameArrayVec[i]->end()) {
	/* Special Case where we can't find the entrance to SSD
	 * which got off then back on the same SSD.
	 * Move the extra waypoint to just after the initial transition 
	 * as a second transition point.
	 */
	if(peiceVecPos >= 2 && strcmp(peiceVec[peiceVecPos], peiceVec[peiceVecPos-2]) == 0) {
	  vector<char *>::iterator iter1 = _routeNameVec.end()-2;
	  vector<float>::iterator iter2 = _routeLatVec.end()-2;
	  vector<float>::iterator iter3 = _routeLonVec.end()-2;
	  while( iter1 != _routeNameVec.begin() ) {
	    if(strcmp(*(iter1), *(iter1-1)) == 0) {
	      _routeNameVec.insert(iter1+1, *(_routeNameVec.end()-1));
	      _routeLatVec.insert(iter2+1, *(_routeLatVec.end()-1));
	      _routeLonVec.insert(iter3+1, *(_routeLonVec.end()-1));
	      iter1 = _routeNameVec.erase(_routeNameVec.end()-1);
	      iter2 = _routeLatVec.erase(_routeLatVec.end()-1);
	      iter3 = _routeLonVec.erase(_routeLonVec.end()-1);
	      break;
	    } else {
	      iter1--;
	      iter2--;
	      iter3--;
	    }
	  }
	} else
	  return false;
      }
      while(nameIter != _SSDnameArrayVec[i]->end() && strcmp(*(nameIter), exit) != 0) {
	char *thisstring = new char[strlen(*(nameIter))+1];
	strcpy(thisstring, *(nameIter));
	_routeNameVec.push_back(thisstring);
	_routeLatVec.push_back(*(latIter));
	_routeLonVec.push_back(*(lonIter));
	nameIter++;
	latIter++;
	lonIter++;
	j++;
      }
      return true;
    }
  }
  return false;
}
/* Function to decode a radial
 * Assumes the radial is in reference to the last fix on
 * the route vector. 
 * Will update the last fix based on the radial
 */
bool routeDecode::_findRadial(char *radial)
{
  bool neg_lon = false, neg_lat = false;
  char directionS[4] = {radial[0],radial[1],radial[2],NUL};
  char distanceS[4] = {radial[3],radial[4],radial[5],NUL};

  char *name = *(_routeNameVec.end()-1);
  float lat = *(_routeLatVec.end()-1);
  float lon = *(_routeLonVec.end()-1);
  if(lon < 0.0) {
    lon = lon*-1;
    neg_lon = true;
  }
  if(lat < 0.0) {
    lat = lat*-1;
    neg_lat = true;
  }

  float latr  = (M_PI/180.0) * lat;
  float lonr  = (M_PI/180.0) * lon;
  float anglr = (M_PI/180.0) * atoi(directionS);
  float disr  = (M_PI/(180.0*60.0)) *atoi(distanceS);

  lat = asin(sin(latr)*cos(disr)+cos(latr)*sin(disr)*cos(anglr));
  if (cos(lat) == 0.0) {
    lon = lonr;
  } else {
    lon = _mod((lonr-asin(sin(anglr)*sin(disr)/cos(lat))+M_PI), (2*M_PI))-M_PI;
    //lon = ((int)(lonr-asin(sin(anglr)*sin(disr)/cos(lat))+M_PI) % (int)(2*M_PI)) -M_PI;
  }
  lat = (180.0/M_PI) * lat;
  lon = (180.0/M_PI) * lon;
  if(neg_lon)
    lon = lon*-1;
  if(neg_lat)
    lat = lat*-1;
  char *newname = new char[strlen(name)+7];
  strcpy(newname, name);
  strcat(newname, radial);
  delete [] name;

  *(_routeNameVec.end()-1) = newname;
  *(_routeLatVec.end()-1) = lat;
  *(_routeLonVec.end()-1) = lon;
  return true;
}

/* Function to split a char array at each location of the splitstring
 * returns a vector of char pointers to the split char arrays
 */
vector<char *> routeDecode::_splitString(char *instring, char *splitstring, int stringLen)
{
  vector<char *> stringVec;
  char *thisstring = new char[stringLen];
  char thissplit[stringLen];
  int instringLen = strlen(instring);
  int splitstringLen = strlen(splitstring);
  int k = 0, l = 0;

  for(int j = 0; j < instringLen; j++) {
    if(l == stringLen-1) {
      cerr << "ERROR: Split string max size reached" << endl;
      thisstring[l] = NUL;
      stringVec.push_back(thisstring);
      return stringVec;
    } else if(instring[j] == splitstring[k]) { // Found part of the splitstring
      thissplit[k] = instring[j];
      k++;
      if(k == splitstringLen) { // Found all of the split string
	thisstring[l] = NUL;
	stringVec.push_back(thisstring);
	thisstring = new char[stringLen];
	l = 0;
	k = 0;
      }
    } else { // Didnt find the next part of the split string
      if(k > 0) { // We had part of the split.. put that into the string
	while(k > 0) {
	  k--;
	  thisstring[l] = thissplit[k];
	  l++;
	  if(l == stringLen-1) {
	    cerr << "ERROR: Split string max size reached" << endl;
	    thisstring[l] = NUL;
	    stringVec.push_back(thisstring);
	    return stringVec;
	  }
	}
	thisstring[l] = instring[j];
	l++;
      } else {
	thisstring[l] = instring[j];
	l++;
      }
    }
  }
  if(l == 0) {
    delete [] thisstring;
  } else {
    thisstring[l] = NUL;
    stringVec.push_back(thisstring);
  }
  return stringVec;
}
/* Function to extract up to a " " character.
 * returns the string in outstring.
 */
void routeDecode::_extractString(char *instring, char *outstring)
{
  int j=0;
  do {
    if (instring[j] == ' ')
      outstring[j] = NUL;
    else
      outstring[j]=instring[j];
    j++;
  } while ((outstring[j-1] != NUL) && (j < _internalStringLenSmall-1));

  if (j == _internalStringLenSmall-1) {
    outstring[j] = NUL;
    cerr << "ERROR: Internal string length reached in _extractString with " << outstring << endl;
  }
  return;
}
/* Function to extract up to the stopPos.
 * returns the string in outstring.
 */
void routeDecode::_extractString(char *instring, char *stopPos, char *outstring)
{
  int j=0;
  int stringLen = strlen(instring);
  do {
    if (instring + j == stopPos)
      outstring[j] = NUL;
    else
      outstring[j]=instring[j];
    j++;
  } while ((outstring[j-1] != NUL) && (j < stringLen-1));
  if (j == stringLen-1)
    outstring[j] = NUL;
  return;
}
/* True if char is a upercase letter */
bool routeDecode::_isLetter(char c)
{
  if('A' <= c && c <= 'Z')
    return true;
  else return false;
}
/* True if char digit 0-9 */
bool routeDecode::_isDigit(char c)
{
  if('0' <= c && c <= '9')
    return true;
  else return false;
}
