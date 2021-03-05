## Building docker image for static gstreamer build

`docker build -t gst-builder:0.1 -f Dockerfile-Gstbuilder --progress=plain .`

## Building docker image

`docker build -t gst-webrtc-echo:0.x --progress=plain .`

## Running docker image

`docker run -it --init --rm -p 8080:8080 gst-webrtc-echo:latest`

Set a gstreamer environment variable for additional logging:
`docker run -it --init --rm -p 8080:8080 -e "GST_DEBUG=4,dtls*:7" gst-webrtc-echo:latest`

Override the gst-echo-app and start a bash shell plus add a local volume mapping:
docker run -it -p 8080:8080 -v %cd%:/pcdodo --entrypoint /bin/bash gst-webrtc-echo:latest