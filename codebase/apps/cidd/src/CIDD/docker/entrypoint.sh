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
  if [[ -e "$@" ]]; then
    echo "using params file: $@"
  else
    echo "$@ is not a valid params file"
    echo 'please specify a valid file'
    exit 1
  fi
fi

# start CIDD

echo "Starting cidd..."
echo "Run w/ -help to see help information"

PROJ_DIR=${HOME}/projDir

CIDD -v 2 -p $@



