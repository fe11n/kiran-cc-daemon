/*
 * @Author       : tangjie02
 * @Date         : 2020-08-18 09:47:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-18 15:13:58
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/xkb-rules-parser.h
 */

#pragma once

#include <string>
#include <vector>

namespace xmlpp
{
class Node;
};

namespace Kiran
{
struct XkbModel
{
    std::string name;
    std::string description;
    std::string vendor;
};

struct XkbVariant
{
    std::string name;
    std::string short_description;
    std::string description;
};

struct XkbLayout
{
    std::string name;
    std::string short_description;
    std::string description;
    std::vector<XkbVariant> variants;
};

struct XkbOption
{
    std::string name;
    std::string description;
};

struct XkbOptionGroup
{
    std::string name;
    std::string description;
    std::vector<XkbOption> options;
};

struct XkbRules
{
    std::vector<XkbModel> models;
    std::vector<XkbLayout> layouts;
    std::vector<XkbOptionGroup> option_groups;
};

class XkbRulesParser
{
public:
    XkbRulesParser(const std::string &file_name);
    virtual ~XkbRulesParser(){};

    bool parse(XkbRules &xkb_rules, std::string &err);

private:
    bool process_config_registry(const xmlpp::Node *node, XkbRules &xkb_rules, std::string &err);

    bool process_models(const xmlpp::Node *node, std::vector<XkbModel> &xkb_models, std::string &err);
    bool process_model(const xmlpp::Node *node, XkbModel &xkb_model, std::string &err);
    bool process_model_config_item(const xmlpp::Node *node, XkbModel &xkb_model, std::string &err);

    bool process_layouts(const xmlpp::Node *node, std::vector<XkbLayout> &xkb_layouts, std::string &err);
    bool process_layout(const xmlpp::Node *node, XkbLayout &xkb_layout, std::string &err);
    bool process_layout_config_item(const xmlpp::Node *node, XkbLayout &xkb_layout, std::string &err);
    bool process_layout_variants(const xmlpp::Node *node, std::vector<XkbVariant> &xkb_variants, std::string &err);
    bool process_layout_variant(const xmlpp::Node *node, XkbVariant &xkb_variant, std::string &err);
    bool process_layout_variant_config_item(const xmlpp::Node *node, XkbVariant &xkb_variant, std::string &err);

    bool process_option_groups(const xmlpp::Node *node, std::vector<XkbOptionGroup> &xkb_option_groups, std::string &err);
    bool process_option_group(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, std::string &err);
    bool process_option_group_config_item(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, std::string &err);
    bool process_option(const xmlpp::Node *node, XkbOption &xkb_option, std::string &err);
    bool process_option_config_item(const xmlpp::Node *node, XkbOption &xkb_option, std::string &err);

    bool process_content_node(const xmlpp::Node *node, std::string &content, std::string &err);

private:
    std::string file_name_;
};
}  // namespace Kiran