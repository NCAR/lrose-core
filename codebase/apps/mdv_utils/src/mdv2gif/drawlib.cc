// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DRAWLIB_MAIN
#include "drawlib.hh"

static void rect_to_polar (double xdiff, double ydiff, double *angle, double *radius);
static void draw10(double x1,double yy,double rigt,int vlag);

void SetLineType(int type)
{
  curlinetype = type;
}

void SetLineThick(int type)
{
  curlinethick = type;
}
void FillRect (int x1,int y1, int x2, int y2)
{
	int x,y,t;
	int savethick = curlinethick;
	curlinethick = 0;
        if (y2 < y1)
        {
          t = y1;
          y1 = y2;
          y2 = t; 
        } 
        if (x2 < x1)
        {
          t = x1;
          x1 = x2;
          x2 = t; 
        } 

	for (y=y1;y<=y2;++y)
		for (x = x1;x<=x2;++x)
	    {
		  aanbis(x,y);
	    }
	curlinethick = savethick;
}
void DrawRect (int x1,int y1, int x2, int y2)
{
	DrawLine(x1,y1,x2,y1);
	DrawLine(x2,y1,x2,y2);
	DrawLine(x2,y2,x1,y2);
	DrawLine(x1,y2,x1,y1);
}
void SetColor(int r,int g, int b)
{	
  curColor.R = (char)r;
  curColor.G = (char)g;
  curColor.B = (char)b;
}
void InitImage(int cx, int cy)
{
  XSIZE = cx;
  YSIZE = cy;
  Rfield = (char *)malloc (XSIZE*YSIZE);
  Gfield = (char *)malloc (XSIZE*YSIZE);
  Bfield = (char *)malloc (XSIZE*YSIZE);
  memset (Rfield,255,XSIZE*YSIZE);
  memset (Gfield,255,XSIZE*YSIZE);
  memset (Bfield,255,XSIZE*YSIZE);
}
int GetXSize()
{
  return (XSIZE);
}
int GetYSize()
{
  return (YSIZE);
}

void aanbis(int x, int y)
{
  if (curlinethick == 0)
  {
    if ((x>=0) && (x < XSIZE) && (y>= 0) && ( y<YSIZE))
    {
		if (Drawmode == SETmode)
		{
           Rfield[y*XSIZE + x] = curColor.R ;
           Gfield[y*XSIZE + x] = curColor.G ;
           Bfield[y*XSIZE + x] = curColor.B ;
		}
		else
		{
		   Rfield[y*XSIZE + x] |= curColor.R ;
           Gfield[y*XSIZE + x] |= curColor.G ;
           Bfield[y*XSIZE + x] |= curColor.B ;
		}
    }
  }
  else
  {
	  int x1,y1,x2,y2;
	  int xt,yt;
	  x1 = x-1; x2 = x+1;
	  y1 = y-1; y2 = y+1;
	  for (yt=y1;yt<=y2;++yt)
		for (xt = x1;xt<=x2;++xt)
	    {
		  if ((xt>=0) && (xt < XSIZE) && (yt>= 0) && ( yt<YSIZE))
			{
			  if (Drawmode == SETmode)
			  {
				Rfield[yt*XSIZE + xt] = curColor.R ;
				Gfield[yt*XSIZE + xt] = curColor.G ;
			    Bfield[yt*XSIZE + xt] = curColor.B ;
			  }
			  else
			  {
				Rfield[yt*XSIZE + xt] |= curColor.R ;
				Gfield[yt*XSIZE + xt] |= curColor.G ;
			    Bfield[yt*XSIZE + xt] |= curColor.B ;
			  }
			}
	    }
  }
}


