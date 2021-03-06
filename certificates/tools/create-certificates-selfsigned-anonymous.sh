#! /usr/bin/env bash

#
#  create-certificates-selfsigned.sh 
#
#  Copyright (C) 2010 by Manfred Morgner
#  manfred@morgner.com
#
#  This script creates a bunch of certifictes to work with for
#  testing and/or developing (on) the 'flow' system
#
#  Usage: create-certificates-selfsigned.sh name1 name2 name3
#
#  This creates self-signed certificates for name1, name2, name3
#
#     name1.crt (as also name1.key and name1.csr) and so on
#  ---
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#

# DES may be nodes to generate the key unencrypted or des3 to do otherwise
DES=nodes

for NAME in $*
  do
  openssl req -new -utf8 -newkey rsa:1024 -${DES} -subj "/CN=John Doe" -keyout ${NAME}.key -out ${NAME}.csr
  openssl x509 -req -days 7300 -in ${NAME}.csr -signkey ${NAME}.key -out ${NAME}.crt
  openssl rsa -pubout -in ${NAME}.key -out ${NAME}.pub
  done

if [ $# -lt 1 ]
  then
  echo "Usage: `basename $0` name1 name2 name3"
  fi
