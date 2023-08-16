/**
 * @file          /kiran-ssr-manager/src/daemon/ssr-categories.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "src/daemon/ssr-categories.h"
#include "ssr-config.h"

namespace Kiran
{
#define SSR_CATEGORIES_BASENAME "ssr-categories.ini"
#define SSR_CATEGORY_KEY_LABEL "label"
#define SSR_CATEGORY_KEY_COMMENT "comment"
#define SSR_CATEGORY_KEY_ICON_NAME "icon_name"
#define SSR_CATEGORY_KEY_PRIORITY "priority"

SSRCategories::SSRCategories()
{
    this->conf_path_ = Glib::build_filename(SSR_INSTALL_DATADIR, SSR_CATEGORIES_BASENAME);
}

SSRCategories* SSRCategories::instance_ = nullptr;
void SSRCategories::global_init()
{
    instance_ = new SSRCategories();
    instance_->init();
}

void SSRCategories::init()
{
    this->load();
}

void SSRCategories::load()
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

    for (const auto& group_name : groups_name)
    {
        auto category = std::make_shared<SSRCategory>();
        category->name = group_name;
        IGNORE_EXCEPTION(category->label = keyfile.get_locale_string(group_name, SSR_CATEGORY_KEY_LABEL));
        IGNORE_EXCEPTION(category->comment = keyfile.get_locale_string(group_name, SSR_CATEGORY_KEY_COMMENT));
        IGNORE_EXCEPTION(category->icon_name = keyfile.get_string(group_name, SSR_CATEGORY_KEY_ICON_NAME));
        IGNORE_EXCEPTION(category->priority = keyfile.get_integer(group_name, SSR_CATEGORY_KEY_PRIORITY));

        this->add_category(category);
    }
}

bool SSRCategories::add_category(std::shared_ptr<SSRCategory> category)
{
    RETURN_VAL_IF_FALSE(category, false);

    auto iter = this->categories_.emplace(category->name, category);
    if (!iter.second)
    {
        KLOG_WARNING("The category is already exist. name: %s.", category->name.c_str());
        return false;
    }
    return true;
}
}  // namespace Kiran