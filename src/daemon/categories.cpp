/**
 * @file          /ks-ssr-manager/src/daemon/categories.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/categories.h"

namespace KS
{
namespace Daemon
{
#define SSR_CATEGORIES_BASENAME "ssr-categories.ini"
#define SSR_CATEGORY_KEY_LABEL "label"
#define SSR_CATEGORY_KEY_DESCRIPTION "description"
#define SSR_CATEGORY_KEY_ICON_NAME "icon_name"
#define SSR_CATEGORY_KEY_PRIORITY "priority"

Categories::Categories()
{
    this->conf_path_ = Glib::build_filename(SSR_INSTALL_DATADIR, SSR_CATEGORIES_BASENAME);
}

Categories* Categories::instance_ = NULL;
void Categories::global_init()
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
    Glib::KeyFile keyfile;

    try
    {
        keyfile.load_from_file(this->conf_path_);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }

    auto groups_name = keyfile.get_groups();

    for (auto iter = groups_name.begin(); iter != groups_name.end(); ++iter)
    {
        auto group_name = (*iter);
        auto category = std::make_shared<Category>();
        category->name = group_name;
        IGNORE_EXCEPTION(category->label = keyfile.get_locale_string(group_name, SSR_CATEGORY_KEY_LABEL));
        IGNORE_EXCEPTION(category->description = keyfile.get_locale_string(group_name, SSR_CATEGORY_KEY_DESCRIPTION));
        IGNORE_EXCEPTION(category->icon_name = keyfile.get_string(group_name, SSR_CATEGORY_KEY_ICON_NAME));
        IGNORE_EXCEPTION(category->priority = keyfile.get_integer(group_name, SSR_CATEGORY_KEY_PRIORITY));

        this->add_category(category);
    }
}

bool Categories::add_category(std::shared_ptr<Category> category)
{
    RETURN_VAL_IF_FALSE(category, false);

    if (this->categories_.find(category->name) != this->categories_.end())
    {
        KLOG_WARNING("The category is already exist. name: %s.", category->name.c_str());
        return false;
    }

    this->categories_[category->name] = category;
    return true;
}
}  // namespace Daemon
}  // namespace KS