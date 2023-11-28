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
class PDF;
}
namespace KS
{
namespace BR
{
class RoundProgressBar;

class PDF : public QWidget
{
    Q_OBJECT

public:
    explicit PDF(const QString &systemName,
                 const QString &IP,
                 const QString &MAC,
                 const QString &kernel,
                 const QString &activeStatus,
                 QWidget *parent = 0);
    virtual ~PDF();

    void setPieChartText(const QString name[],
                         int total[],
                         int conform[],
                         int inconform[]);

private:
    Ui::PDF *m_ui;

    RoundProgressBar *m_pieChart1;
    RoundProgressBar *m_pieChart2;
    RoundProgressBar *m_pieChart3;
    RoundProgressBar *m_pieChart4;
};

}  // namespace BR
}  // namespace KS
