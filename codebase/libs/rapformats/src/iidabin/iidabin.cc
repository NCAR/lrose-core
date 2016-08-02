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


////////////////////////////////////////////////////////////////////////


#include <cstring>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>

#include "toolsa/cal.hh"
#include "rapformats/iidabin.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
/* Variables and functions with the .p02 refer to datafiles made before the summer of 2002.

These do not contain the vv, slw, new_ice variables.


*/
////////////////////////////////////////////////////////////////////////
static const int real_nz = iida_nz + 2;  // I am assuming this make the conversion to a 39 level grid.

static const int iida_3D_field_size             = 4*iida_nx*iida_ny*real_nz;

static const int iida_padded_3D_field_size      = iida_3D_field_size + 8;

static const int iida_2D_field_size             = 4*iida_nx*iida_ny;
static const int iida_padded_2D_field_size      = iida_2D_field_size + 8;

static const int iida_ice_offset_bytes             =  4;
static const int iida_sld_offset_bytes             =    iida_padded_3D_field_size + 4;
static const int iida_height_offset_bytes          =  2*iida_padded_3D_field_size + 4;
static const int iida_temperature_offset_bytes     =  3*iida_padded_3D_field_size + 4;
static const int iida_rh_offset_bytes              =  4*iida_padded_3D_field_size + 4;
static const int iida_pirep_offset_bytes           =  5*iida_padded_3D_field_size + 4;
static const int iida_icetype_offset_bytes         =  6*iida_padded_3D_field_size + 4;
static const int iida_new_ice_offset_bytes        =  7*iida_padded_3D_field_size + 4;
static const int iida_slw_offset_bytes             =  8*iida_padded_3D_field_size + 4;
static const int iida_vv_offset_bytes              =  9*iida_padded_3D_field_size + 4;
static const int iida_inten_offset_bytes              =  10*iida_padded_3D_field_size + 4;

static const int twoDstart = 11*iida_padded_3D_field_size;

static const int iida_cloudcount_offset_bytes      =  twoDstart + 4;
static const int iida_cloudtoptemp_offset_bytes    =  twoDstart +   iida_padded_2D_field_size + 4;
static const int iida_cloudtopheight_offset_bytes  =  twoDstart + 2*iida_padded_2D_field_size + 4;
static const int iida_cloudbaseheight_offset_bytes =  twoDstart + 3*iida_padded_2D_field_size + 4;
static const int iida_anyprcp_offset_bytes         =  twoDstart + 6*iida_padded_2D_field_size + 4;
static const int iida_zprcp_offset_bytes           =  twoDstart + 7*iida_padded_2D_field_size + 4;


////////////////////////////////////////////////////////////////////////
// old varibles

static const int iida_ice_offset_bytes_p02             =  4;
static const int iida_sld_offset_bytes_p02             =    iida_padded_3D_field_size + 4;
static const int iida_height_offset_bytes_p02          =  2*iida_padded_3D_field_size + 4;
static const int iida_temperature_offset_bytes_p02     =  3*iida_padded_3D_field_size + 4;
static const int iida_rh_offset_bytes_p02              =  4*iida_padded_3D_field_size + 4;
static const int iida_pirep_offset_bytes_p02           =  5*iida_padded_3D_field_size + 4;
static const int iida_icetype_offset_bytes_p02         =  6*iida_padded_3D_field_size + 4;


static const int twoDstart_p02 = 7*iida_padded_3D_field_size;

static const int iida_cloudcount_offset_bytes_p02      =  twoDstart_p02 + 4;
static const int iida_cloudtoptemp_offset_bytes_p02    =  twoDstart_p02 +   iida_padded_2D_field_size + 4;
static const int iida_cloudtopheight_offset_bytes_p02  =  twoDstart_p02 + 2*iida_padded_2D_field_size + 4;
static const int iida_cloudbaseheight_offset_bytes_p02 =  twoDstart_p02 + 3*iida_padded_2D_field_size + 4;
static const int iida_anyprcp_offset_bytes_p02         =  twoDstart_p02 + 6*iida_padded_2D_field_size + 4;
static const int iida_zprcp_offset_bytes_p02           =  twoDstart_p02 + 7*iida_padded_2D_field_size + 4;


////////////////////////////////////////////////////////////////////////


static int iida_3_to_1(int x, int y, int z);
static int iida_2_to_1(int x, int y);

static int parse_int(const char *, int);

