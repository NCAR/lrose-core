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
///////////////////////////////////////////////////////////////
// TriangMeshContourAlg.cc
//
// Class implementing a triangular mesh contour algorithm.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <string>

#include <contour/TriangMeshContourAlg.hh>


#define xsect(p1, p2)  ((h[p2]*xh[p1]-h[p1]*xh[p2])/(h[p2]-h[p1]))
#define ysect(p1, p2)  ((h[p2]*yh[p1]-h[p1]*yh[p2])/(h[p2]-h[p1]))

#define min(x,y)  (x<y?x:y)
#define max(x,y)  (x>y?x:y)


/*********************************************************************
 * Constructors
 */

//TriangMeshContourAlg::TriangMeshContourAlg(const bool debug_flag) :
//  ContourAlg(debug_flag)
TriangMeshContourAlg::TriangMeshContourAlg(const bool debug_flag)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

TriangMeshContourAlg::~TriangMeshContourAlg()
{
}


/*********************************************************************
 * generateContour() - Generate the indicated contour.
 *
 * Input:
 *    d                    - matrix of data to contour
 *    ilb, iub, jlb, jub   - index bounds of data matrix
 *    x                    - data matrix column coordinates
 *    y                    - data matrix row coordinates
 *    nc                   - number of contour levels
 *    z                    - contour levels in increasing order
 *
 * Note: Returns a pointer to a new Contour object.  This pointer
 *       must be deleted by the client.
 */

