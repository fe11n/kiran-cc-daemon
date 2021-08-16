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

#include <accounts_dbus_stub.h>

#include "plugins/accounts/accounts-wrapper.h"
#include "plugins/accounts/user-classify.h"
#include "plugins/accounts/user.h"

namespace Kiran
{
class AccountsManager : public SystemDaemon::AccountsStub
{
public:
    AccountsManager() = delete;
    AccountsManager(AccountsWrapper *passwd_wrapper);
    virtual ~AccountsManager();

    static AccountsManager *get_instance() { return instance_; };

    static void global_init(AccountsWrapper *passwd_wrapper);

    static void global_deinit() { delete instance_; };

    std::shared_ptr<User> lookup_user_by_name(const std::string &user_name);
    std::shared_ptr<User> get_autologin_user();

    bool set_automatic_login(std::shared_ptr<User> user, bool enabled, CCErrorCode &error_code);

protected:
    // 获取非系统用户的DBusObjectPath，非系统用户一般为用户自己创建的账号。例如root为系统账户
    virtual void GetNonSystemUsers(MethodInvocation &invocation);
    // 通过uid获取用户的DBusObjectPath，如果DBusObjectPath不存在则会创建一个新的。
    virtual void FindUserById(guint64 uid, MethodInvocation &invocation);
    // 通过name获取用户的DBusObjectPath
    virtual void FindUserByName(const Glib::ustring &name, MethodInvocation &invocation);
    // 通过认证数据（指纹、人脸）查找用户
    virtual void FindUserByAuthData(gint32 mode, const Glib::ustring &data_id, MethodInvocation &invocation);
    // 创建一个用户，用户可以为普通用户和管理员用户，管理员用户的定义可以参考policykit的addAdminRule规则。
    virtual void CreateUser(const Glib::ustring &name,
                            const Glib::ustring &real_name,
                            gint32 account_type,
                            gint64 uid,
                            MethodInvocation &invocation);
    // 删除一个用户
    virtual void DeleteUser(guint64 uid, bool remove_files, MethodInvocation &invocation);

private:
    void init();

    void accounts_file_changed(FileChangedType type);
    bool accounts_file_changed_timeout();
    void gdm_custom_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void update_automatic_login();

    bool reload_users();
    std::map<std::string, std::shared_ptr<User>> load_users();

    std::shared_ptr<User> add_new_user_for_pwent(std::shared_ptr<Passwd> pwent, std::shared_ptr<SPwd> spent);
    std::shared_ptr<User> find_and_create_user_by_id(uint64_t uid);
    std::shared_ptr<User> find_and_create_user_by_name(const std::string &user_name);
    bool list_non_system_users_idle(MethodInvocation invocation);
    void create_user_authorized_cb(MethodInvocation invocation,
                                   const Glib::ustring &name,
                                   const Glib::ustring &realname,
                                   gint32 account_type,
                                   gint64 uid);
    void delete_user_authorized_cb(MethodInvocation invocation, uint64_t uid, bool remove_files);

    bool is_explicitly_requested_user(const std::string &user_name);

    bool read_autologin_from_file(std::string &name, bool &enabled, std::string &err);
    bool save_autologin_to_file(const std::string &name, bool enabled, std::string &err);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static AccountsManager *instance_;

    AccountsWrapper *passwd_wrapper_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
    sigc::connection reload_conn_;

    Glib::RefPtr<Gio::FileMonitor> gdm_custom_monitor_;

    std::map<std::string, std::shared_ptr<User>> users_;
    std::weak_ptr<User> autologin_;
    std::set<std::string> explicitly_requested_users_;
};
}  // namespace Kiran