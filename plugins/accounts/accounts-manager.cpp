/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-08 09:29:49
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.cpp
 */

#include "plugins/accounts/accounts-manager.h"

#include <act/act-user-manager.h>
#include <fmt/format.h>

#include <cinttypes>
#include <cstdint>

#include "lib/helper.h"
#include "lib/log.h"
#include "plugins/accounts/accounts-common.h"

namespace Kiran
{
#define ACCOUNTS_OBJECT_PATH "/com/unikylin/Kiran/System/Accounts"

AccountsManager::AccountsManager() : dbus_connect_id_(0),
                                     object_register_id_(0)
{
    act_user_manager_ = act_user_manager_get_default();
}

AccountsManager::~AccountsManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

AccountsManager *AccountsManager::instance_ = nullptr;
void AccountsManager::global_init()
{
    instance_ = new AccountsManager();
    instance_->init();
}

std::shared_ptr<User> AccountsManager::lookup_user(ActUser *act_user)
{
    std::string user_name = act_user_get_user_name(act_user);
    return lookup_user_by_name(user_name);
}

std::shared_ptr<User> AccountsManager::lookup_user_by_name(const std::string &user_name)
{
    auto iter = this->users_.find(user_name);
    if (iter != this->users_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<User> AccountsManager::lookup_user_by_uid(int64_t uid)
{
    auto iter = this->users_by_uid_.find(uid);
    if (iter != this->users_by_uid_.end())
    {
        if (!iter->second.expired())
        {
            return iter->second.lock();
        }
    }
    return nullptr;
}

void AccountsManager::CreateUser(const Glib::ustring &name,
                                 const Glib::ustring &real_name,
                                 gint32 account_type,
                                 MethodInvocation &invocation)
{
    g_autoptr(GError) error = NULL;
    auto act_user = act_user_manager_create_user(this->act_user_manager_, name.c_str(), real_name.c_str(), ActUserAccountType(account_type), &error);
    auto user = add_user(act_user);

    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        std::string err_message = error ? error->message : fmt::format("failed to create user: {0}.", name.raw());

        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
    }
}

void AccountsManager::DeleteUser(gint64 id,
                                 bool remove_files,
                                 MethodInvocation &invocation)
{
    g_autoptr(GError) error = NULL;
    auto user = lookup_user_by_uid(id);
    ActUser *act_user = user ? user->get_act_user() : NULL;
    del_user(act_user);
    auto ret = act_user_manager_delete_user(this->act_user_manager_, act_user, remove_files, &error);

    if (ret)
    {
        invocation.ret();
    }
    else
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), error->message));
    }
}

void AccountsManager::FindUserById(gint64 id,
                                   MethodInvocation &invocation)
{
    auto user = lookup_user_by_uid(id);
    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        auto message = fmt::format("invalid id: {0}", id);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), message.c_str()));
    }
}

void AccountsManager::FindUserByName(const Glib::ustring &name,
                                     MethodInvocation &invocation)
{
    auto user = lookup_user_by_name(name.raw());

    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        auto message = fmt::format("invalid name: {0}", name.raw());
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), message.c_str()));
    }
}

void AccountsManager::signal_user_added(ActUser *act_user, gpointer user_data)
{
    SETTINGS_PROFILE("");

    AccountsManager *self = (AccountsManager *)user_data;
    g_return_if_fail(self == AccountsManager::get_instance());

    self->add_user(act_user);

    Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);

    self->UserAdded_signal.emit(object_path);
}

void AccountsManager::signal_user_removed(ActUser *act_user, gpointer user_data)
{
    SETTINGS_PROFILE("");

    AccountsManager *self = (AccountsManager *)user_data;
    g_return_if_fail(self == AccountsManager::get_instance());

    self->del_user(act_user);

    Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);

    self->UserDeleted_signal.emit(object_path);
}

void AccountsManager::init()
{
    SETTINGS_PROFILE("");

    g_return_if_fail(this->act_user_manager_ != NULL);

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 ACCOUNTS_DBUS_NAME,
                                                 sigc::mem_fun(this, &AccountsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &AccountsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &AccountsManager::on_name_lost));

    load_users();

    g_signal_connect(this->act_user_manager_, "user-added", G_CALLBACK(AccountsManager::signal_user_added), this);
    g_signal_connect(this->act_user_manager_, "user-removed", G_CALLBACK(AccountsManager::signal_user_removed), this);
}

void AccountsManager::load_users()
{
    SETTINGS_PROFILE("");

    auto users = act_user_manager_list_users(this->act_user_manager_);
    SCOPE_EXIT({ g_slist_free(users); });

    for (auto l = users; l != NULL; l = l->next)
    {
        auto act_user = (ActUser *)(l->data);
        add_user(act_user);
    }
}

std::shared_ptr<User> AccountsManager::add_user(ActUser *act_user)
{
    auto user = std::make_shared<User>(act_user);
    bool is_added = false;

    std::string user_name = act_user_get_user_name(act_user);
    if (user_name.length() > 0)
    {
        auto iter = this->users_.emplace(user_name, user);
        if (!iter.second)
        {
            LOG_DEBUG("failed to inser user %s, it's already exist.", user_name.c_str());
        }
        else
        {
            user->init();
            is_added = true;
        }
    }
    else
    {
        LOG_WARNING("failed to insert user which name is empty.");
    }

    if (is_added)
    {
        int64_t uid = act_user_get_uid(act_user);
        if (uid > 0)
        {
            auto iter = this->users_by_uid_.emplace(uid, user);
            if (!iter.second)
            {
                LOG_DEBUG("fail to inser user uid %" PRIu64 " , it's already exist.", uid);
            }
        }
        else
        {
            LOG_WARNING("failed to insert user which uid equal to 0.");
        }
        return user;
    }
    return nullptr;
}

void AccountsManager::del_user(ActUser *act_user)
{
    do
    {
        std::string user_name = act_user_get_user_name(act_user);

        auto iter = this->users_.find(user_name);
        if (iter == this->users_.end())
        {
            LOG_WARNING("user %s isn't exist.", user_name.c_str());
        }
        else
        {
            this->users_.erase(iter);
        }
    } while (0);

    do
    {
        auto uid = act_user_get_uid(act_user);

        auto iter = this->users_by_uid_.find(uid);
        if (iter == this->users_by_uid_.end())
        {
            LOG_WARNING("user uid %" PRIu64 " isn't exist.", uid);
        }
        else
        {
            this->users_by_uid_.erase(iter);
        }
    } while (0);
}

void AccountsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        LOG_ERROR("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, ACCOUNTS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_ERROR("register object_path %s fail: %s.", ACCOUNTS_OBJECT_PATH, e.what().c_str());
    }
}

void AccountsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void AccountsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_ERROR("failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran
