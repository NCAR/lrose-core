/*************************************************************************
 * DumpXid.c: A simple routine to Dump the contents of a pixmap to an open
 *  File using the xwd format.
 *  
 *  From a template contained in xwd.c : See copyright at the end of the file.
 *    T. Wilsher  RAP 1995
 *  Hacked slightly to remove library dependencies - F Hage. RAP 1995
 *  XUTIL_() function added by Rehak.
 *  Moved into rapplot lib by dixon, 9/26/97.
 */
  
#include <stdlib.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* #include <X11/Xmu/WinUtil.h> */
typedef unsigned long Pixel;
#include <X11/XWDFile.h>

#include <rapplot/xutils.h>

/*
 * Static function prototypes
 */

static int Get_XColors(Display *dpy, XWindowAttributes *win_info,
		       Window root, XColor **colors);

static int _swapshort(char *bp, unsigned n);

static int _swaplong(char *bp, unsigned n);

static int Image_Size(XImage *image);

/***********************************************************************
 * XUTIL_dump_pixmap_xwd(): dump a pixmap to a file which must already
 *                          be open for writing.  The file will be in
 *                          xwd format.
 */

void XUTIL_dump_pixmap_xwd(Display *dpy, Pixmap pixmap,
			   XWindowAttributes *win_info, FILE *out)
{
  static char *routine_name = "XUTIL_dump_xid";
  
  unsigned long swaptest = 1;
  XColor * colors;
  unsigned buffer_size;
  int win_name_size;
  int header_size;
  int ncolors, i;
  char * win_name = 0;
  Bool got_win_name;

  XImage *image;
  int x, y;
  unsigned width, height, border_width;
  unsigned int depth;

  Window root;
  XWDFileHeader header;
  XWDColor xwdcolor;
   

  /*
   * Get the parameters of the pixmap being dumped.
   */

#ifdef DBG
  fprintf(stderr,"xutils:%s - Getting target pixmap information.\n",
	  routine_name);
#endif

  /*
   * Make sure that the caller sent in some window attributes
   */

  if(!win_info)
  {
    fprintf(stderr, "ERROR - xutils:%s\n",
	    routine_name);
    fprintf(stderr, "Invalid window attributes received.");
    return;
  }

  /*
   * Get the pixmap geometry
   */

  if (!XGetGeometry(dpy, pixmap, 
		    &root, &x, &y, &width, &height,
		    &border_width, &depth))
  {
    fprintf(stderr, "ERROR - xutils:%s\n",
	    routine_name);
    fprintf(stderr,"Can't get pixmap geometry.");
    return;
  }
		       
  if (!win_name || !win_name[0])
  {
    win_name = "xwdump";
    got_win_name = 0;
  }
  else
  {
    got_win_name = 1;
  }

  /* sizeof(char) is included for the null string terminator. */
  win_name_size = strlen(win_name) + sizeof(char);

  /*
   * Snarf the pixmap with XGetImage.
   */

  image = XGetImage (dpy, pixmap, x, y, width, height, AllPlanes, ZPixmap);

  if (!image)
  {
    fprintf (stderr, 
	     "ERROR - %s: XGetImage failed\n", routine_name);
    fprintf(stderr,
	    "Unable to get image at width = %d, height = %d, x =%d, y = %d\n",
	     width, height, x, y);
    return;
  }

  /*
   * Determine the pixmap size.
   */

  buffer_size = Image_Size(image);

#ifdef DBG
  fprintf(stderr,"xwd: Getting Colors.\n");
#endif

  ncolors = Get_XColors( dpy, win_info, root, &colors);

  XFlush(dpy);

  /*
   * Calculate header size.
   */

#ifdef DBG
  fprintf(stderr,"xwd: Calculating header size.\n");
#endif
  header_size = SIZEOF(XWDheader) + win_name_size;

  /*
   * Write out header information.
   */

#ifdef DBG
  fprintf(stderr,"Constructing and dumping file header.");
#endif

  header.header_size = (CARD32) header_size;
  header.file_version = (CARD32) XWD_FILE_VERSION;
  header.pixmap_format = (CARD32) ZPixmap; /* FIXME: format; */
  header.pixmap_depth = (CARD32) image->depth;
  header.pixmap_width = (CARD32) image->width;
  header.pixmap_height = (CARD32) image->height;
  header.xoffset = (CARD32) image->xoffset;
  header.byte_order = (CARD32) image->byte_order;
  header.bitmap_unit = (CARD32) image->bitmap_unit;
  header.bitmap_bit_order = (CARD32) image->bitmap_bit_order;
  header.bitmap_pad = (CARD32) image->bitmap_pad;
  header.bits_per_pixel = (CARD32) image->bits_per_pixel;
  header.bytes_per_line = (CARD32) image->bytes_per_line;

  header.visual_class = (CARD32) win_info->visual->class;
  header.red_mask = (CARD32) win_info->visual->red_mask;
  header.green_mask = (CARD32) win_info->visual->green_mask;
  header.blue_mask = (CARD32) win_info->visual->blue_mask;
  header.bits_per_rgb = (CARD32) win_info->visual->bits_per_rgb;
  header.colormap_entries = (CARD32) win_info->visual->map_entries;
  header.ncolors = ncolors;
  header.window_width = (CARD32) width;
  header.window_height = (CARD32) height;
  header.window_x = x;
  header.window_y = y;
  header.window_bdrwidth = (CARD32)0;

  if (*(char *)&swaptest)
  {
    _swaplong((char *) &header, sizeof(header));
    for (i = 0; i < ncolors; i++)
    {
      _swaplong((char *) &colors[i].pixel, sizeof(long));
      _swapshort((char *) &colors[i].red, 3 * sizeof(short));
    }
  }

  if (fwrite((char *)&header, SIZEOF(XWDheader), 1, out) != 1 ||
      fwrite(win_name, win_name_size, 1, out) != 1)
  {
    perror("xwd");
    return;
  }

  /*
   * Write out the color maps, if any
   */

#ifdef DBG
  fprintf(stderr,"xwd: Dumping %d colors.\n", ncolors);
#endif

  for (i = 0; i < ncolors; i++)
  {
    xwdcolor.pixel = colors[i].pixel;
    xwdcolor.red = colors[i].red;
    xwdcolor.green = colors[i].green;
    xwdcolor.blue = colors[i].blue;
    xwdcolor.flags = colors[i].flags;
    if (fwrite((char *) &xwdcolor, SIZEOF(XWDColor), 1, out) != 1)
    {
      perror("xwd");
      return;
    }
  }

  /*
   * Write out the buffer.
   */

#ifdef DBG
  fprintf(stderr,"Dumping pixmap.  bufsize=%d\n",buffer_size);
#endif

  /*
   *    This copying of the bit stream (data) to a file is to be replaced
   *  by an Xlib call which hasn't been written yet.  It is not clear
   *  what other functions of xwd will be taken over by this (as yet)
   *  non-existant X function.
   */

  if (fwrite(image->data, (int) buffer_size, 1, out) != 1)
  {
    perror("xwd");
    return;
  }

  /*
   * free the color buffer.
   */

  if (ncolors > 0)
  { 
#ifdef DBG
    fprintf(stderr,"Freeing colors.\n");
#endif
    free(colors);
  }

  /*
   * Free window name string.
   */

#ifdef DBG
  fprintf(stderr,"Freeing window name string.");
#endif

  if (got_win_name)
    XFree(win_name);

  /*
   * Free image
   */

  XDestroyImage(image);

  return;
}


