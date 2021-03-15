## Building the libwebrtc builder docker image

This image gets as far as producing `libwebrtc-full.a` so that it can be used for application builds.

`docker build -t libwebrtc-builder:0.1 -f Dockerfile-Builder --progress=plain .`

## Building the application builder docker image

This image builds an application that links to `libwebrtc-full.a`. The image still requires a lot of heavy dependencies which make it unsuitable for publishing.

`docker build -t libwebrtc-appbuilder:0.1 -f Dockerfile-AppBuilder --progress=plain .`

## Building docker image

The final application image. It copies the binary from the app build er image and installs the required shared library packages.

`docker build -t libwebrtc-webrtc-echo:0.1 --progress=plain .`

## Running docker image

`docker run -it --init --rm -p 8080:8080 libwebrtc-webrtc-echo:0.1`

## Building webrtc.lib on Windows

Follow the standard instructions at https://webrtc.github.io/webrtc-org/native-code/development/ and then use hte steps below. Pay particular attention to the `--args` argument supplied tot eh `gn` command.

````
src> git checkout branch-heads/4430 -b m90 # See https://chromiumdash.appspot.com/branches for remote branch info.
src> set PATH=C:\Tools\depot_tools;%PATH%
src> glient sync
src> rmdir /q /s out\Default
src> gn gen out/Default --args="is_clang=false use_lld=false" --ide=vs # See https://gn.googlesource.com/gn/+/master/docs/reference.md#IDE-options for more info.
src> gn args out/Default --list # Check use_lld is false. is_clang always shows true but if the false option is not set then linker errors when using webrtc.lib.
src> gn args out/Default --list --short --overrides-only
src> gn clean out/Default # If previous compilation.
src> ninja -C out/Default
````

## Building libwebrtc-full.a on Ubuntu

Follow the standard instructions at https://webrtc.github.io/webrtc-org/native-code/development/ and then use hte steps below. Pay particular attention to the `--args` argument supplied tot eh `gn` command.

````
src$ gclient sync
src$ build/install-build-deps.sh
src$ gn gen out/Default --args="is_component_build=false use_custom_libcxx=false use_custom_libcxx_for_host=false rtc_enable_protobuf=false"
src$ gn args out/Default --list --short --overrides-only
src$ ninja -C out/Default
src$ out/Default/webrtc_lib_link_test
src$ cd out/Default
src$ ar crs out/Default/libwebrtc-full.a $(find out/Default/obj -name '*\.o')
sudo apt install software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install libstdc++-9-dev libevent-dev
````
