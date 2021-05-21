/*
 * @Author       : tangjie02
 * @Date         : 2020-08-24 16:20:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-03 09:31:23
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-manager.cpp
 */

#include "plugins/keybinding/keybinding-manager.h"

#include "keybinding-i.h"

namespace Kiran
{
KeybindingManager::KeybindingManager(SystemShortCutManager *system) : system_shorcut_manager_(system),
                                                                      dbus_connect_id_(0),
                                                                      object_register_id_(0)
{
}

KeybindingManager::~KeybindingManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

KeybindingManager *KeybindingManager::instance_ = nullptr;
void KeybindingManager::global_init(SystemShortCutManager *system)
{
    instance_ = new KeybindingManager(system);
    instance_->init();
}

void KeybindingManager::AddCustomShortcut(const Glib::ustring &name,
                                          const Glib::ustring &action,
                                          const Glib::ustring &key_combination,
                                          MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    auto custom_shortcut = std::make_shared<CustomShortCut>(name, action, key_combination);
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    auto uid = CustomShortCutManager::get_instance()->add(custom_shortcut, error_code);
    if (error_code != CCErrorCode::SUCCESS)
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    else
    {
        invocation.ret(uid);
        this->Added_signal.emit(std::make_tuple(uid, std::string(CUSTOM_SHORTCUT_KIND)));
    }
}

void KeybindingManager::ModifyCustomShortcut(const Glib::ustring &uid,
                                             const Glib::ustring &name,
                                             const Glib::ustring &action,
                                             const Glib::ustring &key_combination,
                                             MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    auto custom_shortcut = std::make_shared<CustomShortCut>(name, action, key_combination);
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!CustomShortCutManager::get_instance()->modify(uid.raw(), custom_shortcut, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    else
    {
        invocation.ret();
        this->Changed_signal.emit(std::make_tuple(uid, std::string(CUSTOM_SHORTCUT_KIND)));
    }
}

void KeybindingManager::DeleteCustomShortcut(const Glib::ustring &uid, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!CustomShortCutManager::get_instance()->remove(uid, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    else
    {
        invocation.ret();
        this->Deleted_signal.emit(std::make_tuple(uid, std::string(CUSTOM_SHORTCUT_KIND)));
    }
}

void KeybindingManager::GetCustomShortcut(const Glib::ustring &uid, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    auto custom_shortcut = CustomShortCutManager::get_instance()->get(uid.raw());
    if (!custom_shortcut)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_3);
    }
    invocation.ret(std::make_tuple(uid, custom_shortcut->name, custom_shortcut->action, custom_shortcut->key_combination));
}

void KeybindingManager::ListCustomShortcuts(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    auto custom_shortcuts = CustomShortCutManager::get_instance()->get();
    std::vector<std::tuple<Glib::ustring, Glib::ustring, Glib::ustring, Glib::ustring>> result;
    for (const auto &shortcut : custom_shortcuts)
    {
        result.push_back(std::make_tuple(shortcut.first, shortcut.second->name, shortcut.second->action, shortcut.second->key_combination));
    }
    invocation.ret(result);
}

void KeybindingManager::ModifySystemShortcut(const Glib::ustring &uid,
                                             const Glib::ustring &key_combination,
                                             MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!SystemShortCutManager::get_instance()->modify(uid.raw(), key_combination.raw(), error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void KeybindingManager::GetSystemShortcut(const Glib::ustring &uid, MethodInvocation &invocation)
{
    auto system_shortcut = this->system_shorcut_manager_->get(uid);
    if (!system_shortcut)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST_2);
    }
    invocation.ret(std::make_tuple(uid, system_shortcut->kind, system_shortcut->name, system_shortcut->key_combination));
}

void KeybindingManager::ListSystemShortcuts(MethodInvocation &invocation)
{
    auto system_shortcuts = this->system_shorcut_manager_->get();
    std::vector<std::tuple<Glib::ustring, Glib::ustring, Glib::ustring, Glib::ustring>> result;
    for (const auto &shortcut : system_shortcuts)
    {
        result.push_back(std::make_tuple(shortcut.first, shortcut.second->kind, shortcut.second->name, shortcut.second->key_combination));
    }
    invocation.ret(result);
}

void KeybindingManager::ListShortcuts(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    std::vector<std::tuple<Glib::ustring, Glib::ustring, Glib::ustring, Glib::ustring>> result;

    auto custom_shortcuts = CustomShortCutManager::get_instance()->get();
    for (const auto &shortcut : custom_shortcuts)
    {
        result.push_back(std::make_tuple(shortcut.first, CUSTOM_SHORTCUT_KIND, shortcut.second->name, shortcut.second->key_combination));
    }

    auto system_shortcuts = this->system_shorcut_manager_->get();
    for (const auto &shortcut : system_shortcuts)
    {
        result.push_back(std::make_tuple(shortcut.first, shortcut.second->kind, shortcut.second->name, shortcut.second->key_combination));
    }
    invocation.ret(result);
}

void KeybindingManager::init()
{
    this->system_shorcut_manager_->signal_shortcut_changed().connect(sigc::mem_fun(this, &KeybindingManager::system_shortcut_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 KEYBINDING_DBUS_NAME,
                                                 sigc::mem_fun(this, &KeybindingManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &KeybindingManager::on_name_acquired),
                                                 sigc::mem_fun(this, &KeybindingManager::on_name_lost));
}
void KeybindingManager::system_shortcut_added(std::shared_ptr<SystemShortCut> system_shortcut)
{
    if (system_shortcut)
    {
    }
}

void KeybindingManager::system_shortcut_deleted(std::shared_ptr<SystemShortCut> system_shortcut)
{
}

void KeybindingManager::system_shortcut_changed(std::shared_ptr<SystemShortCut> system_shortcut)
{
    if (system_shortcut)
    {
        this->Changed_signal.emit(std::make_tuple(system_shortcut->uid, system_shortcut->kind));
    }
}

void KeybindingManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, KEYBINDING_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", KEYBINDING_OBJECT_PATH, e.what().c_str());
    }
}

void KeybindingManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void KeybindingManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran