# 系统安全加固
该项目为系统安全加固非图形程序

## 编译安装
```
# yum install glib2-devel glibmm24-devel kiran-log-gtk3-devel jsoncpp-devel gtest-devel fmt-devel cryptopp-devel xerces-c-devel python2-devel gdbus-codegen-glibmm systemd xsd intltool sed
# mkdir build
# cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr ..
# make
# make install
```

## 运行

后端运行

```
systemctl start ks-ssr-daemon.service
```

## 生成ssr-system-rs.encrypted文件

由于xsd不支持属性名包含':'字符，所以这里将xml:lang替换成了lang

```
intltool-merge -x po/ data/ssr-system-rs.xml.in data/ssr-system-rs.xml
sed -i -e 's/xml:lang/lang/g' ssr-system-rs.xml
./build/src/tool/ks-ssr-tool --encrypt-file=./data/ssr-system-rs.xml --private-key=./data/ssr-private.key --output-file=./data/ssr-system-rs.encrypted
```

## 日志
日志请到/var/log/kylinsec/ks-ssr-manager/目录中查看

## 其他参考
xsd: w3school.com.cn/schema/index.asp
