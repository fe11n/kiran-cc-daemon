/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 14:28:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-19 13:37:39
 * @Description  : 
 * @FilePath     : /kiran-session-daemon/src/plugin-info.h
 */

#include <giomm.h>

#include <memory>
#include <string>

#include "plugin_i.h"

namespace Kiran
{
class PluginInfo
{
public:
    PluginInfo();
    virtual ~PluginInfo();

    bool load_from_file(const std::string &file_name, std::string &err);

    const std::string &get_location() { return this->location_; }
    const std::string &get_name() { return this->name_; }
    const std::string &get_description() { return this->description_; }
    bool is_available() { return this->available_; }

    std::shared_ptr<Plugin> get_plugin() { return this->plugin_; }

    bool activate_plugin(std::string &err);

private:
    bool load_plugin_module(std::string &err);

private:
    std::string file_name_;

    std::string location_;
    std::string name_;
    std::string description_;
    bool available_;

    std::shared_ptr<Glib::Module> module_;
    std::shared_ptr<Plugin> plugin_;
};
}  // namespace Kiran