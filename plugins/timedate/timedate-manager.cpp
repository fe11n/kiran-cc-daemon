/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 10:02:03
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:28:26
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/timedate/timedate-manager.cpp
 */

#include "plugins/timedate/timedate-manager.h"

#include <fcntl.h>
#include <fmt/format.h>
#include <glib/gi18n.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cinttypes>

#include "lib/base/base.h"
#include "plugins/timedate/timedate-util.h"

#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#endif

/* This may be missing in libc headers */
#ifndef ADJ_SETOFFSET
#define ADJ_SETOFFSET 0x0100
#endif

namespace Kiran
{
#define ADJTIME_PATH "/etc/adjtime"
#define HWCLOCK_PATH "/sbin/hwclock"
#define RTC_DEVICE "/dev/rtc"

#define LOCALTIME_PATH "/etc/localtime"
#define ZONEINFO_PATH "/usr/share/zoneinfo/"
#define ZONE_TABLE "zone.tab"
#define ZONE_TABLE_MIN_COLUMN 3
#define LOCALTIME_TO_ZONEINFO_PATH ".."
#define MAX_TIMEZONE_LENGTH 256

#define TIMEZONE_DOMAIN "kiran-cc-daemon-timezones"

#define TIMEDATE_DBUS_NAME "com.unikylin.Kiran.SystemDaemon.TimeDate"
#define TIMEDATE_OBJECT_PATH "/com/unikylin/Kiran/SystemDaemon/TimeDate"

#define POLKIT_ACTION_SET_TIME "com.unikylin.kiran.system-daemon.timedate.set-time"
#define POLKIT_ACTION_SET_NTP_ACTIVE "com.unikylin.kiran.system-daemon.timedate.set-ntp"
#define POLKIT_ACTION_SET_RTC_LOCAL "com.unikylin.kiran.system-daemon.timedate.set-local-rtc"
#define POLKIT_ACTION_SET_TIMEZONE "com.unikylin.kiran.system-daemon.timedate.set-timezone"
#define POLKIT_AUTH_CHECK_TIMEOUT 20

TimedateManager *TimedateManager::instance_ = nullptr;
const std::vector<std::string> TimedateManager::ntp_units_paths_ = {"/etc/systemd/ntp-units.d", "/usr/lib/systemd/ntp-units.d"};

TimedateManager::TimedateManager() : dbus_connect_id_(0),
                                     object_register_id_(0)
{
}

TimedateManager::~TimedateManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

void TimedateManager::global_init()
{
    instance_ = new TimedateManager();
    instance_->init();
}

void TimedateManager::SetTime(gint64 requested_time,
                              bool relative,
                              MethodInvocation &invocation)
{
    SETTINGS_PROFILE("RequestedTime: %" PRId64 " Relative: %d", requested_time, relative);
    if (this->ntp_get())
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "NTP unit is active"));
        return;
    }

    int64_t request_time = g_get_monotonic_time();

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_TIME,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::funish_set_time, this, std::placeholders::_1, request_time, requested_time, relative));
}

void TimedateManager::SetTimezone(const Glib::ustring &time_zone,
                                  MethodInvocation &invocation)
{
    SETTINGS_PROFILE("TimeZone: %s.", time_zone.c_str());
    if (!check_timezone_name(time_zone))
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Invalid timezone"));
        return;
    }

    auto current_timezone = this->time_zone_get();
    if (current_timezone == time_zone)
    {
        invocation.ret();
        return;
    }

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_TIMEZONE,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::finish_set_timezone, this, std::placeholders::_1, time_zone));
}

void TimedateManager::GetZoneList(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    std::vector<std::tuple<Glib::ustring, Glib::ustring, int64_t>> result;

    auto zone_infos = this->get_zone_infos();
    for (auto &zone_info : zone_infos)
    {
        auto local_zone_name = dgettext(TIMEZONE_DOMAIN, zone_info.tz.c_str());
        auto gmt = TimedateUtil::get_gmt_offset(zone_info.tz);
        result.push_back(std::make_tuple(zone_info.tz, std::string(local_zone_name), gmt));
    }
    invocation.ret(result);
}