void DrawLine(int x1,int y1,int x2,int y2)
{
        double hel,ord;
        int klas , mx , y , x , inc;
        int tipe = curlinetype;
        inc = 1;
        if((x1 == x2) && (y1 == y2)) return;
        if(x1 == x2)
        {  /*  helling is oneindig hier */
          klas=0  ;
          mx= (y1 > y2 ) ? y1:y2;
          y= (y1 > y2 ) ? y2:y1;
          while (y <= mx)
          {
            aanbis(x1,y);
            if (tipe == 2)
            {
              aanbis(x1,y-1);
              aanbis(x1-1,y);
              aanbis(x1-1,y-1);
            }  
            y = y+inc;
          }
        }
        else
        {
           hel=(1.0 * (y2-y1) )/(x2-x1);
           ord=y1 - hel * x1;
           if( (hel >=-1.0) && (hel <= 1.0))
           {
              klas=1; /* inkrementeer x */
              mx=(x1 > x2) ? x1:x2;
              x= (x1 > x2) ? x2:x1;
              while(x <= mx)
              {
                 y= (int)(hel * x + ord + 0.5);
                 aanbis(x,y);
                 if (tipe == 2)
                 {
                   aanbis(x,y-1);
                   aanbis(x-1,y);
                   aanbis(x-1,y-1);
                 }  
                 x=x+inc;
              }
           }
           else
           {
              klas=2;
              mx=(y1 > y2) ? y1:y2;
              y =(y1 > y2) ? y2:y1;
              while (y<= mx)
              {   
                x= (int)((y-ord)/hel  + 0.5);
                aanbis(x,y);
                 if (tipe == 2)
                 {
                   aanbis(x,y-1);
                   aanbis(x-1,y);
                   aanbis(x-1,y-1);
                 }  
                y=y+inc;
              }
            }
          }
}      



void pltsym(int x0,int y0,char *s,double sf,double rotation,int b)
{
        int i ,j , k , ipen , x1 , x2 , y1 , y2, inc ,yoors ;
		int xbase,ybase;
        char *pt ;
		int len;
		int rot = 0;
		len = strlen (s);
        inc = (b == 1) ? 6 : 7 ;
        i=0;
		xbase = x0;
		ybase = y0;

        while( ( ( j= s[i] ) != 0) && (i < len))
                {
                        pt= (b==0) ? sym_def(s[i]) : sym_defb(s[i]) ;
                /*      PLOT DIE STRING */
                        ipen=0;
                        yoors = 0;
                        k=0;
                        while( (pt[k]!='7') || (pt[k+1]!= '7'))
                        {
                            if( pt[k] == '8') yoors=pt[k+1] - 48; /* nuwe y-oorsprong */
                                else
                                    {
				      x2= (rot==0) ?
					(int) (x0 + (pt[k]-48) *sf)
					: (int) (x0-(pt[k+1]-48+yoors)*sf);
				      y2=(rot==0) ?
					(int) (y0 + (0-pt[k+1]+48-yoors) *sf)
					: (int) (y0 + (0- pt[k]+48) *sf);
                                       if(pt[k]=='6'&&pt[k+1]=='7')ipen=-1;
                                       if (ipen > 0) DrawLine(x1,y1,x2,y2);
                                       x1=x2;
                                       y1=y2;
                                       ipen +=1;
                                     }
                              k +=2;
                        }
                        i=i+1;
                        x0 = (rot==0) ? (int)(( x0 + inc *sf )) : x0  ;
                        y0 = (rot==0) ? y0 : (int)(( y0 +inc *sf))  ;

                }
}



void pltsymrot(int x0,int y0,char *s,double sf,double rotation,int b)
{
        int i ,j , k , ipen ,inc ,yoors ;
		int rx1,rx2,ry1,ry2;
		int xbase,ybase;
        double x1,x2,y1,y2;
        char *pt ;
		int len;
		int rot = 0;
		len = strlen (s);
        inc = (b == 1) ? 6 : 7 ;
        i=0;
		xbase = x0;
		ybase = y0;

        while( ( ( j= s[i] ) != 0) && (i < len))
                {
                        pt= (b==0) ? sym_def(s[i]) : sym_defb(s[i]) ;
                /*      PLOT DIE STRING */
                        ipen=0;
                        yoors = 0;
                        k=0;
                        while( (pt[k]!='7') || (pt[k+1]!= '7'))
                        {
                            if( pt[k] == '8') yoors=pt[k+1] - 48; /* nuwe y-oorsprong */
                                else
                                    {
                                       x2= (rot==0) ? (x0 + (pt[k]-48) *sf)
                                                  : (x0-(pt[k+1]-48+yoors)*sf);
                                       y2= (rot==0) ? (y0 + (0-pt[k+1]+48-yoors) *sf)
                                                  : (y0 + (0-pt[k]+48) *sf) ;
                                       if(pt[k]=='6'&&pt[k+1]=='7')ipen=-1;
                                       if (ipen > 0) 
	  			       {
					   double angle,radius;
		                           rect_to_polar((double)(x1-xbase),(double)(y1-ybase),&angle,&radius);
		                           angle += (3.141592654/2.0) + rotation ;
                                           rx1 = (long)((radius * sin(angle))+0.5);
	                                   ry1 = 0- (long)((radius * cos(angle))-0.5);
	                                   rx1 = rx1 + xbase;
	                                   ry1 = ry1 + ybase;
		                           rect_to_polar((double)(x2-xbase),(double)(y2-ybase),&angle,&radius);
		                           angle += (3.141592654/2.0)+rotation;
                                           rx2 = (long)((radius * sin(angle))+0.5);
	                                   ry2 = 0-(long)((radius * cos(angle))-0.5);
	                                   rx2 = rx2 + xbase;
	                                   ry2 = ry2 + ybase;
					   DrawLine(rx1,ry1,rx2,ry2);
				       }
                                       x1=x2;
                                       y1=y2;
                                       ipen +=1;
                                     }
                              k +=2;
                        }
                        i=i+1;
                        x0 = (rot==0) ? (int)(( x0 + inc *sf )) : x0  ;
                        y0 = (rot==0) ? y0 : (int)(( y0 +inc *sf))  ;

                }
}

