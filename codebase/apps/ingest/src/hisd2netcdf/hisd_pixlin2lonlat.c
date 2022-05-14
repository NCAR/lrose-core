/* ----------------------------------------------------------------------------
	Sample source code for Himawari Standard Data

	Copyright (C) 2014 MSC (Meteorological Satellite Center) of JMA

	Disclaimer:
		MSC does not guarantee regarding the correctness, accuracy, reliability,
		or any other aspect regarding use of these sample codes.

	Detail of Himawari Standard Format: 
		For data structure of Himawari Standard Format, prelese refer to MSC
		Website and Himawari Standard Data User's Guide.

		MSC Website
		http://www.jma-net.go.jp/msc/en/

		Himawari Standard Data User's Guide
		http://www.data.jma.go.jp/mscweb/en/himawari89/space_segment/hsd_sample/HS_D_users_guide_en.pdf

	History
		April,   2014  First release
		January, 2015  Change for version 1.1
        June,    2015  Fixed bug in funcntion lonlat_to_pixlin() 
                       ((8) check the reverse side of the Earth)

---------------------------------------------------------------------------- */
# include <stdio.h>
# include <math.h>
# include "hisd.h"

#define DEGTORAD (M_PI/180.0)
#define RADTODEG (180.0/M_PI)
#define SCLUNIT  1.525878906250000e-05 	// (= 2^-16)  scaling function 

#define  NORMAL_END  0
#define  ERROR_END 100

/* ----------------------------------------------------------------------------
	Normalized Geostationary Projection is adopted as defined in LRIT/HRIT
	Global Specification Section 4.4. 
	The projection describes the view from the satellite to an idealized earth.

	LRIT/HRIT Global Specification
		http://www.cgms-info.org/publications/technical-publications
 ----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
	lonlat_to_pixlin()
 ----------------------------------------------------------------------------*/
int lonlat_to_pixlin(HisdHeader *head,double lon,double lat,
	float *pix,float *lin){

	// (1) init
	*pix = -9999.0;	// invalid value
	*lin = -9999.0;
	// (2) check latitude
	if(lat < -90.0 || 90.0 < lat ){
		return(ERROR_END);
	}
	// (3) check longitude
	while(lon > 180.0){ lon-=360.0; } // [deg]
	while(lon <-180.0){ lon+=360.0; } // [deg]
	// (4) degree to radian
	lon = lon * DEGTORAD; // [rad]
	lat = lat * DEGTORAD; // [rad]
	// (5) geocentric latitude
	// Global Specification 4.4.3.2
	// phi = arctan( (Rpol^2)/(Req^2) * tan(lat) )
	// 
	// (Rpol^2)/(Req^2) = head->proj->projParam2
	double phi = atan( head->proj->projParam2 * tan(lat) );
	// (6) The length of Re
	// Re = (Rpol) / sqrt( 1 - (Req^2 - Rpol^2) / Req^2 * cos^2(phi) )
	//
	// Rpol = head->proj->polrRadius
	// (Req^2 - Rpol^2) / Req^2 = head->proj->projParam1
	double Re = (head->proj->polrRadius) /
					sqrt(1 - head->proj->projParam1 * cos(phi) * cos(phi)) ;
	// (7) The cartesian components of the vector rs result as follows:
	// r1 = h - Re * cos(phi) * cos(Le-Ld)
	// r2 =    -Re * cos(phi) * sin(Le-Ld)
	// r3 =     Re * sin(phi)
	//
	// Le : longitude
	// Ld : sub_lon = head->proj->subLon
	// h  : distance from Earth's center to satellite (=head->proj->satDis)
	double r1 = head->proj->satDis - Re * cos(phi)
		 		* cos( lon - head->proj->subLon * DEGTORAD );
	double r2 = - Re * cos(phi)
				* sin( lon - head->proj->subLon * DEGTORAD );
	double r3 = Re * sin(phi);
	// (8) check seeablibity
//	double vx = Re * cos(phi) * cos( lon - head->proj->subLon * DEGTORAD );
//	if(0 < -r1 * vx - r2 * r2 + r3 * r3){
//		return(ERROR_END);
//	}
//	2015.06.06  fixed bug
	if( 0 < (r1 * (r1 - head->proj->satDis) + (r2 * r2) + (r3 * r3))){
		return(ERROR_END);
	}

	// (9) The projection function is as follows:
	// x  = arctan(-r2/r1)
	// y  = arcsin(r3/rn)
	// rn = sqrt(r1^2 + r2^2 + r3^2)
	double rn = sqrt(r1*r1 + r2*r2 + r3*r3);
	double x = atan2(-r2,r1) * RADTODEG;
	double y = asin(-r3/rn) * RADTODEG;
	// (10)
	// Global Specification 4.4.4
	// c  = COFF + nint(x * 2^-16 * CFAC)
	// l  = LOFF + nint(y * 2^-16 * LFAC)
	float c = head->proj->coff + x * SCLUNIT * head->proj->cfac;
	float l = head->proj->loff + y * SCLUNIT * head->proj->lfac;

	*pix = c;
	*lin = l;

	return(NORMAL_END);
}

