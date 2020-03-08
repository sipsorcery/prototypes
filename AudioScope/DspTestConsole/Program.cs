using System;
using System.Collections.Generic;
using System.Numerics;
using MathNet.Numerics.IntegralTransforms;

namespace DspTestCore
{
    class Program
    {
        private const int FFT_SIZE = 4;
        private const int TIME_SERIES_SAMPLES = 4;

        static void Main(string[] args)
        {
            Console.WriteLine("FFT Test");

            uint n = FFT_SIZE;
            if (n % 2 == 0)
            {
                n -= 1;
            }

           Console.WriteLine($"n={n}.");

            var analytic = MakeAnalytic(n, FFT_SIZE);

            //Console.WriteLine("Analytic Series:");
            //for (int j = 0; j < FFT_SIZE; j++)
            //{
            //    Console.WriteLine($"{j}: {analytic[j]} {analytic[j].Magnitude}");
            //}

            List<Complex[]> timeSeries = new List<Complex[]>();

            for (int c = 0; c < TIME_SERIES_SAMPLES; c++)
            {
                Complex[] buffer = new Complex[FFT_SIZE];

                for (int i = 0; i < FFT_SIZE; i++)
                {
                    double re = Math.Sin(2.0f * Math.PI * 3 * ((double)i / FFT_SIZE));
                    buffer[i] = new Complex(re, 0);
                }

                timeSeries.Add(buffer);
            }

            ProcessTimeSeries(timeSeries, analytic);
        }

        private static void ProcessTimeSeries(List<Complex[]> sampleBuffers, Complex[] analytic)
        {
            foreach (var buffer in sampleBuffers)
            {
                Fourier.Forward(buffer, FourierOptions.NoScaling);

                //Console.WriteLine("Frequency Series:");
                //for (int j = 0; j < FFT_SIZE; j++)
                //{
                //    Console.WriteLine($"{j}: {buffer[j]} {buffer[j].Magnitude}");
                //}

                for (int j = 0; j < buffer.Length; j++)
                {
                    buffer[j] = buffer[j] * analytic[j];
                }

                //Console.WriteLine("Modified Frequency Series:");
                //for (int j = 0; j < FFT_SIZE; j++)
                //{
                //    Console.WriteLine($"{j}: {buffer[j]} {buffer[j].Magnitude}");
                //}

                Fourier.Inverse(buffer, FourierOptions.NoScaling);

                //Console.WriteLine("Recovered Time Series:");
                //for (int j = 0; j < FFT_SIZE; j++)
                //{
                //    Console.WriteLine($"{j}: {buffer[j]} {buffer[j].Magnitude}");
                //}

                foreach(var val in buffer)
                {
                    float xSample = (float)(val.Real / FFT_SIZE);
                    float ySample = (float)(val.Imaginary / FFT_SIZE);

                    Console.Write($"{xSample} {ySample},");
                }
                Console.WriteLine();
            }
        }

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
