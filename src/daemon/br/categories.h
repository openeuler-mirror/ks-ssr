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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include <QDir>
#include <QMap>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QVector>
#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
struct Category
{
    // 分类名
    QString name;
    // 分类标签，会对用户进行显示
    QString label;
    // 分类描述
    QString description;
    // 图标名
    QString icon_name;
    // 分类显示优先级，值越大显示越靠前
    int32_t priority;
};

typedef QVector<QSharedPointer<Category>> CategoryVec;

class Categories
{
public:
    Categories();
    virtual ~Categories(){};

    static Categories* getInstance() { return instance_; };

    static void globalInit();

    static void globalDeinit() { delete instance_; };

    // 获取分类，如果不存在则返回空指针
    QSharedPointer<Category> getCategory(const QString& name) { return MapHelper::getValue(this->categories_, name); };

    // 获取所有分类
    CategoryVec getCategories() { return MapHelper::getValues(this->categories_); };

private:
    // 初始化
    void init();

    // 加载分类配置
    void load();

    // 添加分类
    bool addCategory(QSharedPointer<Category> category);

private:
    static Categories* instance_;

    // 配置文件路径
    QString conf_path_;

    // 所有分类信息：<分类名，分类>
    QMap<QString, QSharedPointer<Category>> categories_;
};
}  // namespace BRDaemon
}  // namespace KS