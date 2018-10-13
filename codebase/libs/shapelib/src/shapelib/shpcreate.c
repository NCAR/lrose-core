/******************************************************************************
 * $Id: shpcreate.c,v 1.2 2018/10/13 23:24:07 dixon Exp $
 *
 * Project:  Shapelib
 * Purpose:  Sample application for creating a new shapefile.
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
 * $Log: shpcreate.c,v $
 * Revision 1.2  2018/10/13 23:24:07  dixon
 * Sync with EOL/github/lrose-core
 *
 * Revision 1.1  2000/06/28 13:37:01  rehak
 * Initial version of library from http://gdal.velocet.ca/projects/shapelib/
 *
 * Revision 1.3  1999/11/05 14:12:04  warmerda
 * updated license terms
 *
 * Revision 1.2  1995/08/04 03:16:43  warmerda
 * Added header.
 *
 */

#include "shapefil.h"

int main( int argc, char ** argv )

{
    SHPHandle	hSHP;
    int		nShapeType;
    double	*padVertices;

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
    if( argc != 3 )
    {
	printf( "shpcreate shp_file [point/arc/polygon/multipoint]\n" );
	exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*	Figure out the shape type.					*/
/* -------------------------------------------------------------------- */
    if( strcmp(argv[2],"POINT") == 0 || strcmp(argv[2],"point") == 0 )
        nShapeType = SHPT_POINT;
    else if( strcmp(argv[2],"ARC") == 0 || strcmp(argv[2],"arc") == 0 )
        nShapeType = SHPT_ARC;
    else if( strcmp(argv[2],"POLYGON") == 0 || strcmp(argv[2],"polygon") == 0 )
        nShapeType = SHPT_POLYGON;
    else if( strcmp(argv[2],"MULTIPOINT")==0 ||strcmp(argv[2],"multipoint")==0)
        nShapeType = SHPT_MULTIPOINT;
    else
    {
	printf( "Shape Type `%s' not recognised.\n", argv[2] );
	exit( 2 );
    }

/* -------------------------------------------------------------------- */
/*	Create the requested layer.					*/
/* -------------------------------------------------------------------- */
    hSHP = SHPCreate( argv[1], nShapeType );

    if( hSHP == NULL )
    {
	printf( "Unable to create:%s\n", argv[1] );
	exit( 3 );
    }

    SHPClose( hSHP );
}
