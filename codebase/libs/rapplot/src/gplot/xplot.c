/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*********************************************************************
 * xutil.c
 *
 * X graphics utility routines - used by gutil.c routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * December 1991
 *
 *********************************************************************/

#include <rapplot/gplot.h>

/*********************************************************************
 * xFreeColorList()
 *
 * Frees up resources in the color list
 *
 *********************************************************************/

void xFreeColorList(Display *display,
		    Colormap cmap,
		    x_color_list_index_t *list_index)

{

  x_color_list_t *this_entry, *next_entry;

  if (list_index->n_entries == 0) {

    list_index->first_entry = NULL;
    return;

  } else {

    this_entry = list_index->first_entry;

    while (this_entry != NULL) {

      next_entry = this_entry->next;

      XFreeGC (display, this_entry->gc);

      if (this_entry->duplicate == FALSE)
	XFreeColors(display, cmap, &this_entry->x_color.pixel, 1, 0L);

      ufree(this_entry->colorname);
      ufree((char *) this_entry);
      
      this_entry = next_entry;

    } /* while */

  } /* if */

}
/*********************************************************************
 * xFreeFont(): frees an X font
 *
 *********************************************************************/

void xFreeFont(Display *display, XFontStruct *font)

{

  if (font != NULL)
    XFreeFont(display, font);

}

/*********************************************************************
 * xFreeGC(): frees a GC
 *
 *********************************************************************/

void xFreeGC(Display *display, GC gc)

{

  if (gc != 0)
    XFreeGC(display, gc);

}

/*********************************************************************
 * xGetColorGC()
 *
 * Returns a GC with the color field set according to the colorname
 * specified.
 *
 * This routine uses a linked list, which is pointed to by the list_index
 * argument. The list_index struct is set up by the calling program.
 *
 * If the color has been previously asked for, the relevant entry in the
 * linked list is used to provide the GC. Otherwise a new entry
 * is created.
 *
 * Returns the GC (which is a pointer) if success, NULL is failure.
 *
 *********************************************************************/

GC xGetColorGC(Display *display,
	       Colormap cmap,
	       x_color_list_index_t *list_index,
	       const char *colorname)

{

  int pixval;
  int get_next;
  x_color_list_t *new_entry, *this_entry;

  if (list_index->n_entries == 0) {

    new_entry = (x_color_list_t *)
      umalloc (sizeof(x_color_list_t));

    list_index->first_entry = new_entry;

  } else {

    this_entry = list_index->first_entry;

    get_next = TRUE;

    while (get_next) {

      if (!strcmp(colorname, this_entry->colorname))
	return(this_entry->gc);

      if (this_entry->next == NULL) {

	get_next = FALSE;

      } else {

	this_entry = this_entry->next;

      } /* if (this_entry->next == NULL) */

    } /* while */

    new_entry = (x_color_list_t *)
      ucalloc (1, sizeof(x_color_list_t));

    this_entry->next = new_entry;

  } /* if (list_index ..... */

  list_index->n_entries++;
  new_entry->next = NULL;
  new_entry->duplicate = FALSE;
  new_entry->colorname = (char *)
    umalloc (strlen(colorname) + 1);
  strcpy(new_entry->colorname, colorname);

  if (XParseColor(display, cmap, colorname, &new_entry->x_color) == 0) {

    fprintf(stderr, "ERROR - XParseColor\n");
    fprintf(stderr, "Cannot match color '%s'\n", colorname);
    return(NULL);

  }

  XAllocColor(display, cmap, &new_entry->x_color);

  new_entry->gc = XCreateGC(display, DefaultRootWindow(display),
			     0, 0);
  XSetForeground(display, new_entry->gc, new_entry->x_color.pixel);

  /*
   * search through the list to check whether the same pixel value has
   * been allocated to a previous entry - this sometimes happens
   * if the server has run out of color cells. If there is a duplicate
   * entry, set the duplicate flag in the new entry
   */

  pixval = new_entry->x_color.pixel;

  this_entry = list_index->first_entry;

  while (this_entry->next != NULL) {

    if (this_entry->x_color.pixel == pixval) {
      new_entry->duplicate = TRUE;
      break;
    }

    this_entry = this_entry->next;

  } /* while */

  return(new_entry->gc);

}
/**************************************************************************
 * xGetResDouble(): gets a double X from
 *                    (first) X resources default
 *                    (then)  param file and
 *                    (finally) from hard-coded default
 *
 *************************************************************************/

double xGetResDouble(Display *display,
		     const char *name,
		     const char *res_string,
		     double hard_def)

{

  double paramvalue;
  char *paramstr, *end_pt;

  
  if (display == NULL) {
    paramvalue = uGetParamDouble(name, res_string, hard_def);
  } else if ((paramstr = XGetDefault(display, name,
				     res_string)) == NULL) {
    paramvalue = uGetParamDouble(name, res_string, hard_def);
  } else {
    errno = 0;
    paramvalue = strtod(paramstr, &end_pt);
    if(errno != 0)
      paramvalue = uGetParamDouble(name, res_string, hard_def);
  }

  return paramvalue;

}

