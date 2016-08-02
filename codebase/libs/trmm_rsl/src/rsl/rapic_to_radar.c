#include <trmm_rsl/rsl.h> 
#include <unistd.h>

extern Radar *rapic_radar;
extern int rapicparse(void);

Radar *RSL_rapic_to_radar(char *infile)
{
  /* Attach infile to stdin and call the parser. */

  FILE *fp;
  Radar *radar;
  int save_fd;
  
  radar = NULL;
  if (infile == NULL) {
	save_fd = dup(0);
	fp = fdopen(save_fd, "r");
  }  else {
	if ((fp = fopen(infile, "r")) == NULL) {
	  perror(infile);
	  return radar;
	}
  }
  fp = uncompress_pipe(fp); /* Transparently gunzip. */
  close(0); dup(fileno(fp)); /* Redirect stdin. */

  rapicparse();
  radar = rapic_radar;

  rsl_pclose(fp);
	
  return radar;
}