void TimedateManager::SetLocalRTC(bool local,
                                  bool adjust_system,
                                  MethodInvocation &invocation)
{
    SETTINGS_PROFILE("local: %d adjust_system: %d.", local, adjust_system);

    if (local == this->local_rtc_get())
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "the value of the local have not been changed."));
        return;
    }

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_RTC_LOCAL,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::finish_set_rtc_local, this, std::placeholders::_1, local, adjust_system));
}

void TimedateManager::SetNTP(bool active,
                             MethodInvocation &invocation)
{
    SETTINGS_PROFILE("active: %d.", active);

    if (this->ntp_units_.size() == 0)
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "No NTP unit available"));
        return;
    }

    if (active == this->ntp_get())
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "the value of the active have not been changed."));
        return;
    }

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_NTP_ACTIVE,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::finish_set_ntp_active, this, std::placeholders::_1, active));
}

Glib::ustring TimedateManager::time_zone_get()
{
    SETTINGS_PROFILE("");
    g_autofree gchar *link = NULL;

    link = g_file_read_link(LOCALTIME_PATH, NULL);
    if (!link)
    {
        return std::string();
    }

    auto zone = g_strrstr(link, ZONEINFO_PATH);
    if (!zone)
    {
        return std::string();
    }

    zone += strlen(ZONEINFO_PATH);

    return Glib::ustring(zone);
}

bool TimedateManager::local_rtc_get()
{
    SETTINGS_PROFILE("");
    std::string contents;
    try
    {
        contents = Glib::file_get_contents(ADJTIME_PATH);
    }
    catch (const Glib::FileError &e)
    {
        return false;
    }

    if (contents.find("LOCAL") != std::string::npos)
    {
        return true;
    }
    return false;
}

bool TimedateManager::can_ntp_get()
{
    SETTINGS_PROFILE("");
    return (this->ntp_units_.size() > 0);
}

bool TimedateManager::ntp_get()
{
    SETTINGS_PROFILE("");
    if (this->ntp_units_.size() <= 0)
    {
        return false;
    }

    auto result = call_systemd("LoadUnit", Glib::VariantContainerBase(g_variant_new("(s)", this->ntp_units_.front().name.c_str())));

    if (!result.gobj())
    {
        return false;
    }

    g_return_val_if_fail(result.get_n_children() > 0, false);
    auto path_base = result.get_child();
    auto unit_path = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(path_base).get();

    auto proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, SYSTEMD_NAME, unit_path, SYSTEMD_UNIT_INTERFACE);
    g_return_val_if_fail(proxy, false);

    Glib::VariantBase state;

    proxy->get_cached_property(state, "ActiveState");
    g_return_val_if_fail(state.gobj() != NULL, false);

    auto state_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(state).get();

    if (state_str == "active" || state_str == "activating")
    {
        return true;
    }
    else
    {
        return false;
    }
}

