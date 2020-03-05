using System;

namespace AudioScope
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");

            AudioScope audioScope = new AudioScope();
            audioScope.InitAudio();
            audioScope.Start();

            Console.ReadLine();
        }
    }
}
