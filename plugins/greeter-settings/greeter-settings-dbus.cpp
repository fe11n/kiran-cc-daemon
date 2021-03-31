/**
 * @file          /kiran-cc-daemon/plugins/greeter-settings/greeter-settings-dbus.cpp
 * @brief         
 * @author        yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "greeter-settings-dbus.h"

#include <glib/gstdio.h>
#include <pwd.h>
#include <unistd.h>

#include <cinttypes>
#include <cstdint>

#include "greeter-settings_i.h"
#include "lib/base/base.h"
#include "lib/dbus/dbus.h"

namespace Kiran
{
#define GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(fun, callback, auth, type1)                                                             \
    void GreeterSettingsDbus::fun(type1 value,                                                                                        \
                                  MethodInvocation &invocation)                                                                       \
    {                                                                                                                                 \
        SETTINGS_PROFILE("");                                                                                                         \
                                                                                                                                      \
        AuthManager::get_instance()->start_auth_check(AUTH_SET_LOGIN_OPTION,                                                          \
                                                      TRUE,                                                                           \
                                                      invocation.getMessage(),                                                        \
                                                      std::bind(&GreeterSettingsDbus::callback, this, std::placeholders::_1, value)); \
                                                                                                                                      \
        return;                                                                                                                       \
    }

#define GREETERSETTINGSDBUS_SET_TWO_PROP_AUTH(fun, callback, auth, type1, type2)                                                               \
    void GreeterSettingsDbus::fun(type1 value1, type2 value2, MethodInvocation &invocation)                                                    \
    {                                                                                                                                          \
        SETTINGS_PROFILE("");                                                                                                                  \
                                                                                                                                               \
        AuthManager::get_instance()->start_auth_check(AUTH_SET_LOGIN_OPTION,                                                                   \
                                                      TRUE,                                                                                    \
                                                      invocation.getMessage(),                                                                 \
                                                      std::bind(&GreeterSettingsDbus::callback, this, std::placeholders::_1, value1, value2)); \
                                                                                                                                               \
        return;                                                                                                                                \
    }

GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetBackgroundFile, change_background_file_authorized_cb, AUTH_SET_LOGIN_OPTION, const Glib::ustring &)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetAutologinUser, change_auto_login_user_authorized_cb, AUTH_SET_LOGIN_OPTION, uint64_t)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetAutologinTimeout, change_auto_login_timeout_authorized_cb, AUTH_SET_LOGIN_OPTION, uint64_t)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetHideUserList, change_hide_user_list_authorized_cb, AUTH_SET_LOGIN_OPTION, bool)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetAllowManualLogin, change_allow_manual_login_authorized_cb, AUTH_SET_LOGIN_OPTION, bool)
GREETERSETTINGSDBUS_SET_TWO_PROP_AUTH(SetScaleMode, change_scale_mode_authorized_cb, AUTH_SET_LOGIN_OPTION, uint16_t, uint16_t)

#define USER_PROP_SET_HANDLER(prop, type)                                  \
    bool GreeterSettingsDbus::prop##_setHandler(type value)                \
    {                                                                      \
        SETTINGS_PROFILE("value: %s.", fmt::format("{0}", value).c_str()); \
        this->m_##prop = value;                                            \
        return true;                                                       \
    }

USER_PROP_SET_HANDLER(backgroundFile, const Glib::ustring &)
USER_PROP_SET_HANDLER(autologinUser, const Glib::ustring &)
USER_PROP_SET_HANDLER(autologinTimeout, guint64)
USER_PROP_SET_HANDLER(allowManualLogin, bool)
USER_PROP_SET_HANDLER(hideUserList, bool)
USER_PROP_SET_HANDLER(scaleMode, guint16)
USER_PROP_SET_HANDLER(scaleFactor, guint16)

GreeterSettingsDbus::GreeterSettingsDbus()
{
    m_autologinTimeout = 0;
    m_allowManualLogin = false;
    m_hideUserList = false;
    m_scaleMode = 0;
    m_scaleFactor = 0;
    m_dbusConnectId = 0;
    m_objectRegisterId = 0;

    m_prefs = GreeterSettingsManager::get_instance();
}

GreeterSettingsDbus::~GreeterSettingsDbus()
{
    if (this->m_dbusConnectId)
    {
        Gio::DBus::unown_name(this->m_dbusConnectId);
    }

    if (this->m_reloadConn)
    {
        this->m_reloadConn.disconnect();
    }
}

GreeterSettingsDbus *GreeterSettingsDbus::m_instance = nullptr;
void GreeterSettingsDbus::global_init()
{
    m_instance = new GreeterSettingsDbus();
    m_instance->init();
}

void GreeterSettingsDbus::init()
{
    SETTINGS_PROFILE("");

    /*Connect to the system bus and acquire our name*/
    this->m_dbusConnectId = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                GREETER_SETTINGS_DBUS_NAME,
                                                sigc::mem_fun(this, &GreeterSettingsDbus::on_bus_acquired),
                                                sigc::mem_fun(this, &GreeterSettingsDbus::on_name_acquired),
                                                sigc::mem_fun(this, &GreeterSettingsDbus::on_name_lost));

    m_prefs->signal_autologin_delay_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_autologin_delay_changed));
    m_prefs->signal_autologin_user_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_autologin_user_changed));
    m_prefs->signal_background_file_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_background_file_changed));
    m_prefs->signal_enable_manual_login_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_enable_manual_login_changed));
    m_prefs->signal_hide_user_list_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_hide_user_list_changed));
    m_prefs->signal_scale_factor_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_scale_factor_changed));
    m_prefs->signal_scale_mode_changed().connect(sigc::mem_fun(this, &GreeterSettingsDbus::on_scale_mode_changed));

    reload_greeter_settings();
}

