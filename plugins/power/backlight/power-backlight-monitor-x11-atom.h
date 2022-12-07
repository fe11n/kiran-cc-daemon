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

#include <gdkmm.h>
//
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>
#include "plugins/power/backlight/power-backlight-interface.h"

typedef struct _XDisplay Display;

namespace Kiran
{
// 通过Xrandr扩展调节单个显示设备亮度值
class PowerBacklightMonitorX11Atom : public PowerBacklightAbsolute
{
public:
    PowerBacklightMonitorX11Atom(Atom backlight_atom, RROutput output);
    virtual ~PowerBacklightMonitorX11Atom(){};

    // 设置亮度值
    virtual bool set_brightness_value(int32_t brightness_value) override;
    // 获取亮度值
    virtual int32_t get_brightness_value() override;
    // 获取亮度最大最小值
    virtual bool get_brightness_range(int32_t &min, int32_t &max) override;

private:
    GdkDisplay *display_;
    Display *xdisplay_;
    Atom backlight_atom_;
    RROutput output_;
};

}  // namespace Kiran
