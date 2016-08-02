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
/**********************************************************************
 * uparse_path: parses file path, returns the basic elements
 *
 * dir - directory including trailing '/'
 * name - file name or directory name
 * base - base of file name
 * ext - file name extension including leading '.'
 *
 * NULL is returned for any portion which does not exist
 *
 * utility routine
 *
 * Mike Dixon CADSWES CU July 1990
 *
 **********************************************************************/

#include <toolsa/umisc.h>

void uparse_path(const char *path, path_parts_t *parts)

{
  int pathlen = strlen(path) + 1;
  int namelen, extlen;
  char *ext, *name;

  parts->dir = NULL;
  parts->name = NULL;
  parts->base = NULL;
  parts->ext = NULL;

  if(!strcmp(path, "..")) {
    parts->dir = (char *) umalloc(3);
    strcpy(parts->dir, "..");
    return;
  }
  
  if(!strcmp(path, ".")) {
    parts->dir = (char *) umalloc(2);
    strcpy(parts->dir, ".");
    return;
  }

  if(path[strlen(path) -1] == '/') {
    parts->dir = (char *) umalloc((unsigned) pathlen);
    strcpy(parts->dir, path);
    return;
  }
  
  if ( strrchr(path,'/') == NULL ) {
    namelen = pathlen; 
    parts->name = (char *)umalloc((unsigned) pathlen);
    strcpy(parts->name, path); 
    parts->dir = (char *)umalloc(3); 
    strcpy(parts->dir, "./");
  } else {
    name = strrchr(path,'/') + 1;
    namelen = strlen(name) + 1;
    if ( namelen == 1) { 
    } else {
      parts->name = (char *)umalloc((unsigned) namelen);
      strcpy(parts->name, name);
    }
    parts->dir = (char *)
      umalloc((unsigned) (pathlen - namelen + 1));
    (void)strncpy(parts->dir, path, pathlen - namelen);
    parts->dir[pathlen - namelen] = '\0';
    if ( parts->name == NULL ) return;
  }
  if ( !strcmp(parts->name,"..") ) {
    return;
  }
  if ( !strcmp(parts->name,".") ) {
    return;
  }
  if ( strrchr(parts->name,'.') == NULL ) {
    extlen = 0;
  } else {
    ext = strrchr(parts->name,'.');
    extlen = strlen(ext) + 1;
    parts->ext = (char *)umalloc((unsigned) extlen);
    strcpy(parts->ext, ext);
  }
  if ( namelen == extlen ) {
  } else {
    parts->base = (char *)
      umalloc((unsigned) (namelen - extlen + 1));
    strncpy(parts->base, parts->name, namelen - extlen);
    parts->base[namelen - extlen] = '\0';
  }
  
  return;
  
}