/*********************************************************************
 * Window_Dump: dump a window to a file which must already be open for
 *              writing.
 */

void
XwuDumpWindow( Display * dpy, Drawable window, XWindowAttributes * win_info, FILE *  out)
{
    unsigned long swaptest = 1;
    XColor * colors;
    unsigned buffer_size;
    int win_name_size;
    int header_size;
    int ncolors, i;
    char * win_name = 0;
    Bool got_win_name;

    XImage *image;
    int x, y;
    unsigned width, height, border_width;
    unsigned int depth;

    Window root;
    XWDFileHeader header;
    XWDColor xwdcolor;
   

    /*
     * Get the parameters of the window being dumped.
     */
#ifdef DBG
    fprintf(stderr,"xwd: Getting target window information.\n");
#endif

    /* This doesn't seem to work for Pixmaps under R6
     * 
     */
     if(!win_info) {
	 XWindowAttributes attrib;
	 win_info = &attrib;
	 if( !XGetWindowAttributes(dpy, window, win_info)) {
	   fprintf(stderr, "XwuDumpWindow: Can't get target window attributes.\n");
	   return;
	 }
     }


    if( !XGetGeometry( dpy, window, 
		       &root, &x, &y, &width, &height, &border_width, &depth)) {
      fprintf(stderr, "XwuDumpWindow: Can't get target window attributes.\n");
      return;
    }

    /* XFetchName(dpy, window, &win_name); */
    if (!win_name || !win_name[0]) {
	win_name = "xwdump";
	got_win_name = 0;
    } else {
	got_win_name = 1;
    }

    /* sizeof(char) is included for the null string terminator. */
    win_name_size = strlen(win_name) + sizeof(char);

    /*
     * Snarf the pixmap with XGetImage.
     */

    image = XGetImage (dpy, window, x, y, width, height, AllPlanes, ZPixmap);
    if (!image) {
	fprintf (stderr, 
		 "%s:  unable to get image at %dx%d+%d+%d\n",
		 /* FIXME: program_name */ "unknown", 
		 width, height, x, y);
	exit (1);
    }

    /*
     * Determine the pixmap size.
     */
    buffer_size = Image_Size(image);

#ifdef DBG
    fprintf(stderr,"xwd: Getting Colors.\n");
#endif

    ncolors = Get_XColors( dpy, win_info, root, &colors);

    /*
     * Inform the user that the image has been retrieved.
     *
     * XBell(dpy, FEEP_VOLUME);
     * XBell(dpy, FEEP_VOLUME);
     */
    XFlush(dpy);

    /*
     * Calculate header size.
     */
#ifdef DBG
    fprintf(stderr,"xwd: Calculating header size.\n");
#endif
    header_size = SIZEOF(XWDheader) + win_name_size;

    /*
     * Write out header information.
     */
#ifdef DBG
    fprintf(stderr,"Constructing and dumping file header.");
#endif
    header.header_size = (CARD32) header_size;
    header.file_version = (CARD32) XWD_FILE_VERSION;
    header.pixmap_format = (CARD32) ZPixmap; /* FIXME: format; */
    header.pixmap_depth = (CARD32) image->depth;
    header.pixmap_width = (CARD32) image->width;
    header.pixmap_height = (CARD32) image->height;
    header.xoffset = (CARD32) image->xoffset;
    header.byte_order = (CARD32) image->byte_order;
    header.bitmap_unit = (CARD32) image->bitmap_unit;
    header.bitmap_bit_order = (CARD32) image->bitmap_bit_order;
    header.bitmap_pad = (CARD32) image->bitmap_pad;
    header.bits_per_pixel = (CARD32) image->bits_per_pixel;
    header.bytes_per_line = (CARD32) image->bytes_per_line;

    header.visual_class = (CARD32) win_info->visual->class;
    header.red_mask = (CARD32) win_info->visual->red_mask;
    header.green_mask = (CARD32) win_info->visual->green_mask;
    header.blue_mask = (CARD32) win_info->visual->blue_mask;
    header.bits_per_rgb = (CARD32) win_info->visual->bits_per_rgb;
    header.colormap_entries = (CARD32) win_info->visual->map_entries;
    header.ncolors = ncolors;
    header.window_width = (CARD32) width;
    header.window_height = (CARD32) height;
    header.window_x = x;
    header.window_y = y;
    header.window_bdrwidth = (CARD32)0;

    if (*(char *) &swaptest) {
	_swaplong((char *) &header, sizeof(header));
	for (i = 0; i < ncolors; i++) {
	    _swaplong((char *) &colors[i].pixel, sizeof(long));
	    _swapshort((char *) &colors[i].red, 3 * sizeof(short));
	}
    }

    if (fwrite((char *)&header, SIZEOF(XWDheader), 1, out) != 1 ||
	fwrite(win_name, win_name_size, 1, out) != 1) {
	perror("xwd");
	exit(1);
    }

    /*
     * Write out the color maps, if any
     */

#ifdef DBG
    fprintf(stderr,"xwd: Dumping %d colors.\n", ncolors));