char *sym_def(char a)
{
        static char *punte[] =
        {
                "77",                                   /*  0 */
                "77",                                   /*   */
                "77",                                   /*  2 */
                "77",                                   /*   */
                "77",                                   /*  4 */
                "77",                                   /*   */
                "77",                                   /*  6 */
                "77",                                   /*   */
                "77",                                   /*  8 */
                "77",                                   /*   */
                "77",                                   /*  10 */
                "77",                                   /*   */
                "77",                                   /*  12 */
                "77",                                   /*   */
                "77",                                   /*  14 */
                "77",                                   /*   */
                "77",                                   /*  16 */
                "77",                                   /*   */
                "77",                                   /*  18 */
                "77",                                   /*   */
                "77",                                   /*   20*/
                "77",                                   /*  21 */
                "77",                                   /*   */
                "77",                                   /*  23 */
                "77",                                   /*   */
                "77",                                   /*  25 */
                "77",                                   /*   */
                "77",                                   /*  27 */
                "77",                                   /*  28 */
                "77",                                   /*   */
                "77",                                   /*  30 */
                "77",                                   /*   */
                "77",                                   /*  32 */
                "2030312167233337272377",               /* ! 33*/
                "14252717674455574777",                 /*   34 */
                "0246670343670545777",                  /* # 35*/
                "01314243341405164667202777",           /* $ */
                "061617070667005767405051414077",       /* % */
                "50050617273635020110305277",           /* & 38 */
                "2435372777",                           /* ' 39 */
                "2011162777",                           /* (  40 */
                "2031362777",                           /* ) 41 */
                "044467123667163277",                   /* *  42 */
                "044467222677",                         /* + 43 */
                "102122121121777",                      /* , 44*/
                "044477",                               /* - 45*/
                "1020211110777",                        /* . 46*/
                "004777",                               /* / 47*/
                "4700061737464130100177",               /* 0 48*/
                "16272067103077",                       /* 1 49*/
                "061737464501004077",                   /* 2 50*/
                "06173746453414673443413010017777",     /* 3 51 */
                "4101373077",                           /* 4 52 */
                "01103041433404074777",                 /* 5 53*/
                "03143443413010010617374677",           /* 6 54*/
                "0607471077",                           /* 7 55 */
                "1405061737464534140301103041433477",   /* 8 56*/
                "01103041463717060413334477",           /* 9 57*/
                "122223131267142425151477",             /* : 58*/
                "10212212112167132324141377",           /* ; 59*/
                "460442777",                            /* < 60*/
                "02426705457777",                       /* = 61*/
                "02440677",                             /* > 62*/
                "1516274756553332673130404177",         /* ? 63*/
                "615010010617576663523223243545545277", /* @ 64*/
                "00061737464067034377",                 /* A 65*/
                "043445463707003041433477",             /* B 66*/
                "413010010617374677",                   /* C 67*/
                "0007374641300077",                     /* D 68*/
                "4000074767043477",                     /* E 69*/
                "0007476704347777",                     /* F 70*/
                "33434130100106173747777",             /* G 71*/
                "070067044467474077",                   /* H */
                "173767272067103077",                   /* I */
                "01103041477777",                       /* J */
                "07006703476725407777",                 /* K */
                "0700407777",                           /* L */
                "00072447407777",                       /* M */
                "00074047777",                          /* N */
                "10010617374641301077",                 /* O */
                "0007374645340477",                     /* P */
                "2240463717060110304177",               /* Q */
                "0007374645340467344077",               /* R */
                "01103041433414050617374677",           /* S */
                "07476727207777",                       /* T */
                "070110304147407777",                   /* U */
                "072047777",                            /* V */
                "0700234047777",                        /* W */
                "0740670047777",                        /* X */
                "0724206747247777",                     /* Y */
                "0747004077"   ,                         /* Z 90 */
                "4727204077",                           /* [ 91 */
                "170577",                               /* \ 92 */
                "2747402077",                           /* ] 93 */
                "00376077",                             /* ^ 94 */
                "006077",                               /* - 95 */
                "244777",                               /* ' 96 */
                "041535444130100102133342415077",       /* a 97*/
                "0700011040515445150477",               /* b 98*/
                "544515040110405177",                   /* c */
                "5750514010010415455477",               /* d */
                "0353544515040110405177",               /* e */
                "571706050067053577",                   /* f */
                "8201104051564717060312425377",         /* g */
                "0700041545545077",                     /* h */
                "1535300050672636372777",               /* i */
                "163631201001672829393877",             /* j */
                "5023675502070077",                     /* k */
                "173730005077",                         /* l */
                "1510142535303545545077",               /* m */
                "1510142545545077",                     /* n */
                "04011040515445150477",                 /* o */
                "820617475653421203070077",             /* p */
                "825342120306174756505777",             /* q */
                "05000415455477",                       /* r */
                "5445150403525140100177",               /* s */
                "272130405167054577",                   /* t */
                "050110304145415077",                   /* u */
                "0520305577",                           /* v */
                "051025304577",                         /* w */
                "055067005577",                         /* x */
                "83072367571077",                         /* y */
                "0555005077",                            /* z 122*/
                "574736342313233231405077",             /* {123  */
                "373467323077",                         /* | 124 */
                "172736344353433231201077",             /* } 125 */
                "03142442526377"                        /* ~ 126 */
        };
                if(a > 126) a=0;
        return (punte[a]);
}



