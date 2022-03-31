#!/bin/bash
sudo cat > /tmp/KS-SSR.repo <<EOF
[KS-SSR]
name=KylinSec-$releasever - KS-SSR
gpgcheck=0
baseurl=http://192.168.120.17/kojifiles/repos/KY3.3-6-PG-Kiran-SSR-1.0-build/latest/x86_64/
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-kylin-release

[KS-SSR1]
name=KylinSec-$releasever - KS-SSR1
gpgcheck=0
baseurl=http://192.168.120.17/kojifiles/repos/KY3.3-6-PG-Kiran-base-2.2-build/latest/x86_64/
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-kylin-release

[KS-SSR2]
name=KylinSec-$releasever - KS-SSR2
gpgcheck=0
baseurl=http://192.168.120.17/kojifiles/repos/KY3.3-6-PG-KiranUI-2.2-build/latest/x86_64/
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-kylin-release
EOF
sudo mv /tmp/KS-SSR.repo /etc/yum.repos.d/
sudo yum -y install ks-ssr-gui ks-ssr-manager zlog --downloadonly --downloaddir=install-ks-ssr-rpm
mv install-ks-ssr-rpm ks-ssr-rpm
sudo tar cvf install-ssr-rpm.tar.gz ks-ssr-rpm
cat ./ks-ssr.sh install-ssr-rpm.tar.gz > ks-ssr-1.0.run
sudo chmod 0755 ks-ssr-1.0.run
sudo rm -rf /tmp/KS-SSR.repo install-ssr-rpm.tar.gz ks-ssr-rpm
