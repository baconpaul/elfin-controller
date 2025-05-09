# A basic installer setup.
#
# This cmake file introduces two targets
#  elfin-controller-staged:      moves all the built assets to a well named directory
#  elfin-controller-installer:   depends on elfin-controller-staged, builds an installer
#
# Right now elfin-controller-installer builds just the crudest zip file but this is the target
# on which we will hang the proper installers later

set(ELFINCONT_PRODUCT_DIR ${CMAKE_BINARY_DIR}/elfin_controller_products)
file(MAKE_DIRECTORY ${ELFINCONT_PRODUCT_DIR})

add_custom_target(elfin-controller-staged)
add_custom_target(elfin-controller-installer)

function(elfin_controller_package format)
    get_target_property(output_dir elfin-controller RUNTIME_OUTPUT_DIRECTORY)

    if( TARGET elfin-controller_${format})
        add_dependencies(elfin-controller-staged elfin-controller_${format})
        add_custom_command(
                TARGET elfin-controller-staged
                POST_BUILD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMAND echo "Installing ${output_dir}/${format} to ${ELFINCONT_PRODUCT_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${output_dir}/${format}/ ${ELFINCONT_PRODUCT_DIR}/
        )
    endif()
endfunction()

elfin_controller_package(VST3)
elfin_controller_package(AU)
elfin_controller_package(CLAP)
elfin_controller_package(Standalone)

if (WIN32)
    message(STATUS "Including special windows cleanup installer stage")
    add_custom_command(TARGET elfin-controller-staged
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Cleaning up windows goobits"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${ELFINCONT_PRODUCT_DIR}/elfin-controller.exp"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${ELFINCONT_PRODUCT_DIR}/elfin-controller.ilk"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${ELFINCONT_PRODUCT_DIR}/elfin-controller.lib"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${ELFINCONT_PRODUCT_DIR}/elfin-controller.pdb"
            )
endif ()

add_dependencies(elfin-controller-installer elfin-controller-staged)

add_custom_command(
        TARGET elfin-controller-installer
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND echo "Installing ${output_dir}/${format} to ${ELFINCONT_PRODUCT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE-gpl3 ${ELFINCONT_PRODUCT_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/installer/ZipReadme.txt ${ELFINCONT_PRODUCT_DIR}/Readme.txt
)


find_package(Git)


string(TIMESTAMP ELFINCONT_DATE "%Y-%m-%d")
set(ELFINCONT_ZIP elfin-controller-${ELFINCONT_DATE}-${GIT_COMMIT_HASH}-${CMAKE_SYSTEM_NAME}.zip)

if (APPLE)
    message(STATUS "Configuring for mac installer")
    add_custom_command(
            TARGET elfin-controller-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${CMAKE_SOURCE_DIR}/libs/sst/sst-plugininfra/scripts/installer_mac/make_installer.sh "elfin-controller" ${ELFINCONT_PRODUCT_DIR} ${CMAKE_SOURCE_DIR}/resources/installer_mac ${CMAKE_BINARY_DIR}/installer "${ELFINCONT_DATE}-${GIT_COMMIT_HASH}"
    )
elseif (WIN32)
    message(STATUS "Configuring for win installer")
    add_custom_command(
            TARGET elfin-controller-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND 7z a -r installer/${ELFINCONT_ZIP} ${ELFINCONT_PRODUCT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E echo "ZIP Installer in: installer/${ELFINCONT_ZIP}")
    message(STATUS "Skipping NuGet for now")
    #find_program(ELFINCONT_NUGET_EXE nuget.exe PATHS ENV "PATH")
    #if(ELFINCONT_NUGET_EXE)
    #    message(STATUS "NuGet found at ${ELFINCONT_NUGET_EXE}")
    #    add_custom_command(
    #        TARGET elfin-controller-installer
    #        POST_BUILD
    #        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    #        COMMAND ${ELFINCONT_NUGET_EXE} install Tools.InnoSetup -version 6.2.1
    #        COMMAND Tools.InnoSetup.6.2.1/tools/iscc.exe /O"installer" /DELFINCONT_SRC="${CMAKE_SOURCE_DIR}" /DELFINCONT_BIN="${CMAKE_BINARY_DIR}" /DMyAppVersion="${ELFINCONT_DATE}-${GIT_COMMIT_HASH}" "${CMAKE_SOURCE_DIR}/resources/installer_win/monique${BITS}.iss")
    #else()
    #    message(STATUS "NuGet not found")
    #endif()
else ()
    message(STATUS "Basic Installer: Target is installer/${ELFINCONT_ZIP}")
    add_custom_command(
            TARGET elfin-controller-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${CMAKE_COMMAND} -E tar cvf installer/${ELFINCONT_ZIP} --format=zip ${ELFINCONT_PRODUCT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E echo "Installer in: installer/${ELFINCONT_ZIP}")
endif ()
