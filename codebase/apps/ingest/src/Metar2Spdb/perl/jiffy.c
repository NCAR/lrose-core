#include <stdio.h>
#include <math.h>
#include <mysql/mysql.h>

main(argc,argv)
	int	argc;
	char	*argv[];
{
	int i;
    char buf[256];
    char querry[4096];

	MYSQL *my;
	MYSQL_RES *res;
	MYSQL_ROW row;

	memset(querry,0,1024);

	my = mysql_init(NULL);

	mysql_real_connect(my, "debris", "www-data", "or-knots", "weather", 0, NULL, 0);

/*
SELECT Metars.icaoId, obsTime, visib, slp, altim, temp, dewp, wdir, wspd, wgst, 
wxString, cldCvg1, cldCvg2, cldCvg3, cldCvg4, cldBas1, cldBas2, cldBas3, cldBas4, rawOb, 
StationInfo.icaoId, lat, lon, elev, priority 
FROM Metars STRAIGHT_JOIN StationInfo 
WHERE obsTime>(UNIX_TIMESTAMP(NOW())-1740) 
AND Metars.icaoId=StationInfo.icaoId 
ORDER BY priority ASC, obsTime DESC 
*/ 

  strcpy(querry,"SELECT StationInfo.icaoId, lat, lon, elev, Metars.icaoId, reportTime, wxString, temp, dewp,
");
  strcat(querry," wspd, wdir, wgst, slp, pcp24hr, precip, visib, " );
  strcat(querry," cldCvg1, cldBas1, cldCvg2, cldBas2, cldCvg3, cldBas3, cldCvg4, cldBas4 , rawOb ");

  strcat(querry,"FROM Metars STRAIGHT_JOIN StationInfo ");

  sprintf(buf, "WHERE obsTime>(UNIX_TIMESTAMP(NOW())-%d)",(int)(30 * 60));
  strcat(querry, buf);

  strcat(querry," AND Metars.icaoId=StationInfo.icaoId ");

   sprintf(buf, "AND StationInfo.lat>%d ",20);
  strcat(querry, buf);
  sprintf(buf, "AND StationInfo.lat<%d ",50);
  strcat(querry, buf);
  sprintf(buf, "AND StationInfo.lon>%d ",-130);
  strcat(querry, buf);
  sprintf(buf, "AND StationInfo.lon<%d ",-60);
  strcat(querry, buf);
  

  strcat(querry," ORDER BY priority ASC, obsTime DESC");

	 mysql_real_query(my,querry,4096);

	 res= mysql_store_result(my);

	 while((row=mysql_fetch_row(res)) != NULL) {
	  for( i=0; i < mysql_field_count(my); i++) {
	    printf("%s ",row[i]);
	  }
	    printf("\n");
	 }

	 mysql_free_result(res);
	 mysql_close(my);
}
