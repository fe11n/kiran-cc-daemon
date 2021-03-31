/**
 * @file          /kiran-cc-daemon/plugins/power/power-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/power-manager.h"

#include "plugins/power/backlight/power-backlight.h"
#include "power_i.h"

namespace Kiran
{
PowerManager::PowerManager(PowerWrapperManager* wrapper_manager) : wrapper_manager_(wrapper_manager),
                                                                   dbus_connect_id_(),
                                                                   object_register_id_(0)
{
    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
    this->upower_client_ = this->wrapper_manager_->get_default_upower();
}

PowerManager::~PowerManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

PowerManager* PowerManager::instance_ = nullptr;
void PowerManager::global_init(PowerWrapperManager* wrapper_manager)
{
    instance_ = new PowerManager(wrapper_manager);
    instance_->init();
}

void PowerManager::init()
{
    this->upower_client_->signal_on_battery_changed().connect(sigc::mem_fun(this, &PowerManager::on_battery_changed));
    this->upower_client_->signal_lid_is_closed_changed().connect(sigc::mem_fun(this, &PowerManager::on_lid_is_closed_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 POWER_DBUS_NAME,
                                                 sigc::mem_fun(this, &PowerManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &PowerManager::on_name_acquired),
                                                 sigc::mem_fun(this, &PowerManager::on_name_lost));
}

void PowerManager::SetIdleAction(gint32 device,
                                 gint32 supply,
                                 gint32 idle_timeout,
                                 gint32 action,
                                 MethodInvocation& invocation)
{
    SETTINGS_PROFILE("device: %d, supply: %d, idle timeout: %d, action: %d.", device, supply, idle_timeout, action);

    if (action < 0 || action >= PowerAction::POWER_ACTION_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Unknown power action"));
    }

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            this->power_settings_->set_int(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, action);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            this->power_settings_->set_int(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, action);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_1);
        }
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            this->power_settings_->set_int(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, action);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            this->power_settings_->set_int(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, action);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_2);
        }
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_1);
        break;
    }
    invocation.ret();
}

void PowerManager::GetIdleAction(gint32 device,
                                 gint32 supply,
                                 MethodInvocation& invocation)
{
    SETTINGS_PROFILE("device: %d, supply: %d.", device, supply);

    int32_t idle_timeout = 0;
    int32_t action = PowerAction::POWER_ACTION_NOTHING;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_3);
        }
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_4);
        }
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_2);
        break;
    }

    invocation.ret(std::make_tuple(idle_timeout, action));
}

void PowerManager::SetEventAction(gint32 event,
                                  gint32 action,
                                  MethodInvocation& invocation)
{
    SETTINGS_PROFILE("event: %d, action: %d.", event, action);

    if (action < 0 || action >= PowerAction::POWER_ACTION_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_1);
    }
    bool result = false;

    switch (event)
    {
    case PowerEvent::POWER_EVENT_PRESSED_POWEROFF:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BUTTON_POWER_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BUTTON_SUSPEND_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        result = this->power_settings_->set_enum(POWER_SCHEMA_LID_CLOSED_ACTION, action);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_1);
    }

    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SET_ACTION_FAILED);
    }
    else
    {
        invocation.ret();
    }
}

void PowerManager::GetEventAction(gint32 event,
                                  MethodInvocation& invocation)
{
    SETTINGS_PROFILE("event: %d.", event);

    int32_t action = PowerAction::POWER_ACTION_NOTHING;

    switch (event)
    {
    case PowerEvent::POWER_EVENT_PRESSED_POWEROFF:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_POWER_ACTION);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_SUSPEND_ACTION);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION);
        break;
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        action = this->power_settings_->get_enum(POWER_SCHEMA_LID_CLOSED_ACTION);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_2);
    }

    invocation.ret(action);
}

void PowerManager::SetBrightness(gint32 device,
                                 gint32 brightness_percentage,
                                 MethodInvocation& invocation)
{
    SETTINGS_PROFILE("device: %d.", device);

    bool result = false;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
    {
        auto monitor = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
        result = monitor->set_brightness(brightness_percentage);
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
    {
        auto kbd = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
        result = kbd->set_brightness(brightness_percentage);
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_3);
    }

    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SET_BRIGHTNESS_FAILED);
    }
    else
    {
        invocation.ret();
    }
}

void PowerManager::GetBrightness(gint32 device,
                                 MethodInvocation& invocation)
{
    SETTINGS_PROFILE("device: %d.", device);

    int32_t brightness_percentage = -1;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
    {
        auto monitor = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
        brightness_percentage = monitor->get_brightness();
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
    {
        auto kbd = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
        brightness_percentage = kbd->get_brightness();
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_4);
    }

    invocation.ret(brightness_percentage);
}

bool PowerManager::on_battery_get()
{
    return this->upower_client_->get_on_battery();
}

bool PowerManager::lid_is_present_get()
{
    return this->upower_client_->get_lid_is_present();
}

void PowerManager::on_battery_changed(bool on_battery)
{
    this->on_battery_set(on_battery);
}

void PowerManager::on_lid_is_closed_changed(bool lid_is_closed)
{
    this->lid_is_present_set(lid_is_closed);
}

void PowerManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("Failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, POWER_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("Register object_path %s fail: %s.", POWER_OBJECT_PATH, e.what().c_str());
    }
}

void PowerManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    LOG_DEBUG("Success to register dbus name: %s", name.c_str());
}

void PowerManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    LOG_WARNING("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran