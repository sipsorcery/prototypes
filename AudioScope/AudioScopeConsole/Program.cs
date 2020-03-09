using System;
using MathNet.Numerics;

namespace AudioScope
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Audio Scope");

            AudioScope audioScope = new AudioScope();
            audioScope.InitAudio(AudioSourceEnum.Simulation);
            audioScope.Start();

            Console.ReadLine();
        }
    }
}
