/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QLineEdit>
#include <QProcess>
#include <QWidget>

#include "src/ui/box/box-image.h"

class QMouseEvent;
class QMenu;
class BoxManagerProxy;
class QPushButton;
class QLabel;

namespace KS
{
struct BoxInfo
{
    // uid
    QString uid;
    // 名称
    QString name;
    // 是否挂载
    bool mounted;
};

class ModifyPassword;
class RetrievePassword;

class Box : public QWidget
{
    Q_OBJECT

public:
    Box(const QString &uid);
    virtual ~Box();

    // 获取保险箱UID
    QString getUID() { return this->m_uid; }

public slots:
    void boxChanged();
    void onIconBtnClick();
    void checkMountPasswd(const QString &passwd, const QString &boxUID);
    void checkDelPasswd(const QString &passwd, const QString &boxUID);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void initBox();
    void initBoxInfo();
    void initMenu();

    void switchMountedStatus();
    void modifyPassword();
    void delBox();
    void retrievePassword();

private slots:
    void modifyPasswordAccepted();
    void retrievePasswordAccepted();

signals:
    void unlockedClicked(const QString &boxUID);
    // mount密码检测结果
    void checkMountPasswdResult(bool status);
    void delBoxClicked(const QString &boxUID);
    // 左键点击未解锁图标
    void unUnlockedIconClicked();
    // 删除box密码检测结果
    void checkDelPasswdResult(bool status);
    // 修改密码 输入密码检测结果
    void checkModifyPasswdResult(bool status);
    // 找回密码 口令检测结果
    void checkRetrievePasswordResult(bool status);
    // 显示修改密码页面
    void modifyPasswordClicked(ModifyPassword *modifyPassword);
    // 显示找回密码页面
    void retrievePasswordClicked(RetrievePassword *retrievePassword);

private:
    // 保险箱唯一标识
    QString m_uid;
    // 保险箱名称
    QString m_name;
    // 保险箱打开状态
    bool m_mounted;

    // 保险箱显示图标
    QPushButton *m_showingIcon;
    // 保险箱显示名称
    QLabel *m_showingName;

    BoxImage *m_imageLock;
    BoxImage *m_imageUnlock;

    BoxManagerProxy *m_boxManagerProxy;
    ModifyPassword *m_modifyPassword;
    RetrievePassword *m_retrievePassword;
    QMenu *m_popupMenu;
    QAction *m_mountedStatusAction;

    QProcess *m_process;
};

}  // namespace KS