/***************************************************************************
 * xGetResLong(): gets a long from :
 *                    (first) X resources default
 *                    (then)  param file and
 *                    (finally) from hard-coded default
 *
 ***************************************************************************/

long xGetResLong(Display *display,
		 const char *name,
		 const char *res_string,
		 long hard_def)

{

  long paramvalue;
  char *paramstr, *end_pt;

  if (display == NULL) {
    paramvalue = uGetParamLong(name, res_string, hard_def);
  } else if ((paramstr = XGetDefault(display, name,
				     res_string)) == NULL) {
    paramvalue = uGetParamLong(name, res_string, hard_def);
  } else {
    errno = 0;
    paramvalue = strtol(paramstr, &end_pt, 10);
    if(errno != 0)
      paramvalue = uGetParamLong(name, res_string, hard_def);
  }

  return paramvalue;

}
/***************************************************************************
 * xGetResString(): gets a string from
 *                    (first) X resources default
 *                    (then)  param file and
 *                    (finally) from hard-coded default
 *
 **************************************************************************/

char *xGetResString(Display *display,
		    const char *name,
		    const char *res_string,
		    const char *hard_def)

{

  char *paramstr;

  if (display == NULL) {
    paramstr = uGetParamString(name, res_string, hard_def);
  } else if ((paramstr = XGetDefault(display, name,
				     res_string)) == NULL) {
    paramstr = uGetParamString(name, res_string, hard_def);
  }

  return paramstr;

}

/*********************************************************************
 * xGetXColor()
 *
 * This routine uses a linked list, which is pointed to by the list_index
 * argument. The list_index struct is set up by the calling program.
 *
 * If the color has been previously asked for, the relevant entry in the
 * linked list is used to provide the XColor. Otherwise a new entry
 * is created.
 *
 * Returns pointer to an XColor struct if success, NULL if failure.
 *
 *********************************************************************/

#include <string.h>

XColor *xGetXColor(Display *display,
		   Colormap cmap,
		   x_color_list_index_t *list_index,
		   const char *colorname)

{

  int pixval;
  int get_next;
  x_color_list_t *new_entry, *this_entry;

  if (list_index->n_entries == 0) {

    new_entry = (x_color_list_t *)
      umalloc (sizeof(x_color_list_t));

    list_index->first_entry = new_entry;

  } else {

    this_entry = list_index->first_entry;

    get_next = TRUE;

    while (get_next) {

      if (!strcmp(colorname, this_entry->colorname))
	return(&this_entry->x_color);

      if (this_entry->next == NULL) {
	get_next = FALSE;
      } else {
	this_entry = this_entry->next;
      }

    } /* while */

    new_entry = (x_color_list_t *)
      umalloc (sizeof(x_color_list_t));

    this_entry->next = new_entry;

  } /* if (list_index ..... */

  list_index->n_entries++;
  new_entry->next = NULL;
  new_entry->duplicate = FALSE;
  new_entry->colorname = (char *)
    umalloc (strlen(colorname) + 1);
  strcpy(new_entry->colorname, colorname);

  if (XParseColor(display, cmap, colorname, &new_entry->x_color) == 0) {

    fprintf(stderr, "ERROR - xGetXColor\n");
    fprintf(stderr, "Cannot match color '%s'\n", colorname);
    return(NULL);

  }

  XAllocColor(display, cmap, &new_entry->x_color);

  new_entry->gc = XCreateGC(display, DefaultRootWindow(display),
			     0, 0);
  XSetForeground(display, new_entry->gc, new_entry->x_color.pixel);

  /*
   * search through the list to check whether the same pixel value has
   * been allocated to a previous entry - this sometimes happens
   * if the server has run out of color cells. If there is a duplicate
   * entry, set the duplicate flag in the new entry
   */

  pixval = new_entry->x_color.pixel;

  this_entry = list_index->first_entry;

  while (this_entry->next != NULL) {

    if (this_entry->x_color.pixel == pixval) {
      new_entry->duplicate = TRUE;
      break;
    }

    this_entry = this_entry->next;

  } /* while */

  return(&new_entry->x_color);

}
/*********************************************************************
 * xLoadFont(): loads an X font, returns the font struct pointer
 *
 *********************************************************************/

XFontStruct *xLoadFont(Display *display, const char *fontname)

{

  XFontStruct *font;
  
  if((font = XLoadQueryFont(display, fontname)) == 0) {
    if((font = XLoadQueryFont(display, "fixed")) == 0) {
      fprintf(stderr, "ERROR - xLoadFont\n");
      fprintf(stderr, "Cannot find  font '%s'\n", fontname);
      fprintf(stderr, "Cannot find fixed font.\n");
      exit(1);
    } else {
      fprintf(stderr, "WARNING - xLoadFont\n");
      fprintf(stderr, "Cannot find  font '%s'\n", fontname);
      fprintf(stderr, "Using fixed font.\n");
    }
  }

  return font;

}

