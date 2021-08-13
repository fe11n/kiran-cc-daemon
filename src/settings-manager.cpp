/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "src/settings-manager.h"

#include "lib/base/base.h"

namespace Kiran
{
#ifdef KCC_SYSTEM_TYPE
#define CC_DAEMON_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon"
#elif KCC_SESSION_TYPE
#define CC_DAEMON_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon"
#else
#define CC_DAEMON_DBUS_NAME "com.kylinsec.Kiran.CCDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/CCDaemon"
#error need to define KCC_SYSTEM_TYPE or KCC_SESSION_TYPE
#endif

#define PLUGIN_CONFIG_NAME "plugin_options"

SettingsManager::SettingsManager() : dbus_connect_id_(0),
                                     object_register_id_(0)
{
}

SettingsManager::~SettingsManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

SettingsManager* SettingsManager::instance_ = nullptr;
void SettingsManager::global_init()
{
    instance_ = new SettingsManager();
    instance_->init();
}

std::shared_ptr<PluginHelper> SettingsManager::lookup_plugin(const std::string& id)
{
    auto iter = this->plugins_.find(id);
    if (iter != this->plugins_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void SettingsManager::GetPlugins(MethodInvocation& invocation)
{
    KLOG_PROFILE("");
    std::vector<Glib::ustring> ids;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        ids.push_back(iter->first);
    }
    invocation.ret(ids);
}

void SettingsManager::GetActivatedPlugins(MethodInvocation& invocation)
{
    KLOG_PROFILE("");
    std::vector<Glib::ustring> ids;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        if (iter->second && iter->second->activate())
        {
            ids.push_back(iter->first);
        }
    }
    invocation.ret(ids);
}

void SettingsManager::ActivatePlugin(const Glib::ustring& id, MethodInvocation& invocation)
{
    KLOG_PROFILE("plugin id: %s.", id.c_str());
    auto plugin = this->lookup_plugin(id);
    if (!plugin)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_PLUGIN_NOT_EXIST_1);
    }

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!plugin->activate_plugin(error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    invocation.ret();
}

void SettingsManager::DeactivatePlugin(const Glib::ustring& id, MethodInvocation& invocation)
{
    KLOG_PROFILE("plugin id: %s.", id.c_str());
    auto plugin = this->lookup_plugin(id);
    if (!plugin)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_PLUGIN_NOT_EXIST_2);
    }

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!plugin->deactivate_plugin(error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    invocation.ret();
}

void SettingsManager::init()
{
    KLOG_PROFILE("");

    if (!g_module_supported())
    {
        KLOG_WARNING("kiran-session-daemon is not able to initialize the plugins.");
        return;
    }

    // load  plugins
    auto plugin_config_path = Glib::build_filename(KCC_PLUGIN_DIR, PLUGIN_CONFIG_NAME);
    this->load_plugins(plugin_config_path);

    // connect and regist dbus
    this->dbus_init();
}

void SettingsManager::load_plugins(const std::string& file_name)
{
    KLOG_PROFILE("file_name: %s.", file_name.c_str());

    Glib::KeyFile plugins_file;

    try
    {
        plugins_file.load_from_file(file_name);

        for (const auto& group : plugins_file.get_groups())
        {
            PluginInfo info;
            info.id = group;
            info.name = plugins_file.get_string(group, "Name");
            info.description = plugins_file.get_string(group, "Description");
            auto available = plugins_file.get_boolean(group, "Available");
            if (available)
            {
                auto plugin_helper = std::make_shared<PluginHelper>(info);
                CCErrorCode error_code = CCErrorCode::SUCCESS;
                if (!plugin_helper->activate_plugin(error_code))
                {
                    KLOG_WARNING("Failed to load plugin, name: %s.", info.name.c_str());
                }
                else
                {
                    this->plugins_.emplace(info.id, plugin_helper);
                }
            }
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to load plugins: %s.", e.what().c_str());
    }
}

void SettingsManager::dbus_init()
{
#ifdef KCC_SYSTEM_TYPE
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 CC_DAEMON_DBUS_NAME,
                                                 sigc::mem_fun(this, &SettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_lost));
#elif KCC_SESSION_TYPE

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 CC_DAEMON_DBUS_NAME,
                                                 sigc::mem_fun(this, &SettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_lost));
#endif
}  // namespace Kiran

void SettingsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                      Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, CC_DAEMON_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("register object_path %s fail: %s.", CC_DAEMON_OBJECT_PATH, e.what().c_str());
    }
}

void SettingsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                       Glib::ustring name)
{
    KLOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void SettingsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                   Glib::ustring name)
{
    KLOG_WARNING("failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran