cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME greeter)

file(GLOB_RECURSE GREETER_H_FILES ./*.h)
file(GLOB_RECURSE GREETER_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(GREETER greeter com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/greeter/com.kylinsec.Kiran.SystemDaemon.Greeter.xml)

add_library(${TARGET_NAME} SHARED
    ${GREETER_H_FILES}
    ${GREETER_CPP_FILES}
    ${GREETER_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()

target_include_directories(${TARGET_NAME} PUBLIC 
                           ${PROJECT_BINARY_DIR}/generated)


target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/system)


install(FILES ${PROJECT_SOURCE_DIR}/plugins/greeter/kiran-greeter.conf
        DESTINATION ${SYSCONFDIR}/lightdm/)
