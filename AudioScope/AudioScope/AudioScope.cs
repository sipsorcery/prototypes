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
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using MathNet.Numerics;
using MathNet.Numerics.IntegralTransforms;
using NAudio.Utils;
using NAudio.Wave;
using PortAudioSharp;

namespace AudioScope
{
    public enum AudioSourceEnum
    {
        NAudio = 0,
        PortAudio = 1,
        Simulation = 3
    }

    public class AudioScope
    {
        public const int NUM_CHANNELS = 1;
        public const int SAMPLE_RATE = 44100;
        public const float maxAmplitude = 4.0F;
        public const int B = (1 << 16) - 1;
        public const int M = 4;
        public const int FFT_SIZE = 1024;
        public const int MID = (FFT_SIZE - 1) / 2;
        public const float DELAY_TIME = MID / SAMPLE_RATE;
        public const float GAIN = 1.0f;
        public const int BUFFER_SIZE = 256;
        public const int CIRCULAR_BUFFER_SAMPLES = 3;
        public const float CUTOFF_FREQ = 0.5f;

        private const int AUDIO_SAMPLE_PERIOD_MILLISECONDS = 30;

        //private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(8000, 1);
        private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(SAMPLE_RATE, NUM_CHANNELS);

        private WaveInEvent _waveInEvent;
        private CircularBuffer _audioInBuffer;
        private PortAudioSharp.Stream _audStm;
        private Timer _simulationTrigger;
        private int _simulationPeriodMilli;

        private Complex[] _analytic;
        private MathNet.Filtering.OnlineFilter _lowpass;
        private MathNet.Filtering.OnlineFilter _noiseLowpass;

        private Complex[] _timeRingBuffer = new Complex[2 * FFT_SIZE];
        private int _timeIndex = 0;
        private Complex _prevInput = new Complex(0.0f, 0.0f);
        private Complex _prevDiff = new Complex(0.0f, 0.0f);

        private float _simulationFreq = 440.0f;

        private static float[] _data = new float[BUFFER_SIZE * 4];

        public AudioScope()
        {
            //uint n = FFT_SIZE - BUFFER_SIZE;
            uint n = FFT_SIZE;
            if (n % 2 == 0)
            {
                n -= 1;
            }

            _analytic = MakeAnalytic(n, FFT_SIZE);
            _lowpass = MathNet.Filtering.OnlineFilter.CreateLowpass(MathNet.Filtering.ImpulseResponse.Infinite, 0.01, 0.5);
            _noiseLowpass = MathNet.Filtering.OnlineFilter.CreateLowpass(MathNet.Filtering.ImpulseResponse.Infinite, 0.5, 0.7);
        }

        public float[] GetSample()
        {
            return _data;
        }

        public void InitAudio(AudioSourceEnum audioSource)
        {
            if (audioSource == AudioSourceEnum.NAudio)
            {
                _audioInBuffer = new CircularBuffer(BUFFER_SIZE * _waveFormat.BlockAlign * CIRCULAR_BUFFER_SAMPLES);

                _waveInEvent = new WaveInEvent();
                _waveInEvent.BufferMilliseconds = AUDIO_SAMPLE_PERIOD_MILLISECONDS;
                _waveInEvent.NumberOfBuffers = 3;
                _waveInEvent.DeviceNumber = 0;
                _waveInEvent.WaveFormat = _waveFormat;
                _waveInEvent.DataAvailable += NAudioDataAvailable;
            }
            else if (audioSource == AudioSourceEnum.PortAudio)
            {
                PortAudio.Initialize();
                StreamParameters stmInParams = new StreamParameters { device = 0, channelCount = NUM_CHANNELS, sampleFormat = SampleFormat.Float32 };
                _audStm = new Stream(stmInParams, null, SAMPLE_RATE, BUFFER_SIZE, StreamFlags.NoFlag, PortAudioInCallback, null);
            }
            else if (audioSource == AudioSourceEnum.Simulation)
            {
                _simulationPeriodMilli = 1000 * BUFFER_SIZE / SAMPLE_RATE;
                _simulationTrigger = new Timer(GenerateSimulationSample);
            }
        }

        public void Start()
        {
            _waveInEvent?.StartRecording();
            _audStm?.Start();
            _simulationTrigger.Change(0, _simulationPeriodMilli);
        }

