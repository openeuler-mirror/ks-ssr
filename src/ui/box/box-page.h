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
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */

#pragma once

#include <QLineEdit>
#include <QMap>
#include <QWidget>
#include "src/ui/common/titlebar-window.h"

class BoxManagerProxy;
class QJsonObject;

namespace Ui
{
class BoxPage;
}

namespace KS
{
enum INPUT_PASSWORD_TYPE
{
    // 输入mount密码
    MOUNT_PASSWORD = 0,
    // 输入删除box密码
    DELETE_BOX_PASSWORD,
    OTHER_PASSWORD
};

class CreateBox;
class Box;
class ModifyPassword;
class RetrievePassword;
class CustomWindow;

class BoxPage : public QWidget
{
    Q_OBJECT
public:
    BoxPage();
    virtual ~BoxPage(){};

private:
    void initBoxs();
    // 创建并初始化保密箱对象
    Box *buildBox(const QJsonObject &jsonBox);

private:
    void addBox(Box *box);
    void removeBox(const QString &boxUID);
    // 解锁时需输入密码
    TitlebarWindow *buildInputPasswdPage(const QString &boxUID, INPUT_PASSWORD_TYPE type);
    // 提示框
    TitlebarWindow *buildNotifyPage(const QString &notify);
    // 输入密码正确或错误提示
    void inputPasswdNotify(const QString &normal,
                           const QString &error,
                           bool status);

signals:
    void inputMountPasswdClicked(const QString &passwd, const QString &boxUID);
    void inputDelPasswdClicked(const QString &passwd, const QString &boxUID);

private slots:
    void boxAdded(const QString &boxUID, const QString &passphrase);
    void boxDeleted(const QString &boxUID);
    void boxChanged(const QString &boxUID);
    void newBoxClicked(bool checked);
    void createBoxAccepted();
    void showModifyPasswordPage(ModifyPassword *modifyPassword);
    void showRetrievePasswordPage(RetrievePassword *retrievePassword);

private:
    Ui::BoxPage *m_ui;
    BoxManagerProxy *m_boxManagerProxy;
    CustomWindow *m_createBoxPage;
    CreateBox *m_createBox;
    CustomWindow *m_modifyPasswordPage;
    CustomWindow *m_retrievePasswordPage;
    // 所有保密箱对象
    QMap<QString, Box *> m_boxs;
    QLineEdit *m_passwdEdit;
    // 需要输入密码的box ID
    QString m_inputPasswdBoxUID;
};
}  // namespace KS