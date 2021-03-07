FROM ubuntu:16.04

# Install packages for libwebrtc build and app dependencies.
RUN apt update && DEBIAN_FRONTEND="noninteractive" apt install -y curl git \
  lsb-release python sudo wget
  
# Install depot_tools
WORKDIR /src 
# https://webrtc.googlesource.com/src/+/refs/heads/master/docs/native-code/development/prerequisite-sw/index.md
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git --depth 1 depot_tools
ENV PATH="/src/depot_tools:${PATH}"
WORKDIR /src/chromium
RUN fetch --nohooks --no-history chromium
RUN ./src/build/install-build-deps.sh
#RUN curl https://chromium.googlesource.com/chromium/+/refs/heads/trunk/build/install-build-deps.sh?format=TEXT | base64 --decode > install-build-deps.sh
#RUN chmod +x install-build-deps.sh && ./install-build-deps.sh
#RUN gclient runhooks
#RUN gn gen out/Default
# https://webrtc.googlesource.com/src/+/refs/heads/master/docs/native-code/development/index.md#building
WORKDIR /src/webrtc-checkout
RUN fetch --nohooks webrtc
RUN gclient sync
WORKDIR /src/webrtc-checkout/src
RUN gn gen out/Default
RUN ninja -C out/Default
