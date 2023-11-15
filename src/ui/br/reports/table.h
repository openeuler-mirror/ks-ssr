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

#include <QWidget>

namespace Ui
{
class Table;
}
namespace KS
{
namespace BR
{
namespace Reports
{
class Table : public QWidget
{
    Q_OBJECT

public:
    explicit Table(bool isOpenFilesScan = false,
                   bool isScanVulnerability = false,
                   QWidget *parent = 0);
    ~Table();

    void addSpacer();
    void showTailBar();
    void addOneLine(const QString &name,
                    const QString &scanResult,
                    const QString &reinforceResult,
                    const QString &remarks,
                    const QColor &scanColor,
                    const QColor &reinforceColor,
                    const QString &backgroundColor);
    void addOneScanLine(const QString &filesName,
                        const QString &scanType,
                        const QString &remarks,
                        const QString &backgroundColor);

private:
    Ui::Table *m_ui;

    int m_rowHeight = 36;
};
}  // namespace Reports
}  // namespace BR
}  // namespace KS
