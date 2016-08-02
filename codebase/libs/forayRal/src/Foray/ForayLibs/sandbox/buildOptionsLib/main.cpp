

#include <iostream>
using namespace std;

#include <cstdio>
#include <cstdlib>

#include <netcdf.h>

int main(int argc, const char *argv[]){

    const char* ncVersion = nc_inq_libvers();

    printf("NetCDF version string is %s\n",ncVersion);

    return 0;
}
