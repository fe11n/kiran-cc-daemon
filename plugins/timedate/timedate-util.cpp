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

#include "plugins/timedate/timedate-util.h"

#include <fcntl.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lib/base/base.h"
#include "plugins/timedate/timedate-def.h"

namespace Kiran
{
bool TimedateUtil::is_local_rtc()
{
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

std::string TimedateUtil::get_timezone()
{
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

    return std::string(zone);
}

int64_t TimedateUtil::get_gmt_offset(const std::string &zone)
{
    char *tz = getenv("TZ");
    setenv("TZ", zone.c_str(), 1);

    time_t cur_time = time(NULL);
    auto tm = localtime(&cur_time);

    if (tz)
    {
        setenv("TZ", tz, 1);
    }
    else
    {
        unsetenv("TZ");
    }

    if (!tm)
    {
        return -1;
    }

#if defined __USE_MISC
    return tm->tm_gmtoff;
#else
    return tm->__tm_gmtoff;
#endif
}
}  // namespace Kiran
