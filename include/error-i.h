/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    enum CCErrorCode
    {
        // Common
        SUCCESS,
        ERROR_FAILED,

        // Main
        ERROR_PLUGIN_NOT_EXIST_1 = 0x00100,
        ERROR_PLUGIN_NOT_EXIST_2,
        ERROR_PLUGIN_NOTFOUND_NEW_PLUGIN_FUNC,
        ERROR_PLUGIN_NOTFOUND_DEL_PLUGIN_FUNC,
        ERROR_PLUGIN_OPEN_MODULE_FAILED,

        // Accounts
        ERROR_ACCOUNTS_USER_NOT_FOUND_1 = 0x50000,
        ERROR_ACCOUNTS_USER_NOT_FOUND_2,
        ERROR_ACCOUNTS_USER_NOT_FOUND_3,
        ERROR_ACCOUNTS_USER_NOT_FOUND_4,
        ERROR_ACCOUNTS_NOT_FOUND_PWENT,
        ERROR_ACCOUNTS_USER_ALREADY_EXIST,
        ERROR_ACCOUNTS_UNKNOWN_ACCOUNT_TYPE,

        ERROR_ACCOUNTS_SPAWN_SYNC_FAILED = 0x50100,
        ERROR_ACCOUNTS_SPAWN_EXIT_STATUS,
        ERROR_ACCOUNTS_DELETE_ROOT_USER,
        ERROR_ACCOUNTS_SAVE_AUTOLOGIN_FILE,

        ERROR_ACCOUNTS_USER_COMMAND_PW_UPDATE = 0x50200,
        ERROR_ACCOUNTS_USER_COMMAND_USAGE,
        ERROR_ACCOUNTS_USER_COMMAND_BAD_ARG,
        ERROR_ACCOUNTS_USER_COMMAND_UID_IN_USE,
        ERROR_ACCOUNTS_USER_COMMAND_BAD_PWFILE,
        ERROR_ACCOUNTS_USER_COMMAND_NOTFOUND,
        ERROR_ACCOUNTS_USER_COMMAND_USER_BUSY,
        ERROR_ACCOUNTS_USER_COMMAND_NAME_IN_USE,
        ERROR_ACCOUNTS_USER_COMMAND_GRP_UPDATE,
        ERROR_ACCOUNTS_USER_COMMAND_NOSPACE,
        ERROR_ACCOUNTS_USER_COMMAND_HOMEDIR,
        ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_1,
        ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_2,
        ERROR_ACCOUNTS_USER_COMMAND_SUB_UID_UPDATE,
        ERROR_ACCOUNTS_USER_COMMAND_SUB_GID_UPDATE,
        ERROR_ACCOUNTS_USER_COMMAND_UNKNOWN,

        ERROR_ACCOUNTS_UNKNOWN_CALLER_UID_1 = 0x50300,
        ERROR_ACCOUNTS_UNKNOWN_CALLER_UID_2,
        ERROR_ACCOUNTS_QUERY_INFO_FAILED,
        ERROR_ACCOUNTS_FILE_TYPE_NQ_REGULAR,
        ERROR_ACCOUNTS_FILE_SIZE_TOO_BIG,
        ERROR_ACCOUNTS_REPLACE_OUTPUT_STREAM,
        ERROR_ACCOUNTS_SPAWN_READ_FILE_FAILED,
        ERROR_ACCOUNTS_COPY_FILE_FAILED,
        ERROR_ACCOUNTS_GROUP_NOT_FOUND,
        ERROR_ACCOUNTS_USER_IS_LOCKED,
        ERROR_ACCOUNTS_EXPIRATION_POLICY_NOTFOUND,
        ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_1,
        ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_2,
        ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_3,
        ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_4,
        ERROR_ACCOUNTS_AUTHMODE_NAME_ALREADY_EXIST,
        ERROR_ACCOUNTS_AUTH_SAVE_DATA_FAILED,
        ERROR_ACCOUNTS_AUTH_DEL_DATA_FAILED,

        // Appearance
        ERROR_APPEARANCE_THEME_NOT_EXIST = 0xA0000,
        ERROR_APPEARANCE_THEME_TYPE_UNSUPPORTED,

        ERROR_APPEARANCE_THEME_TYPE_INVALID = 0xA0100,
        ERROR_APPEARANCE_FONT_TYPE_INVALID_1,
        ERROR_APPEARANCE_FONT_TYPE_INVALID_2,
        ERROR_APPEARANCE_FONT_TYPE_UNSUPPORTED,
        ERROR_APPEARANCE_SET_BACKGROUND_FAILED,
        ERROR_APPEARANCE_SET_LOCKSCREEN_BACKGROUND_FAILED,

        // Audio
        ERROR_AUDIO_SET_SINK_ACTIVE_PORT_FAILED = 0xC0000,
        ERROR_AUDIO_DEVICE_VOLUME_RANGE_INVLAID,
        ERROR_AUDIO_DEVICE_SET_VOLUME_FAILED,
        ERROR_AUDIO_DEVICE_BALANCE_RANGE_INVLAID,
        ERROR_AUDIO_DEVICE_SET_BALANCE_FAILED,
        ERROR_AUDIO_DEVICE_FADE_RANGE_INVLAID,
        ERROR_AUDIO_DEVICE_SET_FADE_FAILED,
        ERROR_AUDIO_DEVICE_SET_MUTE_FAILED,
        ERROR_AUDIO_DEFAULT_SINK_NOT_FOUND,
        ERROR_AUDIO_SINK_NOT_FOUND,
        ERROR_AUDIO_DEFAULT_SOURCE_NOT_FOUND,
        ERROR_AUDIO_SOURCE_NOT_FOUND,
        ERROR_AUDIO_SINK_INPUT_NOT_FOUND,
        ERROR_AUDIO_SOURCE_OUTPUT_NOT_FOUND,
        ERROR_AUDIO_STREAM_VOLUME_RANGE_INVLAID,
        ERROR_AUDIO_STREAM_SET_VOLUME_FAILED,
        ERROR_AUDIO_STREAM_SET_MUTE_FAILED,

        // Bluetooth
        ERROR_BLUETOOTH_EXIST_REQUEST_INCOMPLETE = 0xF0000,
        ERROR_BLUETOOTH_REQUEST_CANCELED,
        ERROR_BLUETOOTH_REQUEST_REJECTED_1,
        ERROR_BLUETOOTH_REQUEST_REJECTED_2,
        ERROR_BLUETOOTH_REQUEST_REJECTED_3,
        ERROR_BLUETOOTH_REQUEST_REJECTED_4,

        ERROR_BLUETOOTH_NOTFOUND_ADAPTOR = 0xF0100,

        // Display
        ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_1 = 0x140000,
        ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_2,
        ERROR_DISPLAY_COMMON_MODE_NOTFOUND,
        ERROR_DISPLAY_SET_AUTO_MODE_FAILED,
        ERROR_DISPLAY_SET_WINDOW_SCALING_FACTOR_1,
        ERROR_DISPLAY_SET_WINDOW_SCALING_FACTOR_2,
        ERROR_DISPLAY_EXEC_XRANDR_FAILED,
        ERROR_DISPLAY_CONFIG_IS_EMPTY,
        ERROR_DISPLAY_CONFIG_ITEM_NOTFOUND,
        ERROR_DISPLAY_PRIMARY_MONITOR_IS_EMPTY,
        ERROR_DISPLAY_NOTFOUND_PRIMARY_MONITOR_BY_NAME,
        ERROR_DISPLAY_SAVE_CREATE_FILE_FAILED,
        ERROR_DISPLAY_WRITE_CONF_FILE_FAILED,

        ERROR_DISPLAY_ONLY_ONE_ENABLED_MONITOR = 0x140100,
        ERROR_DISPLAY_EXIST_NULL_MODE_IN_LIST,
        ERROR_DISPLAY_EXIST_NULL_MODE_IN_PREFER_LIST,
        ERROR_DISPLAY_MODE_NOT_EXIST,
        ERROR_DISPLAY_NOTFOUND_MATCH_MODE_1,
        ERROR_DISPLAY_NOTFOUND_MATCH_MODE_2,
        ERROR_DISPLAY_NOTFOUND_MODE_BY_ID,
        ERROR_DISPLAY_UNKNOWN_ROTATION_TYPE,
        ERROR_DISPLAY_UNKNOWN_REFLECT_TYPE,

        // Greeter
        ERROR_GREETER_SYNC_TO_FILE_FAILED_1 = 0x190000,
        ERROR_GREETER_SYNC_TO_FILE_FAILED_2,
        ERROR_GREETER_SYNC_TO_FILE_FAILED_3,
        ERROR_GREETER_SYNC_TO_FILE_FAILED_4,
        ERROR_GREETER_SYNC_TO_FILE_FAILED_5,
        ERROR_GREETER_SYNC_TO_FILE_FAILED_6,
        // ERROR_GREETER_NOTFOUND_USER,
        ERROR_GREETER_SCALE_MODE_INVALIDE,

        // Keyboard
        ERROR_KEYBOARD_LAYOUT_EXCEED_LIMIT = 0x1E0000,
        ERROR_KEYBOARD_LAYOUT_INVALID,
        ERROR_KEYBOARD_LAYOUT_ALREADY_EXIST,
        ERROR_KEYBOARD_LAYOUT_SET_FAILED,
        ERROR_KEYBOARD_LAYOUT_NOT_EXIST,
        ERROR_KEYBOARD_LAYOUT_UPDATE_FAILED,
        ERROR_KEYBOARD_LAYOUT_OPTION_ALREADY_EXIST,
        ERROR_KEYBOARD_LAYOUT_OPTION_SET_FAILED,
        ERROR_KEYBOARD_LAYOUT_OPTION_NOT_EXIST,
        ERROR_KEYBOARD_LAYOUT_OPTION_UPDATE_FAILED,
        ERROR_KEYBOARD_LAYOUT_OPTION_CLEAR_FAILED,

        // Mouse
        ERROR_MOUSE_START = 0x230000,

        // Touchpad
        ERROR_TOUCHPAD_START = 0x280000,

        // Keybinding
        ERROR_KEYBINDING_CUSTOM_SHORTCUT_INVALID = 0x2D0000,
        ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_1,
        ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_2,
        ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_3,
        ERROR_KEYBINDING_CUSTOM_SHORTCUT_ALREADY_EXIST,
        ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST_1,
        ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST_2,
        ERROR_KEYBINDING_GEN_UID_FAILED,
        ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID_1,
        ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID_2,
        ERROR_KEYBINDING_GRAB_KEY_FAILED,

        ERROR_KEYBINDING_SYSTEM_KEYCOMB_INVALID_1,
        ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST_1,
        ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST_2,

        // Power
        ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_1 = 0x320000,
        ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_2,
        ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_3,
        ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_4,
        ERROR_POWER_DEVICE_UNSUPPORTED_1,
        ERROR_POWER_DEVICE_UNSUPPORTED_2,
        ERROR_POWER_DEVICE_UNSUPPORTED_3,
        ERROR_POWER_DEVICE_UNSUPPORTED_4,
        ERROR_POWER_DIMMED_SCALE_RANGE_ERROR,
        ERROR_POWER_UNKNOWN_ACTION_1,
        ERROR_POWER_UNKNOWN_ACTION_2,
        ERROR_POWER_EVENT_UNSUPPORTED_1,
        ERROR_POWER_EVENT_UNSUPPORTED_2,
        ERROR_POWER_SET_ACTION_FAILED,
        ERROR_POWER_SET_BRIGHTNESS_FAILED,

        // SystemInfo
        ERROR_SYSTEMINFO_TYPE_INVALID = 0x370000,
        ERROR_SYSTEMINFO_JSON_ASSIGN_FAILED,
        ERROR_SYSTEMINFO_SET_HOSTNAME_FAILED,

        // Timedate
        ERROR_TIMEDATE_NTP_IS_ACTIVE = 0x3C0000,
        ERROR_TIMEDATE_TIMEZONE_INVALIDE,
        ERROR_TIMEDATE_NO_NTP_UNIT,
        ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_1,
        ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_2,
        ERROR_TIMEDATE_SET_DATE_FORMAT_FAILED,
        ERROR_TIMEDATE_SET_HOUR_FORMAT_FAILED,
        ERROR_TIMEDATE_SET_SECONDS_SHOWING_FAILED,
        ERROR_TIMEDATE_START_NTP_FAILED,
        ERROR_TIMEDATE_STOP_NTP_FAILED,

        // XSettings
        ERROR_XSETTINGS_NOTFOUND_PROPERTY = 0x410000,
        ERROR_XSETTINGS_PROPERTY_TYPE_MISMATCH,
        ERROR_XSETTINGS_PROPERTY_INVALID,
        ERROR_XSETTINGS_PROPERTY_UNSUPPORTED,
        ERROR_XSETTINGS_PROPERTY_ONLYREAD,

    };
#ifdef __cplusplus
}
#endif
