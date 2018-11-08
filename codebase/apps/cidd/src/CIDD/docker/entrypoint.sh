#!/usr/bin/env bash

#
# entrypoint.sh: entrypoint to lrose-cidd Docker image
#

#
# centos i386
#

_help () {
  echo 'start CIDD'
  echo 'INPUT (OPTIONAL): directory path from which to invoke cidd'
  exit 0
}

if [[ "$@" == '-h' ]] || [[ "$@" == '--help' ]] ; then
  _help
elif [[ "$@" != '' ]] ; then
  #
  # verify that $@ is a directory
  #
  if [[ -d "$@" ]]; then
    cd $@
  else
    echo "$@ is not a directory"
    echo 'please specify a valid directory'
    exit 1
  fi
fi

echo "Starting cidd..."
echo "Run w/ -h or --help to see help information"

CIDD

