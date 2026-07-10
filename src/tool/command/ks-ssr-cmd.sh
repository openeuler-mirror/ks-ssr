#!/bin/sh


basedir=/usr/libexec

#export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/lib64/ks-ssr/lib64"

exec "$basedir"/ks-ssr-cmd "$@"