char *sym_defb(char a)
{
        static char *punte[] =
        {

                "77",                                 /* blank */
                "77",                                 /* ! */
                "77",                                 /* " */
                "77",                                 /* # */
                "77",                                 /* $ */
                "77",                                 /* % */
                "77",                                 /* & */
                "77",                                 /* ' */
                "77",                                 /* ( */
                "77",                                 /* ) */
                "77",                                 /* * */
                "024267242077",                       /* + */
                "77",                                 /* , */
                "024277",                             /* - */
                "202131302077",                       /* . */
                "77",                                 /* / */
                "44351504011030414477",               /* 0 */
                "04252067004077",                     /* 1 */
                "0304153544433210004077",             /* 2 */
                "054523424130100177",                 /* 3 */
                "4101353077",                         /* 4 */
                "45050333424130100177",               /* 5 */
                "4435150402011030414233130277",       /* 6 */
                "05450077",                           /* 7 */
                "334435150413334241301001021377",     /* 8 */
                "0110304143443515040312324377" ,      /* 9 */
                "77",                                 /* : */
                "77",                                 /* ; */
                "77",                                 /* < */
                "77",                                 /* = */
                "77",                                 /* > */
                "77",                                 /* ? */
                "77",                                 /* @ */
                "0025324067123277",                   /* A */
                "05030030414233033344350577",         /* B */
                "443515040110304177",                 /* C */
                "0500304144350577",                   /* D */
                "4505034303004077",                   /* E */
                "45050343030077",                     /* F */
                "4435150401103041422277",             /* G */
                "050067034367454077",                 /* H */
                "05452520400077",                     /* I */
                "15353120100177",                     /* J */
                "050067024567234077",                 /* K */
                "05004077",                           /* L */
                "0004152420672435444077",             /* M */
                "0005404577",                         /* N */
                "44351504011030414477",               /* O */
                "00053544330377",                     /* P */
                "44351504011030414467402277",         /* Q */
                "00053544433202224077",               /* R */
                "4435150413324130100177",             /* S */
                "0545252077",                         /* T */
                "0501103041404577",                   /* U */
                "05204577",                           /* V */
                "051023304577",                       /* W */
                "054067450077",                       /* X */
                "052367450077",                       /* Y */
                "0545004077"                         /* Z */
        };
        int j;
        j=a-32;
                if(a>90) j=0;
                if(j<0) j=0;
        return (punte[j]);
}

