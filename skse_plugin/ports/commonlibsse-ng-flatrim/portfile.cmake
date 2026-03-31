vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO CharmedBaryon/CommonLibSSE
    REF 7d33bd7546d3aac9ebcde232cdc6b217f89cd90c
    SHA512 227b85395d9421aee248bacd3360acdebaec485cfd6b44459557abbf595e78c679037e54cf3a0243556b9677078bdbe0819574f78a5616928c3f744f1fff3421
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DENABLE_SKYRIM_VR=off
        "-DFAST_CPP_CSV_PARSER_INCLUDE_DIRS=${CURRENT_INSTALLED_DIR}/include/fast-cpp-csv-parser"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)
vcpkg_copy_pdbs()

file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")

file(
    INSTALL "${SOURCE_PATH}/LICENSE"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright)
