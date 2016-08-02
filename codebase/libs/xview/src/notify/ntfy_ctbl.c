#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfy_ctbl.c 1.22 93/06/28";
#endif
#endif

#include <xview_private/ntfy.h>
#include <stdio.h>
#include <signal.h>

NTFY_CNDTBL *ntfy_cndtbl[NTFY_LAST_CND];

/*
 * Add a client into the condition table (ntfy_cndtbl) for the condition it
 * has an interest in.
 */

void ntfy_add_to_table(client, condition, type)
    NTFY_CLIENT    *client;
    NTFY_CONDITION *condition;
    int             type;
{
    NTFY_CNDTBL    *cnd_list = ntfy_cndtbl[type];

    NTFY_BEGIN_CRITICAL;
    if (!cnd_list) {
	/* Create the head, which is never used */
	cnd_list = (NTFY_CNDTBL *) xv_malloc(sizeof(NTFY_CNDTBL));
	cnd_list->client = (NTFY_CLIENT *) NULL;
	cnd_list->condition = (NTFY_CONDITION *) NULL;
	cnd_list->next = (NTFY_CNDTBL *) NULL;
	ntfy_cndtbl[type] = cnd_list;

	/*
	 * Create the first clt/cnd in the list, along with ptrs back to the
	 * actual clt and condition
	 */
	cnd_list = (NTFY_CNDTBL *) xv_malloc(sizeof(NTFY_CNDTBL));
	cnd_list->client = client;
	cnd_list->condition = condition;
	cnd_list->next = ntfy_cndtbl[type]->next;
	ntfy_cndtbl[type]->next = cnd_list;
	NTFY_END_CRITICAL;
	return;
    }
    /* See if a particular client already has registered this condition. */
    cnd_list = cnd_list->next;
    while (cnd_list) {
	ntfy_assert(cnd_list->condition->type == condition->type, 25
		    /* Found wrong condition type in condition table */);
	if ((cnd_list->client == client) &&
	    (cnd_list->condition == condition)) {
	    NTFY_END_CRITICAL;
	    return;
	}
	cnd_list = cnd_list->next;
    }

    cnd_list = (NTFY_CNDTBL *) xv_malloc(sizeof(NTFY_CNDTBL));
    cnd_list->client = client;
    cnd_list->condition = condition;
    cnd_list->next = ntfy_cndtbl[type]->next;
    ntfy_cndtbl[type]->next = cnd_list;
    NTFY_END_CRITICAL;
    return;
}

/*
 * Remove a condition interest for a particular client from the condition
 * table.
 */

void ntfy_remove_from_table(client, condition)
    NTFY_CLIENT    *client;
    NTFY_CONDITION *condition;
{
    NTFY_CNDTBL    *cnd_list, *last_cnd;

    if ((int) condition->type >= NTFY_LAST_CND)
	return;

    NTFY_BEGIN_CRITICAL;
    cnd_list = last_cnd = ntfy_cndtbl[(int) condition->type];

    ntfy_assert(cnd_list, 26 /* Condition list has a NULL head */);

    cnd_list = cnd_list->next;
    while (cnd_list) {
	ntfy_assert(cnd_list->condition->type == condition->type, 27
		    /* Found wrong condition type in condition table */);
	if ((cnd_list->client == client) &&
	    (cnd_list->condition == condition)) {
	    last_cnd->next = cnd_list->next;
	    free(cnd_list);
	    NTFY_END_CRITICAL;
	    return;
	}
	last_cnd = cnd_list;
	cnd_list = cnd_list->next;
    }
    NTFY_END_CRITICAL;
}

/* VARARGS2 */
pkg_private     NTFY_ENUM
ntfy_new_enum_conditions(cnd_list, enum_func, context)
    NTFY_CNDTBL    *cnd_list;
    NTFY_ENUM_FUNC  enum_func;
    NTFY_ENUM_DATA  context;
{
#if 1
    /* See comment below.  mbuck@debian.org */
    NTFY_CNDTBL    *last_cnd_list = cnd_list;
#endif
    if (!cnd_list)
	return (NTFY_ENUM_NEXT);

    cnd_list = cnd_list->next;

#ifdef notdef
    if (cnd_list)
	dump_table(cnd_list->condition->type);
#endif

    while (cnd_list) {
	switch (enum_func(cnd_list->client, cnd_list->condition,
			  context)) {
	  case NTFY_ENUM_SKIP:
	    break;
	  case NTFY_ENUM_TERM:
	    return (NTFY_ENUM_TERM);
	  default:{
	    }
	}
#if 1
	/* The call to enum_func below might result in a call to
	 * ndet_itimer_change which might call ndet_itimer_expired which in
	 * turn might call notify_set_itimer_func. This however could remove
	 * the current condition from our cnd_list, which would make our
	 * cnd_list pointer invalid. A comment in ndet_itimer_expired says
	 * that ntfy_new_enum_conditions is designed to survive this, which,
	 * of course, it is not.
	 * So, to find the next condition in the list, we have to check
	 * whether the previous condition's next pointer still points to the
	 * current condition. If it doesn't, this means that the current
	 * condition was removed and we have to use the previous condition's
	 * next pointer to find the next condition. If it didn't change, we
	 * can simply use the current condition's next pointer.
	 *
	 * Oh, BTW: I've got absolutely no idea what's going on here, but I
	 * hope this is a suitable fix. At least it prevents fullscreen-
	 * programs like vi from causing cmdtool to dump core.
	 *
	 * mbuck@debian.org
	 */

	if (cnd_list != last_cnd_list->next) {
	    cnd_list = last_cnd_list->next;
	} else {
	    last_cnd_list = cnd_list;
	    cnd_list = cnd_list->next;
	}
#else
	cnd_list = cnd_list->next;
#endif
    }
    return (NTFY_ENUM_NEXT);
}

