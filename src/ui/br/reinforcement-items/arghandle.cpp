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

#include "arghandle.h"

#include <qt5-log-i.h>
#include <QEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QStyledItemDelegate>
#include <QToolTip>
#include <QVBoxLayout>
#include <QValidator>
#include <QWheelEvent>
#include "common/combobox.h"
#include "common/spinbox.h"

namespace KS
{
namespace BR
{
ArgHandle::ArgHandle(QWidget *parent,
                     const QString &itemKey,
                     const QString &argName,
                     const QString &label,
                     const QString &valueLimits,
                     const QString &inputExample,
                     const QJsonValue &jsonValue,
                     KS::Protocol::WidgetType::Value widgetType,
                     const QString &note)
    : QWidget(parent),
      m_itemKey(itemKey),
      m_argName(argName),
      m_label(label),
      m_valueLimits(valueLimits),
      m_inputExample(inputExample),
      m_note(note),
      m_widgetType(widgetType)

{
    init(jsonValue, widgetType);
}

void ArgHandle::init(const QJsonValue &jsonValue, KS::Protocol::WidgetType::Value widgetType)
{
    switch (widgetType)
    {
    case KS::Protocol::WidgetType::Value::SWITCH:
        initSwitch(jsonValue);
        break;
    case KS::Protocol::WidgetType::Value::TEXT:
        initText(jsonValue);
        break;
    case KS::Protocol::WidgetType::Value::DATETIME:
        initInteger(jsonValue);
        break;
    case KS::Protocol::WidgetType::Value::DEFAULT:
        initDefault(jsonValue);
        break;
    default:
        KLOG_DEBUG() << "error widget type!";
        break;
    }
}

void ArgHandle::initDefault(const QJsonValue &jsonValue)
{
    switch (jsonValue.type())
    {
    // int类型
    case QJsonValue::Type::Double:
        // QJsonValue 无法直接区分 Double 和 Int，但是它在 toInt 尝试获取值时，如果此值不是个整数，则会返回默认值
        initInteger(jsonValue);
        break;
        // 文本类型
    case QJsonValue::Type::String:
        initText(jsonValue);
        break;
        // bool类型
    case QJsonValue::Type::Bool:
        initSwitch(jsonValue);
        break;
    default:
        KLOG_DEBUG() << "error value type!";
    }
}

QHBoxLayout *ArgHandle::buildLabelLayout()
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto widgetLabel = new QLabel(this);
    widgetLabel->setText(m_label);

    if (m_note == "")
    {
        layout->addWidget(widgetLabel);
    }
    else
    {
        auto noteIcon = new QPushButton(this);
        noteIcon->setObjectName("noteIcon");
        noteIcon->setFixedSize(QSize(18, 18));
        // 鼠标手形
        noteIcon->setCursor(QCursor(Qt::PointingHandCursor));
        noteIcon->setIcon(QIcon(":/images/note"));
        noteIcon->setIconSize(QSize(16, 16));

        connect(noteIcon, &QPushButton::clicked, this, [this]
                {
                    QToolTip::showText(QCursor::pos(), m_note, this, rect(), 5000);
                    ;
                });

        layout->addWidget(widgetLabel, 0, Qt::AlignCenter);
        layout->addWidget(noteIcon, 0, Qt::AlignCenter);
        layout->addStretch();
    }

    return layout;
}

void ArgHandle::initSwitch(const QJsonValue &jsonValue)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    auto labelLayout = buildLabelLayout();

    auto widget = new QWidget(this);
    auto hlayout = new QHBoxLayout(widget);
    m_comboBox = new ComboBox(this);
    // 给QCombobox设置代理才能设置下拉列表项的高度
    auto delegate = new QStyledItemDelegate(this);
    m_comboBox->setItemDelegate(delegate);
    if (m_itemKey == "config-umask-limit")
    {
        m_comboBox->addItem("027");
        m_comboBox->addItem("022");
        m_comboBox->addItem("077");
    }
    else
    {
        m_comboBox->addItem(tr("Yes"));
        m_comboBox->addItem(tr("No"));
    }
    m_comboBox->setCurrentIndex(jsonValue.toBool() ? 0 : 1);

    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changedBoolArgs(int)));

    hlayout->addWidget(m_comboBox);

    layout->addLayout(labelLayout);
    layout->addWidget(widget);

    this->setLayout(layout);
    this->show();
}