guint64 TimedateManager::system_time_get()
{
    SETTINGS_PROFILE("");
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

guint64 TimedateManager::rtc_time_get()
{
    SETTINGS_PROFILE("");
    struct rtc_time rtc;
    struct tm tm;
    time_t rtc_time = 0;
    int fd, r;

    fd = open(RTC_DEVICE, O_RDONLY);
    if (fd < 0)
    {
        return 0;
    }

    r = ioctl(fd, RTC_RD_TIME, &rtc);
    close(fd);
    if (r)
    {
        return 0;
    }

    tm.tm_sec = rtc.tm_sec;
    tm.tm_min = rtc.tm_min;
    tm.tm_hour = rtc.tm_hour;
    tm.tm_mday = rtc.tm_mday;
    tm.tm_mon = rtc.tm_mon;
    tm.tm_year = rtc.tm_year;
    tm.tm_isdst = 0;

    /* This is the raw time as if the RTC was in UTC */
    rtc_time = timegm(&tm);

    return (uint64_t)rtc_time * 1000000;
}

void TimedateManager::init()
{
    SETTINGS_PROFILE("");

    bindtextdomain(TIMEZONE_DOMAIN, KCC_LOCALEDIR);
    bind_textdomain_codeset(TIMEZONE_DOMAIN, "UTF-8");

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 TIMEDATE_DBUS_NAME,
                                                 sigc::mem_fun(this, &TimedateManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &TimedateManager::on_name_acquired),
                                                 sigc::mem_fun(this, &TimedateManager::on_name_lost));

    this->systemd_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_MANAGER_INTERFACE);
    this->polkit_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, POLKIT_NAME, POLKIT_PATH, POLKIT_INTERFACE);

    this->read_ntp_units();
}

std::vector<ZoneInfo> TimedateManager::get_zone_infos()
{
    std::vector<ZoneInfo> zone_infos;

    auto file_path = Glib::build_filename(ZONEINFO_PATH, ZONE_TABLE);

    std::string contents;
    try
    {
        contents = Glib::file_get_contents(file_path);
    }
    catch (const Glib::FileError &e)
    {
        LOG_WARNING("failed to get file contents: %s.", file_path.c_str());
        return zone_infos;
    }

    auto lines = StrUtil::split_lines(contents);

    for (auto &line : lines)
    {
        if (line[0] == '#' || line.length() == 0)
        {
            continue;
        }
        auto regex = Glib::Regex::create("\\s+");
        std::vector<std::string> tokens = regex->split(line);
        if (tokens.size() >= ZONE_TABLE_MIN_COLUMN)
        {
            ZoneInfo zone_info;
            zone_info.country_code = tokens[0];
            zone_info.coordinates = tokens[1];
            zone_info.tz = tokens[2];
            zone_infos.push_back(std::move(zone_info));
        }
        else
        {
            LOG_WARNING("ignore line: %s, the line is less than %d columns.", line.c_str(), ZONE_TABLE_MIN_COLUMN);
        }
    }
    return zone_infos;
}

Glib::VariantContainerBase TimedateManager::call_systemd(const std::string &method_name, const Glib::VariantContainerBase &parameters)
{
    SETTINGS_PROFILE("method_name: %s.", method_name.c_str());
    Glib::VariantContainerBase retval;
    try
    {
        retval = this->systemd_proxy_->call_sync(method_name, parameters);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("failed to call systemd method: %s", e.what().c_str());
    }
    return retval;
}

bool TimedateManager::call_systemd_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters)
{
    SETTINGS_PROFILE("method_name: %s.", method_name.c_str());
    auto retval = call_systemd(method_name, parameters);
    if (retval.gobj())
    {
        return true;
    }
    return false;
}

void TimedateManager::finish_hwclock_call(GPid pid, gint status, gpointer user_data)
{
    auto hwclock_call = (HWClockCall *)user_data;
    GError *error = NULL;

    Glib::spawn_close_pid(pid);

    if (g_spawn_check_exit_status(status, &error))
    {
        if (hwclock_call->handler && hwclock_call->invocation)
            (hwclock_call->handler)(hwclock_call->invocation);
    }
    else
    {
        LOG_WARNING("hwclock failed: %s\n", error->message);
        if (hwclock_call->invocation)
        {
            auto err_message = fmt::format("hwclock failed: %s", error->message);
            hwclock_call->invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
        }

        g_error_free(error);
    }

    delete hwclock_call;
}

