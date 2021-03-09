cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(libwebrtc-webrtc-echo VERSION 1.0)

find_package(libevent CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_definitions(-DNOMINMAX -DWEBRTC_WIN -D_ITERATOR_DEBUG_LEVEL=0 -D_DEBUG -D_CONSOLE)

add_executable(libwebrtc-webrtc-echo libwebrtc-webrtc-echo.cpp)
target_sources(libwebrtc-webrtc-echo PRIVATE HttpSimpleServer.cpp PcFactory.cpp PcObserver.cpp)

set_property(TARGET libwebrtc-webrtc-echo PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(libwebrtc-webrtc-echo PRIVATE 
    c:/dev/webrtc-checkout/src
    c:/dev/webrtc-checkout/src/third_party/abseil-cpp)

target_link_directories(libwebrtc-webrtc-echo PRIVATE c:/dev/webrtc-checkout/src/out/Default/obj)

target_link_libraries(libwebrtc-webrtc-echo 
    nlohmann_json
    libevent::core
    libevent::extra
    webrtc
    ws2_32
    wmcodecdspuuid
    Msdmo
    Dmoguids
    secur32
    winmm)
