/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:38:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-05 10:51:22
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/home/tangjie02/git/kiran-system-daemon/src/main.cpp
 */

#include <gio/gio.h>
#include <glib-unix.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "lib/auth-manager.h"
#include "lib/log.h"
#include "src/settings-manager.h"

class ScreenLogger : public Kiran::ILogger
{
public:
    void write_log(const char* buff, uint32_t len)
    {
        g_print(buff);
    }
};

int main(int argc, char* argv[])
{
    Gio::init();
    Kiran::Log::global_init();

    Glib::OptionContext context;
    Glib::OptionGroup group("kiran-system-daemon", "kiran-system-daemon option group");

    // version
    Glib::OptionEntry version_entry;
    version_entry.set_long_name("version");
    version_entry.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    version_entry.set_description(N_("Output version infomation and exit"));

    // show-log
    Glib::OptionEntry show_log_entry;
    show_log_entry.set_long_name("show-log");
    show_log_entry.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    show_log_entry.set_description(N_("Print log to screen."));

    group.add_entry(version_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        g_print("kiran-system-daemon: 2.0\n");
        return false;
    });

    group.add_entry(show_log_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        Kiran::Log::get_instance()->set_logger(new ScreenLogger());
        return true;
    });

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, KIRAN_SYSTEM_DAEMON_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    // context.set_translate_func([](const Glib::ustring& str) -> Glib::ustring { g_print("%s\n", str.c_str()); return _(str.c_str()); });
    context.set_translation_domain(GETTEXT_PACKAGE);
    context.set_main_group(group);

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

    auto loop = Glib::MainLoop::create();

    Kiran::AuthManager::global_init();
    Kiran::SettingsManager::global_init();

    loop->run();
    return 0;
}