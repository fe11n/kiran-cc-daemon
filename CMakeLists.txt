cmake_minimum_required(VERSION 3.2)

project(kiran-cc-daemon VERSION 2.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(KSD REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIBMM REQUIRED glibmm-2.4)
pkg_search_module(GIOMM REQUIRED giomm-2.4)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)
pkg_search_module(GIOUNIX REQUIRED gio-unix-2.0)
pkg_search_module(LIBXML2 REQUIRED libxml++-2.6)
pkg_search_module(KLOG_GTK3 REQUIRED klog-gtk3)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(GTEST REQUIRED gtest)
pkg_search_module(FMT REQUIRED fmt)

if(build-session-daemon)
  pkg_search_module(GDKMM3 REQUIRED gdkmm-3.0)
  pkg_search_module(GTKMM3 REQUIRED gtkmm-3.0)
  pkg_search_module(X11 REQUIRED x11)
  pkg_search_module(XEXT REQUIRED xext)
  pkg_search_module(XERCES REQUIRED xerces-c)
  pkg_search_module(FONTCONFIG REQUIRED fontconfig)
  pkg_search_module(CAIROMM_XLIB REQUIRED cairomm-xlib-1.0)
  pkg_search_module(LIBNOTIFY REQUIRED libnotify)
  pkg_search_module(LIBPULSE REQUIRED libpulse-mainloop-glib)
endif()

if(build-system-daemon)
  pkg_search_module(SYSTEMD REQUIRED systemd)
  pkg_search_module(LIBGTOP REQUIRED libgtop-2.0)
  pkg_search_module(GUDEV REQUIRED gudev-1.0)
endif()

find_program(GDBUS-CODEGEN-GLIBMM-FOUND gdbus-codegen-glibmm3
             PATHS ${CMAKE_BINARY_DIR}/bin/)

set_common_compiler_flags()

if(NOT GDBUS-CODEGEN-GLIBMM-FOUND)
  message(FATAL_ERROR "not found gdbus-codegen-glibmm3.")
else()
  set(GDBUS_CODEGEN ${GDBUS-CODEGEN-GLIBMM-FOUND})
endif()

message("gdbus-codegen: ${GDBUS_CODEGEN}")

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(test)