void ArgHandle::initText(const QJsonValue &jsonValue)
{
    // 由于std::string的特性，传入的字符串会带一个双引号，在QJsonValue中string类型不需要双引号
    auto value = jsonValue.toString();
    value.replace("\"", "");
    auto layout = new QVBoxLayout(this);
    auto labelLayout = buildLabelLayout();
    auto widget = new QWidget(this);
    auto hlayout = new QHBoxLayout(widget);

    m_lineEdit = new QLineEdit(this);
    QRegExp regExp(m_valueLimits);
    m_lineEdit->setValidator(new QRegExpValidator(regExp, this));

    if (value.isEmpty())
        m_lineEdit->setPlaceholderText(m_inputExample);
    else
        m_lineEdit->setText(value);
    m_lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(changedStringArgs(QString)));

    hlayout->addWidget(m_lineEdit);

    layout->addLayout(labelLayout);
    layout->addWidget(widget);

    this->show();
}

void ArgHandle::initInteger(const QJsonValue &jsonValue)
{
    auto layout = new QVBoxLayout(this);
    auto labelLayout = buildLabelLayout();
    auto widget = new QWidget(this);
    auto hlayout = new QHBoxLayout(widget);
    // TODO :  对config-umask-limit做了特殊处理， 看有无更好的方法
    if (m_itemKey == "config-umask-limit")
    {
        m_comboBox = new ComboBox(this);
        // 给QCombobox设置代理才能设置下拉列表项的高度
        auto delegate = new QStyledItemDelegate(this);
        m_comboBox->setItemDelegate(delegate);

        m_comboBox->addItem("027");
        m_comboBox->addItem("022");
        m_comboBox->addItem("077");
        if (jsonValue.toInt() == m_comboBox->itemText(0).toInt())
        {
            m_comboBox->setCurrentIndex(0);
        }
        else if (jsonValue.toInt() == m_comboBox->itemText(1).toInt())
        {
            m_comboBox->setCurrentIndex(1);
        }
        else if (jsonValue.toInt() == m_comboBox->itemText(2).toInt())
        {
            m_comboBox->setCurrentIndex(2);
        }

        connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changedIntArgs(int)));

        hlayout->addWidget(m_comboBox);
    }
    else
    {
        m_spinBox = new SpinBox(this);
        // 利用规则，只对最小值进行限制，不限制最大值的情况。
        m_spinBox->setRange(0, 99999);
        m_spinBox->setGroupSeparatorShown(false);
        m_spinBox->setValue(jsonValue.toInt());

        connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(changedIntArgs(int)));

        hlayout->addWidget(m_spinBox);
    }

    layout->addLayout(labelLayout);
    layout->addWidget(widget);
    this->show();
}

void ArgHandle::changedIntArgs(int value)
{
    m_widgetType = KS::Protocol::WidgetType::Value::DATETIME;
    if (m_itemKey == "config-umask-limit")
    {
        emit valueChanged(m_itemKey, m_argName, m_comboBox->itemText(value), m_widgetType);
    }
    else
    {
        emit valueChanged(m_itemKey, m_argName, QString::number(value), m_widgetType);
    }
}

void ArgHandle::changedBoolArgs(int index)
{
    QString boolText;
    QString yes = tr("Yes");
    QString no = tr("No");
    if (yes == m_comboBox->itemText(index) || m_comboBox->itemText(index) == "027")
    {
        boolText = "true";
    }
    else if (no == m_comboBox->itemText(index) || m_comboBox->itemText(index) == "077" || m_comboBox->itemText(index) == "022")
    {
        boolText = "false";
    }
    m_widgetType = KS::Protocol::WidgetType::SWITCH;
    emit valueChanged(m_itemKey, m_argName, boolText, m_widgetType);
}

