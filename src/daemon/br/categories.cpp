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

#include "categories.h"
#include <QStringBuilder>

namespace KS
{
namespace BRDaemon
{
#define BR_CATEGORIES_BASENAME "br-categories.ini"
#define BR_CATEGORY_KEY_LABEL "label"
#define BR_CATEGORY_KEY_DESCRIPTION "description"
#define BR_CATEGORY_KEY_ICON_NAME "icon_name"
#define BR_CATEGORY_KEY_PRIORITY "priority"

Categories::Categories()
{
    this->conf_path_ = QDir::cleanPath(SSR_BR_INSTALL_DATADIR "/" BR_CATEGORIES_BASENAME);
    // this->conf_path_ = Glib::build_filename(SSR_BR_INSTALL_DATADIR, BR_CATEGORIES_BASENAME);
}

Categories* Categories::instance_ = NULL;
void Categories::globalInit()
{
    instance_ = new Categories();
    instance_->init();
}

void Categories::init()
{
    this->load();
}

void Categories::load()
{
    QSettings keyfile(this->conf_path_, QSettings::IniFormat);
    auto groups_name = keyfile.childGroups();

    for (auto iter = groups_name.begin(); iter != groups_name.end(); ++iter)
    {
        auto group_name = (*iter);
        auto category = QSharedPointer<Category>(new Category());
        category->name = group_name;
        // 据了解头文件包含 QStringBuilder 会改变 QString 的 operator+ 的行为,但是没有验证过,如果出现性能问题再优化
        category->label = keyfile.value(group_name + '/' + BR_CATEGORY_KEY_LABEL).toString();
        category->description = keyfile.value(group_name + '/' + BR_CATEGORY_KEY_DESCRIPTION).toString();
        category->icon_name = keyfile.value(group_name + '/' + BR_CATEGORY_KEY_ICON_NAME).toString();
        category->priority = keyfile.value(group_name + '/' + BR_CATEGORY_KEY_PRIORITY).toInt();
        this->addCategory(category);
    }
}

bool Categories::addCategory(QSharedPointer<Category> category)
{
    RETURN_VAL_IF_FALSE(category, false);

    if (this->categories_.find(category->name) != this->categories_.end())
    {
        KLOG_WARNING() << "The category is already exist. name: " << category->name.toLatin1();
        return false;
    }

    this->categories_[category->name] = category;
    return true;
}
}  // namespace BRDaemon
}  // namespace KS