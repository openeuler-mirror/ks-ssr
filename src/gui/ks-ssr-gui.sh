#!/bin/sh


basedir=/usr/libexec

#export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/lib64/ks-ssr/lib64"
export QT_PLUGIN_PATH="/usr/lib64/ks-ssr/lib64/plugins/qt/plugins/"

exec "$basedir"/ks-ssr-gui
