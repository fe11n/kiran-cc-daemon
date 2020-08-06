/*
 * @Author       : tangjie02
 * @Date         : 2020-08-06 10:37:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 15:04:07
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/mouse/device-helper.h
 */
#pragma once

#include <giomm.h>

// xlib.h must be defined after gdkmm giomm header file.
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <string>

namespace Kiran
{
class DeviceHelper
{
public:
    DeviceHelper() = delete;
    DeviceHelper(XDeviceInfo *device_info);
    virtual ~DeviceHelper();

    // 获取属性名对应的Atom，如果不存在不会新建且返回None
    Atom get_atom(const std::string &property_name);
    // 判断设备是否存在指定的属性名
    bool has_property(const std::string &property_name);
    // 判断设备是否为触摸板
    bool is_touchpad();
    // 设置属性值
    void set_property(const std::string &property_name, bool property_value);
    void set_property(const std::string &property_name, float property_value);

    operator bool() const
    {
        return (this->device_info_ != NULL && this->device_ != NULL);
    }

private:
    XDeviceInfo *device_info_;
    XDevice *device_;
};
}  // namespace Kiran