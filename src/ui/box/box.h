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

#include <QLabel>

class QMouseEvent;
class QMenu;
class BoxManagerProxy;

namespace KS
{
// struct BoxInfo
// {
//     // uid
//     QString uid;
//     // 名称
//     QString name;
//     // 是否挂载
//     bool mounted;
// };

class ModifyPassword;

class Box : public QLabel
{
    Q_OBJECT

public:
    Box(const QString &boxUID);
    virtual ~Box(){};

    QString getBoxUID() { return this->m_boxUID; }

public Q_SLOTS:
    void boxChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void switchMountedStatus();
    void modifyPassword();

private Q_SLOTS:
    void modifyPasswordAccepted();

private:
    QString m_boxUID;
    BoxManagerProxy *m_boxManagerProxy;
    ModifyPassword *m_modifyPassword;
    QMenu *m_popupMenu;
    QAction *m_mountedStatusAction;
};

}  // namespace KS
