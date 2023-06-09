cmake_minimum_required(VERSION 3.0)

SET_COMMON_EXE_LINKER_FLAGS()

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)

GEN_DBUS_STUB(CC_DAEMON cc_daemon com.kylinsec. ${CMAKE_SOURCE_DIR}/src/com.kylinsec.Kiran.CCDaemon.xml)

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

if(build-system-daemon)
# system daemon
set (TARGET_SYSTEM_DAEMON kiran-system-daemon)

add_executable(${TARGET_SYSTEM_DAEMON} 
    ${CORE_H_FILES}
    ${CORE_CPP_FILES}
    ${CC_DAEMON_GENERATED_STUB})

target_include_directories(${TARGET_SYSTEM_DAEMON} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GMODULE_INCLUDE_DIRS})

target_compile_definitions(${TARGET_SYSTEM_DAEMON} PRIVATE
    -DKCC_TYPE="system"
    -DKCC_SYSTEM_TYPE
    -DKCC_PLUGIN_DIR="${KCC_PLUGIN_DIR}/system")

target_link_libraries(${TARGET_SYSTEM_DAEMON} PRIVATE
    ${GMODULE_LIBRARIES}
    lib-base
    lib-dbus
    lib-iso)


install(TARGETS ${TARGET_SYSTEM_DAEMON}
        DESTINATION ${KCC_INSTALL_BINDIR})

endif()

if(build-session-daemon)
# session daemon
set (TARGET_SESSION_DAEMON kiran-session-daemon)

add_executable(${TARGET_SESSION_DAEMON} 
    ${CORE_H_FILES}
    ${CORE_CPP_FILES}
    ${CC_DAEMON_GENERATED_STUB})

target_include_directories(${TARGET_SESSION_DAEMON} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GMODULE_INCLUDE_DIRS}
    ${GTKMM3_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS})

target_compile_definitions(${TARGET_SESSION_DAEMON} PRIVATE
    -DKCC_TYPE="session"
    -DKCC_SESSION_TYPE
    -DKCC_PLUGIN_DIR="${KCC_PLUGIN_DIR}/session")


target_link_libraries(${TARGET_SESSION_DAEMON} PRIVATE
    ${GMODULE_LIBRARIES}
    ${GTKMM3_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
    lib-base
    lib-dbus
    lib-iso
    lib-display)


install(TARGETS ${TARGET_SESSION_DAEMON}
        DESTINATION ${KCC_INSTALL_BINDIR})

endif()
