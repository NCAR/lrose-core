/* static char SccsID [] = "@(#)gettext.h	1.4\t10/15/91" */

#define DEFAULT_DOMAIN	"default"
#define DEFAULT_BINDING "/usr/lib/locale\n"
#define COOKIE 0xFF
#define BINDINGLISTDELIM '\n'

#define MAX_VALUE_LEN		2047
#define MAX_DOMAIN_LENGTH	255
#define LC_NAMELEN		255

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
 
struct domain_binding {
    char    *domain_name;
    char    *binding;
    struct   domain_binding *nextdomain;
};


#include <fcntl.h> 
#include <sys/file.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 

#define MAX_MSG 64 

struct struct_mo_info {
    int		message_mid;
    int		message_count;
    int		string_count_msgid;
    int		string_count_msg;
    int		message_struct_size;
} ;

struct message_struct {
    int		less;
    int		more;
    int		msgid_offset;
    int		msg_offset;
};

struct message_so {
    char *message_so_path;   /* name of message shared object */
    int fd;				/* file descriptor		*/
    struct struct_mo_info *mess_file_info; /* information of message file */
    struct message_struct *message_list;/* message list */
    char *msg_ids;			/* actual message ids */
    char *msgs;				/* actual messages */
};
