/**
 * @file          /kiran-sse-manager/lib/core/sse-categories.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/core/sse-categories.h"
#include "sse-config.h"

namespace Kiran
{
#define SSE_CATEGORIES_BASENAME "sse-categories.ini"
#define SSE_CATEGORY_KEY_LABEL "label"
#define SSE_CATEGORY_KEY_COMMENT "comment"
#define SSE_CATEGORY_KEY_ICON_NAME "icon_name"
#define SSE_CATEGORY_KEY_PRIORITY "priority"

SSECategories::SSECategories()
{
    this->conf_path_ = Glib::build_filename(SSE_INSTALL_DATADIR, SSE_CATEGORIES_BASENAME);
}

SSECategories* SSECategories::instance_ = nullptr;
void SSECategories::global_init()
{
    instance_ = new SSECategories();
    instance_->init();
}

void SSECategories::init()
{
    this->load();
}

void SSECategories::load()
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
        auto category = std::make_shared<SSECategory>();
        category->name = group_name;
        IGNORE_EXCEPTION(category->label = keyfile.get_locale_string(group_name, SSE_CATEGORY_KEY_LABEL));
        IGNORE_EXCEPTION(category->comment = keyfile.get_locale_string(group_name, SSE_CATEGORY_KEY_COMMENT));
        IGNORE_EXCEPTION(category->icon_name = keyfile.get_string(group_name, SSE_CATEGORY_KEY_ICON_NAME));
        IGNORE_EXCEPTION(category->priority = keyfile.get_integer(group_name, SSE_CATEGORY_KEY_PRIORITY));

        this->add_category(category);
    }
}

bool SSECategories::add_category(std::shared_ptr<SSECategory> category)
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