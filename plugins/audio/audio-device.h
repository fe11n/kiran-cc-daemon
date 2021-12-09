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

#pragma once

#include "lib/base/base.h"

#include <audio_device_dbus_stub.h>
#include "plugins/audio/pulse/pulse-device.h"

namespace Kiran
{
class AudioDevice : public SessionDaemon::Audio::DeviceStub
{
public:
    AudioDevice(std::shared_ptr<PulseDevice> device);
    virtual ~AudioDevice();

    bool init(const std::string &object_path_prefix);

    std::string get_object_path() { return this->object_path_; };

    virtual guint32 index_get() { return this->device_->get_index(); };
    virtual Glib::ustring name_get() { return this->device_->get_name(); };
    virtual bool mute_get() { return this->mute_; };
    virtual double volume_get() { return this->volume_; }
    virtual double balance_get() { return this->balance_; }
    virtual double fade_get() { return this->fade_; }
    virtual double base_volume_get();
    virtual guint32 card_index_get() { return this->device_->get_card_index(); };
    virtual Glib::ustring active_port_get() { return this->active_port_; };
    virtual guint32 state_get() { return this->device_->get_flags(); };

protected:
    // 设置正在使用的端口
    virtual void SetActivePort(const Glib::ustring &name, MethodInvocation &invocation);
    // 获取所有绑定端口
    virtual void GetPorts(MethodInvocation &invocation);
    // 设置音量
    virtual void SetVolume(double volume, MethodInvocation &invocation);
    // 设置左声道和右声道的平衡
    virtual void SetBalance(double balance, MethodInvocation &invocation);
    // 设置远声道和近声道的平衡
    virtual void SetFade(double fade, MethodInvocation &invocation);
    // 设置静音
    virtual void SetMute(bool mute, MethodInvocation &invocation);
    // 获取属性
    virtual void GetProperty(const Glib::ustring &key, MethodInvocation &invocation);

    virtual bool index_setHandler(guint32 value) { return true; };
    virtual bool name_setHandler(const Glib::ustring &value) { return true; };
    virtual bool mute_setHandler(bool value);
    virtual bool volume_setHandler(double value);
    virtual bool balance_setHandler(double value);
    virtual bool fade_setHandler(double value);
    virtual bool base_volume_setHandler(double value) { return true; };
    virtual bool card_index_setHandler(guint32 value) { return true; };
    virtual bool active_port_setHandler(const Glib::ustring &value);
    virtual bool state_setHandler(guint32 value) { return true; }

private:
    bool dbus_register();
    void dbus_unregister();

    void on_node_info_changed_cb(PulseNodeField field);
    void on_active_port_changed_cb(const std::string &active_port_name);

private:
    std::shared_ptr<PulseDevice> device_;

    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;

    bool mute_;
    double volume_;
    double balance_;
    double fade_;
    std::string active_port_;
};

}  // namespace Kiran
