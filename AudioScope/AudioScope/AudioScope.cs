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
using MathNet.Numerics;
using MathNet.Numerics.IntegralTransforms;
using NAudio.Utils;
using NAudio.Wave;
using PortAudioSharp;
using FFTW.NET;

namespace AudioScope
{
    public struct Vec4
    {
        public float[] Vec
        {
            get { return new float[] { X0, X1, X2, X3 }; }
        }

        public float X0;
        public float X1;
        public float X2;
        public float X3;
    }

    public class AudioScope
    {
        public const int NUM_CHANNELS = 2;
        public const int SAMPLE_RATE = 8000;
        public const float maxAmplitude = 4.0F;
        public const int B = (1 << 16) - 1;
        public const int M = 4;
        public const int FFT_SIZE = 256;
        public const int MID = (FFT_SIZE - 1) / 2;
        public const float DELAY_TIME = MID / SAMPLE_RATE;
        public const float GAIN = 1.0f;
        public const int BUFFER_SIZE = 256;
        public const int CIRCULAR_BUFFER_SAMPLES = 3;
        public const float CUTOFF_FREQ = 0.5f;

        private const int AUDIO_SAMPLE_PERIOD_MILLISECONDS = 30;

        private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(8000, 1);
        //private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(44100, 1);

        private WaveInEvent _waveInEvent;
        private CircularBuffer _audioInBuffer;
        private PortAudioSharp.Stream _audStm;

        private Complex[] _analytic;
        private MathNet.Filtering.OnlineFilter _lowpass;
        private MathNet.Filtering.OnlineFilter _noiseLowpass;

        private Complex[] _timeRingBuffer = new Complex[2 * FFT_SIZE];
        private Vec4[] _analyticBuffer = new Vec4[BUFFER_SIZE];
        private int _timeIndex = 0;
        private Complex32 _prevInput = new Complex32(0.0f, 0.0f);
        private Complex32 _prevDiff = new Complex32(0.0f, 0.0f);

        private static float[] _data = new float[BUFFER_SIZE * 4];

        public AudioScope()
        {
            uint n = FFT_SIZE;
            if(n % 2 == 0)
            {
                n -= 1;
            }
            //_analytic = Enumerable.Repeat(new Complex32(1.0f, 1.0f), FFT_SIZE).ToArray();
            _analytic = MakeAnalytic(n, FFT_SIZE);
            //_lowpass = MathNet.Filtering.OnlineFilter.CreateLowpass(MathNet.Filtering.ImpulseResponse.Infinite, 0.01, 0.5);
            //_noiseLowpass = MathNet.Filtering.OnlineFilter.CreateLowpass(MathNet.Filtering.ImpulseResponse.Infinite, 0.5, 0.7);
        }

        public float[] GetSample()
        {
            lock (_data)
            {
                return _data;
            }
        }

        public void InitAudio()
        {
            _audioInBuffer = new CircularBuffer(BUFFER_SIZE * _waveFormat.BlockAlign * CIRCULAR_BUFFER_SAMPLES);

            _waveInEvent = new WaveInEvent();
            _waveInEvent.BufferMilliseconds = AUDIO_SAMPLE_PERIOD_MILLISECONDS;
            _waveInEvent.NumberOfBuffers = 3;
            _waveInEvent.DeviceNumber = 0;
            _waveInEvent.WaveFormat = _waveFormat;
            _waveInEvent.DataAvailable += AudioDataAvailable;

            PortAudio.Initialize();
            StreamParameters stmInParams = new StreamParameters { device = 0, channelCount = 1, sampleFormat = SampleFormat.Float32 };
            //StreamParameters stmInParams = new StreamParameters { };
            _audStm = new Stream(stmInParams, null, 44100, 256, StreamFlags.NoFlag, AudioInCallback, null);
        }

        public void Start()
        {
            //_waveInEvent.StartRecording();
            _audStm.Start();
        }

        public void Stop()
        {
            // _waveInEvent.StopRecording();
            _audStm.Stop();
        }

