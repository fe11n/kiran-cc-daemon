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

namespace Kiran
{
#define ADMIN_GROUP "wheel"
#define AUTH_USER_ADMIN "com.kylinsec.kiran.system-daemon.accounts.user-administration"
#define AUTH_CHANGE_OWN_USER_DATA "com.kylinsec.kiran.system-daemon.accounts.change-own-user-data"
#define AUTH_CHANGE_OWN_PASSWORD "com.kylinsec.kiran.system-daemon.accounts.change-own-password"

#define SPAWN_WITH_LOGIN_UID(invocation, ...)                                               \
    {                                                                                       \
        std::vector<std::string> argv = {__VA_ARGS__};                                      \
        CCErrorCode error_code;                                                             \
        if (!AccountsUtil::spawn_with_login_uid(invocation.getMessage(), argv, error_code)) \
        {                                                                                   \
            DBUS_ERROR_REPLY_AND_RET(error_code);                                           \
        }                                                                                   \
    }

class AccountsUtil
{
public:
    AccountsUtil(){};
    virtual ~AccountsUtil(){};

    static bool get_caller_pid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, GPid &pid);
    static bool get_caller_uid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t &uid);

    static void get_caller_loginuid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, std::string &loginuid);

    static void setup_loginuid(const std::string &id);
    static bool spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                     const std::vector<std::string> argv,
                                     CCErrorCode &error_code);

    // 翻译命令行返回的错误码
    static bool parse_exit_status(int32_t exit_status, CCErrorCode &error_code);
};
}  // namespace Kiran