#endif

    for (i = 0; i < ncolors; i++) {
	xwdcolor.pixel = colors[i].pixel;
	xwdcolor.red = colors[i].red;
	xwdcolor.green = colors[i].green;
	xwdcolor.blue = colors[i].blue;
	xwdcolor.flags = colors[i].flags;
	if (fwrite((char *) &xwdcolor, SIZEOF(XWDColor), 1, out) != 1) {
	    perror("xwd");
	    exit(1);
	}
    }

    /*
     * Write out the buffer.
     */
#ifdef DBG
    fprintf(stderr,"Dumping pixmap.  bufsize=%d\n",buffer_size);
#endif

    /*
     *    This copying of the bit stream (data) to a file is to be replaced
     *  by an Xlib call which hasn't been written yet.  It is not clear
     *  what other functions of xwd will be taken over by this (as yet)
     *  non-existant X function.
     */
    if (fwrite(image->data, (int) buffer_size, 1, out) != 1) {
	perror("xwd");
	exit(1);
    }

    /*
     * free the color buffer.
     */

    if(ncolors > 0) { 
#ifdef DBG
	fprintf(stderr,"Freeing colors.\n");
#endif
	free(colors);
    }

    /*
     * Free window name string.
     */
#ifdef DBG
    fprintf(stderr,"Freeing window name string.");
#endif
    if (got_win_name) XFree(win_name);

    /*
     * Free image
     */
    XDestroyImage(image);
}


/*******************************************************************
 * STATIC FUNCTIONS
 *******************************************************************/

/*******************************************************************
 * Image_Size(): Determine the pixmap size.
 */

static int Image_Size(XImage *image)
{
  if (image->format != ZPixmap)
    return(image->bytes_per_line * image->height * image->depth);

  return(image->bytes_per_line * image->height);
}

#define lowbit(x) ((x) & (~(x) + 1))

/*******************************************************************
 * Get_XColors(): Get the XColors of all pixels in image -
 *                returns # of colors
 */

static int Get_XColors(Display * dpy, 
		       XWindowAttributes *win_info,
		       Window root, 
		       XColor **colors)  /* RETURN */
{
  int i, ncolors = 0;
  Colormap cmap = win_info->colormap; 
  Visual * visual = win_info->visual;

  /* FIXME */
  if (0)
  {
    /* assume the visual will be OK ... */
    cmap = XListInstalledColormaps(dpy, root, &i)[0];
  }

  if (!cmap)
  {
    return(0);
  }

  ncolors =  visual->map_entries;
  *colors = (XColor *) calloc(1,sizeof(XColor) * ncolors);


  if ( visual->class == DirectColor ||
      visual->class == TrueColor)
  {
    Pixel red, green, blue, red1, green1, blue1;

    red = green = blue = 0;
    red1 = lowbit( visual->red_mask);
    green1 = lowbit( visual->green_mask);
    blue1 = lowbit( visual->blue_mask);
    for (i=0; i<ncolors; i++)
    {
      (*colors)[i].pixel = red|green|blue;
      (*colors)[i].pad = 0;
      red += red1;
      if (red >  visual->red_mask)
	red = 0;
      green += green1;
      if (green >  visual->green_mask)
	green = 0;
      blue += blue1;
      if (blue >  visual->blue_mask)
	blue = 0;
    }
  }
  else
  {
    for (i=0; i<ncolors; i++)
    {
      (*colors)[i].pixel = i;
      (*colors)[i].pad = 0;
    }
  }

  XQueryColors(dpy, cmap, *colors, ncolors);
    
  return(ncolors);
}


/*********************************************************************
 * _swapshort()
 */

static int _swapshort(char *bp, unsigned n)
{
  char c;
  char *ep = bp + n;

  while (bp < ep)
  {
    c = *bp;
    *bp = *(bp + 1);
    bp++;
    *bp++ = c;
  }
  return n;
}


/*********************************************************************
 * _swaplong()
 */

static int _swaplong(char *bp, unsigned n)
{
  char c;
  char *ep = bp + n;
  char *sp;

  while (bp < ep)
  {
    sp = bp + 3;
    c = *sp;
    *sp = *bp;
    *bp++ = c;
    sp = bp + 1;
    c = *sp;
    *sp = *bp;
    *bp++ = c;
    bp += 2;
  }

  return n;
}

/* $XConsortium: xwd.c,v 1.62 94/04/17 20:23:57 rws Exp $ */

/* XwuDumpWindow: based on X11 xwd application, copyright below */

/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

/*
 * xwd.c MIT Project Athena, X Window system window raster image dumper.
 *
 * This program will dump a raster image of the contents of a window into a 
 * file for output on graphics printers or for other uses.
 *
 *  Author:	Tony Della Fera, DEC
 *		17-Jun-85
 * 
 *  Modification history:
 *
 *  11/14/86 Bill Wyatt, Smithsonian Astrophysical Observatory
 *    - Removed Z format option, changing it to an XY option. Monochrome 
 *      windows will always dump in XY format. Color windows will dump
 *      in Z format by default, but can be dumped in XY format with the
 *      -xy option.
 *
 *  11/18/86 Bill Wyatt
 *    - VERSION 6 is same as version 5 for monchrome. For colors, the 
 *      appropriate number of Color structs are dumped after the header,
 *      which has the number of colors (=0 for monochrome) in place of the
 *      V5 padding at the end. Up to 16-bit displays are supported. I
 *      don't yet know how 24- to 32-bit displays will be handled under
 *      the Version 11 protocol.
 *
 *  6/15/87 David Krikorian, MIT Project Athena
 *    - VERSION 7 runs under the X Version 11 servers, while the previous
 *      versions of xwd were are for X Version 10.  This version is based
 *      on xwd version 6, and should eventually have the same color
 *      abilities. (Xwd V7 has yet to be tested on a color machine, so
 *      all color-related code is commented out until color support
 *      becomes practical.)
 */

/*%
 *%    This is the format for commenting out color-related code until
 *%  color can be supported.
%*/


