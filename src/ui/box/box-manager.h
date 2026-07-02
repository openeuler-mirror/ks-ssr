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

#include <QMap>
#include <QWidget>

class BoxManagerProxy;
class QJsonObject;

namespace Ui
{
class BoxManager;
}

namespace KS
{
class CreateBox;
class Box;

class BoxManager : public QWidget
{
    Q_OBJECT
public:
    BoxManager();
    virtual ~BoxManager(){};

private:
    void initBoxs();
    // 创建并初始化保密箱对象
    Box *buildBox(const QJsonObject &jsonBox);

private:
    void addBox(Box *box);
    void removeBox(const QString &boxUID);

private slots:
    void boxAdded(const QString &boxUID);
    void boxDeleted(const QString &boxUID);
    void boxChanged(const QString &boxUID);
    void newBoxClicked(bool checked);
    void createBoxAccepted();

private:
    Ui::BoxManager *m_ui;
    BoxManagerProxy *m_boxManagerProxy;
    CreateBox *m_createBox;
    // 所有保密箱对象
    QMap<QString, Box *> m_boxs;
};
}  // namespace KS
