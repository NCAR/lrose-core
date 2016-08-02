#!/usr/bin/perl -w
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 19:31:59 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# MdvTunnel: A Simple Tunneling mechanism for Apache http server for the
#  DsMdvServer/MDV  protocol.
# (c)  F. Hage. NCAR/RAP 1999
# 
# Set Package.
package Apache::MdvpTunnel;
use strict;
use diagnostics;
# Import Constants for Mod_perl
use Apache::Constants qw(OK DECLINED FORBIDDEN NOT_FOUND SERVER_ERROR);
# URL parsing, etc
use Apache::URI ();
# Logging
use Apache::Log ();
# TCP/IP Socketing
use Socket;
#
# Subroutine name must be called "handler" for APACHE:mod_perl 
sub handler
{
   # Set unbuffered IO
   $| = 1;

   # Get the Apache Request Object
   my $r = shift;

   # Get the log Object
   my $log = $r->log();

   # Get the request content 
   my $c;
   my $content_len = $r->header_in('Content-length');

   # Make sure the request has this content_len set
   if(!(defined $content_len)) { 
        $r->log_error("MdvTunnel: Does not appear to be a real DsServer request\n");
	return FORBIDDEN;
   }

   # Read the whole message into $c
   $r->read($c,$content_len);
   if($content_len < 84) {
        $r->log_error("MdvTunnel: Does not appear to be a real DsServer request\n");
	return FORBIDDEN;
   }
      
   # Look inside the header for the number of parts in the request
   #  56 bytes in DsMsgHdr_t before nParts
   my $nparts = unpack "x56N", $c;
   # $r->log_error("Number of Message parts: $nparts\n");

   # Seek into message length
   # Length is 8 Code bytes + 12 SKU_header_t bytes +  64 bytes DsMsgPart_t bytes 
   # is  64 bytes - Start there
   my $seek_len = 84;
   my $msg_offset = 88;
   my $url_len = 0;

   my $found = 0;
   while( $found == 0 && $seek_len < ($content_len -24) ) {
       my $datatype = unpack "x$seek_len N", $c;
       if($datatype == 1) {
	   $found = 1;
	   $seek_len += 4;
	   $msg_offset =  unpack "x$seek_len N", $c;
	   # Add 8 code bytes and 12 SKU_header bytes to get the real location
	   $msg_offset +=  20;
	   $seek_len += 4;
	   $url_len =  unpack "x$seek_len N", $c;
	}
	# Length of DsMsgPart_t is  24 bytes
	$seek_len += 24
    }

    if($found == 0) {
        $r->log_error("MdvTunnel: Could not find URL part of message\n");
	return DECLINED;
   }
      
   # $r->log_error("Found URL at: $msg_offset , Len: $url_len\n");
       

   # Seek to the URL in the message content
   my $url =  unpack "x$msg_offset a$url_len", $c;

   # Extract the host and port using the URI object class
   # Doesn't work on mdvp URL's
   #my $uri = Apache::URI->parse($r,$url);
   #my $host = $uri->hostname;
   #my  $port = $uri->port;

   # Extract the host and port using  split()
   my ($protocol,$translator,$host,$port,$path) = split /:/, $url;
   # Strip off leading // in host string
   $host =~ s%//%%;

   if(!(defined $port)) {
       $r->log_error("MdvTunnel: Undefined Port in URL\n");
	return DECLINED;
   }

   # Apply some security checking - Don't allow sockets to system services
   # or unreasonably long messages
   if($port <= 1024 || $content_len > 4096 ) {
       return FORBIDDEN;
   }

   # Comes out in error.log
   $log->notice("MdvTunnel to Host: $host, PORT: $port, URL: $url\n");

   # Set up a Socket to the DsMdv Server
   my ($iaddr,$paddr,$proto);
   if(!defined($iaddr = inet_aton($host))) {
       return NOT_FOUND;
   }
   if(!defined($paddr = sockaddr_in($port,$iaddr))) {
       return SERVER_ERROR;
   }

   if(!defined($proto = getprotobyname('tcp'))) {
       return SERVER_ERROR;
   }

   if(!(socket(SOCK,PF_INET, SOCK_STREAM, $proto))) {
	return NOT_FOUND;
   }

   if(!(connect(SOCK,$paddr))) {
	close(SOCK);
	return NOT_FOUND;
   }

   # Copy the Commaand content to the socket
   my $len = syswrite(SOCK,$c,$content_len);
   if($len != $content_len) {
	close(SOCK);
	return SERVER_ERROR;
   }
   #$log->notice("MdvTunnel Sent $len request message bytes\n");

   # Set the Document type and Send the MIME Header
   # $r->content_type("application/Dsmdv_protocol");
   # $r->send_http_header;

   # Read the reply on the Socket and pass it through to the client
   my $buf;
   while(($len = read(SOCK,$buf, 1024)) > 0 ) {
      # $log->notice("MdvTunnel Returning $len bytes\n");
       $r->print("$buf");
   }

   close(SOCK);

   # Send the Response Code.
   return OK;
}
1;
