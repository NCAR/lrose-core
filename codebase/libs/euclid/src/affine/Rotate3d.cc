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
//////////////////////////////////////////////////////////
// Rotate3d.cc
//
// Base class for Rotate3d classes.
//
// The Rotate3d classes are designed as a back-end math package for
// projective geometry.
//
// July 2008
//
//////////////////////////////////////////////////////////

#include <euclid/Rotate3d.hh>
#include <cmath>
#include <cstdio>
using namespace std;

const double Rotate3d::Rad2Deg = 180.0 / M_PI;
const double Rotate3d::Deg2Rad = 1.0 / Rotate3d::Rad2Deg;

//////////////////////////////////////////////////////////
// vector norm (length)

double Rotate3d::norm(const Rotate3d::Vector &vec)
  
  
{
  
  double len = sqrt(vec.xx * vec.xx +
                    vec.yy * vec.yy +
                    vec.zz * vec.zz);
  
  return len;

}

//////////////////////////////////////////////////////////
// return normalized a vector - so its norm is 1.0

Rotate3d::Vector Rotate3d::normalize(const Rotate3d::Vector &vec)
  
  
{
  
  double len = norm(vec);
  return Vector(vec.xx / len, vec.yy / len, vec.zz / len);

}

//////////////////////////////////////////////////////////
// vector dot product

double Rotate3d::dot(const Rotate3d::Vector &vecL,
                       const Rotate3d::Vector &vecR)
  
  
{
  
  double r = (vecL.xx * vecR.xx +
              vecL.yy * vecR.yy +
              vecL.zz * vecR.zz);
  
  return r;

}

//////////////////////////////////////////////////////////
// vector dot product

Rotate3d::Vector Rotate3d::cross(const Rotate3d::Vector &vecL,
                                     const Rotate3d::Vector &vecR)
  
  
{
  
  double r0 = (vecL.yy * vecR.zz -
               vecL.zz * vecR.yy);
  
  double r1 = (vecL.zz * vecR.xx -
               vecL.xx * vecR.zz);
  
  double r2 = (vecL.xx * vecR.yy -
               vecL.yy * vecR.xx);
  
  return Vector(r0, r1, r2);

}

//////////////////////////////////////////////////////////
// multiply vector by matrix
// vecL on left, matR on right
// vecL is row
// returns row vector

Rotate3d::Vector Rotate3d::mult(const Rotate3d::Vector &vecL,
                                    const Rotate3d::Matrix &matR)

  
{
  
  double r0 = (vecL.xx * matR.row1.xx +
               vecL.yy * matR.row2.xx +
               vecL.zz * matR.row3.xx);

  double r1 = (vecL.xx * matR.row1.yy +
               vecL.yy * matR.row2.yy +
               vecL.zz * matR.row3.yy);

  double r2 = (vecL.xx * matR.row1.zz +
               vecL.yy * matR.row2.zz +
               vecL.zz * matR.row3.zz);

  return Vector(r0, r1, r2);

}

//////////////////////////////////////////////////////////
// multiply matrix by vector
// matL on left, vecR on right
// vecR is column
// returns column vector

Rotate3d::Vector Rotate3d::mult(const Rotate3d::Matrix &matL,
                                    const Rotate3d::Vector &vecR)

{

  double r0 = (matL.row1.xx * vecR.xx +
               matL.row1.yy * vecR.yy + 
               matL.row1.zz * vecR.zz);

  double r1 = (matL.row2.xx * vecR.xx +
               matL.row2.yy * vecR.yy + 
               matL.row2.zz * vecR.zz);
  
  double r2 = (matL.row3.xx * vecR.xx +
               matL.row3.yy * vecR.yy + 
               matL.row3.zz * vecR.zz);
  
  return Vector(r0, r1, r2);

}

//////////////////////////////////////////////////////////
// multiply matrix by matrix
// mat1 on left, mat2 on right

Rotate3d::Matrix Rotate3d::mult(const Rotate3d::Matrix &matL,
                                    const Rotate3d::Matrix &matR)

