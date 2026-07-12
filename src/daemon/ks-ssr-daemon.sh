#!/bin/sh


basedir=/usr/libexec

#export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/lib64/ks-ssr/lib64"
export GIO_EXTRA_MODULES='/root/gio/modules'
exec "$basedir"/ks-ssr-daemon
