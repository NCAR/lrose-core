/*	Copyright (c) 1990 Sun Microsystems	*/
/*	  All Rights Reserved  	*/

#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)gettext.c 50.21 93/06/28";
#endif
#endif

#include <string.h>
#include <xview_private/gettext.h>

/* char *strdup(); */
/* char * dgettext();*/
char *xv_bindtextdomain(char *domain_name, char *binding);
char *xv_textdomain(char *domain_name);
char *xv_gettext(char *msg_id);
char *xv_dgettext(char *domain_name, char *msg_id);
static char *_gettext( struct message_so messages, char *key_string );
char *fgets(), *getenv();
#if !defined(__linux) || !defined(__GLIBC__)
//caddr_t mmap();
#endif

static struct domain_binding *firstbind=0, *lastbind=0;

static int  first_free = 0;                 /* first free entry in list */
static struct message_so messages_so[MAX_MSG];
static struct message_so  cur_mess_so;      /* holds current message domain
*/


static void
initbindinglist()
{
    if (! firstbind) {
	firstbind =
	    (struct domain_binding *) malloc(sizeof(struct domain_binding));
	firstbind->domain_name = strdup("");
	firstbind->binding = strdup(DEFAULT_BINDING);
	firstbind->nextdomain = (struct domain_binding *) 0;
	lastbind = firstbind;
    }
}

static int
searchmmaplist(path)
    char *path;
{
    int msg_inc = 0;
    while (msg_inc < first_free) {
        if (!strcmp(path, messages_so[msg_inc].message_so_path)) {
            if (messages_so[msg_inc].fd != -1 &&
                messages_so[msg_inc].mess_file_info !=
                    (struct struct_mo_info *) -1) {
		return(msg_inc);
            } else {
                return(-1);
            }   
        }   
        msg_inc++;
    }
    return(MAX_MSG);
}

static char *
lookupdefbind(domain_name)
    char	*domain_name;
{
    static char *binding = NULL; /* psuedo static ptr for return value */
    static int bindinglen = 0;
    char *bindptr = firstbind->binding;
    char *current_locale = (char *)NULL;
    char chartmp;
    char *bindtmptr, pathtmp[MAXPATHLEN], bindtmp[MAXPATHLEN];
    int newlen, ret;

    struct stat statbuf;

    bindtmptr = bindtmp;

#ifdef OS_HAS_LOCALE
    current_locale = setlocale(LC_MESSAGES, NULL);
#endif /* OS_HAS_LOCALE */
    if (!current_locale)  {
	current_locale = "C";
    }

    while (chartmp = *bindptr++)  {
	switch (chartmp) {

	    case BINDINGLISTDELIM:
		*bindtmptr = '\0';
		strcpy(pathtmp, bindtmp);
		strcat(pathtmp, "/");
		strcat(pathtmp, current_locale);
		strcat(pathtmp, "/LC_MESSAGES/");
		strcat(pathtmp, domain_name);
		strcat(pathtmp, ".mo");
		
		if ((ret = searchmmaplist(pathtmp)) == MAX_MSG) {
		    if(first_free == MAX_MSG) 
			return (NULL);
		    if (stat(pathtmp, &statbuf)) {
			bindtmptr = bindtmp;
			messages_so[first_free].fd = -1;
			messages_so[first_free].message_so_path =
				strdup(pathtmp);
			first_free++;
			break;
		    } else {
			xv_bindtextdomain(domain_name, bindtmp);
		    }
		} else if (ret == -1) {
		    bindtmptr = bindtmp;
		    break;
		}

		if ((newlen = strlen(bindtmp)) > bindinglen) {
		    bindinglen = newlen;
		    if (binding) {
			free (binding);
		    }
		    binding = malloc(newlen+1);
		    strcpy(binding, bindtmp);
		}
		return(binding);

	    default:
		*bindtmptr++ = chartmp;
		break;
	}
    }
    /*
     * NOT FOUND, return NULL
    */
    
    return (NULL);
}

