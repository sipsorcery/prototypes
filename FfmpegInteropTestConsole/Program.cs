using System;
using FFmpeg.AutoGen;
using FfmpegInterop;

namespace FfmpegInteropTestConsole
{
    unsafe class Program
    {
        private static int FRAME_WIDTH = 640;
        private static int FRAME_HEIGHT = 480;
        private static int FRAMES_PER_SECOND = 30;

        static void Main(string[] args)
        {
            Console.WriteLine("Ffmpeg Interop Test Console");

            FfmpegInit.Initialise();

            VideoEncoder vp8Encoder = new VideoEncoder(FFmpeg.AutoGen.AVCodecID.AV_CODEC_ID_VP8, FRAME_WIDTH, FRAME_HEIGHT, FRAMES_PER_SECOND);

            int linesz = ffmpeg.av_image_get_linesize(AVPixelFormat.AV_PIX_FMT_YUV420P, FRAME_WIDTH, 0);

            AVFrame* frame = ffmpeg.av_frame_alloc();
            frame->format = (int)AVPixelFormat.AV_PIX_FMT_YUV420P;
            frame->width = FRAME_WIDTH;
            frame->height = FRAME_HEIGHT;
            frame->pts = 0;

            ffmpeg.av_frame_get_buffer(frame, 0).ThrowExceptionIfError();
            ffmpeg.av_frame_make_writable(frame).ThrowExceptionIfError();

            for (int y = 0; y < FRAME_HEIGHT; y++)
            {
                for (int x = 0; x < FRAME_WIDTH; x++)
                {
                    frame->data[0][y * frame->linesize[0] + x] = (byte)(x + y + 1 * 3);
                }
            }

            for (int y = 0; y < FRAME_HEIGHT / 2; y++)
            {
                for (int x = 0; x < FRAME_WIDTH / 2; x++)
                {
                    frame->data[1][y * frame->linesize[1] + x] = (byte)(128 + y + 2);
                    frame->data[2][y * frame->linesize[2] + x] = (byte)(64 + y + 5);
                }
            }

            byte[] encoded = vp8Encoder.Encode(frame);

            if(encoded == null)
            {
                Console.WriteLine("Video encode failed.");
            }
            else
            {
                Console.WriteLine($"Video encode succeeded, encoded frame size {encoded.Length}.");
            }

            //var videoFrameConverter = new VideoFrameConverter(
            //    new Size(frameWidth, frameHeight),
            //    AVPixelFormat.AV_PIX_FMT_RGB24,
            //    new Size(frameWidth, frameHeight),
            //    AVPixelFormat.AV_PIX_FMT_YUV420P);

            //Console.WriteLine($"VP8 decoder successfully initialised {vp8Decoder.GetDecoderName()}.");

            ffmpeg.av_frame_free(&frame);
        }
    }
}
