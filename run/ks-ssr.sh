#!/bin/bash
lines=56
sudo tail -n +$lines $0 > /tmp/ks-ssr-rpm.tar.gz
sudo tar -xf /tmp/ks-ssr-rpm.tar.gz -C /opt
sudo cat /etc/.kyinfo | grep 3.3-6C > /tmp/336c.txt
sudo cat /etc/.kyinfo | grep 3.3-6C-2105-101504 > /tmp/336c-2105-101504.txt
sudo rpm -qa |grep kiranwidgets > /tmp/336-kiranwidgets.txt
sudo cat /etc/.kyinfo | grep 3.3-6A-2004-151738 > /tmp/336a-2004-151738.txt
sudo cat /etc/.kyinfo | grep 3.3-6A-2005-140932 > /tmp/336a-2005-140932.txt
if [[ -s /tmp/336c.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/jsoncpp-* /opt/ks-ssr-rpm/qt5-qtsvg-* /opt/ks-ssr-rpm/zlog-* /opt/ks-ssr-rpm/protobuf-* /opt/ks-ssr-rpm/sqlcipher-* /opt/ks-ssr-rpm/Rockey3-* /opt/ks-ssr-rpm/gsettings-qt-*
fi
if [[ ! -s /tmp/336a-2004-151738.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/SDL2-*
fi
if [[ ! -s /tmp/336a-2005-140932.txt ]]; then
        sudo rm -rf /opt/ks-ssr-rpm/qt5-qtsvg-devel-*
fi
if [[ -s /tmp/336c-2105-101504.txt ]]; then
        sudo cp /usr/lib64/liblicense.so /tmp/
        sudo rpm -e kylin-license-core --nodeps
        sudo mv /tmp/liblicense.so /usr/lib64/
fi
if [[ ! -s /tmp/336-kiranwidgets.txt ]]; then
        sudo rpm -U /opt/ks-ssr-rpm/kiran-widgets-qt5-* --nodeps
fi
sudo rm -rf /opt/ks-ssr-rpm/kiran-cpanel-system-*
sudo ls /opt/ks-ssr-rpm > /tmp/rpm_list.txt
sudo rm -rf /opt/ks-ssr/
sudo mkdir -p /opt/ks-ssr/
mkdir test_kiran_widget
cd test_kiran_widget
sudo rpm2cpio /opt/ks-ssr-rpm/kiran-widgets-qt5-*  |cpio -idmv
cd -
sudo mv test_kiran_widget/* /opt/ks-ssr/
sudo rm -rf test_kiran_widget /opt/ks-ssr-rpm/kiran-widgets-qt5-*
#sudo /usr/bin/cp -r /opt/ks-ssr/test_kiran_widget/* /
sudo cat > /tmp/ks-ssr-uninstall <<EOF
sudo rpm -e ks-ssr-gui ks-ssr-manager --nodeps
xdg-desktop-icon uninstall --novendor /usr/share/applications/ks-ssr-gui.desktop
sudo rm -rf /usr/bin/ks-ssr-uninstall
EOF
sudo mv /tmp/ks-ssr-uninstall /usr/bin/ks-ssr-uninstall && sudo chmod 0755 /usr/bin/ks-ssr-uninstall 
sudo rpm -U /opt/ks-ssr-rpm/* --force
sudo rm -rf /tmp/rpm_list.txt /tmp/ks-ssr-rpm.tar.gz /opt/ks-ssr-rpm /tmp/336*.txt
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
