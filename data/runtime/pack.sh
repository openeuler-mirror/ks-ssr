#!/bin/bash


buildroot=$1
sourcedir=$2
lib_installdir=${buildroot}/usr/lib64/ks-ssr/lib64
bin_installdir=${buildroot}/usr/lib64/ks-ssr/bin
plugins_installdir=${lib_installdir}/plugins
qtplugins_installdir=${plugins_installdir}/qt/plugins/
gio_modules_installdir=${lib_installdir}/gio/modules

mkdir -p ${lib_installdir}
mkdir -p ${bin_installdir}
mkdir -p ${qtplugins_installdir}
mkdir -p ${gio_modules_installdir}

set +x
DEPS_PATH_LIST=$(ldconfig -p | grep -oP '/[^ ]*')
for libdep in $(cat ${sourcedir}/data/runtime/requires/lib64); do
    dep_path=$(echo $DEPS_PATH_LIST | tr ' ' '\n' | grep $libdep || echo "")
    if [ -z "$dep_path" ];then
        echo "WARNING: miss libdep: ${libdep}"
            continue;
    fi
    cp $dep_path ${lib_installdir}
done

for bindep in $(cat ${sourcedir}/data/runtime/requires/bin); do
    if ! dep_path=$(which ${bindep} 2>/dev/null);then
        echo "WARNING: miss bindep: ${bindep}"
        continue
    fi
    cp $dep_path ${bin_installdir}
    patchelf --force-rpath --set-rpath "/usr/lib64/ks-ssr/lib64" ${bin_installdir}/$bindep
done

set -x
QT_PLUGINS_PATH="/usr/lib64/qt5/plugins/"
cp -r ${QT_PLUGINS_PATH}/* ${qtplugins_installdir}/

GIO_MODULES_PATH="/usr/lib64/gio/modules/"
cp -r ${GIO_MODULES_PATH}/* ${gio_modules_installdir}/