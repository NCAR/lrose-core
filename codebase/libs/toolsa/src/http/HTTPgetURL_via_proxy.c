/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/******************************************************************************
 *  HTTPgetURLvia_proxy.c: : Get the Resource given a  URL  and a Proxy URL
 *  F. Hage NCAR/RAP Feb 1999
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <toolsa/sockutil.h>

#define HTTP_PORT 80
#define HTTP_HEAD_BUF_SIZE 2048
#define HTTP_COMM_BUF_SIZE 1024
#define HTTP_DATA_BUF_SIZE 1024
#define HTTP_HEAD_READ_LEN 128

/******************************************************************************
 * HTTPgetURLvia_proxy: Return the resource data specified by the input url.
 * Returns -1 bad url, -2 failure to connect,  -3 read reply failure,
 * -4 alloc failure, -5 read resource data failure. Values > 0 
 * are the transaction status as returned by the http server. This
 * routine allocates space for the resource data. It is the caller's
 * responsibility for freeing this memory.  On success: sets **resource
 * to point to the allocated data and *resource_len * to its length.
 * This routine is thread safe.
 */

int
HTTPgetURL_via_proxy(const char* proxy_url, const char* url, size_t timeout_msec, char **resource, int * resource_len) 
{

    int infd;
    int  port;
    long status;

    int header_complete;
    char * cur_pos;
    char * pos;
    char * mem_ptr;
    int len_read;
    int total_read;
    int header_len;
    long res_len;
    int target_size;
    char *start,*end,*ptr;

    char hostname[256];
    char document[1024];
    char command[HTTP_COMM_BUF_SIZE];
    char buf[HTTP_HEAD_BUF_SIZE];

    /* Set result variables to nulls */
    *resource = NULL;
    *resource_len  = 0;

    if(url == NULL)  return(-1); /* Not a valid HTTP URL */
    if(proxy_url == NULL)  return(-1); /* Not a valid HTTP URL */

    /* FIRST PARSE THE URL */
    /* Find the Host Name part; */
    if((start = strstr(proxy_url,"//")) == NULL) {
        /* Not a valid HTTP URL */
        return(-1);
    }
    start += 2; 
    if((end = strchr(start,'/')) == NULL) {
        /* Not a valid HTTP URL */
        return(-1);
    }
    strncpy(hostname,start,(end - start));
    hostname[end - start] = '\0';

    if((ptr = strchr(hostname,':')) != NULL) {
        *ptr = '\0'; /* null terminate the host string at the colon */
        ptr++;
        port = strtol(ptr,NULL,10);
        if(port <= 0) port = HTTP_PORT;
    } else {
        port = HTTP_PORT;
    }

    /* Find the Resource part */
    strncpy(document,end,1024);
    document[1023] = '\0';

    /* Open a connection to the HTTP server */
    if((infd = SKU_open_client(hostname,port)) < 0) {
        return -2;
    }

    /* Build up request message */
    sprintf(command,"GET %s HTTP/1.0\r\n",url);
    sprintf(buf,"User-Agent: NCAR/RAP HTTPgetURL_via_proxy()/1.0\r\n");
    sprintf(buf,"Proxy-Connections: Keep-Alive\r\n");

    strcat(command,buf);
    strcat(command,"\r\n"); /* End of request message indicator */

    /* Send the request to the HTTP server */
    if(SKU_write(infd,command,strlen(command),-1) != strlen(command)) {
		SKU_close(infd);
        return -2;
    }

    /* clear out the buffer */
    memset(buf,0,HTTP_HEAD_BUF_SIZE);
    /* READ THE SERVER REPLY */
    total_read = 0;
    header_complete = 0;
    /* Read until the header is complete - or out of buf space */
    do {
        len_read = SKU_read_timed(infd,(buf + total_read), HTTP_HEAD_READ_LEN,-1,2000);
        total_read += len_read;
        if((pos = strstr(buf,"\r\n\r\n")) != NULL) header_complete = 1;
    } while(header_complete == 0 &&
           ((total_read + HTTP_HEAD_READ_LEN) < HTTP_HEAD_BUF_SIZE));

    if(total_read + HTTP_HEAD_READ_LEN >= HTTP_HEAD_BUF_SIZE) {
		SKU_close(infd);
        return -3;
    }

    /*  Calc the length of the header + 4 bytes for "\r\n\r\n" */
    header_len = pos - buf + 4;

    /* Pick out the Status and data length */
    pos = strtok(buf," ");  /* prime strtok  */
    pos = strtok(NULL," "); /* get the second token */
    status = strtol(pos,NULL,10);

    if(status >= 300 || status == LONG_MIN) {
		SKU_close(infd);
        return status;
    }

    /* skip over the status tokens - past the nulls that strtok */
    /* wrote in our buffer */
    pos += strlen(pos) + 1;

    /* Search for the "Content-Length:" string */
    if((cur_pos = strstr(pos,"Content-Length:")) == NULL) {
		SKU_close(infd);
        return -5;
    }
    cur_pos += strlen("Content-Length:");

    /* Tokenize the string */
    cur_pos = strtok(cur_pos," \r\n");
    res_len = strtol(cur_pos,NULL,10);
    if(res_len == LONG_MAX || res_len == LONG_MIN) {
        /*Bad Length Indicated in HTTP reply */
		SKU_close(infd);
        return -5;
    }

    /* Allocate space for this memory +  a NULL  */
    if((mem_ptr = calloc(1, res_len+1)) == NULL) {
		SKU_close(infd);
        return -4;
    }
    *resource = mem_ptr;
    cur_pos = mem_ptr; /* keep track of where we are */

    /* Copy that part of the data we read while scanning for the header */
    target_size = total_read - header_len;
    if(target_size > 0) {
         memcpy(cur_pos,(buf + header_len),target_size);
         cur_pos += target_size;
         target_size = res_len - target_size;
    } else {
         target_size = res_len;
    }

    /* Now read the rest of the resource data from the socket */
    do {
        len_read = SKU_read_timed(infd,cur_pos, target_size,-1, 2000);
        target_size -= len_read;
        cur_pos += len_read;
    } while(target_size > 0 && len_read >= 0);

    if(target_size > 0) {
        *resource_len = res_len - target_size;
	mem_ptr[*resource_len] = '\0'; /* Null terminate */
		SKU_close(infd);
        return -5;
    }

    *resource_len = res_len;
    mem_ptr[res_len] = '\0'; /* Null terminate */
	SKU_close(infd);
    return status;
}