{

  Vector r0;
  
  r0.xx = (matL.row1.xx * matR.row1.xx +
           matL.row1.yy * matR.row2.xx + 
           matL.row1.zz * matR.row3.xx);
  
  r0.yy = (matL.row1.xx * matR.row1.yy +
           matL.row1.yy * matR.row2.yy + 
           matL.row1.zz * matR.row3.yy);

  r0.zz = (matL.row1.xx * matR.row1.zz +
           matL.row1.yy * matR.row2.zz + 
           matL.row1.zz * matR.row3.zz);

  Vector r1;
  
  r1.xx = (matL.row2.xx * matR.row1.xx +
           matL.row2.yy * matR.row2.xx + 
           matL.row2.zz * matR.row3.xx);

  r1.yy = (matL.row2.xx * matR.row1.yy +
           matL.row2.yy * matR.row2.yy + 
           matL.row2.zz * matR.row3.yy);

  r1.zz = (matL.row2.xx * matR.row1.zz +
           matL.row2.yy * matR.row2.zz + 
           matL.row2.zz * matR.row3.zz);

  Vector r2;
  
  r2.xx = (matL.row3.xx * matR.row1.xx +
           matL.row3.yy * matR.row2.xx + 
           matL.row3.zz * matR.row3.xx);

  r2.yy = (matL.row3.xx * matR.row1.yy +
           matL.row3.yy * matR.row2.yy + 
           matL.row3.zz * matR.row3.yy);

  r2.zz = (matL.row3.xx * matR.row1.zz +
           matL.row3.yy * matR.row2.zz + 
           matL.row3.zz * matR.row3.zz);

  return Matrix(r0, r1, r2);

}

//////////////////////////////////////////////////////////
// Create matrices for rotation around X, Y and Z axes

Rotate3d::Matrix Rotate3d::createRotRadAboutX(double angleRad)

{

  double sinAng, cosAng;
  EG_sincos(angleRad, &sinAng, &cosAng);

  Vector r0(1.0, 0.0, 0.0);
  Vector r1(0.0, cosAng, -sinAng);
  Vector r2(0.0, sinAng, cosAng);

  return Matrix(r0, r1, r2);

}

Rotate3d::Matrix Rotate3d::createRotRadAboutY(double angleRad)

{

  double sinAng, cosAng;
  EG_sincos(angleRad, &sinAng, &cosAng);

  Vector r0(cosAng, 0.0, sinAng);
  Vector r1(0.0, 1.0, 0.0);
  Vector r2(-sinAng, 0.0, cosAng);

  return Matrix(r0, r1, r2);

}

Rotate3d::Matrix Rotate3d::createRotRadAboutZ(double angleRad)

{

  double sinAng, cosAng;
  EG_sincos(angleRad, &sinAng, &cosAng);

  Vector r0(cosAng, -sinAng, 0.0);
  Vector r1(sinAng, cosAng, 0.0);
  Vector r2(0.0, 0.0, 1.0);

  return Matrix(r0, r1, r2);

}

Rotate3d::Matrix Rotate3d::createRotDegAboutX(double angleDeg)
{
  return createRotRadAboutX(angleDeg * Deg2Rad);
}

Rotate3d::Matrix Rotate3d::createRotDegAboutY(double angleDeg)
{
  return createRotRadAboutY(angleDeg * Deg2Rad);
}

Rotate3d::Matrix Rotate3d::createRotDegAboutZ(double angleDeg)
{
  return createRotRadAboutZ(angleDeg * Deg2Rad);
}

//////////////////////////////////////////////////////////
// Printing

// print vector

void Rotate3d::print(const Rotate3d::Vector &vec, ostream &out)
{
  cerr << vector2Str(vec) << endl;
}

// convert vector to string

string Rotate3d::vector2Str(const Rotate3d::Vector &vec)
{
  char text[1024];
  sprintf(text, "(%g,%g,%g)",
          vec.xx, vec.yy, vec.zz);
  return text;
}

// print vector az el/az string

void Rotate3d::printElAz(const Rotate3d::Vector &vec, ostream &out)
{
  cerr << vector2ElAzStr(vec) << endl;
}

// convert vector to el/az string

string Rotate3d::vector2ElAzStr(const Rotate3d::Vector &vec)
{
  double x = vec.xx;
  double y = vec.yy;
  double z = vec.zz;
  double r = norm(vec);
  double elDeg = asin(z / r) * Rad2Deg;
  double azDeg = atan2(x, y) * Rad2Deg;
  if (azDeg < 0) azDeg += 360.0;
  char text[1024];
  sprintf(text, "(el: %g, az: %g)", elDeg, azDeg);
  return text;
}

// print matrix

void Rotate3d::print(const Rotate3d::Matrix &mat, ostream &out)
{
  cerr << matrix2Str(mat) << endl;
}

// convert matrix to string

string Rotate3d::matrix2Str(const Rotate3d::Matrix &mat)
{
  char text[1024];
  sprintf(text, "(%s,%s,%s)",
          Rotate3d::vector2Str(mat.row1).c_str(),
          Rotate3d::vector2Str(mat.row2).c_str(),
          Rotate3d::vector2Str(mat.row3).c_str());
  return text;
}

