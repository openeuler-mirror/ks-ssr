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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */
#pragma once

#include <QWidget>

namespace Ui
{
class PDFDetails;
}
namespace KS
{
namespace BR
{
class PDFDetails : public QWidget
{
    Q_OBJECT

public:
    explicit PDFDetails(QWidget *parent,
                   bool isOpenFilesScan = false,
                   bool isScanVulnerability = false);
    virtual ~PDFDetails();

    void addSpacer();
    void showTailBar();
    void addLine(const QString &name,
                 const QString &scanResult,
                 const QColor &scanColor,
                 const QString &backgroundColor);
    void addScanLine(const QString &filesName,
                     const QString &scanType,
                     const QString &remarks,
                     const QString &backgroundColor);

private:
    Ui::PDFDetails *m_ui;

    int m_rowHeight;
};
}  // namespace BR
}  // namespace KS
