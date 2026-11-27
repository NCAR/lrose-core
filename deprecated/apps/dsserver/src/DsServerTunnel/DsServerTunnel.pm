#!/usr/bin/perl -w
# DsServerTunnel: simple Apache tunneling mechanism for RAL DsServer protocols.
# (c) F. Hage.  NCAR/RAP 1999
# Modified: Tres Hofmeister <tres@ucar.edu>  Mon Mar  8 10:25:15 MST 2010
# CVS: $Id: DsServerTunnel.pm,v 1.10 2012/05/08 22:52:05 tres Exp $

# Set Package.
package Apache2::DsServerTunnel;

# use strict;
use diagnostics;

# Import constants for mod_perl.
use Apache2::Const -compile => qw(:common OK DECLINED FORBIDDEN NOT_FOUND SERVER_ERROR);

# Apache request object API.
use Apache2::RequestIO ();
use Apache2::RequestRec ();
use APR::Table ();

# Apache URL parsing, etc. API.
use Apache2::URI ();

# Apache logging API.
use Apache2::Log ();

# TCP/IP socketing.
use Socket;

# Tag for log messages.
$ltag = 'DsServerTunnel';

# Parse the configuration file.
$cfile = '/etc/apache2/Apache2/DsServerTunnel.cf';
open(CONFIG, $cfile) || die "open: $cfile: $!";
while (<CONFIG>) {
    s/\s*#.*//;		# Strip comments.
    s/^\s*//;		# Strip leading white space.
    s/\s*$//;		# Strip trailing white space; includes "\n".
    next if /^\s*$/;	# Skip blank lines.

    push(@valid, $_);
}

