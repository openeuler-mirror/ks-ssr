#!/bin/sh


basedir=/usr/libexec

#export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/lib64/ks-ssr/lib64"
export GIO_EXTRA_MODULES='/usr/lib64/ks-ssr/lib64/gio/modules/'
export GIO_MODULE_DIR='/usr/lib64/ks-ssr/lib64/gio/modules/'
export PYTHONPATH="/usr/lib64/ks-ssr/lib64/python/lib-dynload/:/usr/lib64/ks-ssr/lib/python/lib-dynload/:/usr/lib64/ks-ssr/lib64/python/:/usr/lib64/ks-ssr/lib64/python/site-packages/:/usr/lib64/ks-ssr/lib/python/:/usr/lib64/ks-ssr/lib/python/site-packages/:${PYTHONPATH}"
exec "$basedir"/ks-ssr-daemon