        public void Stop()
        {
            _waveInEvent?.StopRecording();
            _audStm?.Stop();
            _simulationTrigger?.Dispose();
        }

        private StreamCallbackResult PortAudioInCallback(IntPtr input, IntPtr output, uint frameCount, ref StreamCallbackTimeInfo timeInfo, StreamCallbackFlags statusFlags, IntPtr userDataPtr)
        {
            Console.WriteLine($"AudioInCallback frame count {frameCount}.");

            float[] samples = new float[frameCount];
            Marshal.Copy(input, samples, 0, (int)frameCount);

            ProcessAudioInBuffer(samples);

            return StreamCallbackResult.Continue;
        }

        /// <summary>
        /// Event handler for audio sample being supplied by local capture device.
        /// </summary>
        private void NAudioDataAvailable(object sender, WaveInEventArgs args)
        {
            _audioInBuffer.Write(args.Buffer, 0, args.BytesRecorded);
            while (_audioInBuffer.Count > (BUFFER_SIZE * 4))
            {
                int bytesPerSample = _waveFormat.BlockAlign;

                byte[] buffer = new byte[BUFFER_SIZE * bytesPerSample];
                _audioInBuffer.Read(buffer, 0, BUFFER_SIZE * bytesPerSample);

                List<float> samples = new List<float>();
                for (int i = 0; i < BUFFER_SIZE * bytesPerSample; i += bytesPerSample)
                {
                    samples.Add(BitConverter.ToSingle(buffer, i));
                }

                ProcessAudioInBuffer(samples.ToArray());
            }
        }

        private void GenerateSimulationSample(Object userState)
        {
            float[] sample = new float[BUFFER_SIZE];

            for (int i = 0; i < sample.Length; i++)
            {
                double val = 2.0f * Math.PI * 3 * ((double)i / (double)_simulationFreq);
                double re = Math.Sin(val);
                sample[i] = (float)re;
            }

            ProcessAudioInBuffer(sample);
        }

        /// <summary>
        /// Called to process the audio input once the required number of samples are available.
        /// </summary>
        private void ProcessAudioInBuffer(float[] samples)
        {
            Console.WriteLine($"ProcessAudioInBuffer {samples[0]} {samples[63]} {samples[127]} {samples[191]} {samples[255]}, length {samples.Length}.");

            for (int i = 0; i < samples.Length; i++)
            {
                Complex mono = new Complex(GAIN * samples[i], 0.0f);
                _timeRingBuffer[_timeIndex + i] = mono;       // Left.
                _timeRingBuffer[_timeIndex + FFT_SIZE + i] = mono; // right
            }

            _timeIndex = (_timeIndex + BUFFER_SIZE) % FFT_SIZE;

            var freqBuffer = _timeRingBuffer.Skip(_timeIndex).Take(FFT_SIZE).ToArray();

            Fourier.Forward(freqBuffer, FourierOptions.NoScaling);

            for (int j = 0; j < freqBuffer.Length; j++)
            {
                freqBuffer[j] = freqBuffer[j] * _analytic[j];
            }

            Fourier.Inverse(freqBuffer, FourierOptions.NoScaling);

            //_analyticBuffer[0] = _analyticBuffer[BUFFER_SIZE];
            //_analyticBuffer[1] = _analyticBuffer[BUFFER_SIZE + 1];
            //_analyticBuffer[2] = _analyticBuffer[BUFFER_SIZE + 2];
            float scale = (float)FFT_SIZE;// / 64;

            var complexAnalyticBuffer = freqBuffer.Skip(FFT_SIZE - BUFFER_SIZE).Take(BUFFER_SIZE).ToArray();

            for (int k = 0; k < complexAnalyticBuffer.Length; k++)
            {
                //var diff = complexAnalyticBuffer[k] - _prevInput;
                //_prevInput = complexAnalyticBuffer[k];

                //var angle = Math.Max(Math.Log(Math.Abs(GetAngle(diff, _prevDiff)), 2), -1.0e12);
                //_prevDiff = diff;
                //var output = _lowpass.ProcessSample(angle);

                _data[k * 4] = (float)(complexAnalyticBuffer[k].Real / scale);
                _data[k * 4 + 1] = (float)(complexAnalyticBuffer[k].Imaginary / scale);
                _data[k * 4 + 2] = 0.75f; //(float)Math.Pow(2, angle), // (float)Math.Pow(2, output), // Smoothed angular velocity.
                _data[k * 4 + 3] = 0; //(float)Math.Abs(angle), //(float)_noiseLowpass.ProcessSample(Math.Abs(angle - output)) // Average angular noise.
            }

            //for (int k = 0; k < _data.Length; k += 4)
            //{
            //    Console.WriteLine($"{k / 4}: {_data[k + 0]} {_data[k + 1]}");
            //}

            //Console.WriteLine("Sample ready.");
        }

