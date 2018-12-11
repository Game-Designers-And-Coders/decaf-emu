project(decaf-qt)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(".")

file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE UI_FILES *.ui)

if(MSVC)
   set(RESOURCE_FILES
      ${CMAKE_SOURCE_DIR}/resources/decaf-sdl.rc
      ${CMAKE_SOURCE_DIR}/resources/hidpi.manifest)
else()
   set(RESOURCE_FILES "")
endif()

qt5_wrap_ui(UIS_HDRS ${UI_FILES})

add_executable(decaf-qt
   ${SOURCE_FILES}
   ${HEADER_FILES}
   ${UIS_HDRS}
   ${RESOURCE_FILES})

target_link_libraries(decaf-qt
   common
   libconfig
   libdecaf
   ${CPPTOML_LIBRARIES}
   ${EXCMD_LIBRARIES}
   ${SDL2_LINK}
   Qt5::Widgets
   Qt5::WinMain)

add_custom_command(
    TARGET decaf-qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Widgets>
        $<TARGET_FILE_DIR:decaf-qt>
)

add_custom_command(
    TARGET decaf-qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Gui>
        $<TARGET_FILE_DIR:decaf-qt>
)

add_custom_command(
    TARGET decaf-qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Core>
        $<TARGET_FILE_DIR:decaf-qt>
)

if(MSVC)
   set_target_properties(decaf-qt PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

install(TARGETS decaf-qt RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")