void TimedateManager::start_hwclock_call(bool hctosys,
                                         bool local,
                                         bool utc,
                                         Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                         AuthManager::AuthCheckHandler handler)
{
    std::vector<std::string> argv;
    std::vector<std::string> envp;
    Glib::Pid pid;
    struct stat st;

    if (stat(RTC_DEVICE, &st) || !(st.st_mode & S_IFCHR))
    {
        if (invocation)
        {
            invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "No RTC device"));
        }
        return;
    }

    argv.push_back(HWCLOCK_PATH);
    argv.push_back("-f");
    argv.push_back(RTC_DEVICE);
    if (hctosys)
    {
        argv.push_back("--hctosys");
    }
    else
    {
        argv.push_back("--systohc");
    }

    if (local)
    {
        argv.push_back("--local");
    }

    if (utc)
    {
        argv.push_back("--utc");
    }

    try
    {
        Glib::spawn_async(std::string(),
                          argv,
                          envp,
                          Glib::SPAWN_DO_NOT_REAP_CHILD | Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL,
                          Glib::SlotSpawnChildSetup(),
                          &pid);
    }
    catch (const Glib::SpawnError &e)
    {
        LOG_WARNING("%s\n", e.what().c_str());
        if (invocation)
        {
            invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, e.what().c_str()));
        }
        return;
    }

    auto hwclock_call = new HWClockCall();
    hwclock_call->invocation = invocation;
    hwclock_call->handler = handler;

    g_child_watch_add(pid, (GChildWatchFunc)(TimedateManager::finish_hwclock_call), hwclock_call);
}

void TimedateManager::read_ntp_units()
{
    SETTINGS_PROFILE("");
    this->ntp_units_.clear();

    for (auto iter = ntp_units_paths_.begin(); iter != ntp_units_paths_.end(); ++iter)
    {
        auto &unit_dir = *iter;
        try
        {
            Glib::Dir dir(unit_dir);

            for (auto dir_iter = dir.begin(); dir_iter != dir.end(); ++dir_iter)
            {
                auto entry = *dir_iter;
                auto path = fmt::format("{0}/{1}", unit_dir, entry);
                std::string contents;

                try
                {
                    contents = Glib::file_get_contents(path);
                }
                catch (const Glib::FileError &e)
                {
                    LOG_WARNING("failed to get contents of the file %s: %s", path.c_str(), e.what().c_str());
                    continue;
                }

                auto lines = StrUtil::split_lines(contents);

                for (auto line_iter = lines.begin(); line_iter != lines.end(); ++line_iter)
                {
                    auto line = *line_iter;
                    if (line.length() == 0 || line.at(0) == '#')
                    {
                        LOG_DEBUG("the line %s is ingored. length: %d", line.c_str(), line.length());
                        continue;
                    }

                    if (!call_systemd_noresult("LoadUnit", Glib::VariantContainerBase(g_variant_new("(s)", line.c_str()), false)))
                    {
                        LOG_DEBUG("failed to LoadUnit: %s.", line.c_str());
                        continue;
                    }

                    LOG_DEBUG("insert ntp unit: %s %s.", line.c_str(), entry.c_str());
                    this->ntp_units_.emplace_back(NtpUnit{line, entry});
                }
            }
        }
        catch (const Glib::FileError &e)
        {
            LOG_DEBUG("%s", e.what().c_str());
        }
    }

    // Sort the units by name
    std::stable_sort(this->ntp_units_.begin(), this->ntp_units_.end(), [](const NtpUnit &a, const NtpUnit &b) -> bool {
        return a.name < b.name;
    });

    // Remove duplicates.
    auto iter = std::unique(this->ntp_units_.begin(), this->ntp_units_.end(), [](NtpUnit &a, NtpUnit &b) -> bool {
        return a.name == b.name;
    });
    this->ntp_units_.erase(iter, this->ntp_units_.end());
}

void TimedateManager::funish_set_time(MethodInvocation invocation, int64_t request_time, int64_t requested_time, bool relative)
{
    struct timeval tv;
    struct timex tx;
    std::string err_message;

    if (relative)
    {
        tx.modes = ADJ_SETOFFSET | ADJ_NANO;

        tx.time.tv_sec = requested_time / 1000000;
        tx.time.tv_usec = requested_time - tx.time.tv_sec * 1000000;
        if (tx.time.tv_usec < 0)
        {
            tx.time.tv_sec--;
            tx.time.tv_usec += 1000000;
        }

        /* Convert to nanoseconds */
        tx.time.tv_usec *= 1000;

        if (adjtimex(&tx) < 0)
        {
            err_message = fmt::format("Failed to set system clock: {0}", strerror(errno));
        }
    }
    else
    {
        /* Compensate for the time taken by the authorization check */
        requested_time += g_get_monotonic_time() - request_time;

        tv.tv_sec = requested_time / 1000000;
        tv.tv_usec = requested_time - tv.tv_sec * 1000000;
        if (settimeofday(&tv, NULL))
        {
            err_message = fmt::format("Failed to set system clock: {0}", strerror(errno));
        }
    }

    if (err_message.length() > 0)
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
    }
    else
    {
        invocation.ret();
        /* Set the RTC to the new system time */
        start_hwclock_call(false, false, false, Glib::RefPtr<Gio::DBus::MethodInvocation>(), nullptr);
    }
}

void TimedateManager::set_localtime_file_context(const std::string &path)
{
#ifdef HAVE_SELINUX
    security_context_t con;

    if (!is_selinux_enabled())
        return;

    if (matchpathcon_init_prefix(NULL, LOCALTIME_PATH))
        return;

    if (!matchpathcon(LOCALTIME_PATH, S_IFLNK, &con))
    {
        lsetfilecon(path.c_str(), con);
        freecon(con);
    }

    matchpathcon_fini();
#endif
}

void TimedateManager::update_kernel_utc_offset(void)
{
    struct timezone tz;
    struct timeval tv;
    struct tm *tm;

    bool updated = false;

    do
    {
        if (gettimeofday(&tv, &tz))
        {
            break;
        }

        tm = localtime(&tv.tv_sec);
        if (!tm)
        {
            break;
        }

        /* tm_gmtoff is in seconds east of UTC */
        tz.tz_minuteswest = -tm->tm_gmtoff / 60;
        if (settimeofday(NULL, &tz))
        {
            break;
        }

        updated = true;
    } while (0);

    if (!updated)
    {
        LOG_WARNING("Failed to update kernel UTC offset");
    }
}

bool TimedateManager::check_timezone_name(const std::string &name)
{
    /* Check if the name is sane */
    if (name.length() == 0 ||
        name.front() == '/' ||
        strstr(name.c_str(), "//") ||
        strstr(name.c_str(), "..") ||
        name.length() > MAX_TIMEZONE_LENGTH)
        return false;

    auto iter = std::find_if(name.begin(), name.end(), [](char c) -> bool {
        return !g_ascii_isalnum(c) && !strchr("+-_/", c);
    });

    if (iter != name.end())
    {
        return false;
    }

    /* Check if the correspoding file exists in the zoneinfo directory, it
	   doesn't have to be listed in zone.tab */
    auto link = fmt::format("{0}{1}", ZONEINFO_PATH, name);
    struct stat st;
    if (stat(link.c_str(), &st) || !(st.st_mode & S_IFREG))
        return false;

    return true;
}

