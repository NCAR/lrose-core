/* @(#)gettext.h 50.11 93/06/28 SMI */

#define DEFAULT_DOMAIN	"default"
#if !defined(__linux) && !defined(__APPLE__)
#define DEFAULT_BINDING "/usr/lib/locale\n"
#else
#define DEFAULT_BINDING "/usr/openwin/lib/locale\n"
#endif
#define COOKIE -127
#define BINDINGLISTDELIM '\n'

#define MAX_VALUE_LEN		2047
#define MAX_DOMAIN_LENGTH	255
#define LC_NAMELEN		255

#include <ctype.h>
#include <errno.h>
#ifdef OS_HAS_LOCALE
#include <locale.h>
#if (defined(__linux) || defined(__APPLE__)) && !defined(LC_MESSAGES) && defined(LC_RESPONSE)
#define LC_MESSAGES LC_RESPONSE
#endif
#endif /* OS_HAS_LOCALE */
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
#include <xview/base.h>
#include <xview_private/portable.h>

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
