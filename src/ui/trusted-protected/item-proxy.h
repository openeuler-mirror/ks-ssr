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
#ifndef ITEMPROXY_H
#define ITEMPROXY_H

#include <QLabel>
#include <QWidget>

namespace KS
{
class ItemProxy : public QWidget
{
    Q_OBJECT
public:
    ItemProxy(const QString &text,
              const QString &icon,
              int type,
              QWidget *parent = nullptr);
    virtual ~ItemProxy(){};

    void showArrow(bool isShow);

private:
    void initUI(const QString &icon);

private:
    QString m_text;
    int m_type;
    QLabel *m_rightIcon;
};
}  // namespace KS
#endif  // ITEMPROXY_H
