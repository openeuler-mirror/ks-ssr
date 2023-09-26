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

#include "br-plugin-i.h"
#include "lib/base/base.h"

namespace KS
{
namespace Network
{
class BRPluginNetwork : public BRPluginInterface
{
public:
    BRPluginNetwork(){};
    virtual ~BRPluginNetwork(){};

    virtual void activate();

    virtual void deactivate();

    virtual std::shared_ptr<BRReinforcementInterface> get_reinforcement(const std::string &name) { return MapHelper::get_value(this->reinforcements_, name); };

private:
    std::map<std::string, std::shared_ptr<BRReinforcementInterface>> reinforcements_;
};
}  // namespace Network

}  // namespace KS
