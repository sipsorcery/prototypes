//-----------------------------------------------------------------------------
// Filename: Program.cs
//
// Description: Creates a WebRTC peer on top of the Microsoft
// https://github.com/microsoft/MixedReality-WebRTC C# library (which is wrapping
// Google's libwebrtc et al).
//
// At the time of writing this test program worked with the WebRTCReceiver sample
// from https://github.com/sipsorcery/sipsorcery/tree/master/examples.
//
// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
// 
// History:
// 05 Feb 2020	Aaron Clauson	Created, Dublin, Ireland.
//
// License: 
// BSD 3-Clause "New" or "Revised" License, see included LICENSE.md file.
//-----------------------------------------------------------------------------

using System;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.MixedReality.WebRTC;

namespace TestNetCoreConsole
{
    class Program
    {
        private const string WEBSOCKET_SERVER_URI = "wss://localhost:8081";

        static async Task Main()
        {
            try
            {
                Console.WriteLine("Starting...");

                ClientWebSocket ws = new ClientWebSocket();
                CancellationTokenSource cts = new CancellationTokenSource();

                // Set up the peer connection.
                var pc = new PeerConnection();

                pc.LocalSdpReadytoSend += async (string type, string sdp) =>
                {
                    Console.WriteLine($"Local SDP ready {type}");
                    //Console.WriteLine(sdp);

                    // Send out SDP offer to the remote peer.
                    await ws.SendAsync(Encoding.UTF8.GetBytes(sdp), WebSocketMessageType.Text, true, cts.Token);
                };

                var config = new PeerConnectionConfiguration();
                await pc.InitializeAsync(config);

                Console.WriteLine("Peer connection initialized.");

                await pc.AddLocalAudioTrackAsync();
                await pc.AddLocalVideoTrackAsync(new PeerConnection.LocalVideoTrackSettings());
                
                await ws.ConnectAsync(new Uri(WEBSOCKET_SERVER_URI), cts.Token);

                pc.CreateOffer();

                // Wait for the SDP answer to arrive from the remote peer.
                byte[] answerSdpBuffer = new byte[8192];
                var recvRes = await ws.ReceiveAsync(answerSdpBuffer, cts.Token);
                string answerSdp = Encoding.UTF8.GetString(answerSdpBuffer, 0, recvRes.Count);

                //Console.WriteLine($"answer sdp: {answerSdp}");

                pc.SetRemoteDescription("answer", answerSdp);

                // Don't need the web socket anymore.
                await ws.CloseAsync(WebSocketCloseStatus.NormalClosure, null, cts.Token);

                Console.WriteLine("Press any key to exit...");
                Console.ReadLine();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }
    }
}
