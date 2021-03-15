FROM libwebrtc-appbuilder:0.1 as appbuilder
FROM ubuntu:latest as final

# Install packages with the required runtime libraries.
RUN apt update && DEBIAN_FRONTEND="noninteractive" apt install -y \
   libevent-dev libx11-dev libglib2.0-dev libatomic1 --no-install-recommends

COPY --from=appbuilder /libwebrtc-webrtc-echo /libwebrtc-webrtc-echo

EXPOSE 8080
ENTRYPOINT ["/libwebrtc-webrtc-echo"]