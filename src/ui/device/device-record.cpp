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

#include "src/ui/device/device-record.h"
#include "src/ui/device/device-permission.h"
#include "src/ui/device/device-record-delegate.h"
#include "src/ui/device/table-filter-model.h"
#include "src/ui/ui_device-record.h"

#include <kiran-log/qt5-log-i.h>
#include <QHeaderView>
#include <QPainter>
#include <QStyleOption>

namespace KS
{
DeviceRecord::DeviceRecord(QWidget *parent) : QWidget(parent),
                                              m_ui(new Ui::DeviceRecord)
{
    m_ui->setupUi(this);
    m_ui->m_title->setText(tr("Connect Record"));
    m_ui->m_records->setText(tr("0 records in total"));
    m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);

    initTableStyle();

    connect(m_ui->m_search, &QLineEdit::textChanged, this, &DeviceRecord::searchTextChanged);
}

DeviceRecord::~DeviceRecord()
{
    delete m_ui;
}

void DeviceRecord::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DeviceRecord::initTableStyle()
{
    m_ui->m_table->setHeaderSections(QStringList() << tr("Device Name")
                                                   << tr("Device Type")
                                                   << tr("Device Time")
                                                   << tr("Device Status"));
    auto headView = m_ui->m_table->horizontalHeader();
    headView->resizeSection(RecordTableField::RECORD_TABLE_FIELD_NAME, 200);
    headView->resizeSection(RecordTableField::RECORD_TABLE_FIELD_TIME, 200);
    headView->resizeSection(RecordTableField::RECORD_TABLE_FIELD_TYPE, 200);

    m_ui->m_table->setItemDelegate(new DeviceRecordDelegate(this));

    //just test add table data.
    QList<RecordsInfo> infos;
    for (int i = 0; i < 50; i++)
    {
        auto deviceInfo = RecordsInfo{.name = "1",
                                      .type = i,
                                      .time = "1",
                                      .status = i};
        infos << deviceInfo;
    }
    m_ui->m_table->setData(infos);
}

void DeviceRecord::searchTextChanged(const QString &text)
{
    KLOG_DEBUG() << "The search text is change to " << text;

    auto filterProxy = this->m_ui->m_table->getFilterProxy();
    filterProxy->setFilterFixedString(text);
}

}  //namespace KS