void rect_to_polar (double xdiff, double ydiff, double *angle, double *radius)
{
  double pidiv2 = 1.570796327;
  double piby2 = 6.283185307;
  *radius = sqrt(xdiff*xdiff + ydiff*ydiff);
  *angle  = 0.0;

  if (*radius > 0.0000000001)
  {
    if (fabs (xdiff) > 0.00000000001)
      *angle = ((ydiff >= 0)? atan2 (ydiff, xdiff): atan2(ydiff, xdiff) + piby2);
    else
      *angle = ((ydiff >= 0)? pidiv2: pidiv2*3.0);
  }
}



#define  HOEK -70./180.*3.141592654
void plowind(double x,double y,double rigtin,int spoed,int skag,int vlag)
{
  int  t,klnwnd,h,j,k,i;
  double rigt;
  double x1,yy,x2,y2,xt,yt;
  if (spoed<=2) return;
  rigt=rigtin;
  t=(int)(spoed/10.);
  t=t*10;
  klnwnd=spoed-t;
  spoed=t;
  if (klnwnd>7) spoed=spoed+10;
  rigt=(rigt)/180.*3.141592654;
  h=0;
  k=0;
  while (spoed >= 50)
  {
        h=h+1;
        spoed=spoed-50;
        if (spoed>47 && spoed<50) spoed =50;
  }
  if (klnwnd>=3 && klnwnd<=7)
  {
        if (spoed<10)
        {
                x1=.5+x+sin(rigt)*(skag+k);
                yy=.5+y+(0-cos(rigt)*(skag+k));
                xt=.5+x+sin(rigt);
                yt=.5+y+(0-cos(rigt));
                DrawLine((int)xt,(int)yt,(int)x1,(int)yy);
        }
        x1=.5+x+sin(rigt)*(skag+k);
        yy=.5+y+(0-cos(rigt)*(skag+k));
        x2=.5+x1+sin(rigt+HOEK)*(vlag/2.);
        y2=.5+yy+(0-cos(rigt+HOEK)*(vlag/2.));
        DrawLine((int)x1,(int)yy,(int)x2,(int)y2);
        k=k+5;
  }
  for (i=1;i<=spoed;i=i+10)
  {
        x1=.5+x+sin(rigt)*(skag+k);
        yy=.5+y+(0-cos(rigt)*(skag+k));
        draw10(x1,yy,rigt,vlag);
        k=k+5;
   }
   for (i=1;i<=h;++i)
   {
        x1=.5+x+sin(rigt)*(skag+k);
        yy=.5+y+(0-cos(rigt)*(skag+k));
        x2=.5+x1+sin(rigt+HOEK)*vlag;
        y2=.5+yy+(0-cos(rigt+HOEK)*vlag);
        DrawLine((int)x1,(int)yy,(int)x2,(int)y2);
        for (j=1;j<=3;++j)
        {
            x1=.5 +x+sin(rigt)*(skag+k-j);
            yy=.5 +y+(0-cos(rigt)*(skag+k-j));
            DrawLine((int)x1,(int)yy,(int)x2,(int)y2);
         }
         k=k+5;
   }
    x=.5+x+sin(rigt);
    y=.5+y+(0-cos(rigt));
   DrawLine((int)x,(int)y,(int)x1,(int)yy);
}

void draw10(double x1,double yy,double rigt,int vlag)
 {
   double x2,y2;
   x2=.5 +x1+sin(rigt+HOEK)*vlag;
   y2=.5 +yy+(0-cos(rigt+HOEK)*vlag);
   DrawLine((int)x1,(int)yy,(int)x2,(int)y2);
 }

/************************************************************************/
/* Write the radar image to a file in rgb format                        */
/************************************************************************/
void WriteRGB(char *name)
{
  FILE *fp;
  int x,y;

  fp = fopen (name,"w");
  for (y = 0; y <YSIZE;++y)
    for (x = 0; x<XSIZE;++x)
    {
      fwrite (&Rfield[(y*XSIZE)+x],1,1,fp);
      fwrite (&Gfield[(y*XSIZE)+x],1,1,fp);
      fwrite (&Bfield[(y*XSIZE)+x],1,1,fp);
    }
  fclose (fp);
}

