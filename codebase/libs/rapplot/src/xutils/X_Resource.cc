// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2012 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2012/9/18 21:3:46 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
////////////////////////////////////////////////////////////////////////////////
// X_RES.C : CLass to support X resource default parameters for X applications 
//
// -F. Hage  Nov 1994


#include <rapplot/X_Resource.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
using namespace std;

#define BAD_DATABASE 23999

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
X_res_db::X_res_db(char *file_name)
{
   if((db = XrmGetFileDatabase(file_name)) ==  NULL) {
     THROW(BAD_DATABASE,file_name);
   }
}

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTORS
X_res_db::~X_res_db(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// ACCESS DB AND SET VALUE Functions 
int X_res_db::extract(int &value, int def, char *match_string){  // returns 0 if using defualt, 1 if set from data base
  char  *end_pt;
  char	*stype;
  XrmValue	rvalue;

  if (XrmGetResource(db, match_string, "", &stype, &rvalue))  {
    errno = 0;
    value = (int) strtol(rvalue.addr, &end_pt,0);
    if(errno != 0) { 
      value = def;
      return 0;
     }
     return 1;
  }
  value = def;
  return 0;
}

int X_res_db::extract(long &value, int def, char *match_string){  // returns 0 if using defualt, 1 if set from data base
  char  *end_pt;
  char	*stype;
  XrmValue	rvalue;

  if (XrmGetResource(db, match_string, "", &stype, &rvalue))  {
    errno = 0;
    value = (long) strtol(rvalue.addr, &end_pt,0);
    if(errno != 0) { 
      value = def;
      return 0;
     }
     return 1;
  } 
  value = def;
  return 0;
}

int X_res_db::extract(long &value, long def, char *match_string){  // returns 0 if using defualt, 1 if set from data base
  char  *end_pt;
  char	*stype;
  XrmValue	rvalue;

  if (XrmGetResource(db, match_string, "", &stype, &rvalue))  {
    errno = 0;
    value = (long) strtol(rvalue.addr, &end_pt,0);
    if(errno != 0) { 
      value = def;
      return 0;
     }
     return 1;
  } 
  value = def;
  return 0;
}

int X_res_db::extract(float &value, float def, char *match_string){  // returns 0 if using defualt, 1 if set from data base
  char  *end_pt;
  char	*stype;
  XrmValue	rvalue;

  if (XrmGetResource(db, match_string, "", &stype, &rvalue))  {
    errno = 0;
    value = (float) strtod(rvalue.addr, &end_pt);
    if(errno != 0) { 
      value = def;
      return 0;
     }
     return 1;
  }
  value = def;
  return 0;
}

int X_res_db::extract(double &value, double def, char *match_string){  // returns 0 if using defualt, 1 if set from data base
  char  *end_pt;
  char	*stype;
  XrmValue	rvalue;

  if (XrmGetResource(db, match_string, "", &stype, &rvalue))  {
    errno = 0;
    value =  strtod(rvalue.addr, &end_pt);
    if(errno != 0) { 
      value = def;
      return 0;
     }
     return 1;
  } 
  value = def;
  return 0;
}

int X_res_db::extract(char*& value, char*  def, char* match_string){  // returns 0 if using defualt, 1 if set from data base

  char	*stype;
  XrmValue	rvalue;
  char sbuf[1024];
  char *expanded;

  if (XrmGetResource(db,match_string,"",&stype,&rvalue))  {
    STRncopy(sbuf, rvalue.addr, 1024);
    usubstitute_env(sbuf, 1024);
    if (strlen(sbuf) != strlen(rvalue.addr)) {
      expanded = (char *) umalloc(strlen(sbuf) + 1);
      strcpy(expanded, sbuf);
      value = expanded;
    } else {
      value = rvalue.addr;
    }
    return 1;
  } else {
    value = def;
    return 0;
  }
}