# Subroutine name must be called "handler" for APACHE:mod_perl
sub handler
{
    # Enable debugging: 1 for basic, 2 for socket I/O.
    $debug = 0;

    # Set unbuffered I/O.
    $| = 1;

    # Get the Apache request object.
    my $r = shift;

    # Get the Apache log object.
    my $log = $r->log();

    $log->notice("========== Start DsServerTunnel ==========") if ($debug);

    # Get the request Content-Length header.
    my $content_len = $r->headers_in->{'Content-length'};

    if ( !(defined $content_len) ) {
	$log->error("$ltag: content_len not defined: invalid request");
	return Apache2::Const::FORBIDDEN;
    }
    if ($content_len < 84) {
	$log->error("$ltag: content_len too short ($content_len): invalid request");
	return Apache2::Const::FORBIDDEN;
    }

    # Get the request User-Agent header.
    my $user_agent = $r->headers_in->{'User-Agent'};

    if ( !(defined $user_agent) ) {
	$log->error("$ltag: user_agent not defined: invalid request");
	return Apache2::Const::FORBIDDEN;
    }

    if ($debug) {
	$log->notice("$ltag: user_agent: $user_agent");
	$log->notice("$ltag: content_len: $content_len");
    }

    my $server_label;
    my $server_host;
    my $server_port;
    my $content;

    # Attempt to get the system, host, and port out of the User-Agent header.
    if ( (($server_label, $server_host, $server_port) = split /:/, $user_agent) == 3) {

	# If successful, clean up the host and port values.
	$server_host =~ s/ +//g;
	$server_port =~ s/ +//g;

	if ($debug) {
	    $log->notice("$ltag: read server, host, and port from User-Agent header");
	    $log->notice("$ltag: server_label: $server_label");
	    $log->notice("$ltag: server_host: $server_host");
	    $log->notice("$ltag: server_port: $server_port");
	}

	if ( !($server_label =~ /DsServer/) ) {
	    $log->error("$ltag: invalid user_agent ($server_label): invalid request");
	    return Apache2::Const::FORBIDDEN;
	}

    } else {

	# If not successful, attempt to get these values from inside the message.
	if ($debug) {
	    $log->notice("$ltag: unable to read server, host, and port from User-Agent");
	    if ($user_agent =~ /MdvMsgSender/) {
		$log->notice("$ltag: message originating from MdvMsgSender agent");
	    }
	    $log->notice("$ltag: attempting to retrieve values from message body");
	}

	# Read the whole message into $content
	$r->read($content, $content_len);

	# Look inside the header for the number of parts in the request.
	#  56 bytes in DsMsgHdr_t before nParts
	my $nparts = unpack "x56N", $content;

	# Seek into message length.
	# Length is 8 Code bytes + 12 SKU_header_t bytes +  64 bytes DsMsgPart_t bytes.
	# is 64 bytes -- Start there.
	my $seek_len = 84;
	my $msg_offset = 88;
	my $url_len = 0;

	my $found = 0;
	while ( $found == 0 && $seek_len < ($content_len -24) ) {

	    my $datatype = unpack "x$seek_len N", $content;
	    if ($datatype == 1) {
		$found = 1;
		$seek_len += 4;
		$msg_offset = unpack "x$seek_len N", $content;
		# Add 8 code bytes and 12 SKU_header bytes to get the real location.
		$msg_offset +=  20;
		$seek_len += 4;
		$url_len = unpack "x$seek_len N", $content;
	    }
	    # Length of DsMsgPart_t is 24 bytes.
	    $seek_len += 24;
	}

	if ($found == 0) {
	    $log->error("DsServerTunnel: Could not find URL part of message");
	    return Apache2::Const::DECLINED;
	}

	# Seek to the URL in the message content.
	my $url = unpack "x$msg_offset a$url_len", $content;

	# Extract the host and port.
	my $protocol;
	my $translator;
	my $path;
	($protocol, $translator, $server_host, $server_port, $path) = split /:/, $url;

	# Strip off leading // in host string.
	$server_host =~ s%//%%;

	if ($debug) {
	    $log->notice("$ltag: URI from message body (\$url): $url");
	    $log->notice("$ltag: server_host from message body: $server_host");
	    $log->notice("$ltag: server_port from message body: $server_port");
	}
    }

    # At this point, we should have a host and port.
    if ( !(defined $server_host) ) {
	$log->error("$ltag: no server hostname: invalid request");
	return Apache2::Const::FORBIDDEN;
    }

    if ( !(defined $server_port) ) {
	$log->error("$ltag: no port number: invalid request");
	return Apache2::Const::FORBIDDEN;
    }

    # Verify this is a valid server:port pair as found in the configuration file.
    unless ( grep(/^$server_host:$server_port$/, @valid) ) {
	$log->error("$ltag: disallowed server:port pair: $server_host:$server_port");
	return Apache2::Const::FORBIDDEN;
    }

    # Set up a Socket to the DsServer.
    my ($iaddr, $paddr, $proto);
    if ( !defined($iaddr = inet_aton($server_host)) ) {
	$log->error("$ltag: $server_host not found");
	return Apache2::Const::NOT_FOUND;
    }

    if ( !defined($paddr = sockaddr_in($server_port, $iaddr)) ) {
	return Apache2::Const::SERVER_ERROR;
    }

    if ( !defined($proto = getprotobyname('tcp')) ) {
	return Apache2::Const::SERVER_ERROR;
    }

    if ( !(socket(SOCK, PF_INET, SOCK_STREAM, $proto)) ) {
	$log->error("$ltag: socket failed: $server_host:$server_port");
	return Apache2::Const::NOT_FOUND;
    }

    if ( !(connect(SOCK, $paddr)) ) {
	close(SOCK);
	$log->error("$ltag: connect failed: $server_host:$server_port");
	return Apache2::Const::NOT_FOUND;
    }

    # If we have not yet read the incoming request content...
    if ( !defined($content) ) {

	# Copy the content to the DsServer socket, block by block.
	my $nread = 0;
	my $blocksize = 1024;

	while ($nread < $content_len) {
	    my $nbytes = $content_len - $nread;
	    if ($nbytes > $blocksize) {
		$nbytes = $blocksize;
	    }

	    # Read in nbytes.
	    $r->read($content, $nbytes);
	    $nread += $nbytes;

	    # Write out nbytes.
	    my $len = syswrite(SOCK, $content, $nbytes);
	    $log->notice("$ltag: wrote $nbytes bytes to socket.") if ($debug > 1);
	    if ($len != $nbytes) {
		close(SOCK);
		return Apache2::Const::SERVER_ERROR;
	    }
	}

    } else {
	# Otherwise, copy the content to the DsServer socket all at once.
	my $len = syswrite(SOCK, $content, $content_len);
	$log->notice("$ltag: wrote $len bytes to socket.") if ($debug > 1);
	if ($len != $content_len) {
	    close(SOCK);
	    return Apache2::Const::SERVER_ERROR;
	}
    }

    $log->notice("$ltag: sent $content_len request message bytes") if ($debug);

    # Set the HTTP reply's Document Type and send the MIME header.
    $r->content_type("application/DsServer_protocol");
    # $r->send_http_header;

    # Read the reply on the socket and pass it through to the client.
    my $outbuf;
    while ( ($len = read(SOCK, $outbuf, 1024) ) > 0 ) {
	$log->notice("$ltag: returning $len bytes") if ($debug > 1);
	$r->print("$outbuf");
    }

    close(SOCK);

    $log->notice("========== End DsServerTunnel ==========") if ($debug);

    # Send the Response Code.
    return Apache2::Const::OK;
}
1;
