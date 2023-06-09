cmake_minimum_required(VERSION 3.0)

find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)

# system-daemon
if(build-system-daemon)

    # policy files
    file(GLOB POLICY_IN_FILES ./*SystemDaemon*.policy.in)

    foreach(POLICY_IN_FILE IN LISTS POLICY_IN_FILES)
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" POLICY_FILE ${POLICY_IN_FILE})
        execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/ ${POLICY_IN_FILE} ${PROJECT_BINARY_DIR}/data/${POLICY_FILE})

        install(FILES ${PROJECT_BINARY_DIR}/data/${POLICY_FILE}
                DESTINATION ${KCC_INSTALL_DATADIR}/polkit-1/actions/)
    endforeach()


    # service files
    file(GLOB SERVICE_IN_FILES ./*SystemDaemon*service.in ./kiran-system-daemon.service.in)

    foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
        configure_file(${SERVICE_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE})
    endforeach()

    if (SYSTEMD_FOUND)
        pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
        message(STATUS "systemdsystemunitdir: ${SYSTEM_UNIT_DIR}")
        install(FILES ${PROJECT_BINARY_DIR}/data/kiran-system-daemon.service
                DESTINATION ${SYSTEM_UNIT_DIR})
    endif()


    install(FILES ${PROJECT_BINARY_DIR}/data/com.kylinsec.Kiran.SystemDaemon.service
                  ${PROJECT_BINARY_DIR}/data/com.kylinsec.Kiran.SystemDaemon.Accounts.service
                  ${PROJECT_BINARY_DIR}/data/com.kylinsec.Kiran.SystemDaemon.TimeDate.service
                  ${PROJECT_BINARY_DIR}/data/com.kylinsec.Kiran.SystemDaemon.Greeter.service
                  ${PROJECT_BINARY_DIR}/data/com.kylinsec.Kiran.SystemDaemon.SystemInfo.service
            DESTINATION ${KCC_INSTALL_DATADIR}/dbus-1/system-services)

    # conf files
    file(GLOB CONF_FILES ./*.conf)

    foreach(CONF_FILE IN LISTS CONF_FILES)    
        install(FILES ${CONF_FILE}
                DESTINATION ${SYSCONFDIR}/dbus-1/system.d)
    endforeach()

    # plugins configuration
    configure_file(${PROJECT_SOURCE_DIR}/data/plugin_options.system.in ${PROJECT_BINARY_DIR}/data/plugin_options.system)
    install(FILES ${PROJECT_BINARY_DIR}/data/plugin_options.system
            DESTINATION ${KCC_PLUGIN_DIR}/system
            RENAME plugin_options)
endif()

# session-daemon
if(build-session-daemon)

    # policy files
    file(GLOB POLICY_IN_FILES ./*SessionDaemon*.policy.in)

    foreach(POLICY_IN_FILE IN LISTS POLICY_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" POLICY_FILE ${POLICY_IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/ ${POLICY_IN_FILE} ${PROJECT_BINARY_DIR}/data/${POLICY_FILE})

    install(FILES ${PROJECT_BINARY_DIR}/data/${POLICY_FILE}
        DESTINATION ${KCC_INSTALL_DATADIR}/polkit-1/actions/)
    endforeach()

    # gschema.xml
    file(GLOB SCHEMA_IN_FILES ./*.gschema.xml.in)

    foreach(SCHEMA_IN_FILE IN LISTS SCHEMA_IN_FILES)
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SCHEMA_FILE ${SCHEMA_IN_FILE})
        configure_file(${SCHEMA_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SCHEMA_FILE})

        install(FILES ${PROJECT_BINARY_DIR}/data/${SCHEMA_FILE}
                DESTINATION  ${KCC_INSTALL_DATADIR}/glib-2.0/schemas/)    
    endforeach()

    install(CODE "execute_process(COMMAND glib-compile-schemas \"${KCC_INSTALL_DATADIR}/glib-2.0/schemas/\")")

    # plugins configuration
    configure_file(${PROJECT_SOURCE_DIR}/data/plugin_options.session.in ${PROJECT_BINARY_DIR}/data/plugin_options.session)
    install(FILES ${PROJECT_BINARY_DIR}/data/plugin_options.session
            DESTINATION ${KCC_PLUGIN_DIR}/session
            RENAME plugin_options)

    # service files
    file(GLOB SERVICE_IN_FILES ./*SessionDaemon*service.in)

    foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
        configure_file(${SERVICE_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE})
        install(FILES ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE}
                DESTINATION ${KCC_INSTALL_DATADIR}/dbus-1/services)
    endforeach()

    # desktop files
    configure_file(./kiran-session-daemon.desktop.in ${PROJECT_BINARY_DIR}/data/kiran-session-daemon.desktop)
    install(FILES ${PROJECT_BINARY_DIR}/data/kiran-session-daemon.desktop
                DESTINATION ${SYSCONFDIR}/xdg/autostart)
    
endif()


# include files
install(FILES ${PROJECT_SOURCE_DIR}/include/accounts_i.h
              ${PROJECT_SOURCE_DIR}/include/timedate_i.h
              ${PROJECT_SOURCE_DIR}/include/greeter_i.h
              ${PROJECT_SOURCE_DIR}/include/systeminfo_i.h
              ${PROJECT_SOURCE_DIR}/include/accounts-i.h
              ${PROJECT_SOURCE_DIR}/include/timedate-i.h
              ${PROJECT_SOURCE_DIR}/include/greeter-i.h
              ${PROJECT_SOURCE_DIR}/include/systeminfo-i.h
        DESTINATION ${KCC_INSTALL_INCLUDE}/kiran-system-daemon)

install(FILES ${PROJECT_SOURCE_DIR}/include/display_i.h
              ${PROJECT_SOURCE_DIR}/include/appearance-i.h
              ${PROJECT_SOURCE_DIR}/include/display-i.h
              ${PROJECT_SOURCE_DIR}/include/keybinding-i.h
              ${PROJECT_SOURCE_DIR}/include/keyboard-i.h
              ${PROJECT_SOURCE_DIR}/include/mouse-i.h
              ${PROJECT_SOURCE_DIR}/include/power-i.h
              ${PROJECT_SOURCE_DIR}/include/touchpad-i.h
              ${PROJECT_SOURCE_DIR}/include/xsettings-i.h
              ${PROJECT_SOURCE_DIR}/include/network-i.h
              ${PROJECT_SOURCE_DIR}/include/clipboard-i.h
        DESTINATION ${KCC_INSTALL_INCLUDE}/kiran-session-daemon)

install(FILES ${PROJECT_SOURCE_DIR}/include/error-i.h
        DESTINATION ${KCC_INSTALL_INCLUDE})

# pc file
file(GLOB PC_IN_FILES ./*.pc.in)

foreach(PC_IN_FILE IN LISTS PC_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" PC_FILE ${PC_IN_FILE})
    configure_file(${PC_IN_FILE} ${PROJECT_BINARY_DIR}/data/${PC_FILE})

    install(FILES ${PROJECT_BINARY_DIR}/data/${PC_FILE}
            DESTINATION ${KCC_INSTALL_LIBDIR}/pkgconfig)
endforeach()

