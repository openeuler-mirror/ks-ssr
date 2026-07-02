#!/bin/bash


intltool-merge -x po/ data/ssr-system-rs.xml.in data/ssr-system-rs.xml
sed -i -e 's/xml:lang/lang/g' data/ssr-system-rs.xml
./build/src/tool/kiran-ssr-tool --encrypt-file=./data/ssr-system-rs.xml --private-key=./data/ssr-private.key --output-file=./data/ssr-system-rs.encrypted