cmake_minimum_required(VERSION 3.10)
project(libwebrtc-webrtc-echo VERSION 1.0)

find_package(jsoncpp CONFIG REQUIRED)

add_executable(libwebrtc-webrtc-echo libwebrtc-webrtc-echo.c)

#target_include_directories(libwebrtc-webrtc-echo PRIVATE 
#    /usr/local/include/gstreamer-1.0)

TARGET_LINK_LIBRARIES(libwebrtc-webrtc-echo 
    jsoncpp_lib 
    jsoncpp_object
    event
    webrtc)
