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
using System.Text;

namespace HilbertAudioVisualiser
{
    public class HilbertFilter
    {
        public const int NUM_CHANNELS = 2;
        public const int SAMPLE_RATE = 8000;
        public const float maxAmplitude = 4.0F;
        public const int B = (1 << 16) - 1;
        public const int M = 4;
        public const int N = 512;
        public const int FILTER_LENGTH = 767;
        public const int MID = (FILTER_LENGTH - 1) / 2;
        public const float DELAY_TIME = MID / SAMPLE_RATE; 

        /// <summary>
        /// 
        /// </summary>
        /// <param name="textureData"></param>
        /// <param name="samplesX"></param>
        /// <param name="samplesY"></param>
        /// <param name="N"></param>
        public static void updateTextureData(byte[] textureData, float[] samplesX, float[] samplesY, int n)
        {
            for (int i = 0; i < n; i++)
            {
                var x = (int)Math.Max(0, Math.Min(2 * maxAmplitude, 0.5 + 0.5 * samplesX[i] / maxAmplitude));
                var y = (int)Math.Max(0, Math.Min(2 * maxAmplitude, 0.5 + 0.5 * samplesY[i] / maxAmplitude));

                x = (x * B) | 0;
                y = (y * B) | 0;

                var j = i * M;
                textureData[j + 0] = (byte)(x >> 8);
                textureData[j + 1] = (byte)(x & 0xFF);
                textureData[j + 2] = (byte)(y >> 8);
                textureData[j + 3] = (byte)(y & 0xFF);
            }
        }

        public static void createHilbertFilter()
        {
            //int filterLength = 768;
            // let filterLength = FFT_SIZE - N
            //if (filterLength % 2 == 0)
            //{
            //    filterLength -= 1;
            //}
            float[] impulse = new float[FILTER_LENGTH];

            //int mid = ((filterLength - 1) / 2) | 0;
            //int mid = (FILTER_LENGTH - 1) / 2;

            for (int i = 0; i <= MID; i++)
            {
                // hamming window
                float k = (float)(0.53836 + 0.46164 * Math.Cos(i * Math.PI / (MID + 1)));
                if (i % 2 == 1)
                {
                    float im = (float)(2 / Math.PI / i);
                    impulse[MID + i] = k * im;
                    impulse[MID - i] = k * -im;
                }
            }

            var impulseBuffer = createBuffer(NUM_CHANNELS, FILTER_LENGTH, SAMPLE_RATE);
            //impulseBuffer.copyToChannel(impulse, 0);
            //impulseBuffer.copyToChannel(impulse, 1);
            //var hilbert = context.createConvolver();
            //hilbert.normalize = false;
            //hilbert.buffer = impulseBuffer;

            //int delayTime = MID / SAMPLE_RATE;
            //var delay = context.createDelay(delayTime);
            //delay.delayTime.value = delayTime;

            //return hilbert;
        }

        public static List<float> createBuffer(int numChannels, int filterLength, int sampleRate)
        {
            return new List<float>(filterLength * numChannels);
        }
    }
}
