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

#include <QList>
#include "ssr-i.h"

// 文件扫描 非法数据相关定义
#define CHECK_INVALID_CVE_VULNERABLITY "config-vulnerability-scanning"
#define CHECK_INVALID_NOUSER_FILES "config-nouser-files"
#define CHECK_INVALID_AUTHORITY_FILES "config-authority-files"
#define CHECK_INVALID_SUID_SGID_FILES "config-suid-sgid-files"

#define CHECK_INVALID_CVE_VULNERABLITY_KEY "invalid_rpms"
#define CHECK_INVALID_NOUSER_FILES_KEY "nouser_files"
#define CHECK_INVALID_AUTHORITY_FILES_KEY "authority_files"
#define CHECK_INVALID_SUID_SGID_FILES_KEY "suid_sgid_files"

struct InvalidData
{
    QList<QString> vulnerabilityScanInvalidList = {""};
    QList<QString> NouserFilesList = {""};
    QList<QString> AuthorityFilesList = {""};
    QList<QString> SuidSgidFilesList = {""};
};
