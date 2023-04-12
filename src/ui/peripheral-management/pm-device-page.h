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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <QWidget>

namespace Ui {
class PMDevicePage;
}

namespace  KS {

class EditPermissions;
class PMDevicePage : public QWidget
{
    Q_OBJECT

public:
    PMDevicePage(QWidget *parent = nullptr);
    ~PMDevicePage();

protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void editClicked(bool checked);
    void update();

private:
    Ui::PMDevicePage *m_ui;
    EditPermissions *m_editPermission;
};
} //namespace KS

