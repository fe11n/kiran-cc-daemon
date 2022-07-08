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

#include "plugins/display/display-util.h"

namespace Kiran
{
std::string DisplayUtil::rotation_to_str(DisplayRotationType rotation)
{
    switch (rotation)
    {
    case DisplayRotationType::DISPLAY_ROTATION_90:
        return "left";
    case DisplayRotationType::DISPLAY_ROTATION_180:
        return "inverted";
    case DisplayRotationType::DISPLAY_ROTATION_270:
        return "right";
    default:
        return "normal";
    }
}

std::string DisplayUtil::reflect_to_str(DisplayReflectType reflect)
{
    switch (reflect)
    {
    case DisplayReflectType::DISPLAY_REFLECT_X:
        return "x";
    case DisplayReflectType::DISPLAY_REFLECT_Y:
        return "y";
    case DisplayReflectType::DISPLAY_REFLECT_XY:
        return "xy";
    default:
        return "normal";
        break;
    }
}

DisplayRotationType DisplayUtil::str_to_rotation(const std::string &str)
{
    switch (shash(str.c_str()))
    {
    case "left"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_90;
    case "inverted"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_180;
    case "right"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_270;
    default:
        return DisplayRotationType::DISPLAY_ROTATION_0;
    }
    return DisplayRotationType::DISPLAY_ROTATION_0;
}

DisplayReflectType DisplayUtil::str_to_reflect(const std::string &str)
{
    switch (shash(str.c_str()))
    {
    case "x"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_X;
    case "y"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_Y;
    case "xy"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_XY;
    default:
        return DisplayReflectType::DISPLAY_REFLECT_NORMAL;
    }
    return DisplayReflectType::DISPLAY_REFLECT_NORMAL;
}
}  // namespace Kiran
