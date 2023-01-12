cmake_minimum_required(VERSION 3.15)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Build for 10.1")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE Type Unspecified; picking Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

project(airwin-to-clap VERSION 0.9.0 LANGUAGES C CXX)


set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)


option(COPY_AFTER_BUILD "Copy the clap to ~/Library on MACOS, ~/.clap on linux" FALSE)

# use asan as an option (currently mac only)
option(USE_SANITIZER "Build and link with ASAN" FALSE)

# Copy on mac (could expand to other platforms)
option(COPY_AFTER_BUILD "Copy the clap to ~/Library on MACOS, ~/.clap on linux" FALSE)

add_subdirectory(libs/clap EXCLUDE_FROM_ALL)
add_subdirectory(libs/clap-helpers EXCLUDE_FROM_ALL)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/individuals")
    message(STATUS "Creating individuals directory")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/individuals")
endif()

add_library(${PROJECT_NAME} MODULE
        src/airwin-to-clap.cpp
        )
target_include_directories(${PROJECT_NAME} PRIVATE include ${CMAKE_BINARY_DIR}/individuals)
target_link_libraries(${PROJECT_NAME} clap-core clap-helpers)
set_property(TARGET ${PROJECT_NAME} PROPERTY AIRWIN_INCLUDES "")
set_property(TARGET ${PROJECT_NAME} PROPERTY AIRWIN_COUNT 0)
set(AIRWIN_DIR ${CMAKE_SOURCE_DIR}/libs/airwindows/plugins/MacVST/)

function (addAirwin airwin)
    set(awt aw2c_${airwin})
    set(aw_src ${AIRWIN_DIR}/${airwin}/source)
    add_library(${awt} STATIC ${aw_src}/${airwin}.cpp ${aw_src}/${airwin}Proc.cpp)
    target_include_directories(${awt} PRIVATE include)
    if (APPLE)
        target_compile_options(${awt} PRIVATE
               -Wno-unused-value
               -Wno-parentheses   # these ones look like actual bugs tbh
                )
    endif()
    if (MSVC)
	target_compile_definitions(${awt} PRIVATE -D_USE_MATH_DEFINES)
    endif()

    set(CREATE_FN "${awt}CreateEffectInstance")
    set(AIRWIN_NAME "${airwin}")
    configure_file(src/individual_airwin.in.h individuals/${awt}.h)
    configure_file(src/individual_airwin.in.cpp individuals/${awt}.cpp)

    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/individuals/${awt}.cpp)

    target_compile_definitions(${awt} PRIVATE -DcreateEffectInstance=${CREATE_FN})
    target_link_libraries(${PROJECT_NAME} ${awt})

    get_target_property(AW ${PROJECT_NAME} AIRWIN_INCLUDES)
    list(APPEND AW ${airwin})
    set_property(TARGET ${PROJECT_NAME} PROPERTY AIRWIN_INCLUDES ${AW})

    get_target_property(AWC ${PROJECT_NAME} AIRWIN_COUNT)
    math(EXPR AWC "${AWC} + 1")
    set_property(TARGET ${PROJECT_NAME} PROPERTY AIRWIN_COUNT ${AWC})
endfunction()

file(GLOB ALL_AIRWIN "${AIRWIN_DIR}*")

foreach(awdir ${ALL_AIRWIN})
    string(REPLACE ${AIRWIN_DIR} "" AWT ${awdir})
    addAirwin(${AWT})
endforeach()


get_target_property(AWC ${PROJECT_NAME} AIRWIN_COUNT)
message(STATUS "Generating ${AWC} plugins" )

set(sumfn "${CMAKE_BINARY_DIR}/individuals/aw2c_configured.hxx")
file(WRITE ${sumfn} "// machine generated. Include once\n")

get_target_property(AW ${PROJECT_NAME} AIRWIN_INCLUDES)
foreach(awt ${AW})
    message(STATUS "Airwindow: ${awt}")
    file(APPEND ${sumfn} "#include \"aw2c_${awt}.h\"\n")
endforeach()

file(APPEND ${sumfn} "uint32_t aw2c_get_plugin_count(const clap_plugin_factory *f) { return ${AWC}; }\n")
file(APPEND ${sumfn} "const clap_plugin_descriptor *aw2c_get_plugin_descriptor(const clap_plugin_factory *f, uint32_t w)\n")
file(APPEND ${sumfn} "{ switch(w) {\n")
set(CASEW 0)
foreach(awt ${AW})

    file(APPEND ${sumfn} "case ${CASEW}:\n")
    file(APPEND ${sumfn} "   return &aw2c_${awt}_desc;\n")
    file(APPEND ${sumfn} "   break;\n")
    math(EXPR CASEW "${CASEW} + 1")
endforeach()
file(APPEND ${sumfn} "} return nullptr; } \n")

file(APPEND ${sumfn} "std::pair<AudioEffect *, const clap_plugin_descriptor *> aw2c_get_aeffInstance(audioMasterCallback c, const char* plugin_id)\n")
file(APPEND ${sumfn} "{")
set(CASEW 0)
foreach(awt ${AW})
    file(APPEND ${sumfn} "if (strcmp(plugin_id, \"unofficial.com.airwindows.${awt}\") == 0) return { aw2c_${awt}CreateEffectInstance(c), &aw2c_${awt}_desc};\n")
endforeach()
file(APPEND ${sumfn} "return {nullptr, nullptr}; } \n")


if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUNDLE True
            BUNDLE_EXTENSION clap
            MACOSX_BUNDLE_GUI_IDENTIFIER org.surge-synth-team.${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}.plist.in
            )

    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_MAC=1)
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Werror -Wno-unused-value
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
            )
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
            )

    if (${COPY_AFTER_BUILD})
        message(STATUS "Will copy plugin after every build" )
        set(products_folder ${CMAKE_BINARY_DIR})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Installing ${products_folder}/${PROJECT_NAME}.clap to ~/Library/Audio/Plug-Ins/CLAP/"
                COMMAND ${CMAKE_COMMAND} -E make_directory "~/Library/Audio/Plug-Ins/CLAP"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${products_folder}/${PROJECT_NAME}.clap" "~/Library/Audio/Plug-Ins/CLAP/${PROJECT_NAME}.clap"
                )
    endif()
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_LINUX=1)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
    if (${COPY_AFTER_BUILD})
        message(STATUS "Will copy plugin after every build" )
        set(products_folder ${CMAKE_BINARY_DIR})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Installing ${products_folder}/${PROJECT_NAME}.clap to ~/.clap"
                COMMAND ${CMAKE_COMMAND} -E make_directory "~/.clap"
                COMMAND ${CMAKE_COMMAND} -E copy "${products_folder}/${PROJECT_NAME}.clap" "~/.clap"
                )
    endif()

else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_WIN=1)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
endif()
