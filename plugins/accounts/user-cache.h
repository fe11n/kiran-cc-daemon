/**
 * @file          /kiran-cc-daemon/plugins/accounts/user-cache.h
 * @brief         管理缓存文件的用户属性
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
using VPSS = std::vector<std::pair<std::string, std::string>>;

#define KEYFILE_USER_GROUP_NAME "User"
#define KEYFILE_USER_GROUP_KEY_LANGUAGE "Language"
#define KEYFILE_USER_GROUP_KEY_XSESSION "XSession"
#define KEYFILE_USER_GROUP_KEY_SESSION "Session"
#define KEYFILE_USER_GROUP_KEY_SESSION_TYPE "SessionType"
#define KEYFILE_USER_GROUP_KEY_EMAIL "Email"
#define KEYFILE_USER_GROUP_KEY_PASSWORD_HINT "PasswordHint"
#define KEYFILE_USER_GROUP_KEY_ICON "Icon"
#define KEYFILE_USER_GROUP_KEY_AUTH_MODES "AuthModes"

#define KEYFILE_FINGERPRINT_GROUP_NAME "FingerPrint"
#define KEYFILE_FACE_GROUP_NAME "Face"

class User;

class UserCache
{
public:
    UserCache(std::shared_ptr<User> user);
    virtual ~UserCache();

    // 获取字符串值，如果不存在则返回空
    std::string get_string(const std::string &group_name, const std::string &key);
    // 获取bool值
    bool get_boolean(const std::string &group_name, const std::string &key);
    // 获取int值
    int32_t get_int(const std::string &group_name, const std::string &key);
    // 获取某个group中的所有key/value
    VPSS get_group_kv(const std::string &group_name);

    // 设置key/value
    bool set_value(const std::string &group_name, const std::string &key, bool value);
    bool set_value(const std::string &group_name, const std::string &key, const std::string &value);
    bool set_value(const std::string &group_name, const std::string &key, int32_t value);

    // 删除key
    bool remove_key(const std::string &group_name, const std::string &key);

private:
    // 加载缓存文件
    bool load_cache_file();
    // 保存缓存数据到文件
    bool save_cache_file();

private:
    // 绑定的用户对象
    std::weak_ptr<User> user_;

    std::shared_ptr<Glib::KeyFile> keyfile_;
};
}  // namespace Kiran