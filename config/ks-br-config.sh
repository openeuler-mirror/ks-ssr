#!/bin/sh


basedir=/usr/libexec

export LD_LIBRARY_PATH="LD_LIBRARY_PATH:/opt/ks-ssr/usr/lib64"

exec "$basedir"/ks-br-config "$@"