Contour *TriangMeshContourAlg::generateContour(float **d,
					       int ilb,
					       int iub,
					       int jlb,
					       int jub,
					       double *x,
					       double *y,
					       int nc,
					       double *z)
{
  const string method_name = "TriangMeshContourAlg::generateContour()";
  
  // The indexing of im and jm should be noted as it has to start
  // from zero unlike the fortran counterpart.

  int im[4] = { 0, 1, 1, 0 };
  int jm[4] = { 0, 0, 1, 1 };
  
  // Note that castab is arranged differently from the FORTRAN code
  // because FORTRAN and C/C++ arrays are transposed of each other,
  // in this case it is more tricky as castab is in 3 dimensions.

  int castab[3][3][3] =
  { { {	0, 0, 8 },
      { 0, 2, 5 },
      { 7, 6, 9 } },
    { {	0, 3, 4 },
      { 1, 3, 1 },
      { 4, 3, 0 } },
    { {	9, 6, 7 },
      { 5, 2, 0 },
      { 8, 0, 0 } } };
  
  Contour *contour = new Contour();
  
//  for (int j = jub-1; j >= jlb; --j)
  for (int j = jub-2; j >= jlb+1; --j)
  {
//    for (int i = ilb; i <= iub-1; ++i)
    for (int i = ilb+1; i <= iub-2; ++i)
    {
      double temp1, temp2;
      double dmin, dmax;
      
      temp1 = min(d[i][j], d[i][j+1]);
      temp2 = min(d[i+1][j], d[i+1][j+1]);
      dmin = min(temp1, temp2);
      
      temp1 = max(d[i][j], d[i][j+1]);
      temp2 = max(d[i+1][j], d[i+1][j+1]);
      dmax = max(temp1, temp2);
      
      if (dmax >= z[0] && dmin <= z[nc-1])
      {
	for (int k = 0; k < nc; ++k)
	{
	  if (z[k] >= dmin && z[k] <= dmax)
	  {
	    double h[5];
	    int sh[5];
	    double xh[5];
	    double yh[5];
	      
	    for (int m = 4; m >= 0; --m)
	    {
	      if (m > 0)
	      {
		//The indexing of im and jm should be noted
		// as it has to start from zero

		h[m] = d[i + im[m-1]][j + jm[m-1]] - z[k];
		xh[m] = x[i + im[m-1]];
		yh[m] = y[j + jm[m-1]];
	      }
	      else
	      {
		h[0] = 0.25 * (h[1] + h[2] + h[3] + h[4]);
		xh[0] = 0.5 * (x[i] + x[i+1]);
		yh[0] = 0.5 * (y[j] + y[j+1]);
	      }
	      
	      if (h[m] > 0.0)
		sh[m] = 1;
	      else if (h[m] < 0.0)
		sh[m] = -1;
	      else
		sh[m] = 0;
	    } /* endfor - m */
	    
	    // Note: At this stage, the realtive heights of the
	    // corners and the center are in the h array, and the
	    // corresponding coordinates are in the xh and yh
	    // arrays.  The center of the box is indexed by 0
	    // and the 4 corners by 1 to 4 as shown below.
	    //
	    // Each triangle is then indexed by the parameter m,
	    // and the 3 vertices of each triangle are indexed
	    // by parameters m1, m2 and m3.
	    //
	    // It is assumed that the center of the box is always
	    // vertex 2 though this is important only when all 3
	    // vertices lie exactly on the same contour level, in
	    // which case only the side of the box is drawn.
	    //
	    //   vertex 4 +-------------------+ vertex 3
	    //            | \               / |
	    //            |   \    m=3    /   |
	    //            |     \       /     |
	    //            |       \   /       |
	    //            |  m=2    X   m=2   |   the center is vertex 0
	    //            |       /   \       |
	    //            |     /       \     |
	    //            |   /    m=1    \   |
	    //            | /               \ |
	    //   vertex 1 +-------------------+ vertex 2
	    //

	    // Scan each triangle in the box

	    for (int m = 1; m <= 4; ++m)
	    {
	      int m1, m2, m3;
	      
	      m1 = m;
	      m2 = 0;
	      

	      if (m != 4)
		m3 = m + 1;
	      else
		m3 = 1;
	      
	      int case_value = castab[sh[m1]+1][sh[m2]+1][sh[m3]+1];
	      
	      if (case_value != 0)
	      {
		double x1 = 0, y1 = 0;
		double x2 = 0, y2 = 0;
		
		switch (case_value)
		{
		  // Case 1 - Line between vertices 1 and 2
		case 1:
		  x1 = xh[m1];
		  y1 = yh[m1];
		  x2 = xh[m2];
		  y2 = yh[m2];
		  break;
		  
		  // case 2 - Line between vertices 2 and 3
		case 2:
		  x1 = xh[m2];
		  y1 = yh[m2];
		  x2 = xh[m3];
		  y2 = yh[m3];
		  break;
		  
		  // case 3 - Line between vertices 3 and 1
		case 3:
		  x1 = xh[m3];
		  y1 = yh[m3];
		  x2 = xh[m1];
		  y2 = yh[m1];
		  break;
		  
		  // case 4 - Line between vertex 1 and side 2-3
		case 4:
		  x1 = xh[m1];
		  y1 = yh[m1];
		  x2 = xsect(m2, m3);
		  y2 = ysect(m2, m3);
		  break;
		  
		  // case 5 - Line between vertex 2 and side 3-1
		case 5:
		  x1 = xh[m2];
		  y1 = yh[m2];
		  x2 = xsect(m3, m1);
		  y2 = ysect(m3, m1);
		  break;
		  
		  // case 6 - Line between vertex 3 and side 1-2
		case 6:
		  x1 = xh[m1];
		  y1 = yh[m1];
		  x2 = xsect(m1, m2);
		  y2 = ysect(m1, m2);
		  break;
		  
		  // case 7 - Line between sides 1-2 and 2-3
		case 7:
		  x1 = xsect(m1, m2);
		  y1 = ysect(m1, m2);
		  x2 = xsect(m2, m3);
		  y2 = ysect(m2, m3);
		  break;
		  
		  // case 8 - Line between sides 2-3 and 3-1
		case 8:
		  x1 = xsect(m2, m3);
		  y1 = ysect(m2, m3);
		  x2 = xsect(m3, m1);
		  y2 = ysect(m3, m1);
		  break;
		  
		  // case 9 - Line between sides 3-1 and 1-2
		case 9:
		  x1 = xsect(m3, m1);
		  y1 = ysect(m3, m1);
		  x2 = xsect(m1, m2);
		  y2 = ysect(m1, m2);
		  break;
		  
		default:
		  cerr << "ERROR: " << method_name << endl;
		  cerr << "Invalid case " << case_value <<
		    " encountered" << endl;
		  break;
		} /* endswitch - case_value */
		
		printf("x1 = %f, y1 = %f, x2 = %f, y2 = %f, z = %f\n",
		       x1, y1, x2, y2, z[k]);

		ContourPoint begin(x1, y1);
		ContourPoint end(x2, y2);
		
		ContourPolyline polyline;
		polyline.addPoint(begin);
		polyline.addPoint(end);
		
		contour->addPolyline(z[k], polyline);
		
	      } /* endif - (case_value != 0) */

	    } /* endfor - m */
	  } /* endif - (z[k] >= dmin && z[k] <= dmax) */
	} /* endfor - k */
      } /* endif - (dmax >= z[0] && dmin <= z[nc-1]) */

    } /* endfor - i */
  } /* endfor - j */
  
  return contour;
  
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

