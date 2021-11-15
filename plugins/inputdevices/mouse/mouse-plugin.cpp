/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "plugins/inputdevices/mouse/mouse-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
#include "plugins/inputdevices/mouse/mouse-manager.h"

PLUGIN_EXPORT_FUNC_DEF(MousePlugin);

#define MATE_MOUSE_SCHEMA_ID "org.mate.SettingsDaemon.plugins.mouse"
#define MATE_MOUSE_SCHEMA_KEY_ACTIVE "active"

namespace Kiran
{
MousePlugin::MousePlugin()
{
}

MousePlugin::~MousePlugin()
{
}

void MousePlugin::activate()
{
    KLOG_PROFILE("active mouse plugin.");

    // kiran和mate的相同插件不要同时运行，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_MOUSE_SCHEMA_ID) != schemas.end())
    {
        auto mate_mouse = Gio::Settings::create(MATE_MOUSE_SCHEMA_ID);
        if (mate_mouse->get_boolean(MATE_MOUSE_SCHEMA_KEY_ACTIVE))
        {
            mate_mouse->set_boolean(MATE_MOUSE_SCHEMA_KEY_ACTIVE, false);
        }
    }
    MouseManager::global_init();
}

void MousePlugin::deactivate()
{
    KLOG_PROFILE("deactive mouse plugin.");

    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_MOUSE_SCHEMA_ID) != schemas.end())
    {
        auto mate_mouse = Gio::Settings::create(MATE_MOUSE_SCHEMA_ID);
        if (!mate_mouse->get_boolean(MATE_MOUSE_SCHEMA_KEY_ACTIVE))
        {
            mate_mouse->set_boolean(MATE_MOUSE_SCHEMA_KEY_ACTIVE, true);
        }
    }

    MouseManager::global_deinit();
}
}  // namespace Kiran