C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
C    ** Copyright UCAR (c) 1992 - 2010 
C    ** University Corporation for Atmospheric Research(UCAR) 
C    ** National Center for Atmospheric Research(NCAR) 
C    ** Research Applications Laboratory(RAL) 
C    ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
C    ** 2010/10/7 23:12:29 
C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
      SUBROUTINE XSTORE(BUFFER,VALUE,LEN)
C
C     THIS SUBROUTINE MAY NOT BE NEEDED.  I COPIED IT FROM SOMEWHERE
C     ON THE WEB SO THAT MY C++ PROGRAM WOULD LINK WITH THE GRIB
C     LIBRARY.  HOPEFULLY, THIS DOES THE RIGHT THING.
C
      CHARACTER*1 BUFFER(*)
      CHARACTER*1 VALUE
      INTEGER*4   LEN

      DO 1 I=1,LEN
1     BUFFER(I)=VALUE
      RETURN
      END
