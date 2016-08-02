/*
 * bmp.h
 * 
 * bmp format file read/write routines
 * 
 */
 
 #define SWAPBYTES		/* define if byte order swap needed, */
				/* needed for SGI. not for PC */

typedef unsigned short	UINT;	/* 16bit unsigned */
typedef unsigned long	DWORD;	/* 32bit unsigned */
typedef unsigned char	BYTE;	/*  8bit unsigned */
typedef long		LONG;	/* 32bit   signed */
typedef unsigned short	WORD;	/* 16bit unsigned */

typedef struct tagBITMAPFILEHEADER {    /* bmfh */
    UINT    bfType;
    DWORD   bfSize;
    UINT    bfReserved1;
    UINT    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;
  
typedef struct tagBITMAPINFOHEADER {    /* bmih */
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;
  
typedef struct tagRGBQUAD {     /* rgbq */
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {  /* bmi */
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
  
typedef struct tagBITMAP {  /* bm */
    BITMAPINFO	        *bmInfo;
    char		*bmData;
} BITMAP;
  
/* read_bmpfile allocates the bmpinfo and data structures */
/* required to read the bmp file as the size of both of these */
/* is determined by the file */
/* returns 0 if successful, -1 if failed */

int read_bmpfile(char *filename,  BITMAP *bitmap);
