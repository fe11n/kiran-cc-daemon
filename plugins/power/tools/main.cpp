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

#include <glib/gi18n.h>
#include <gtk3-log-i.h>

#include "config.h"
#include "plugins/power/tools/power-backlight-helper.h"

int main(int argc, char* argv[])
{
    Kiran::PowerBacklightHelper backlight_helper;

    Gio::init();
    backlight_helper.init();

    klog_gtk3_init(std::string(), "kylinsec-system", "kiran-cc-daemon", "kiran-power-backlight-helper");

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, KCC_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    Glib::OptionContext context;
    Glib::OptionGroup group("backlight-helper", _("power backlight helper"));

    Glib::OptionEntry entry1;
    entry1.set_long_name("get-brightness-value");
    entry1.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    entry1.set_description(N_("Get the current brightness value"));

    Glib::OptionEntry entry2;
    entry2.set_long_name("get-max-brightness-value");
    entry2.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    entry2.set_description(N_("Get the max brightness value"));

    Glib::OptionEntry entry3;
    entry3.set_long_name("set-brightness-value");
    entry3.set_description(N_("Set the brightness value"));

    group.add_entry(entry1, [&backlight_helper](const Glib::ustring& option_name, const Glib::ustring&, bool) -> bool {
        auto brightness_value = backlight_helper.get_brightness_value();
        if (brightness_value >= 0)
        {
            fmt::print("{0}", brightness_value);
        }
        else
        {
            fmt::print("{0}", _("Could not get the value of the backlight"));
            return false;
        }
        return true;
    });

    group.add_entry(entry2, [&backlight_helper](const Glib::ustring& option_name, const Glib::ustring&, bool) -> bool {
        auto brightness_value = backlight_helper.get_brightness_max_value();
        if (brightness_value >= 0)
        {
            fmt::print("{0}", brightness_value);
        }
        else
        {
            fmt::print("{0}", _("Could not get the maximum value of the backlight"));
            return false;
        }
        return true;
    });

    group.add_entry(entry3, [&backlight_helper](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        std::string error;
        auto brightness_value = std::strtol(value.c_str(), nullptr, 0);
        if (!backlight_helper.set_brightness_value(brightness_value, error))
        {
            fmt::print("{0}", error);
        }
        return true;
    });

    group.set_translation_domain(GETTEXT_PACKAGE);
    context.set_main_group(group);

    // 不支持获取和设置则直接返回
    if (!backlight_helper.support_backlight())
    {
        fmt::print("{0}", _("No backlights were found on your system"));
        return EXIT_FAILURE;
    }

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}