char *
xv_bindtextdomain(char *domain_name, char *binding)
{

    struct domain_binding *bind;
    char *lastpath;
    char pathtmp[MAXPATHLEN+1];
    int newlen;

    pathtmp[0] = '\0';

    /* Initialize list */
    if (! firstbind) {
	initbindinglist();
    }
    
    if (!domain_name) {
	return (NULL);
    }

    if (*domain_name == '\0') {
	if (!binding) {
	    /* query, add COOKIE to binding
	     * return new binding cookie
	    */
	    pathtmp[0] = (unsigned char) COOKIE;
	    pathtmp[1] = '\0';
	    strcat(pathtmp, firstbind->binding);
	    return (strdup(pathtmp));
	} else if (binding[0] == COOKIE) {
	    /* result of a previous query,
	     * restore old binding
	    */
	    firstbind->binding = strdup((char*)binding+1);
	    free((char *)binding);
	    return (NULL);
	} else {
	    /* add binding to default binding list
	    */
	    strcat(pathtmp, firstbind->binding);
	    free(firstbind->binding);
	    strcat(pathtmp, (char *)binding);
	    strcat(pathtmp, "\n");
	    firstbind->binding = strdup(pathtmp);
	    return (NULL);
	}
    }
    
    /* linear search for binding, rebind if found, add if not */
    bind = firstbind;
    while (bind) {
	if (!strcmp(domain_name, bind->domain_name)) {
	    if (!binding) {
		return(bind->domain_name);
	    }
	    if (bind->domain_name) {
		free(bind->domain_name);
	    }
	    if (bind->binding) {
		free(bind->binding);
	    }
	    
	    bind->domain_name = strdup(domain_name);
	    bind->binding = strdup((char *)binding);
	    return (bind->binding);
	}
	bind = bind->nextdomain;
    }
    
    /* Not found in list, add it to the end */

    if (!binding) {
	return (NULL);
    }
    lastbind = bind = lastbind->nextdomain =
	(struct domain_binding *) malloc(sizeof(struct domain_binding));
    bind->domain_name = strdup(domain_name);
    bind->binding = strdup((char*)binding);
    bind->nextdomain = NULL;
    return (bind->binding);
}

static char *
findtextdomain(domain_name)
    char *domain_name;
{
    struct domain_binding *bind;
    char *tmptr;

    bind = firstbind;

    if (!bind) {
        initbindinglist();
        return (lookupdefbind(domain_name));
    }

    while (bind) {
	if (!strcmp(domain_name, bind->domain_name)) { 
		return (bind->binding);
	}
	bind = bind->nextdomain;
    }

    /* not found, look for binding in default binding list */
    return (lookupdefbind(domain_name));
}
 
    

char *xv_textdomain(char *domain_name)
{
	
    static int entered = 0;

    static char	current_domain[MAX_DOMAIN_LENGTH + 1];

    if (! entered) {
	strcpy(current_domain, DEFAULT_DOMAIN);
 	entered = 1;
    }

    if (domain_name == NULL) {
	return(current_domain);
    }

    if ((int)strlen(domain_name) > MAX_DOMAIN_LENGTH) {
	return(NULL);
    }

    if (*domain_name == '\0') {
	strcpy(current_domain, DEFAULT_DOMAIN);
    } else {
	strcpy(current_domain, domain_name);
    }

    return(current_domain);
}

char *xv_gettext(char *msg_id)
{
    return (xv_dgettext(NULL, msg_id));
}


