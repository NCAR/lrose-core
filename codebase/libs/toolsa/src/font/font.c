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
/* font.c
   provides scalable stroked fonts, using fontname.binf files

   KEYWORD: Fonts, font, stroked text, scalable text

   Creation:    6/6/91 JCaron
   Modify:
	10/14/92: ANSI version
	03/25/93: SYSV port, clean up

*/

#if defined(IRIX5) || defined(IRIX4) /* SGI doesnt declare open() !! */
extern int  open(const char *, int, ...);
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/font.h>
#include <toolsa/str.h>
#include <toolsa/ansi.h>
#include <toolsa/mem.h>

#include <dataport/bigend.h>

#include "font_int.h"

#define SIGN( f) (f > 0 ? 1 : -1)

FONTstruct *FONTinit ( char *font_filename)
{
  char *file_buffer;
  int  buffer_size;
  char *start_chars;
  si32 *file_index;
  
  int i, j, stroke;
  int font_fd;
  struct stat statb;
  char 	*buffer_ptr;
  Font_header *font_hdr;
  FONTstruct  *font_struct;
  FONTchar    *char_struct;
  int jump_to_next;
  
  /* allocate a font structure */
  font_struct = (FONTstruct *)ucalloc(1, sizeof(FONTstruct));

  /* read entire file into memory */
  if ((font_fd = open(font_filename, 0)) < 0)
    return (NULL);

  fstat(font_fd, &statb);
  buffer_size = statb.st_size;

  file_buffer = (char *)umalloc(buffer_size);

  read(font_fd, file_buffer, buffer_size);
  close(font_fd);

  /*
   * Swap the data in the file header.  The data can be left in
   * the data buffer because the offsets are all okay and we just
   * use the data for reference.
   */

  buffer_ptr = file_buffer;
  font_hdr = (Font_header *) buffer_ptr;
  buffer_ptr += sizeof(Font_header);

  font_hdr->font_code = BE_to_si16(font_hdr->font_code);
  font_hdr->name_size = BE_to_si16(font_hdr->name_size);
  font_hdr->sf = BE_to_si16(font_hdr->sf);
  font_hdr->scale = BE_to_si16(font_hdr->scale);
  font_hdr->num_chr = BE_to_si16(font_hdr->num_chr);

  if (font_hdr->name_size < MAX_FONTNAME)
    memcpy(font_struct->name, buffer_ptr, font_hdr->name_size);
  else
    memcpy(font_struct->name, buffer_ptr, MAX_FONTNAME-1);
  
  font_struct->descender = 7; /* kludge */
  font_struct->height = font_hdr->scale;

  /*
   * Swap the character indices in the file buffer.  We will use
   * these for converting the file information to the internal
   * format.  The internal indices are indexes into the character
   * array while the file indices are offsets into the character
   * information.
   */
      
  buffer_ptr += font_hdr->name_size;
  file_index = (si32 *) buffer_ptr;

  BE_to_array_32(file_index, font_hdr->num_chr * sizeof(si32));

  /*
   * start_chars, descender, height
   */
      
  buffer_ptr += font_hdr->num_chr * sizeof(si32);
  start_chars = buffer_ptr;
      
  /*
   * swap each character.  Note that at this point we have to start
   * copying the data from the file buffer using memcpy because the
   * structures are not guaranteed to start on proper byte boundaries.
   */

  for (i = 0; i < font_hdr->num_chr; i++)
  {
    /*
     * Some chars reuse an index posn.
     * Check that this index has not been swapped before.
     */

    jump_to_next = 0;
    for (j = 0; j < i; j++)
    {
      if (file_index[j] == file_index[i])
      {
	font_struct->index[i] = font_struct->index[j];
	jump_to_next = 1;
	break;
      }
    } /* j */
    if (jump_to_next) {
      continue;
    }

    /* We have a new charcter to store - allocate space for it.
     * Right now we'll just allocate space for the header information.
     * Once we find out the number of strokes, we'll reallocate
     * the space. */

    char_struct = (FONTchar *)umalloc(sizeof(FONTchar));
    
    buffer_ptr = start_chars + file_index[i];

    memcpy(&char_struct->ascii_code, buffer_ptr, sizeof(si16));
    char_struct->ascii_code = BE_to_si16(char_struct->ascii_code);
    buffer_ptr += sizeof(si16);
    
    if (char_struct->ascii_code >= 0)    /* The character information is stored in
                                          * the short format */
    {
      /* The information is stored in bytes so we can just copy
       * them and no swapping is necessary. */

      char_struct->xnext = *buffer_ptr;
      buffer_ptr += 1;
      
      char_struct->ynext = *buffer_ptr;
      buffer_ptr += 1;
      
      char_struct->n_strokes = *buffer_ptr;
      buffer_ptr += 1;
      
      /* Skip the "spare" byte in the short header */
      buffer_ptr += 1;
      
      /* Now we know the size of the character, so reallocate the pointer */
      char_struct = (FONTchar *)urealloc(char_struct,
					 sizeof(FONTchar) +
					 ((char_struct->n_strokes - 1) *
					  sizeof(FONTstroke)));
      
      /* Now copy each of the strokes */
      for (stroke = 0; stroke < char_struct->n_strokes; stroke++)
      {
	char_struct->strokes[stroke].pen = *buffer_ptr;
	buffer_ptr += 1;
	
	char_struct->strokes[stroke].x = *buffer_ptr;
	buffer_ptr += 1;
	
	char_struct->strokes[stroke].y = *buffer_ptr;
	buffer_ptr += 1;
	
      } /* endfor - stroke */
      
    }
    else      /* The character information is stored in the long format */
    {
      memcpy(&char_struct->xnext, buffer_ptr, sizeof(si16));
      char_struct->xnext = BE_to_si16(char_struct->xnext);
      buffer_ptr += sizeof(si16);
      
      memcpy(&char_struct->ynext, buffer_ptr, sizeof(si16));
      char_struct->ynext = BE_to_si16(char_struct->ynext);
      buffer_ptr += sizeof(si16);
      
      memcpy(&char_struct->n_strokes, buffer_ptr, sizeof(si16));
      char_struct->n_strokes = BE_to_si16(char_struct->n_strokes);
      buffer_ptr += sizeof(si16);
      
      /* Now we know the size of the character, so reallocate the pointer */
      char_struct = (FONTchar *)urealloc(char_struct,
					 sizeof(FONTchar) +
					 ((char_struct->n_strokes - 1) *
					  sizeof(FONTstroke)));
      
      /* In this case, the stroke information is stored in the format that
       * we want, so we can just copy the data. */

      memcpy(char_struct->strokes, buffer_ptr,
	     char_struct->n_strokes * sizeof(FONTstroke));

      BE_to_array_16(&char_struct->strokes,
		     char_struct->n_strokes * sizeof(FONTstroke));
	  
    }

    /* Add the new character to the character array */

    font_struct->index[i] = font_struct->num_chars;
    font_struct->chars[font_struct->num_chars++] = char_struct;
    
  } /* i */

  return (font_struct);
}

