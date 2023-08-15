/**
 * @file          /kiran-sse-manager/lib/core/core-work.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/core/core-work.h"
#include "sse-config.h"

namespace Kiran
{
void core_init()
{
    SSEConfiguration::global_init(SSE_INSTALL_DATADIR "/sse.ini");
    SSECategories::global_init();
    SSEPlugins::global_init(SSEConfiguration::get_instance());
}

void core_deinit()
{
    SSEPlugins::global_deinit();
    SSECategories::global_deinit();
    SSEConfiguration::global_deinit();
}
}  // namespace Kiran
