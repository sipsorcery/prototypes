using System;
using System.Numerics;
using MathNet.Numerics.IntegralTransforms;

namespace DspTestCore
{
    class Program
    {
        private const int SAMPLE_COUNT = 100;

        static void Main(string[] args)
        {
            Console.WriteLine("FFT Test");

            Complex[] buffer = new Complex[SAMPLE_COUNT];

            for(int i=0; i< SAMPLE_COUNT; i++)
            {
                double re = Math.Sin(2.0f * Math.PI * 3 * ((double)i / SAMPLE_COUNT));
                buffer[i] = new Complex(re, 0);
            }

            Fourier.Forward(buffer, FourierOptions.NoScaling);

            for(int j = 0; j<SAMPLE_COUNT; j++)
            {
                Console.WriteLine($"{j}: {buffer[j]}");
                if(buffer[j].Magnitude > 1.0)
                {
                    Console.WriteLine("Freq hit.");
                }
            }
        }
    }
}