static void shuffle_4(void *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class IidaBinaryFile
   //


////////////////////////////////////////////////////////////////////////


IidaBinaryFile::IidaBinaryFile()

{

fd = -1;

valid_time = 0;

}


////////////////////////////////////////////////////////////////////////


IidaBinaryFile::~IidaBinaryFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


IidaBinaryFile::IidaBinaryFile(const IidaBinaryFile &)

{

cerr << "\n\n  IidaBinaryFile::IidaBinaryFile(const IidaBinaryFile &) should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


IidaBinaryFile & IidaBinaryFile::operator=(const IidaBinaryFile &)

{

cerr << "\n\n  IidaBinaryFile::operator=(const IidaBinaryFile &) -> should never be called!\n\n";

exit ( 1 );

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::close()

{

valid_time = 0;

if ( fd >= 0 )  {

   ::close(fd);   fd = -1;

}

return;

}


////////////////////////////////////////////////////////////////////////


int IidaBinaryFile::open(const char *filename)

{

close();

if ( (fd = ::open(filename, O_RDONLY)) < 0 )  {

   fd = -1;

   return ( 0 );

}

int j;
int month, day, year, hour;
const char *short_name = (const char *) 0;

j = strlen(filename) - 1;

while ( (j >= 0) && (filename[j] != '/') )  --j;

++j;

short_name = filename + j;

year  = parse_int(short_name, 4);
month = parse_int(short_name + 4, 2);
day   = parse_int(short_name + 6, 2);
hour  = parse_int(short_name + 8, 2);

valid_time = mdyhms_to_unix(month, day, year, hour, 0, 0);

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::threeD_data(int offset_bytes, int x, int y, int z)

{

int n, n_read;
double v;
float f[2];   //  don't let the compiler promote it to a double

n = offset_bytes + 4*iida_3_to_1(x, y, z);

if ( lseek(fd, n, SEEK_SET) < 0 )  {

   cerr << "\n\n  IidaBinaryFile::threeD_data() -> lseek error!\n\n";

   exit ( 1 );

}

if ( (n_read = read(fd, f, 4)) != 4 )  {

   cerr << "\n\n  IidaBinaryFile::threeD_data() -> read error!\n\n";

   exit ( 1 );

}

shuffle_4(f);

v = (double) f[0];

return ( v );

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::twoD_data(int offset_bytes, int x, int y)

{

int n, n_read;
double v;
float f[2];   //  don't let the compiler promote it to a double

n = offset_bytes + 4*iida_2_to_1(x, y);

if ( lseek(fd, n, SEEK_SET) < 0 )  {

   cerr << "\n\n  IidaBinaryFile::twoD_data() -> lseek error!\n\n";

   exit ( 1 );

}

if ( (n_read = read(fd, f, 4)) != 4 )  {
   cerr << "\n\n Nread = "<< n_read << "and " << n ;
   cerr << "\n\n  IidaBinaryFile::twoD_data() -> read error!\n\n";

   exit ( 1 );

}

shuffle_4(f);

v = (double) f[0];

return ( v );

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::ice(int x, int y, int z)

{

return ( threeD_data(iida_ice_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::ice_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_ice_offset_bytes, datamin, datamax);

return;

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::ice_type(int x, int y, int z)

{

return ( threeD_data(iida_icetype_offset_bytes, x, y, z) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::new_ice(int x, int y, int z)

{

return ( threeD_data(iida_new_ice_offset_bytes, x, y, z) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::slw(int x, int y, int z)

{

return ( threeD_data(iida_slw_offset_bytes, x, y, z) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::vv(int x, int y, int z)

{

return ( threeD_data(iida_vv_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::inten(int x, int y, int z)

{

return ( threeD_data(iida_inten_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////

void IidaBinaryFile::ice_type_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_icetype_offset_bytes, datamin, datamax);

return;

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::sld(int x, int y, int z)

{


return ( threeD_data(iida_sld_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::sld_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_sld_offset_bytes, datamin, datamax);

return;

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::temperature(int x, int y, int z)

{

return ( threeD_data(iida_temperature_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::temperature_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_temperature_offset_bytes, datamin, datamax);

return;

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::rh(int x, int y, int z)

{

return ( threeD_data(iida_rh_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::rh_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_rh_offset_bytes, datamin, datamax);

return;

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::height_m(int x, int y, int z)

{

return ( threeD_data(iida_height_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::height_m_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_height_offset_bytes, datamin, datamax);

return;

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::pirep(int x, int y, int z)

{

return ( threeD_data(iida_pirep_offset_bytes, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::pirep_minmax(double &datamin, double &datamax)

{

data_minmax_3D(iida_pirep_offset_bytes, datamin, datamax);

return;

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudcount(int x, int y)

{

return ( twoD_data(iida_cloudcount_offset_bytes, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudtoptemp(int x, int y)

{

return ( twoD_data(iida_cloudtoptemp_offset_bytes, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudtopheight(int x, int y)

{

return ( twoD_data(iida_cloudtopheight_offset_bytes, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudbaseheight(int x, int y)

{

return ( twoD_data(iida_cloudbaseheight_offset_bytes, x, y) );

}




////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::anyprcp(int x, int y)

{

return ( twoD_data(iida_anyprcp_offset_bytes, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::zprcp(int x, int y)

{

return ( twoD_data(iida_zprcp_offset_bytes, x, y) );

}


////////////////////////////////////////////////////////////////////////


void IidaBinaryFile::data_minmax_3D(int offset_bytes, double &datamin, double &datamax)

{

int real_z, j, j_max, bytes, bytes_left, n_read;
int iida_n;
const int n_floats = 1024;
float buf[n_floats];
double x;

bytes_left = 4*iida_nx*iida_ny*real_nz;

datamax = -1.0e10;
datamin =  1.0e10;

if ( lseek(fd, offset_bytes, SEEK_SET) < 0 )  {

   cerr << "\n\n  IidaBinaryFile::data_minmax_3D() -> lseek error\n\n";

   exit ( 1 );

}

iida_n = 0;

while ( bytes_left > 0 )  {

   bytes = bytes_left;

   if ( bytes > ((int) sizeof(buf)) )  bytes = (int) sizeof(buf);

   if ( (n_read = ::read(fd, buf, bytes)) != bytes )  {

      cerr << "\n\n  IidaBinaryFile::data_minmax_3D() -> read error\n\n";

      exit ( 1 );

   }

   j_max = bytes/4;

   for (j=0; j<j_max; ++j)  {

      shuffle_4(buf + j);

      x = (double) buf[j];

      real_z = iida_n%real_nz;

      if ( real_z >= 2 )  {

         if ( x > datamax )  datamax = x;
         if ( x < datamin )  datamin = x;

      }

      ++iida_n;

   }

   bytes_left -= n_read;

}   //  while

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int iida_3_to_1(int x, int y, int z)

{

if ( (x < 0) || (x >= iida_nx) || (y < 0) || (y >= iida_ny) || (z < 0) || (z >= iida_nz) )  {

   cerr << "\n\n  iida_3_to_1(int x, int y, int z) -> range check error\n\n";

   exit ( 1 );

}

int n;

z += 2;

n = x*iida_ny*real_nz + y*real_nz + z;

return ( n );

}


////////////////////////////////////////////////////////////////////////


int iida_2_to_1(int x, int y)

{

if ( (x < 0) || (x >= iida_nx) || (y < 0) || (y >= iida_ny) )  {

   cerr << "\n\n  iida_2_to_1(int x, int y, int z) -> range check error\n\n";

   exit ( 1 );

}

int n;

n = y*iida_nx + x;

return ( n );

}


////////////////////////////////////////////////////////////////////////


int parse_int(const char *s, int n)

{

int j, a;


a = 0;

for (j=0; j<n; ++j)  {

   if ( (s[j] >= '0') && (s[j] <= '9') )  a = 10*a + (s[j] - '0');
   else {

      cerr << "\n\n  parse_int() -> bad char: \"" << s[j] << "\"\n\n";

      exit ( 1 );

   }

}

return ( a );

}


////////////////////////////////////////////////////////////////////////


void shuffle_4(void *p)

{

unsigned char *u = (unsigned char *) p;
unsigned char t;

t = u[0];
u[0] = u[3];
u[3] = t;

t = u[1];
u[1] = u[2];
u[2] = t;

return;

}
///////////////////////////////////////////////////////////////////////
// data retrieval functions for values from pre2002 models.



double IidaBinaryFile::ice_p02(int x, int y, int z)

{

return ( threeD_data(iida_ice_offset_bytes_p02, x, y, z) );

}



////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::ice_type_p02(int x, int y, int z)

{

return ( threeD_data(iida_icetype_offset_bytes_p02, x, y, z) );

}

////////////////////////////////////////////////////////////////////////



double IidaBinaryFile::sld_p02(int x, int y, int z)

{


return ( threeD_data(iida_sld_offset_bytes_p02, x, y, z) );

}



////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::temperature_p02(int x, int y, int z)

{

return ( threeD_data(iida_temperature_offset_bytes_p02, x, y, z) );

}



////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::rh_p02(int x, int y, int z)

{

return ( threeD_data(iida_rh_offset_bytes_p02, x, y, z) );

}



////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::height_m_p02(int x, int y, int z)

{

return ( threeD_data(iida_height_offset_bytes_p02, x, y, z) );

}


////////////////////////////////////////////////////////////////////////



double IidaBinaryFile::pirep_p02(int x, int y, int z)

{

return ( threeD_data(iida_pirep_offset_bytes_p02, x, y, z) );

}


////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudcount_p02(int x, int y)

{

return ( twoD_data(iida_cloudcount_offset_bytes_p02, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudtoptemp_p02(int x, int y)

{

return ( twoD_data(iida_cloudtoptemp_offset_bytes_p02, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudtopheight_p02(int x, int y)

{

return ( twoD_data(iida_cloudtopheight_offset_bytes_p02, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::cloudbaseheight_p02(int x, int y)

{

return ( twoD_data(iida_cloudbaseheight_offset_bytes_p02, x, y) );

}




////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::anyprcp_p02(int x, int y)

{

return ( twoD_data(iida_anyprcp_offset_bytes_p02, x, y) );

}

////////////////////////////////////////////////////////////////////////


double IidaBinaryFile::zprcp_p02(int x, int y)

{

return ( twoD_data(iida_zprcp_offset_bytes_p02, x, y) );

}








////////////////////////////////////////////////////////////////////////




