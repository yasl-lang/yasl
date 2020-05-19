#!/bin/bash

# Check that yasl exists
if [[ ! -f yasl ]]; then
    echo 'Compile `yasl` first'
    exit 1
fi

# Check that the yaslapi exists
if [[ ! -f libyaslapi.a ]]; then
    echo 'Compile `libyaslapi.a` first'
    exit 1
fi

# if the include directory for yasl doesn't exist, create it.
if [[ ! -d /usr/local/include/yasl ]]; then
    mkdir /usr/local/include/yasl
fi

# Copy the files to the appropriate locations
cp yasl.h yasl_error.h yasl_aux.h yasl_conf.h /usr/local/include/yasl/
cp libyaslapi.a /usr/local/lib/
cp yasl /usr/local/bin/
