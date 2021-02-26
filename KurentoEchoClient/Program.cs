using System;
using System.Net.WebSockets;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.VisualStudio.Threading;
using Serilog;
using Serilog.Extensions.Logging;
using SIPSorcery.Net;
using SIPSorceryMedia.Abstractions;
using StreamJsonRpc;

namespace KurentoEchoClient
{
    class Program
    {
        private const string KURENTO_JSONRPC_URL = "ws://192.168.0.43:8888/kurento";
        private const int KEEP_ALIVE_INTERVAL_MS = 240000;

        private static Microsoft.Extensions.Logging.ILogger logger = NullLogger.Instance;

        static async Task Main(string[] args)
        {
            Console.WriteLine("Kurento Echo Test Client");
            logger = AddConsoleLogger();

            CancellationTokenSource cts = new CancellationTokenSource();

            var ws = new ClientWebSocket();
            await ws.ConnectAsync(new Uri(KURENTO_JSONRPC_URL), cts.Token);

            logger.LogDebug($"Successfully connected web socket client to {KURENTO_JSONRPC_URL}.");

            try
            {
                using (var jsonRpc = new JsonRpc(new WebSocketMessageHandler(ws)))
                {
                    jsonRpc.AddLocalRpcMethod("ping", new Action(() =>
                       {
                           logger.LogDebug($"Ping received");
                       }
                    ));

                    jsonRpc.AddLocalRpcMethod("onEvent", new Action<KurentoEvent>((evt) =>
                    {
                        logger.LogDebug($"Event received type={evt.type}, source={evt.data.source}");
                    }
                    ));

                    jsonRpc.StartListening();

                    // Check the server is there.
                    var pingResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.ping.ToString(),
                        new { interval = KEEP_ALIVE_INTERVAL_MS },
                        cts.Token);
                    logger.LogDebug($"Ping result={pingResult.value}.");

                    // Create a media pipeline.
                    var createPipelineResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.create.ToString(),
                        new { type = "MediaPipeline" },
                        cts.Token);
                    logger.LogDebug($"Create media pipeline result={createPipelineResult.value}, sessionID={createPipelineResult.sessionId}.");

                    var sessionID = createPipelineResult.sessionId;
                    var mediaPipeline = createPipelineResult.value;

                    // Create a WebRTC end point.
                    var createEndPointResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.create.ToString(),
                        new
                        {
                            type = "WebRtcEndpoint",
                            constructorParams = new { mediaPipeline = mediaPipeline },
                            sessionId = sessionID
                        },
                        cts.Token);
                    logger.LogDebug($"Create WebRTC endpoint result={createEndPointResult.value}.");

                    var webRTCEndPointID = createEndPointResult.value;

                    // Connect the WebRTC end point to itself to create a loopback connection (no result for this operation).
                    await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.invoke.ToString(),
                        new
                        {
                            @object = webRTCEndPointID,
                            operation = "connect",
                            operationParams = new { sink = webRTCEndPointID },
                            sessionId = sessionID
                        },
                        cts.Token);