#define NTFY_BEGIN_PARANOID     ntfy_paranoid_count++
#define NTFY_IN_PARANOID        (ntfy_paranoid_count > 0)
#define NTFY_END_PARANOID       ntfy_paranoid_count--;

/* Variables used in paranoid enumerator */
static NTFY_CONDITION *ntfy_enum_condition;
static NTFY_CONDITION *ntfy_enum_condition_next;
static int ntfy_paranoid_count = 0;

pkg_private     NTFY_ENUM
ntfy_new_paranoid_enum_conditions(cnd_list, enum_func, context)
    NTFY_CNDTBL    *cnd_list;
    NTFY_ENUM_FUNC  enum_func;
    NTFY_ENUM_DATA  context;
{
#if 1
    /* See comment above.  mbuck@debian.org */
    NTFY_CNDTBL    *last_cnd_list = cnd_list;
#endif
    extern NTFY_CLIENT *ntfy_enum_client;
    extern NTFY_CLIENT *ntfy_enum_client_next;
    NTFY_ENUM       return_code = NTFY_ENUM_NEXT;
    extern sigset_t    ndet_sigs_managing;
    sigset_t oldmask, newmask;
    newmask = ndet_sigs_managing;  /* assume interesting < 32 */
    sigprocmask(SIG_BLOCK, &newmask , &oldmask);

    /*
     * Blocking signals because async signal sender uses this paranoid
     * enumerator.
     */

    ntfy_assert(!NTFY_IN_PARANOID, 28
		/* More then 1 paranoid using enumerator */);
    NTFY_BEGIN_PARANOID;

    if (!cnd_list)
	goto Done;

    cnd_list = cnd_list->next;

#ifdef notdef
    if (cnd_list)
	dump_table(cnd_list->condition->type);
#endif

    while (cnd_list) {
	ntfy_enum_client = cnd_list->client;
	ntfy_enum_condition = cnd_list->condition;
	switch (enum_func(ntfy_enum_client, ntfy_enum_condition,
			  context)) {
	  case NTFY_ENUM_SKIP:
	    break;
	  case NTFY_ENUM_TERM:
	    return_code = NTFY_ENUM_TERM;
	    goto Done;
	  default:
	    if (ntfy_enum_client == NTFY_CLIENT_NULL)
		goto BreakOut;
	}
#if 1
	/* See comment above.  mbuck@debian.org */
	if (cnd_list != last_cnd_list->next) {
	    cnd_list = last_cnd_list->next;
	} else {
	    last_cnd_list = cnd_list;
	    cnd_list = cnd_list->next;
	}
#else
	cnd_list = cnd_list->next;
#endif
    }
BreakOut:
    {
    }
Done:
    /* Reset global state */
    ntfy_enum_client = ntfy_enum_client_next = NTFY_CLIENT_NULL;
    ntfy_enum_condition = ntfy_enum_condition_next = NTFY_CONDITION_NULL;
    NTFY_END_PARANOID;
    sigprocmask(SIG_SETMASK, &oldmask, (sigset_t *) 0);
    return (return_code);
}

#ifdef notdef
dump_table(type)
    int             type;
{
    NTFY_CNDTBL    *cnd_list;
    int             i;

    fprintf(stderr, "\n\n");
    fprintf(stderr, "Searching for type %d\n", type);
    for (i = 0; i < NTFY_LAST_CND; i++) {
	if (ntfy_cndtbl[i]) {
	    cnd_list = ntfy_cndtbl[i]->next;
	    fprintf(stderr, "%d: ", i);
	    while (cnd_list) {
		/*
		 * fprintf (stderr, "%d, ", cnd_list->condition->type);
		 */
		fprintf(stderr, "%d (0x%x [0x%x] 0x%x), ", cnd_list->condition->type, cnd_list->client, cnd_list->client->nclient, cnd_list->condition);
		cnd_list = cnd_list->next;
	    }
	    fprintf(stderr, "\n");
	}
    }
    fprintf(stderr, "\n");
}

#endif