void FONTfree(FONTstruct *font_struct)
{
  int i;
  
  for (i = 0; i < NUM_ASCII; i++)
    ufree(font_struct->chars[i]);
  
  ufree(font_struct);
}



int FONTextent(FONTstruct *font_struct, char *text, int *penups, int *xwidth)
/* this counts the number of strokes that are needed for text.
   It also returns the number of penups in the test, and the length */
{
  int		len, i, j, char_idx;
  int		count = 0, x_count = 0, up_count=0;
  FONTchar      *char_hdr;

  len = strlen(text);
  for (i = 0; i < len; i++)
  {
    char_idx = (int) text[i] - (int) ' ';

    char_hdr = font_struct->chars[font_struct->index[char_idx]];
    
    for (j = 0; j < char_hdr->n_strokes; j++)
    {
      if (char_hdr->strokes[j].pen == 1)
	up_count++;
    } /* loop over strokes in char */
    
    x_count += char_hdr->xnext;
    count += char_hdr->n_strokes;
  } /* loop over chars in text */

  *penups = up_count;
  *xwidth = x_count;
  return (count);
}


int FONTtext(FONTstruct *font_struct, char *text,
	     FONTstroke **stroke_array, int *stroke_array_len, 
	     int *height, int *width)
{
#define MAX_STROKES 1000
  int		len, i, j, char_idx;
  int		start_x, next;
  FONTchar	*char_hdr;
  static FONTstroke	static_stroke_array[MAX_STROKES];

  next = 0;
  start_x = 0;

  len = strlen(text);
  for (i = 0; i < len; i++)
  {
    char_idx = (int) text[i] - (int) ' ';

    char_hdr = font_struct->chars[font_struct->index[char_idx]];
    
    for (j = 0; j < char_hdr->n_strokes; j++)
    {
      static_stroke_array[next].pen = char_hdr->strokes[j].pen;
      static_stroke_array[next].y = char_hdr->strokes[j].y;
      static_stroke_array[next].x = start_x + char_hdr->strokes[j].x;
      next++;
    } /* loop over strokes in char */

    start_x += char_hdr->xnext;

  } /* loop over chars in text */

  *stroke_array = static_stroke_array;
  *stroke_array_len = next;
  *height = font_struct->height;
  *width = start_x;
  return (TRUE);
}
 
