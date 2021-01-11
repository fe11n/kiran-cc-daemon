/**
 * @FilePath      /kiran-cc-daemon/plugins/appearance/appearance-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <appearance_dbus_stub.h>

#include "plugins/appearance/background/appearance-background.h"
#include "plugins/appearance/font/appearance-font.h"
#include "plugins/appearance/theme/appearance-theme.h"

namespace Kiran
{
class AppearanceManager : public SessionDaemon::AppearanceStub
{
public:
    AppearanceManager();
    virtual ~AppearanceManager();

    static AppearanceManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    // 获取特定类型(参考AppearanceThemeType)的主题，返回的是一个json格式的字符串，方便后续做字段兼容性扩展
    virtual void GetThemes(gint32 type, MethodInvocation& invocation);
    // 将type类型的主题设置为theme_name。
    virtual void SetTheme(gint32 type, const Glib::ustring& theme_name, MethodInvocation& invocation);
    // 获取类型为type的字体，字体包括了字体名和字体大小(注意：其他项目的命名规范可能是字体名已经包含了字体大小，可能存在术语不统一的情况）。
    // 例如字体'San 10'的字体名为'San'，字体大小为10，type分类可参考AppearanceFontType
    virtual void GetFont(gint32 type, MethodInvocation& invocation);
    // 设置类型为type的字体
    virtual void SetFont(gint32 type, const Glib::ustring& font, MethodInvocation& invocation);
    // 设置桌面背景
    virtual void SetDesktopBackground(const Glib::ustring& desktop_background, MethodInvocation& invocation);

    virtual bool gtk_theme_setHandler(const Glib::ustring& value) { return true; };
    virtual bool metacity_theme_setHandler(const Glib::ustring& value) { return true; };
    virtual bool icon_theme_setHandler(const Glib::ustring& value) { return true; };
    virtual bool cursor_theme_setHandler(const Glib::ustring& value) { return true; };
    virtual bool desktop_background_setHandler(const Glib::ustring& value);

    virtual Glib::ustring gtk_theme_get() { return this->appearance_theme_.get_theme(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK); };
    virtual Glib::ustring metacity_theme_get() { return this->appearance_theme_.get_theme(AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY); };
    virtual Glib::ustring icon_theme_get() { return this->appearance_theme_.get_theme(AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON); };
    virtual Glib::ustring cursor_theme_get() { return this->appearance_theme_.get_theme(AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR); };
    virtual Glib::ustring desktop_background_get() { return this->desktop_background_; }

private:
    void init();

    void load_from_settings();

    void on_settings_changed(const Glib::ustring& key);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);

private:
    static AppearanceManager* instance_;

    AppearanceTheme appearance_theme_;
    AppearanceFont appearance_font_;
    AppearanceBackground appearance_background_;

    Glib::RefPtr<Gio::Settings> appearance_settings_;

    Glib::ustring desktop_background_;

    // dbus
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran
