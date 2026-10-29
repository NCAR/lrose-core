/* The NTFY_CNDTBL is an array of lists where each element in the array 
 * represents one of the nine conditions (listed below) associated with the 
 * notifier.  The lists which are associated to each element are made up of
 * structs which contain ptrs back to the client and the conditons the
 * client is interested in.
 * The purpose of the NTFY_CNDTBL is to provide a fast look up for the
 * notifier when a unix type condition occurs.  For the notifier to 
 * determine who is interested in a particular condition, it must only
 * look to the proper element in the ntfy_cndtbl to obtain the list of
 * clients who have that particular interest.
 */

typedef struct ntfy_cndtbl {
	NTFY_CLIENT		*client;
	NTFY_CONDITION		*condition;
	struct ntfy_cndtbl	*next;
} NTFY_CNDTBL;

#define NTFY_LAST_CND		9