/* ----------------------------------------------------------------------------
	pixlin_to_lonlat()
 ----------------------------------------------------------------------------*/
int pixlin_to_lonlat(HisdHeader *head,float pix,float lin,
	double *lon,double *lat){
	double Sd,Sn,S1,S2,S3,Sxy;

	// (0) init
	*lon = -9999;	// invalid value
	*lat = -9999;
	// (1) pix,lin ==> c,l
	float c = pix;
	float l = lin;
	// (2) the intermediate coordinates (x,y)
	// Global Specification 4.4.4 Scaling Function 
	//    c = COFF + nint(x * 2^-16 * CFAC)
	//    l = LOFF + nint(y * 2^-16 * LFAC)
	// The intermediate coordinates (x,y) are as follows :
	//    x = (c -COFF) / (2^-16 * CFAC)
	//    y = (l -LOFF) / (2^-16 * LFAC)
	//    SCLUNIT = 2^-16
	double x = DEGTORAD * ( c - head->proj->coff) / ( SCLUNIT * head->proj->cfac);
	double y = DEGTORAD * ( l - head->proj->loff) / ( SCLUNIT * head->proj->lfac);
	// (3) longtitude,latitude
	// Global Specification 4.4.3.2
	// The invers projection function is as follows : 
	//   lon = arctan(S2/S1) + sub_lon
	//   lat = arctan( (Req^2/Rpol^2) * S3 / Sxy )
	// 
	// Thererin the variables S1,S2,S3,Sxy are as follows :
	//    S1  = Rs - Sn * cos(x) * cos(y)
	//    S2  = Sn * sin(x) * cos(y)
	//    S3  =-Sn * sin(y)
	//    Sxy = sqrt(S1^2 + S2^2)
	//    Sn  =(Rs * cos(x) * cos(y) - Sd ) /
	//         (cos(y) * cos(y) + (Req^2/Rpol^2) * sin(y) * sin(y))
	//    Sd  =sqrt( (Rs * cos(x) * cos(y))^2
	//               - ( cos(y) * cos(y) + (Req^2/Rpol^2) * sin(y) * sin(y) )
	//               * (Rs^2 - Req^2)
	// The variables Rs,Rpol,Req,(Req^2/Rpol^2),(Rs^2 - Req^2) are as follows :
	//    Rs  : distance from Earth center to satellite= head->proj->satDis
	//    Rpol: polar radius of the Earth              = head->proj->polrRadius
	//    Req : equator raidus of the Earth            = head->proj->eqtrRadius
	//    (Req^2/Rpol^2)                               = head->proj->projParam3
	//    (Rs^2 - Req^2)                               = head->proj->projParamSd
	Sd = (head->proj->satDis * cos(x) * cos(y)) *
		 (head->proj->satDis * cos(x) * cos(y)) -
		 (cos(y) * cos(y) + head->proj->projParam3 * sin(y) * sin(y)) *
		 head->proj->projParamSd;
	if(Sd < 0 )	{ return(ERROR_END);} 
	else		{ Sd = sqrt(Sd);}
	Sn = (head->proj->satDis * cos(x) * cos(y) -Sd) /
	     (cos(y) * cos(y) + head->proj->projParam3 * sin(y) * sin(y));
	S1 = head->proj->satDis - (Sn * cos(x) * cos(y));
	S2 = Sn * sin(x) * cos(y);
	S3 =-Sn * sin(y);
	Sxy=sqrt( S1 * S1 + S2 * S2);

	*lon = RADTODEG * atan2(S2,S1) + head->proj->subLon;
	*lat = RADTODEG * atan(head->proj->projParam3 * S3 / Sxy);

	//(4) check longtitude
	while(*lon > 180.0 ){ *lon = *lon-360.0;}
	while(*lon <-180.0 ){ *lon = *lon+360.0;}

	return(NORMAL_END);
}