                    // Subscribe for events from the WebRTC end point.
                    var subscribeResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.subscribe.ToString(),
                        new
                        {
                            @object = webRTCEndPointID,
                            type = "IceCandidateFound",
                            sessionId = sessionID
                        },
                        cts.Token);
                    logger.LogDebug($"Subscribe to WebRTC endpoint subscription ID={subscribeResult.value}.");

                    var subscriptionID = subscribeResult.value;

                    subscribeResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.subscribe.ToString(),
                        new
                        {
                            @object = webRTCEndPointID,
                            type = "OnIceCandidate",
                            sessionId = sessionID
                        },
                        cts.Token);
                    logger.LogDebug($"Subscribe to WebRTC endpoint subscription ID={subscribeResult.value}.");

                    var pc = CreatePeerConnection();
                    var offer = pc.createOffer(null);
                    await pc.setLocalDescription(offer);

                    // Send SDP offer.
                    var processOfferResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.invoke.ToString(),
                        new
                        {
                            @object = webRTCEndPointID,
                            operation = "processOffer",
                            operationParams = new { offer = offer.sdp },
                            sessionId = sessionID
                        },
                        cts.Token);

                    logger.LogDebug($"SDP answer={processOfferResult.value}.");

                    var setAnswerResult = pc.setRemoteDescription(new RTCSessionDescriptionInit
                    {
                        type = RTCSdpType.answer,
                        sdp = processOfferResult.value
                    });

                    logger.LogDebug($"Set WebRTC peer connection answer result={setAnswerResult}.");

                    // Tell Kurento to start ICE.
                    var gatherCandidatesResult = await jsonRpc.InvokeWithParameterObjectAsync<KurentoResult>(
                        KurentoMethodsEnum.invoke.ToString(),
                        new
                        {
                            @object = webRTCEndPointID,
                            operation = "gatherCandidates",
                            sessionId = sessionID
                        },
                        cts.Token);

                    logger.LogDebug($"Gather candidates result={gatherCandidatesResult.value}.");

                    Console.ReadLine();
                }
            }
            catch (RemoteInvocationException invokeExcp)
            {
                logger.LogError($"JSON RPC invoke exception, error code={invokeExcp.ErrorCode}, msg={invokeExcp.Message}.");
            }
        }

        private static RTCPeerConnection CreatePeerConnection()
        {
            var pc = new RTCPeerConnection(new RTCConfiguration { X_UseRtpFeedbackProfile = true });

            MediaStreamTrack audioTrack = new MediaStreamTrack(SDPWellKnownMediaFormatsEnum.PCMU);
            pc.addTrack(audioTrack);

            pc.onicecandidateerror += (candidate, error) => logger.LogWarning($"Error adding remote ICE candidate. {error} {candidate}");
            pc.OnTimeout += (mediaType) => logger.LogWarning($"Timeout for {mediaType}.");
            pc.oniceconnectionstatechange += (state) => logger.LogInformation($"ICE connection state changed to {state}.");
            pc.onsignalingstatechange += () => logger.LogInformation($"Signaling state changed to {pc.signalingState}.");
            pc.OnReceiveReport += (re, media, rr) => logger.LogDebug($"RTCP Receive for {media} from {re}\n{rr.GetDebugSummary()}");
            pc.OnSendReport += (media, sr) => logger.LogDebug($"RTCP Send for {media}\n{sr.GetDebugSummary()}");
            pc.OnRtcpBye += (reason) => logger.LogDebug($"RTCP BYE receive, reason: {(string.IsNullOrWhiteSpace(reason) ? "<none>" : reason)}.");

            pc.onsignalingstatechange += () =>
            {
                if (pc.signalingState == RTCSignalingState.have_remote_offer)
                {
                    logger.LogTrace("Remote SDP:");
                    logger.LogTrace(pc.remoteDescription.sdp.ToString());
                }
                else if (pc.signalingState == RTCSignalingState.have_local_offer)
                {
                    logger.LogTrace("Local SDP:");
                    logger.LogTrace(pc.localDescription.sdp.ToString());
                }
            };

            return pc;
        }

        /// <summary>
        ///  Adds a console logger. Can be omitted if internal SIPSorcery
        ///  debug and warning messages are not required.
        /// </summary>
        private static Microsoft.Extensions.Logging.ILogger AddConsoleLogger()
        {
            var seriLogger = new LoggerConfiguration()
                .Enrich.FromLogContext()
                .MinimumLevel.Is(Serilog.Events.LogEventLevel.Debug)
                .WriteTo.Console()
                .CreateLogger();
            var factory = new SerilogLoggerFactory(seriLogger);
            SIPSorcery.LogFactory.Set(factory);
            return factory.CreateLogger<Program>();
        }
    }
}
