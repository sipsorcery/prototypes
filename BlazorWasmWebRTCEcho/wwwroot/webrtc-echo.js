"use strict";

function RestSignaler(signalUrl) {

    this.sendOffer = async function (offer) {
        return (await fetch(signalUrl, {
            method: 'POST',
            body: JSON.stringify(offer),
            headers: { 'Content-Type': 'application/json' }
        })).json();
    }
}

const STUN_SERVER = "stun:stun.sipsorcery.com";
const NOISE_FRAME_WIDTH = 80;
const NOISE_FRAME_HEIGHT = 60;

var pc;
var isClosed = true;

async function start() {

    if (!isClosed) {
        // Close any existing peer connection.
        await closePeer();
    }

    let signalingUrl = document.getElementById('signalingUrl').value;

    // Create the noise.
    let noiseStm = whiteNoise(NOISE_FRAME_WIDTH, NOISE_FRAME_HEIGHT);
    document.getElementById("localVideoCtl").srcObject = noiseStm;

    let signaler = new RestSignaler(signalingUrl);

    // Create the peer connections.
    pc = createPeer("echoVideoCtl", noiseStm);

    let offer = await pc.createOffer();
    await pc.setLocalDescription(offer);

    var answer = await signaler.sendOffer(offer);

    if (answer != null) {
        console.log(answer.sdp)
        await pc.setRemoteDescription(answer);
    }
    else {
        console.log("Failed to get an answer from the Echo Test server.")
        pc.close();
    }
}

function createPeer(videoCtlID, noiseStm) {

    console.log("Creating peer ...");
    isClosed = false;

    let pc = new RTCPeerConnection({ iceServers: [{ urls: STUN_SERVER }] });
    noiseStm.getTracks().forEach(track => pc.addTrack(track, noiseStm));
    pc.ontrack = evt => document.getElementById(videoCtlID).srcObject = evt.streams[0];
    pc.onicecandidate = evt => evt.candidate && console.log(evt.candidate);

    // Diagnostics.
    pc.onicegatheringstatechange = () => console.log(`onicegatheringstatechange: ${pc.iceGatheringState}.`);
    pc.oniceconnectionstatechange = () => console.log(`oniceconnectionstatechange: ${pc.iceConnectionState}.`);
    pc.onsignalingstatechange = () => console.log(`onsignalingstatechange: ${pc.signalingState}.`);
    pc.onconnectionstatechange = () => console.log(`onconnectionstatechange: ${pc.connectionState}.`);

    return pc;
}

async function closePeer() {
    console.log("Closing...")
    isClosed = true;
    await pc?.close();
};

function whiteNoise(width, height) {
    const canvas = Object.assign(document.createElement("canvas"), { width, height });
    const ctx = canvas.getContext('2d');
    ctx.fillRect(0, 0, width, height);
    const p = ctx.getImageData(0, 0, width, height);
    requestAnimationFrame(function draw() {
        if (!isClosed) {
            for (var i = 0; i < p.data.length; i++) {
                p.data[i++] = Math.random() * 255;
                p.data[i++] = Math.random() * 255;
                p.data[i++] = Math.random() * 255;
            }
            ctx.putImageData(p, 0, 0);
            requestAnimationFrame(draw);
        }
    });
    return canvas.captureStream();
}
