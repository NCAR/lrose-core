#ifndef _FILEUTIL_H_
#define _FILEUTIL_H_
int CreateDirectory(char *dirname, char separator = '\\');
int DirExists(char *dirname, int mustbewriteable = 0, int createdir = 0);
int FileExists(char *fname, int mustbewriteable = 0, int deleteflagfile = 0);
#endif