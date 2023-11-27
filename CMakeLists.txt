cmake_minimum_required(VERSION 3.0.0)
project(nth VERSION 0.1.0)

# set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdlib")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdlib")

execute_process(
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMIT_FULL ERROR_QUIET)
execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_COMMIT      ERROR_QUIET)

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_COMMIT}" STREQUAL "")
    set(GIT_CLEAN       "false")
    set(GIT_COMMIT_FULL "N/A")
    set(GIT_COMMIT      "N/A")
    set(GIT_BRANCH      "N/A")
    set(GIT_DATE        "N/A")
    set(GIT_UTC         "N/A")
    set(GIT_TAG         "N/A")
    set(GIT_REV         "N/A")
else ()
    execute_process(
        COMMAND git status --porcelain
        OUTPUT_VARIABLE GIT_CLEAN)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH)
    execute_process(
        COMMAND git log -n 1 --pretty=%cd --pretty=%cI
        OUTPUT_VARIABLE GIT_DATE)
    execute_process(
        COMMAND date +%s
        OUTPUT_VARIABLE GIT_UTC)
    # execute_process(
    #     COMMAND git log -n 1 --date=format:%s --pretty=%cd
    #     OUTPUT_VARIABLE GIT_UTC)
    execute_process(
        COMMAND git describe --exact-match --tags
        OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)

    string(STRIP "${GIT_COMMIT_FULL}"   GIT_COMMIT_FULL)
    string(STRIP "${GIT_COMMIT}"        GIT_COMMIT)
    string(STRIP "${GIT_BRANCH}"        GIT_BRANCH)
    string(STRIP "${GIT_DATE}"          GIT_DATE)
    string(STRIP "${GIT_TAG}"           GIT_TAG)
    string(STRIP "${GIT_UTC}"           GIT_UTC)

    if (GIT_CLEAN STREQUAL "")
        set(GIT_CLEAN   "true")
        set(GIT_REV     "${GIT_BRANCH}-${GIT_COMMIT}")
    else()
        set(GIT_CLEAN   "false")
        set(GIT_REV     "${GIT_BRANCH}-${GIT_COMMIT}-${GIT_UTC}")
    endif()
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/nth/misc/git.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/nth/misc/git.h) 

add_library(nth INTERFACE)
target_include_directories(nth INTERFACE inc)
target_compile_features(nth INTERFACE cxx_std_23)
target_compile_options(nth INTERFACE 
    "-Wall" 
    "-Wextra" 
    "-Wpedantic"
    "-Wno-implicit-fallthrough"
    # "-fno-rtti"
    "-fno-exceptions"
    "-fno-asynchronous-unwind-tables"
    "-fdata-sections" 
    "-ffunction-sections"
    "-fvisibility=hidden"
)
# target_link_options(nth INTERFACE "-Wl,--gc-sections")

add_executable(execnth main.cpp 
    # inc/nth/dragonbox.cpp
    # inc/nth/dragonbox_simple.cpp
    # inc/nth/mydragonbox.cpp
    # inc/nth/mydragonbox_simple.cpp
)
target_link_libraries(execnth PRIVATE nth)
target_compile_features(execnth PRIVATE cxx_std_23)
target_compile_options(execnth PRIVATE 
    "-Wall" 
    "-Wextra" 
    "-Wpedantic"
    # "-fno-rtti"
    "-fno-exceptions"
    "-fno-asynchronous-unwind-tables"
    "-fdata-sections" 
    "-ffunction-sections" 
    "-fvisibility=hidden"
)
# target_link_options(execnth PRIVATE "-Wl,--gc-sections")

add_executable(testnth 
    test/cbor/dec.cpp
    test/cbor/enc.cpp
    test/coap/address.cpp
    test/coap/option.cpp
    test/coap/packet_option_delete.cpp
    test/coap/packet_option_general.cpp
    test/coap/packet_option_insert.cpp
    test/coap/packet.cpp
    test/container/list.cpp
    test/container/pool.cpp
    test/container/ring.cpp
    test/container/stack.cpp
    test/container/vector.cpp
    # test/crypto/cipher/aes.cpp
    test/crypto/cipher/chacha20.cpp
    # test/crypto/ecc/crc.cpp
    # test/crypto/hash/hash.cpp
    # test/crypto/kdf/hkdf.cpp
    # test/crypto/mac/hmac.cpp
    # test/crypto/mode/mode.cpp
    # test/crypto/otp/hotp.cpp
    # test/crypto/util.cpp
    test/io/print.cpp
    test/math/bigint.cpp
    test/math/finite.cpp
    test/math/galois.cpp
    test/math/general.cpp
    test/math/interpolation.cpp
    test/math/trigonometry.cpp
    test/misc/cobs.cpp
    test/misc/crc.cpp
    test/misc/git.cpp
    test/misc/qr.cpp
    test/util/bit.cpp
    test/util/bitset.cpp
    test/util/bitslide.cpp
    test/util/expected.cpp
    test/util/half.cpp
    test/util/literal.cpp
    test/util/meta.cpp
    test/util/scope.cpp
    test/util/string.cpp
    test/util/typeid.cpp
)
target_include_directories(testnth PRIVATE test)
target_compile_features(testnth PRIVATE cxx_std_23)
target_link_libraries(testnth PRIVATE gtest_main nth)

enable_testing()
include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)
gtest_discover_tests(testnth)