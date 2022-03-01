/**
 * @file          /ks-ssr-manager/src/daemon/categories.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace KS
{
namespace Daemon
{
struct Category
{
    // 分类名
    std::string name;
    // 分类标签，会对用户进行显示
    std::string label;
    // 分类描述
    std::string description;
    // 图标名
    std::string icon_name;
    // 分类显示优先级，值越大显示越靠前
    int32_t priority;
};

using CategoryVec = std::vector<std::shared_ptr<Category>>;

class Categories
{
public:
    Categories();
    virtual ~Categories(){};

    static Categories* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    // 获取分类，如果不存在则返回空指针
    std::shared_ptr<Category> get_category(const std::string& name) { return MapHelper::get_value(this->categories_, name); };

    // 获取所有分类
    CategoryVec get_categories() { return MapHelper::get_values(this->categories_); };

private:
    // 初始化
    void init();

    // 加载分类配置
    void load();

    // 添加分类
    bool add_category(std::shared_ptr<Category> category);

private:
    static Categories* instance_;

    // 配置文件路径
    std::string conf_path_;

    // 所有分类信息：<分类名，分类>
    std::map<std::string, std::shared_ptr<Category>> categories_;
};
}  // namespace Daemon
}  // namespace KS