void GreeterSettingsDbus::change_background_file_authorized_cb(Kiran::SystemDaemon::GreeterSettingsStub::MethodInvocation invocation, Glib::ustring file_path)
{
    SETTINGS_PROFILE("file_path: %s", file_path.c_str());

    if (backgroundFile_get() != file_path)
    {
        m_prefs->set_background_file(file_path);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_1);
        }

        backgroundFile_set(file_path);
    }
    invocation.ret();
}

void GreeterSettingsDbus::change_auto_login_user_authorized_cb(SystemDaemon::GreeterSettingsStub::MethodInvocation invocation, guint64 autologin_user)
{
    SETTINGS_PROFILE("autologin_user: %d", autologin_user);
    Glib::ustring autologin_user_name = uid_to_name(autologin_user);
    if (autologin_user_name.empty())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_NOTFOUND_USER);
    }

    if (autologinUser_get() != autologin_user_name)
    {
        m_prefs->set_autologin_user(autologin_user_name);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_2);
        }

        autologinUser_set(autologin_user_name);
    }

    invocation.ret();
}

void GreeterSettingsDbus::change_auto_login_timeout_authorized_cb(SystemDaemon::GreeterSettingsStub::MethodInvocation invocation, guint64 seconds)
{
    SETTINGS_PROFILE("seconds: %d", seconds);
    if (autologinTimeout_get() != seconds)
    {
        m_prefs->set_autologin_delay(seconds);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_3);
        }

        autologinTimeout_set(seconds);
    }

    invocation.ret();
}

void GreeterSettingsDbus::change_hide_user_list_authorized_cb(SystemDaemon::GreeterSettingsStub::MethodInvocation invocation, bool hide)
{
    SETTINGS_PROFILE("hide: %d", hide);
    if (hideUserList_get() != hide)
    {
        m_prefs->set_hide_user_list(hide);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_4);
        }

        hideUserList_set(hide);
    }

    invocation.ret();
}

void GreeterSettingsDbus::change_allow_manual_login_authorized_cb(SystemDaemon::GreeterSettingsStub::MethodInvocation invocation, bool allow)
{
    SETTINGS_PROFILE("allow: %d", allow);
    if (allowManualLogin_get() != allow)
    {
        m_prefs->set_enable_manual_login(allow);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_5);
        }

        allowManualLogin_set(allow);
    }

    invocation.ret();
}

void GreeterSettingsDbus::change_scale_mode_authorized_cb(SystemDaemon::GreeterSettingsStub::MethodInvocation invocation, guint16 mode, guint16 factor)
{
    SETTINGS_PROFILE("mode: %d factor: %d", mode, factor);

    if (mode < GREETER_SETTINGS_SCALING_MODE_AUTO || mode > GREETER_SETTINGS_SCALING_MODE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SCALE_MODE_INVALIDE);
    }
    //*
    m_prefs->set_scale_mode((GreeterSettingsScalingMode)mode);
    m_prefs->set_scale_factor(factor);
    if (!m_prefs->save())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_6);
    }

    scaleMode_set(mode);
    scaleFactor_set(factor);

    invocation.ret();
}

bool GreeterSettingsDbus::reload_greeter_settings()
{
    SETTINGS_PROFILE("");

    m_prefs->load();
    backgroundFile_set(m_prefs->get_background_file());
    autologinUser_set(m_prefs->get_autologin_user());
    autologinTimeout_set(m_prefs->get_autologin_delay());
    hideUserList_set(m_prefs->get_hide_user_list());
    allowManualLogin_set(m_prefs->get_enable_manual_login());
    scaleMode_set(m_prefs->get_scale_mode());
    scaleFactor_set(m_prefs->get_scale_factor());

    return true;
}

void GreeterSettingsDbus::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->m_objectRegisterId = this->register_object(connect, GREETER_SETTINGS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", GREETER_SETTINGS_OBJECT_PATH, e.what().c_str());
    }
}

void GreeterSettingsDbus::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void GreeterSettingsDbus::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}

void GreeterSettingsDbus::on_autologin_delay_changed()
{
    autologinTimeout_set(m_prefs->get_autologin_delay());
}

void GreeterSettingsDbus::on_autologin_user_changed()
{
    autologinUser_set(m_prefs->get_autologin_user());
}

void GreeterSettingsDbus::on_background_file_changed()
{
    backgroundFile_set(m_prefs->get_background_file());
}

void GreeterSettingsDbus::on_enable_manual_login_changed()
{
    allowManualLogin_set(m_prefs->get_enable_manual_login());
}

void GreeterSettingsDbus::on_hide_user_list_changed()
{
    hideUserList_set(m_prefs->get_hide_user_list());
}

void GreeterSettingsDbus::on_scale_factor_changed()
{
    scaleFactor_set(m_prefs->get_scale_factor());
}

void GreeterSettingsDbus::on_scale_mode_changed()
{
    scaleMode_set(m_prefs->get_scale_mode());
}

Glib::ustring GreeterSettingsDbus::uid_to_name(uid_t uid)
{
    struct passwd *pw_ptr;
    if ((pw_ptr = getpwuid(uid)) == NULL)
    {
        LOG_WARNING("failed to find user name by uid: %d", uid);
        return Glib::ustring();
    }

    return Glib::ustring(pw_ptr->pw_name);
}

}  // namespace Kiran
