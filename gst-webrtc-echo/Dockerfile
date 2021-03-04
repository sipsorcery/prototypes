FROM ubuntu:latest

# Install packages for gstreamer build and app dependencies.
RUN apt update &&  DEBIAN_FRONTEND="noninteractive" apt install -y \
  bison build-essential cmake flex git libevent-dev libglib2.0-dev libnice-dev \
  libsrtp2-dev libssl-dev libvpx-dev ninja-build python-is-python3 python3-pip \
  && rm -rf /var/lib/apt/lists/*
  
# Build gstreamer
RUN python -m pip install meson
WORKDIR /src 
RUN git clone https://gitlab.freedesktop.org/gstreamer/gst-build.git gst-build
WORKDIR /src/gst-build
RUN meson -Dgood=enabled -Dgst-plugins-good:vpx=enabled \
  -Dbad=enabled -Dgst-plugins-bad:dtls=enabled \
  -Dbad=enabled -Dgst-plugins-bad:srtp=enabled \
  -Dbad=enabled -Dgst-plugins-bad:webrtc=enabled \
  builddir
RUN ninja -C builddir
RUN meson install -C builddir

# Install source app dependency.
WORKDIR /src
RUN git clone https://github.com/DaveGamble/cJSON.git cJSON
WORKDIR /src/cJSON/builddir
RUN cmake .. && make && make install

# Copy app source and build
WORKDIR /src/gst-webrtc-echo
COPY ["CMakeLists.txt", ""]
COPY ["gst-webrtc-echo.c", ""]
WORKDIR /src/gst-webrtc-echo/builddir
RUN cmake .. && make && ldconfig
RUN cp gst-webrtc-echo /
RUN rm -rf /src

EXPOSE 8080
ENTRYPOINT ["/gst-webrtc-echo"]