char *xv_dgettext(char *domain_name, char *msg_id)
{
    char msgfile[MAXPATHLEN+1];

    char *current_locale = (char *)NULL;
    char *current_domain;
    char *current_binding;
    char *msgptr, *openwinhome = NULL;
    static int gotenv = 0;
    static char *shunt = NULL;

    struct stat statbuf;
    int	fd = -1;
    caddr_t addr;

    int   msg_inc;

    if (!gotenv) {
      shunt = getenv("SHUNT_GETTEXT");
      gotenv = 1;
    }
    if (shunt)
	return (msg_id);

#ifdef OS_HAS_LOCALE
    current_locale = setlocale(LC_MESSAGES, NULL);
#endif /* OS_HAS_LOCALE */
    if (!current_locale)  {
	current_locale = "C";
    }

    if (domain_name == NULL) {
        current_domain = xv_textdomain(NULL);
    } else if ((int)strlen(domain_name) > MAX_DOMAIN_LENGTH) {
        return(msg_id);
    } else if (*domain_name == '\0') {
        current_domain = DEFAULT_DOMAIN;
    } else {
        current_domain = domain_name;
    }

    /* check to see if textdomain has changed	*/


    XV_BZERO(msgfile, sizeof(msgfile));
    if (current_binding = findtextdomain(current_domain)) {
	strcpy(msgfile, current_binding);
	strcat(msgfile, "/");
	strcat(msgfile, current_locale);
	strcat(msgfile, "/LC_MESSAGES/");
	strcat(msgfile, current_domain);
	strcat(msgfile, ".mo");
    } else {
	return(msg_id);
    }

    msg_inc = 0;
    while (msg_inc < first_free) {
	if (!strcmp(msgfile, messages_so[msg_inc].message_so_path)) {
	    if (messages_so[msg_inc].fd != -1 &&
		messages_so[msg_inc].mess_file_info !=
		    (struct struct_mo_info *) -1) {
		cur_mess_so = messages_so[msg_inc];
		return (_gettext(cur_mess_so, msg_id));
	    } else {
		return(msg_id);
	    }
	}
	msg_inc++;
    }

    /*
       been though entire queue and not found 
       open new entry if there is space.
    */

    if (msg_inc == MAX_MSG) {
	return (msg_id);		/* not found and no more space */
    }
    if (first_free == MAX_MSG) {
	return (msg_id);		/* no more space		*/
    }

    /*
     * There is an available entry in the queue, so make a
     * message_so for it and put it on the queue, 
     * return msg_id if message file isn't opened -or-
     * mmap'd correctly
    */

/*
    if ((fd = open(msgfile, O_RDONLY)) == -1) {
	return (msg_id);
    } else if (fstat(fd, &statbuf) == -1) {
	close(fd);
	return (msg_id);
    } else if ((addr =
	mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))
	    == (caddr_t) -1 ) {
	close(fd);
	return (msg_id);
    }
    close(fd);
*/

    fd = open(msgfile, O_RDONLY);

    messages_so[first_free].fd = fd;
    messages_so[first_free].message_so_path = strdup(msgfile);

    if (fd == -1) {
	first_free++;
	close(fd);
	return (msg_id);
    }

    fstat(fd, &statbuf);

#ifdef OS_HAS_MMAP
    /*
     * use mmap if on SunOS4.1 or later
     */
    addr = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
#else
    /*
     * use malloc if mmap is not available
     */
    addr = malloc(statbuf.st_size);

    if (!addr)  {
        close(fd);
	return(msg_id);
    }

    if (read(fd, addr, statbuf.st_size) != statbuf.st_size)  {
        close(fd);
	free(addr);
	return(msg_id);
    }
#endif /* OS_HAS_MMAP */

    close(fd);
    messages_so[first_free].mess_file_info = (struct struct_mo_info *) addr;

    if (addr == (caddr_t) -1) {
	first_free++;
	return (msg_id);
    }

    messages_so[first_free].message_list =
	(struct message_struct *) &messages_so[first_free].mess_file_info[1];
    messages_so[first_free].msg_ids =
	(char *) &messages_so[first_free].message_list[messages_so[first_free].mess_file_info->message_count];
    messages_so[first_free].msgs =
	(char *) messages_so[first_free].msg_ids + messages_so[first_free].mess_file_info->string_count_msgid;
    cur_mess_so = messages_so[first_free];
    first_free++;


    /* return pointer to message */

    return (_gettext(cur_mess_so,msg_id));
}


static char *_gettext( struct message_so messages, char *key_string )
{
    register int check;
    register int val;
    check = messages.mess_file_info->message_mid;
    for (;;) {
	if ((val=strcmp(key_string,
	    messages.msg_ids+messages.message_list[check].msgid_offset)) < 0) {
	    if (messages.message_list[check].less == -99) {
		return (key_string);
	    } else {
		check = messages.message_list[check].less;
	    }
	} else if (val > 0) {
	    if (messages.message_list[check].more == -99) {
                return (key_string);
	    } else {
		check = messages.message_list[check].more;
            }
	} else {
	    return (messages.msgs+messages.message_list[check].msg_offset);
        } /* if ((val= ... */
    } /* for (;;) */
}