        // Angle between two complex numbers scaled into [0, 0.5].
        public static float GetAngle(Complex32 u, Complex32 v)
        {
            var lenProduct = u.Magnitude * v.Magnitude;
            var theta = (u.Real * v.Real - u.Imaginary * v.Imaginary) / lenProduct;
            var angle = Math.Acos(theta);
            return (float)(angle / (2 * Math.PI));
        }

        //    private Complex32[] MakeAnalytic(uint n, uint m)
        //    {
        //        Console.WriteLine($"make_analytic, n={n}, m={m}.");

        //        var impulse = new Complex32[m];

        //        var mid = (n - 1) / 2;

        //        impulse[mid] = new Complex32(1.0f, 0.0f);
        //        float re = -1.0f / (mid - 1);
        //        for (int i = 1; i < mid + 1; i++)
        //        {
        //            if (i % 2 == 0)
        //            {
        //                impulse[mid + i] = new Complex32(re, impulse[mid + i].Imaginary);
        //                impulse[mid - i] = new Complex32(re, impulse[mid - i].Imaginary);
        //            }
        //            else
        //            {
        //                float im = (float)(2.0 / Math.PI / i);
        //                impulse[mid + i] = new Complex32(impulse[mid + i].Real, im);
        //                impulse[mid - i] = new Complex32(impulse[mid - i].Real, -im);
        //            }
        //            // hamming window
        //            var k = 0.53836 + 0.46164 * Math.Cos(i * Math.PI / (mid + 1));
        //            impulse[mid + i] = new Complex32((float)(impulse[mid + i].Real * k), (float)(impulse[mid + i].Imaginary * k));
        //            impulse[mid - i] = new Complex32((float)(impulse[mid - i].Real * k), (float)(impulse[mid - i].Imaginary * k));
        //        }

        //        Fourier.Forward(impulse, FourierOptions.NoScaling);

        //        return impulse;
        //    }

        private static Complex[] MakeAnalytic(uint n, uint m)
        {
            Console.WriteLine($"MakeAnalytic n={n}, m={m}.");

            var impulse = new Complex[m];

            var mid = (n - 1) / 2;

            impulse[mid] = new Complex(1.0f, 0.0f);
            float re = -1.0f / (mid - 1);
            for (int i = 1; i < mid + 1; i++)
            {
                if (i % 2 == 0)
                {
                    impulse[mid + i] = new Complex(re, impulse[mid + i].Imaginary);
                    impulse[mid - i] = new Complex(re, impulse[mid - i].Imaginary);
                }
                else
                {
                    float im = (float)(2.0 / Math.PI / i);
                    impulse[mid + i] = new Complex(impulse[mid + i].Real, im);
                    impulse[mid - i] = new Complex(impulse[mid - i].Real, -im);
                }
                // hamming window
                var k = 0.53836 + 0.46164 * Math.Cos(i * Math.PI / (mid + 1));
                impulse[mid + i] = new Complex((float)(impulse[mid + i].Real * k), (float)(impulse[mid + i].Imaginary * k));
                impulse[mid - i] = new Complex((float)(impulse[mid - i].Real * k), (float)(impulse[mid - i].Imaginary * k));
            }

            //for (int i=0; i<m; i++)
            //{
            //    Console.WriteLine($"{i}:{impulse[i]}");
            //}

            Fourier.Forward(impulse, FourierOptions.NoScaling);

            //for (int i = 0; i < m; i++)
            //{
            //    Console.WriteLine($"{i}:{impulse[i]}");
            //}

            return impulse;
        }
    }
}
