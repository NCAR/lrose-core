C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
C    ** Copyright UCAR (c) 1992 - 2010 
C    ** University Corporation for Atmospheric Research(UCAR) 
C    ** National Center for Atmospheric Research(NCAR) 
C    ** Research Applications Laboratory(RAL) 
C    ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
C    ** 2010/10/7 23:12:29 
C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
       SUBROUTINE XMOVEX(OUT,IN,IBYTES)
C
C      THIS SUBROUTINE MAY NOT BE NEEDED, ITS WAS IN
C      ASSEMBLER LANGUAGE TO MOVE DATA, IT RAN ABOUT THREE
C      TIMES FASTER THAN A FORTAN DO LOOP, IT WAS USED TO
C      MAKE SURE THE DATA TO BE UNPACKED WAS ON A WORD BOUNDARY,
C      THIS MAY NOT BE NEEDED ON SOME BRANDS OF COMPUTERS.
C
       CHARACTER*1 OUT(*)
       CHARACTER*1 IN(*)
C
       INTEGER*4   IBYTES
C
       SAVE
C
       DO 100 I = 1,IBYTES
         OUT(I) = IN(I)
  100  CONTINUE
C
       RETURN
       END