void TimedateManager::finish_set_timezone(MethodInvocation invocation, std::string time_zone)
{
    auto link = fmt::format("{0}{1}{2}", LOCALTIME_TO_ZONEINFO_PATH, ZONEINFO_PATH, time_zone);
    auto tmp = fmt::format("%s.%06u", LOCALTIME_PATH, g_random_int());
    bool successed = false;
    do
    {
        if (symlink(link.c_str(), tmp.c_str()))
            break;

        set_localtime_file_context(tmp);

        if (rename(tmp.c_str(), LOCALTIME_PATH))
        {
            unlink(tmp.c_str());
            break;
        }

        this->time_zone_set(time_zone);

        update_kernel_utc_offset();

        /* RTC in local needs to be set for the new timezone */
        if (this->local_rtc_get())
        {
            start_hwclock_call(false, false, false, Glib::RefPtr<Gio::DBus::MethodInvocation>(), nullptr);
        }
        successed = true;
    } while (0);

    if (!successed)
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Failed to update " LOCALTIME_PATH));
    }
    else
    {
        invocation.ret();
    }
}

void TimedateManager::finish_set_rtc_local_hwclock(MethodInvocation invocation, bool local)
{
    this->local_rtc_set(local);
    invocation.ret();
}

void TimedateManager::finish_set_rtc_local(MethodInvocation invocation,
                                           bool local,
                                           bool adjust_system)
{
    start_hwclock_call(adjust_system,
                       local,
                       !local,
                       invocation.getMessage(),
                       std::bind(&TimedateManager::finish_set_rtc_local_hwclock, this, std::placeholders::_1, local));
}

void TimedateManager::finish_set_ntp_active(MethodInvocation invocation, bool active)
{
    SETTINGS_PROFILE("");

    GVariantBuilder builder1, builder2;
    int enable = 0;
    int disable = 0;
    std::string failed_name;

    /* Reload the list to get new NTP units installed on the system */
    this->read_ntp_units();

    /* Start and enable the first NTP unit if active is true. Stop and disable
    	   everything else. Errors are ignored for other units than first. */

    for (auto iter = this->ntp_units_.begin(); iter != this->ntp_units_.end(); ++iter)
    {
        auto &unit_name = iter->name;

        if (iter == this->ntp_units_.begin() && active)
        {
            if (!call_systemd_noresult("StartUnit", Glib::VariantContainerBase(g_variant_new("(ss)", unit_name.c_str(), "replace"), false)) &&
                iter == this->ntp_units_.begin())
            {
                failed_name = unit_name;
                break;
            }
            if (!enable++)
                g_variant_builder_init(&builder1, G_VARIANT_TYPE("as"));
            g_variant_builder_add(&builder1, "s", unit_name.c_str());
            LOG_DEBUG("start NTP unit: %s.", unit_name.c_str());
        }
        else
        {
            if (!call_systemd_noresult("StopUnit", Glib::VariantContainerBase(g_variant_new("(ss)", unit_name.c_str(), "replace"), false)) &&
                iter == this->ntp_units_.begin())
            {
                failed_name = unit_name;
                break;
            }
            if (!disable++)
                g_variant_builder_init(&builder2, G_VARIANT_TYPE("as"));
            g_variant_builder_add(&builder2, "s", unit_name.c_str());
            LOG_DEBUG("stop NTP unit: %s.", unit_name.c_str());
        }
    }

    if (failed_name.length() > 0)
    {
        std::string err_message = fmt::format("Failed to start/stop NTP unit: {0}.", failed_name);
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
    }
    else
    {
        if (enable)
        {
            call_systemd_noresult("EnableUnitFiles", Glib::VariantContainerBase(g_variant_new("(asbb)", &builder1, FALSE, TRUE), false));
        }

        if (disable)
        {
            call_systemd_noresult("DisableUnitFiles", Glib::VariantContainerBase(g_variant_new("(asb)", &builder2, FALSE), false));
        }

        /* This seems to be needed to update the unit state reported by systemd */
        if (enable || disable)
        {
            call_systemd_noresult("Reload", Glib::VariantContainerBase(g_variant_new("()"), false));
        }

        this->ntp_set(active);
        invocation.ret();
    }
}

void TimedateManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, TIMEDATE_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", TIMEDATE_OBJECT_PATH, e.what().c_str());
    }
}

void TimedateManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void TimedateManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran