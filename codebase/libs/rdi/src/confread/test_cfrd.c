/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2012 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2012/9/18 21:11:50 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */


#include <string.h>

#include <rdi/confread.h>
#define MAX_ELEVATION	32


static void err_func (char *msg);

/*********************************************************************

	Test 
*/


void main ()
{
    int i, ll;
    char *buf;
    int err, ret;


	float elevation [MAX_ELEVATION];
	int cnt;			/* number of elevations */

	if (CFRD_open ("t_conf", err_func) == CFRD_FAILURE) {
	    printf ("Failed in opening the configuration file\n");
	    exit (-1);
	}

/* testing CFRD_read_array
    for (i = 1; i < 10; i++) {
	CFRD_get_next_line ("CFRD_BEGIN", NULL);
        ret = CFRD_read_array ("Elevation_list", CFRD_FLOAT, i, elevation, &err);
	printf ("i = %d, ret = %d, err = %d, ele [0] = %f, ele [i - 1] = %f\n",
		i, ret, err, elevation [0], elevation [ret - 1]);
    }
    exit (0);
*/


	cnt = 0;
	while (1) {
	    char *line, *tk;
	    int ln, n;

	    if ((ln = CFRD_get_next_line ("Elevation_list", &line)) == CFRD_FAILURE) {
		printf ("Elevation_list not found in the configuration\n");
		exit (1);
	    }

	    if (line[0] == '\0')		/* all data are read */
		break;

	    n = 1;					/* from the first token */
	    tk = strtok (line, " \t");
	    while (tk != NULL) {	/* for each token */

		if (cnt >= MAX_ELEVATION) {
		    printf ("Error found in conf: too many elevations in line %d\n", ln);
		    exit (1);
		}

		if (sscanf(tk, "%f", &elevation[cnt]) != 1) {
		    printf ("Error found in conf: at line %d\n", ln);	
		    exit (1);
		}

	        printf ("elevation: %f\n", elevation[cnt]);
		cnt++;
		tk = strtok (NULL, " \t\n");
	    }
	}

    for (i=0;i<3;i++) {
        printf ("CFRD_get_next_line: ret = %d\n", CFRD_get_next_line ("key1", &buf));
        printf ("buf = %s\n", buf);
    }
    for (i=0;i<3;i++) {
        printf ("CFRD_get_next_line: ret = %d\n", CFRD_get_next_line ("key2", &buf));
        printf ("buf = %s\n", buf);
    }

    printf ("CFRD_read_check, ret = %d\n", CFRD_read_check (&ll));
    printf ("ll = %d\n", ll);

    exit (0);

}


static void err_func (char *msg)
{

    printf ("%s\n", msg);
}

