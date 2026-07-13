#!/bin/bash


buildroot=$1
sourcedir=$2
lib32_installdir=${buildroot}/usr/lib64/ks-ssr/lib
lib64_installdir=${buildroot}/usr/lib64/ks-ssr/lib64
bin_installdir=${buildroot}/usr/lib64/ks-ssr/bin
plugins_installdir=${lib64_installdir}/plugins
qtplugins_installdir=${plugins_installdir}/qt/plugins/
gio_modules_installdir=${lib64_installdir}/gio/modules
python32_installdir=${lib32_installdir}/python
python64_installdir=${lib64_installdir}/python

mkdir -p ${lib64_installdir}
mkdir -p ${bin_installdir}
mkdir -p ${qtplugins_installdir}
mkdir -p ${gio_modules_installdir}
mkdir -p ${python32_installdir}
mkdir -p ${python64_installdir}

set +x
DEPS_PATH_LIST=$(ldconfig -p | grep -oP '/[^ ]*')
for libdep in $(cat ${sourcedir}/data/runtime/requires/lib64); do
    dep_path=$(echo $DEPS_PATH_LIST | tr ' ' '\n' | grep $libdep || echo "")
    if [ -z "$dep_path" ];then
        echo "WARNING: miss libdep: ${libdep}"
            continue;
    fi
    cp $dep_path ${lib64_installdir}
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

# python2 的输出输出至标准错误输出
PYTHON_MAJOR_VERSION=`/usr/bin/python -Esc "import sys; sys.stdout.write('{0.major}.{0.minor}'.format(sys.version_info))"`
PYTHON_MODULES_PATH="/usr/lib/python${PYTHON_MAJOR_VERSION}"
PYTHON_MODULES_PATH_64="/usr/lib64/python${PYTHON_MAJOR_VERSION}"
cp -r ${PYTHON_MODULES_PATH}/* ${python32_installdir}/
cp -r ${PYTHON_MODULES_PATH_64}/* ${python64_installdir}/