void FONTscale(FONTstroke *stroke_array, int stroke_array_len,
	       int height, int width, int scaled_height,
	       int scaled_width, int preserve_aspect)
/* Scale the stroked text array to fit into a box scaled_height x scaled_width.
   stroke_array, stroke_len, height, width are output from FONTtext.
   Call this before you call FONToffset.
   */
{
  int 	i;
  double	xscale, yscale, pscale;

  xscale = (width == 0) ? 1.0 : (double) scaled_width / width;
  yscale = (height == 0) ? 1.0 : (double) scaled_height / height;

  if (preserve_aspect)
  {
    pscale = MIN( fabs(xscale), fabs(yscale));
    xscale = pscale * SIGN(xscale);
    yscale = pscale * SIGN(yscale);
  }

  for (i = 0; i< stroke_array_len; i++)
  {
    stroke_array[i].x = (short) (stroke_array[i].x * xscale);
    stroke_array[i].y = (short) (stroke_array[i].y * yscale);
  }
}

void FONToffset(FONTstroke *stroke_array, int stroke_array_len,
		int offset_x, int offset_y)
/* Add a constant offset to a stroked text array. 
   stroke_array, stroke_len are output from FONTtext.
   Call this after you call FONTscale.
   */
{
  int 	i;
  for (i = 0; i< stroke_array_len; i++)
  {
    stroke_array[i].x += offset_x;
    stroke_array[i].y += offset_y;
  }
}

#ifdef TEST

#ifdef 0

void Dump (buff)
char *buff;
{
  int 	i, j, *index;
  char 	*ptr, *start;
  Char_header char_hdr;
  Stroke	  *stroke;
  char     	asciic[2];

  ptr = buff;
  font_hdr = (Font_header *) ptr;
  ptr += sizeof(Font_header);

  printf("check %s font %d scale %d #chars %d\n",
	 font_hdr->check, font_hdr->font_code, font_hdr->scale, font_hdr->num_chr);

  printf("name <%s>\n", ptr);
  ptr += font_hdr->name_size;

  index = (int *) ptr;
  for (i=0; i< NUM_ASCII; i++)
    printf("%d %d\n", i, index[i]);
  ptr += NUM_ASCII * sizeof(int);
  printf("\n");
  start = ptr;

  for (i=0; i < font_hdr->num_chr; i++)
  {
    STRcopy(&char_hdr, ptr, sizeof(Char_header));
    asciic[0] = (char) char_hdr.asc_cod;
    asciic[1] = EOS;
    printf("%s index %d ascii %d xnext %d ynext %d # strokes %d\n",
	   asciic, ptr - start, char_hdr.asc_cod, char_hdr.xnext, char_hdr.ynext, char_hdr.strokes);
    ptr += sizeof( Char_header);
    
    for (j=0; j < char_hdr.strokes; j++)
    {
      stroke = (Stroke *) ptr;
      printf("  pen %d x %d y %d\n", stroke->pen, stroke->x, stroke->y);
      ptr += sizeof( Stroke);
    }
  }

}

#endif

int main (argc, argv)
int	argc;
char *argv[];
{
  int stroke_array_len, height, width, i, dummy;
  FONTstroke *stroke_array;

  FONTstruct *current_font;

  current_font = FONTinit("roman_simplex.binf");

/*  Dump( current_font->buffer);  */

  FONTtext(current_font, "ABC",
	   &stroke_array, &stroke_array_len, &height, &width);
  printf("len %d height %d width %d\n", stroke_array_len, height, width);
  for (i = 0; i < stroke_array_len; i++)
    printf(" pen %d x %5d y %5d\n",
	   stroke_array[i].pen, stroke_array[i].x, stroke_array[i].y);
    
  FONTfree (current_font);
  return (0);
}

#endif
