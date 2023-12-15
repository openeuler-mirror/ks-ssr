/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */

#pragma once

#include <QLineEdit>
#include <QProcess>
#include <QWidget>

#include "src/ui/private-box/box-image.h"

class QMouseEvent;
class QMenu;
class BoxManagerProxy;
class QPushButton;
class QLabel;

namespace KS
{
class PasswordModification;

namespace PrivateBox
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

class BoxPasswordRetrieve;
class BoxPasswordChecked;
class MessageDialog;

class Box : public QWidget
{
    Q_OBJECT

public:
    Box(const QString &uid);
    virtual ~Box(){};

    // 获取保险箱UID
    QString getUID() { return m_uid; }

public slots:
    void boxChanged();
    void onIconBtnClick();

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
    void acceptedModifyPassword();
    void acceptedRetrievePassword();
    void acceptedInputMountPassword();
    void acceptedInputDelBoxPassword();

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
    PasswordModification *m_modifyPassword;
    BoxPasswordRetrieve *m_retrievePassword;
    BoxPasswordChecked *m_inputMountPassword;
    BoxPasswordChecked *m_inputDelBoxPassword;
    QMenu *m_popupMenu;
    QAction *m_mountedStatusAction;

    QProcess *m_process;
};
}  // namespace PrivateBox
}  // namespace KS