        private StreamCallbackResult AudioInCallback(IntPtr input, IntPtr output, uint frameCount, ref StreamCallbackTimeInfo timeInfo, StreamCallbackFlags statusFlags, IntPtr userDataPtr)
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
        private void AudioDataAvailable(object sender, WaveInEventArgs args)
        {
            _audioInBuffer.Write(args.Buffer, 0, args.BytesRecorded);
            while(_audioInBuffer.Count > (BUFFER_SIZE * 4))
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

        /// <summary>
        /// Called to process the audio input once the required number of samples are available.
        /// </summary>
        private void ProcessAudioInBuffer(float[] samples)
        {
            for (int i = 0; i < samples.Length; i++)
            {
                Complex mono = new Complex(GAIN * samples[i], 0.0f);
                _timeRingBuffer[_timeIndex + i] = mono;       // Left.
                _timeRingBuffer[_timeIndex + FFT_SIZE + i] = mono; // right

                //Console.WriteLine(GAIN * re);
            }

            _timeIndex = (_timeIndex + BUFFER_SIZE) % FFT_SIZE;

            //Console.WriteLine($"timeIndex {_timeIndex}.");

            var freqBuffer = _timeRingBuffer.Skip(_timeIndex).Take(FFT_SIZE).ToArray();

            //MathNet.Numerics.IntegralTransforms..Forward(freqBuffer, FourierOptions.NoScaling | FourierOptions.NumericalRecipes);
            //Fourier.Forward(freqBuffer, FourierOptions.NoScaling);
            //Fourier.Forward(freqBuffer);

            //var signal = Accord.Audio.Signal.FromArray(samples, SAMPLE_RATE, Accord.Audio.SampleFormat.Format32BitIeeeFloat).ToComplex();
            //signal.ForwardFourierTransform();
            //var freqBuffer = signal.ToArray(0);

            var freqBufferOut = new Complex[FFT_SIZE];
            using (var pinIn = new PinnedArray<Complex>(freqBuffer))
            using (var pinOut = new PinnedArray<Complex>(freqBufferOut))
            {
                DFT.FFT(pinIn, pinOut);
            }

            for (int j = 0; j < _analytic.Length; j++)
            {
                //Console.WriteLine($"{j}: {freqBuffer[j]} * {_analytic[j]}");
                freqBufferOut[j] = freqBufferOut[j] * _analytic[j];
                //Console.WriteLine($"result {freqBuffer[j]}");
            }

            var analyseOut = new Complex[FFT_SIZE];
            using (var pinIn = new PinnedArray<Complex>(freqBufferOut))
            using (var pinOut = new PinnedArray<Complex>(analyseOut))
            {
                DFT.FFT(pinIn, pinOut);
            }


            //Fourier.Inverse(freqBuffer, FourierOptions.NoScaling);
            //Fourier.Inverse(freqBuffer);

            //_analyticBuffer[0] = _analyticBuffer[BUFFER_SIZE];
            //_analyticBuffer[1] = _analyticBuffer[BUFFER_SIZE + 1];
            //_analyticBuffer[2] = _analyticBuffer[BUFFER_SIZE + 2];
            float scale = 100.0f;  //(float)FFT_SIZE;

            var complexAnalyticBuffer = analyseOut.Skip(FFT_SIZE - BUFFER_SIZE).ToArray();

            //var complexAnalyticBuffer = signal.ToArray(0);
            lock (_data)
            {
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

                    //Console.WriteLine($"{k}: {_data[k].X0},{_data[k].X1},{_data[k].X2},{_data[k].X3}");
                }
            }

            Console.WriteLine("Sample ready.");
        }

        // Angle between two complex numbers scaled into [0, 0.5].
        public static float GetAngle(Complex32 u, Complex32 v)
        {
            var lenProduct = u.Magnitude * v.Magnitude;
            var theta = (u.Real * v.Real - u.Imaginary * v.Imaginary) / lenProduct;
            var angle = Math.Acos(theta);
            return (float)(angle / (2*Math.PI));
        }

        private Complex[] MakeAnalytic(uint n, uint m)
        {
            Console.WriteLine($"MakeAnalytic n={n}, m={m}.");

            var impulse = new Complex[m];

            var mid = (n - 1) / 2;

            impulse[mid] = new Complex(1.0f, 0.0f);
            float re = -1.0f / (mid - 1);
            for (int i = 0; i < mid + 1; i++)
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

            var output = new Complex[m];

            //Fourier.Forward(impulse, FourierOptions.NoScaling);
            using (var pinIn = new PinnedArray<Complex>(impulse))
            using (var pinOut = new PinnedArray<Complex>(output))
            {
                DFT.FFT(pinIn, pinOut);
            }

            for (int i = 0; i < m; i++)
            {
               Console.WriteLine($"{i}:{output[i]}");
            }

            return output;
        }
    }
}
