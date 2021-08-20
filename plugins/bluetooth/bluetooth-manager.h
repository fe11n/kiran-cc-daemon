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

#include <bluetooth_dbus_stub.h>
#include <object_manager_dbus_proxy.h>

#include "plugins/bluetooth/bluetooth-adapter.h"
#include "plugins/bluetooth/bluetooth-agent.h"

namespace Kiran
{
class BluetoothManager : public SessionDaemon::BluetoothStub
{
public:
    BluetoothManager();
    virtual ~BluetoothManager();

    static BluetoothManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<BluetoothAdapter> get_adapter(const std::string &object_path);
    std::shared_ptr<BluetoothAdapter> get_adapter_by_device(const std::string &object_path);

    sigc::signal<void, bool, const std::string &> &signal_agent_feeded() { return this->agent_feeded_; };

protected:
    virtual void GetAdapters(MethodInvocation &invocation);
    virtual void GetDevices(const Glib::DBusObjectPathString &adapter_object_path, MethodInvocation &invocation);
    virtual void FeedPinCode(const Glib::DBusObjectPathString &device,
                             bool accept,
                             const Glib::ustring &pincode,
                             MethodInvocation &invocation);
    virtual void FeedPasskey(const Glib::DBusObjectPathString &device,
                             bool accept,
                             guint32 passkey,
                             MethodInvocation &invocation);
    virtual void Confirm(const Glib::DBusObjectPathString &device,
                         bool accept,
                         MethodInvocation &invocation);

private:
    void init();

    void on_bluez_ready(Glib::RefPtr<Gio::AsyncResult> &result);

    void on_interface_added(Glib::DBusObjectPathString object_path, std::map<Glib::ustring, std::map<Glib::ustring, Glib::VariantBase>> interfaces);
    void on_interface_removed(Glib::DBusObjectPathString object_path, std::vector<Glib::ustring> interfaces);

    void load_objects();

    void add_adapter(const std::string &object_path);
    void remove_adapter(const std::string &object_path);
    void add_device(const std::string &object_path);
    void remove_device(const std::string &object_path);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

protected:
    sigc::signal<void, bool, const std::string &> agent_feeded_;

private:
    static BluetoothManager *instance_;

    Glib::RefPtr<DBus::ObjectManagerProxy> objects_proxy_;

    std::map<std::string, std::shared_ptr<BluetoothAdapter>> adapters_;

    std::shared_ptr<BluetoothAgent> agent_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    friend class BluetoothAgent;
};
}  // namespace Kiran