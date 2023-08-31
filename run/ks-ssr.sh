#!/bin/bash
lines=84
sudo tail -n +$lines $0 > /tmp/ks-ssr-rpm.tar.gz
sudo tar -xf /tmp/ks-ssr-rpm.tar.gz -C /opt
mkdir -p /tmp/ks-ssr-adapter
sudo cat /etc/.kyinfo | grep 3.3-6C > /tmp/ks-ssr-adapter/336c.txt
sudo cat /etc/.kyinfo | grep 3.3-6C-2105-101504 > /tmp/ks-ssr-adapter/336c-2105-101504.txt
sudo rpm -qa |grep kiranwidgets > /tmp/ks-ssr-adapter/336-kiranwidgets.txt
sudo cat /etc/.kyinfo | grep 3.3-6A-2004-151738 > /tmp/ks-ssr-adapter/336a-2004-151738.txt
sudo rpm -qa | grep qt5-qtsvg-devel > /tmp/ks-ssr-adapter/336-qtsvg-devel.txt
sudo rpm -qa | grep kiran-log-gtk3-devel > /tmp/ks-ssr-adapter/336-kiran-log-gtk-devel.txt
sudo rpm -qa | grep kiran-log-qt5-devel > /tmp/ks-ssr-adapter/336-kiran-log-qt-devel.txt
sudo rpm -qa | grep qt5-qtbase | grep 5.9.2 > /tmp/ks-ssr-adapter/336-qt.txt
if [[ -s /tmp/336c.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/jsoncpp-* /opt/ks-ssr-rpm/qt5-qtsvg-* /opt/ks-ssr-rpm/zlog-* /opt/ks-ssr-rpm/protobuf-* /opt/ks-ssr-rpm/sqlcipher-* /opt/ks-ssr-rpm/Rockey3-* /opt/ks-ssr-rpm/gsettings-qt-*
fi
if [[ ! -s /tmp/ks-ssr-adapter/336a-2004-151738.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/SDL2-*
fi
if [[ ! -s /tmp/ks-ssr-adapter/336-qt.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/adwaita-qt5-1.0-1.ky3.kb3.x86_64.rpm /opt/ks-ssr-rpm/qgnomeplatform-0.3-3.ky3.kb2.x86_64.rpm 
fi
if [[ ! -s /tmp/ks-ssr-adapter/336-qtsvg-devel.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/qt5-qtsvg-devel-*
fi
if [[ ! -s /tmp/ks-ssr-adapter/336-kiran-log-gtk-devel.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/kiran-log-gtk3-devel-*
fi
if [[ ! -s /tmp/ks-ssr-adapter/336-kiran-log-qt-devel.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/kiran-log-qt5-devel-*
fi
if [[ -s /tmp/ks-ssr-adapter/336c-2105-101504.txt ]]; then
        sudo cp /usr/lib64/liblicense.so /tmp/
        sudo rpm -e kylin-license-core --nodeps
        sudo mv /tmp/liblicense.so /usr/lib64/
fi
if [[ ! -s /tmp/ks-ssr-adapter/336-kiranwidgets.txt ]]; then
        sudo rpm -U /opt/ks-ssr-rpm/kiran-widgets-qt5-* --nodeps
fi
sudo rm -rf /opt/ks-ssr-rpm/kiran-cpanel-system-*
sudo rm -rf /opt/ks-ssr/
sudo mkdir -p /opt/ks-ssr/
mkdir test_kiran_widget
cd test_kiran_widget
sudo rpm2cpio /opt/ks-ssr-rpm/kiran-widgets-qt5-*  |cpio -idmv
sudo rpm2cpio /opt/ks-ssr-rpm/qt5-qtbase-5.9.2-3.ky3.kb4.x86_64.rpm |cpio -idmv
sudo rpm2cpio /opt/ks-ssr-rpm/qt5-qtbase-gui-5.9.2-3.ky3.kb4.x86_64.rpm  |cpio -idmv
sudo rpm2cpio /opt/ks-ssr-rpm/qt5-qtx11extras-5.9.2-1.ky3.kb2.x86_64.rpm  |cpio -idmv
sudo rpm2cpio /opt/ks-ssr-rpm/pcre2-utf16-10.23-2.ky3.kb3.x86_64.rpm  |cpio -idmv
cd -
sudo mv test_kiran_widget/* /opt/ks-ssr/
sudo rm -rf test_kiran_widget /opt/ks-ssr-rpm/kiran-widgets-qt5-*  /opt/ks-ssr-rpm/qt5-qtbase*  /opt/ks-ssr-rpm/qt5-qtx11extras-5.9.2-1.ky3.kb2.x86_64.rpm /opt/ks-ssr-rpm/pcre2-utf16-10.23-2.ky3.kb3.x86_64.rpm
if [[ ! -s /tmp/ks-ssr-adapter/336-qt.txt ]]; then
	sudo /usr/bin/cp -r /opt/ks-ssr/usr/lib64/qt5 /usr/lib64/
else
	sudo rm -rf /opt/ks-ssr-rpm/xcb-util-*
fi
sudo cat > /tmp/ks-ssr-uninstall <<EOF
sudo rpm -e ks-ssr-gui ks-ssr-manager --nodeps
xdg-desktop-icon uninstall --novendor /usr/share/applications/ks-ssr-gui.desktop
sudo rm -rf /usr/bin/ks-ssr-uninstall
EOF
sudo mv /tmp/ks-ssr-uninstall /usr/bin/ks-ssr-uninstall && sudo chmod 0755 /usr/bin/ks-ssr-uninstall 
sudo ls /opt/ks-ssr-rpm | grep qt > /tmp/rpm_list.txt
while read LINE
do
sudo rpm -U /opt/ks-ssr-rpm/$LINE --force --nodeps
sudo rm -rf /opt/ks-ssr-rpm/$LINE
done < /tmp/rpm_list.txt
sudo rpm -U /opt/ks-ssr-rpm/ks-ssr-gui* --force --nodeps
sudo rm -rf /opt/ks-ssr-rpm/ks-ssr-gui*
sudo rpm -U /opt/ks-ssr-rpm/* --force
sudo rm -rf /tmp/rpm_list.txt /tmp/ks-ssr-rpm.tar.gz /opt/ks-ssr-rpm /tmp/ks-ssr-adapter
if [[ ! -s /tmp/336c.txt ]]; then
	sudo rm -rf /etc/profile.d/license.*sh
fi
if [ -f /usr/bin/ky-verify ]; then
	sudo systemctl daemon-reload && sudo systemctl enable ky-verify.service && sudo systemctl restart ky-verify.service
fi	
sudo systemctl daemon-reload && sudo systemctl restart ks-ssr-daemon.service
#sudo gtk-update-icon-cache -f /usr/share/icons/hicolor/
xdg-desktop-icon install --novendor /usr/share/applications/ks-ssr-gui.desktop
exit 0
