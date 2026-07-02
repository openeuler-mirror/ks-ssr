/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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

#include <QWidget>

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

class Box : public QWidget
{
    Q_OBJECT

public:
    Box(const QString &uid);
    virtual ~Box(){};

    // 获取保险箱UID
    QString getUID() { return this->m_uid; }

public Q_SLOTS:
    void boxChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void initBox();
    void initBoxInfo();
    void initMenu();

    void switchMountedStatus();
    void modifyPassword();

private Q_SLOTS:
    void modifyPasswordAccepted();

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

    BoxManagerProxy *m_boxManagerProxy;
    ModifyPassword *m_modifyPassword;
    QMenu *m_popupMenu;
    QAction *m_mountedStatusAction;
};

}  // namespace KS
