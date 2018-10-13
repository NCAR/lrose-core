/******************************************************************************
 * $Id: shpadd.c,v 1.2 2018/10/13 23:24:07 dixon Exp $
 *
 * Project:  Shapelib
 * Purpose:  Sample application for adding a shape to a shapefile.
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see LICENSE.LGPL).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log: shpadd.c,v $
 * Revision 1.2  2018/10/13 23:24:07  dixon
 * Sync with EOL/github/lrose-core
 *
 * Revision 1.1  2000/06/28 13:37:01  rehak
 * Initial version of library from http://gdal.velocet.ca/projects/shapelib/
 *
 * Revision 1.9  1999/11/05 14:12:04  warmerda
 * updated license terms
 *
 * Revision 1.8  1998/12/03 16:36:26  warmerda
 * Use r+b rather than rb+ for binary access.
 *
 * Revision 1.7  1998/11/09 20:57:04  warmerda
 * Fixed SHPGetInfo() call.
 *
 * Revision 1.6  1998/11/09 20:19:16  warmerda
 * Changed to use SHPObject based API.
 *
 * Revision 1.5  1997/03/06 14:05:02  warmerda
 * fixed typo.
 *
 * Revision 1.4  1997/03/06 14:01:16  warmerda
 * added memory allocation checking, and free()s.
 *
 * Revision 1.3  1995/10/21 03:14:37  warmerda
 * Changed to use binary file access
 *
 * Revision 1.2  1995/08/04  03:18:01  warmerda
 * Added header.
 *
 */

#include "shapefil.h"

int main( int argc, char ** argv )

{
    SHPHandle	hSHP;
    int		nShapeType, nVertices, nParts, *panParts, i;
    double	*padfX, *padfY;
    SHPObject	*psObject;

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
    if( argc < 4 )
    {
	printf( "shpadd shp_file [[x y] [+]]*\n" );
	exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
    hSHP = SHPOpen( argv[1], "r+b" );

    if( hSHP == NULL )
    {
	printf( "Unable to open:%s\n", argv[1] );
	exit( 1 );
    }

    SHPGetInfo( hSHP, NULL, &nShapeType, NULL, NULL );

/* -------------------------------------------------------------------- */
/*	Build a vertex/part list from the command line arguments.	*/
/* -------------------------------------------------------------------- */
    padfX = (double *) malloc(sizeof(double) * 1000);
    padfY = (double *) malloc(sizeof(double) * 1000);
    
    nVertices = 0;

    if( (panParts = (int *) malloc(sizeof(int) * 1000 )) == NULL )
    {
        printf( "Out of memory\n" );
        exit( 1 );
    }
    
    nParts = 1;
    panParts[0] = 0;

    for( i = 2; i < argc;  )
    {
	if( argv[i][0] == '+' )
	{
	    panParts[nParts++] = nVertices;
	    i++;
	}
	else if( i < argc-1 )
	{
	    sscanf( argv[i], "%lg", padfX+nVertices );
	    sscanf( argv[i+1], "%lg", padfY+nVertices );
	    nVertices += 1;
	    i += 2;
	}
    }

/* -------------------------------------------------------------------- */
/*      Write the new entity to the shape file.                         */
/* -------------------------------------------------------------------- */
    psObject = SHPCreateObject( nShapeType, -1, nParts, panParts, NULL,
                                nVertices, padfX, padfY, NULL, NULL );
    SHPWriteObject( hSHP, -1, psObject );
    SHPDestroyObject( psObject );
    
    SHPClose( hSHP );

    free( panParts );
    free( padfX );
    free( padfY );
}