void ArgHandle::changedStringArgs(const QString &str)
{
    if (str.isEmpty())
    {
        m_lineEdit->setPlaceholderText(m_inputExample);
    }

    m_widgetType = KS::Protocol::WidgetType::TEXT;
    emit valueChanged(m_itemKey, m_argName, str, m_widgetType);
}

void ArgHandle::argReset()
{
    emit reset(m_itemKey, m_argName);
}

void ArgHandle::confirmType(const QJsonValue &jsonValue)
{
    switch (jsonValue.type())
    {
    case QJsonValue::Type::Double:
        if (m_itemKey == "config-umask-limit")
        {
            if (jsonValue.toInt() == m_comboBox->itemText(0).toInt())
            {
                m_comboBox->setCurrentIndex(0);
            }
            else if (jsonValue.toInt() == m_comboBox->itemText(1).toInt())
            {
                m_comboBox->setCurrentIndex(1);
            }
            else if (jsonValue.toInt() == m_comboBox->itemText(2).toInt())
            {
                m_comboBox->setCurrentIndex(2);
            }
        }
        else
            m_spinBox->setValue(jsonValue.toInt());
        break;
    case QJsonValue::Type::String:
    {
        auto value = jsonValue.toString();
        value.replace("\"", "");
        m_lineEdit->setText(value);
        break;
    }
    case QJsonValue::Type::Bool:
        m_comboBox->setCurrentIndex(jsonValue.toBool() ? 0 : 1);
        break;
    default:
        KLOG_DEBUG() << "error value type!";
    }
}

void ArgHandle::setValue(const QJsonValue &jsonValue)
{
    switch (m_widgetType)
    {
    case KS::Protocol::WidgetType::Value::DATETIME:
        if (m_itemKey == "config-umask-limit")
        {
            if (jsonValue.toInt() == m_comboBox->itemText(0).toInt())
            {
                m_comboBox->setCurrentIndex(0);
            }
            else if (jsonValue.toInt() == m_comboBox->itemText(1).toInt())
            {
                m_comboBox->setCurrentIndex(1);
            }
            else if (jsonValue.toInt() == m_comboBox->itemText(2).toInt())
            {
                m_comboBox->setCurrentIndex(2);
            }
        }
        else
            m_spinBox->setValue(jsonValue.toInt());
        break;
    case KS::Protocol::WidgetType::SWITCH:
        m_comboBox->setCurrentIndex(jsonValue.toBool() ? 0 : 1);
        break;
    case KS::Protocol::WidgetType::TEXT:
    {
        auto value = jsonValue.toString();
        value.replace("\"", "");
        m_lineEdit->setText(value);
        break;
    }
    case KS::Protocol::WidgetType::DEFAULT:
        confirmType(jsonValue);
        break;
    default:
        KLOG_DEBUG() << "Type Error!";
        break;
    }
}

QString ArgHandle::getCategoryName()
{
    return m_itemKey;
}

QString ArgHandle::getArgName()
{
    return m_argName;
}

QString ArgHandle::getLabel()
{
    return m_label;
}

bool ArgHandle::valueCheck()
{
    if (m_widgetType == KS::Protocol::WidgetType::Value::DATETIME && m_itemKey != "config-umask-limit")
    {
        if (m_spinBox->value() < m_valueLimits.toInt())
        {
            m_spinBox->setValue(m_valueLimits.toInt());
            return false;
        }
    }
    else if (m_widgetType == KS::Protocol::WidgetType::Value::TEXT)
    {
        QRegExp regex(m_valueLimits);
        if (m_lineEdit->text().isEmpty())
            return true;
        if (!regex.exactMatch(m_lineEdit->text()))
        {
            m_lineEdit->setText("");
            return false;
        }
    }
    return true;
}

int ArgHandle::getValueLimits()
{
    return m_valueLimits.toInt();
}

KS::Protocol::WidgetType::Value ArgHandle::getWidgetType()
{
    return m_widgetType;
}
}  // namespace BR
}  // namespace KS
