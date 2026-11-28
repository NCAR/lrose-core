#include <netcdf.h>

#include <cstdio>

extern "C" {

static int _ncID;
static char _buffer[256];
char *cleanString(char *str, int len);

  ////////////////////////////////////////////
  //
  // Open a netcdf file.
  //
  void opennetcdf_( char *filename,
		    int *ok,
		    int *debug,
		    int lenStr
		    ){

    *ok = 0;

    if (*debug){
      fprintf(stderr,"Opening netcdf file \"%s\"\n", cleanString(filename,lenStr));
    }

    if (NC_NOERR == nc_open(cleanString(filename,lenStr), 0, &_ncID)){
      *ok = 1;
      return;
    }
    return;
  }

  ////////////////////////////////////////////
  //
  // Get a variable.
  //
  void getnetcdfvar_( char *varName,
		    float *var,
		    int *ok,
		    int *debug,
		    int lenStr
		    ){
    *ok = 0;

    if (*debug){
      fprintf(stderr,"Getting varriable %s\n", cleanString(varName,lenStr));
    }

    int varID;
    if (NC_NOERR == nc_inq_varid(_ncID, 
				 cleanString(varName,lenStr),
				 &varID)){

      if (*debug){
	fprintf(stderr,"Got variable ID.\n");
      }

      if (NC_NOERR == nc_get_var_float(_ncID, varID,var)){
	*ok = 1;
	return;
      }
    }
    return;
  }

  ////////////////////////////////////////////
  //
  // Get a floating point global attribute.
  //
  void getnetcdfatt_( char *attName,
		    float *att,
		    int *ok,
		    int *debug,
		    int lenStr
		    ){
    *ok = 0;

    if (*debug){
      fprintf(stderr,"Getting attribute %s\n", cleanString(attName,lenStr));
    }

    if (NC_NOERR == nc_get_att_float(_ncID, NC_GLOBAL,
				     cleanString(attName,lenStr),
				     att)){
      *ok = 1;
      return;
    }
    return;

  }

  ////////////////////////////////////////////
  //
  // Close the netcdf file.
  //
  void closenetcdf_(int *ok, int *debug){
    
    *ok = 0;
    
    if (*debug){
      fprintf(stderr,"Closing netcdf file.\n");
    }

    if (NC_NOERR == nc_close(_ncID)){
      *ok = 1;
      return;
    }
    return;
  }

  ////////////////////////////////////////////
  //
  // Small routine to put a null terminator at the end of a fortran
  // style string given the length of the string. Also removes
  // trailing blanks.
  //
  char *cleanString(char *str, int len){
    int j=0;
    for (int i=0; i < len; i++){
      if (str[i] != ' '){
	_buffer[j] = str[i];
	j++;
      }
    }
    _buffer[j]=char(0);
    return _buffer;
  }


}
