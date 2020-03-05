//-----------------------------------------------------------------------------
// Filename: HilbertFilter.cs
//
// Description: Implementation of a Hilbert filter to visualise audio input.
// Originally based on https://github.com/conundrumer/visual-music-workshop.

// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
//
// History:
// 29 Feb 2020	Aaron Clauson	Created, Dublin, Ireland.
//
// License: 
// BSD 3-Clause "New" or "Revised" License, see included LICENSE.md file.
//-----------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using MathNet.Numerics;
using MathNet.Numerics.IntegralTransforms;
using NAudio.Wave;

namespace AudioScope
{
    public struct Vec4
    {
        public float[] Vec; // Needs to always be a 4 element array.

        public Vec4(float x0, float x1, float x2, float x3)
        {
            Vec = new float[] { x0, x1, x2, x3 };
        }
    }

    public class AudioScope
    {
        public const int NUM_CHANNELS = 2;
        public const int SAMPLE_RATE = 8000;
        public const float maxAmplitude = 4.0F;
        public const int B = (1 << 16) - 1;
        public const int M = 4;
        public const int N = 240;
        public const int FFT_SIZE = 1024;
        public const int MID = (FFT_SIZE - 1) / 2;
        public const float DELAY_TIME = MID / SAMPLE_RATE;
        public const float GAIN = 1.0f;

        private const int AUDIO_SAMPLE_PERIOD_MILLISECONDS = 30;

        private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(8000, 1);

        private WaveInEvent _waveInEvent;

        private Complex32[] _analytic;
        private Complex32[] _timeRingBuffer = new Complex32[2 * FFT_SIZE];
        private int _timeIndex = 0;

        private static Vec4[] _data = new Vec4[N];

        public AudioScope()
        {
            _analytic = MakeAnalytic(N, FFT_SIZE);
        }

        public Vec4[] GetSample()
        {
            return _data;
        }

        public void InitAudio()
        {
            _waveInEvent = new WaveInEvent();
            _waveInEvent.BufferMilliseconds = AUDIO_SAMPLE_PERIOD_MILLISECONDS;
            _waveInEvent.NumberOfBuffers = 1;
            _waveInEvent.DeviceNumber = 0;
            _waveInEvent.WaveFormat = _waveFormat;
            _waveInEvent.DataAvailable += LocalAudioSampleAvailable;
        }

        public void Start()
        {
            _waveInEvent.StartRecording();
        }

        public void Stop()
        {
            _waveInEvent.StopRecording();
        }

        /// <summary>
        /// Event handler for audio sample being supplied by local capture device.
        /// </summary>
        private void LocalAudioSampleAvailable(object sender, WaveInEventArgs args)
        {
            byte[] buffer = args.Buffer;
            int bytesRecorded = args.BytesRecorded;
            int bytesPerSample = _waveFormat.BlockAlign;

            int sampleCount = bytesRecorded / bytesPerSample;

            int indexRingBuffer = 0;
            for (int i = 0; i < bytesRecorded; i += bytesPerSample)
            {
                float re = BitConverter.ToSingle(buffer, i);
                Complex32 mono = new Complex32(GAIN * re, 0.0f);
                _timeRingBuffer[_timeIndex + indexRingBuffer] = mono;       // Left.
                _timeRingBuffer[_timeIndex + MID + indexRingBuffer] = mono; // right
                indexRingBuffer++;
            }

            _timeIndex = (_timeIndex + sampleCount) % FFT_SIZE;

            Console.WriteLine($"timeIndex {_timeIndex}.");

            var freqBuffer = _timeRingBuffer.Skip(_timeIndex).Take(_timeIndex + FFT_SIZE).ToArray();

            Fourier.Forward(freqBuffer);

            for (int j=0; j<N; j++)
            {
                _data[j] = new Vec4(freqBuffer[j].Real, freqBuffer[j].Imaginary, 0.0f, 0.0f);
            }

            Console.WriteLine("Sample ready.");
        }

        private Complex32[] MakeAnalytic(uint n, uint m)
        {
            var impulse = new Complex32[m];
            var freqs = new Complex32[m];

            var mid = (n - 1) / 2;

            impulse[mid] = new Complex32(1.0f, 0.0f);
            float re = -1.0f / (mid - 1);
            for (int i = 0; i < mid + 1; i++)
            {
                if (i % 2 == 0)
                {
                    impulse[mid + i] = new Complex32(re, impulse[mid + i].Imaginary);
                    impulse[mid - i] = new Complex32(re, impulse[mid - i].Imaginary);
                }
                else
                {
                    float im = (float)(2.0 / Math.PI / i);
                    impulse[mid + i] = new Complex32(impulse[mid + i].Real, im);
                    impulse[mid - i] = new Complex32(impulse[mid - i].Real, -im);
                }
                // hamming window
                var k = 0.53836 + 0.46164 * Math.Cos(i * Math.PI / (mid + 1));
                impulse[mid + i] = new Complex32((float)(impulse[mid + i].Real * k), (float)(impulse[mid + i].Imaginary * k));
                impulse[mid - i] = new Complex32((float)(impulse[mid - i].Real * k), (float)(impulse[mid - i].Imaginary * k));
            }

            Fourier.Forward(impulse);

            return freqs;
        }
    }
}
