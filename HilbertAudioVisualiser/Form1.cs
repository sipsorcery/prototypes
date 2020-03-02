using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using NAudio;
using NAudio.Wave;

namespace HilbertAudioVisualiser
{
    public partial class Form1 : Form
    {
        private const int AUDIO_SAMPLE_PERIOD_MILLISECONDS = 100;
        private const int BITS_PER_SAMPLE = 16;
        private const int SAMPLES_PER_SECOND = 8000;
        private const float VOLUME = 1.0f;
        private const int DISPLAY_WIDTH = 256;
        private const int DISPLAY_HEIGHT = 192;

        private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(SAMPLES_PER_SECOND, 1);
        private WaveBuffer _dstWaveBuffer;
        private byte[] _texture = new byte[HilbertFilter.N * 4];

        private WaveInEvent _waveInEvent;
        private Graphics _g;

        public Form1()
        {
            InitializeComponent();
            InitAudio();

            this.Load += Form1_Load;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            _dstWaveBuffer = new WaveBuffer(new byte[SAMPLES_PER_SECOND / AUDIO_SAMPLE_PERIOD_MILLISECONDS * 4]);

            Array.Fill<byte>(_texture, 0x00);

           _g = Graphics.FromHwnd(_audioPicBox.Handle);

            _waveInEvent.StartRecording();
        }

        private void InitAudio()
        {
            _waveInEvent = new WaveInEvent();
            _waveInEvent.BufferMilliseconds = AUDIO_SAMPLE_PERIOD_MILLISECONDS;
            _waveInEvent.NumberOfBuffers = 1;
            _waveInEvent.DeviceNumber = 0;
            _waveInEvent.WaveFormat = _waveFormat;
            _waveInEvent.DataAvailable += OnAudioSample;
        }

        private void OnAudioSample(object sender, WaveInEventArgs args)
        {
            if (args.BytesRecorded > 0)
            {
                WaveBuffer srcWaveBuffer = new WaveBuffer(args.Buffer);

                //for (int sample = 0; sample < srcWaveBuffer.FloatBufferCount; sample++)
                //{
                //    //var floatSample = (srcWaveBuffer.ShortBuffer[sample] / 32768f) * VOLUME;
                //    //_dstWaveBuffer.FloatBuffer[destOffset++] = floatSample;
                //    _dstWaveBuffer.FloatBuffer[sample] = srcWaveBuffer.FloatBuffer[sample];
                //}

                //Debug.Write($"{_dstWaveBuffer.FloatBuffer[0]:0.#####},");
                //Debug.Write($"{_dstWaveBuffer.FloatBuffer[1]:0.#####},");
                //Debug.Write($"{_dstWaveBuffer.FloatBuffer[2]:0.#####},");
                //Debug.Write($"{_dstWaveBuffer.FloatBuffer[3]:0.#####},");
                //Debug.WriteLine($"{_dstWaveBuffer.FloatBuffer[4]:0.#####}");

                //NAudio.Dsp.ImpulseResponseConvolution

                // For time samples:
                // 1. Hilbert filter.

                // For quad samples:
                // 1. Hilbert filter,
                // 2. Delay (phase shift).

                HilbertFilter.updateTextureData(_texture, srcWaveBuffer.FloatBuffer, srcWaveBuffer.FloatBuffer, DISPLAY_HEIGHT);

                DrawTexture(_texture);
            }
        }

        public void DrawTexture(byte[] texture)
        {
            unsafe
            {
                

                fixed (byte* s = texture)
                {
                    System.Drawing.Bitmap bmpImage = new System.Drawing.Bitmap(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH * 4, System.Drawing.Imaging.PixelFormat.Format24bppRgb, (IntPtr)s);
                    TextureBrush textureBrush = new TextureBrush(bmpImage);
                    //textureBrush.
                    _g.FillRectangle(textureBrush, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
                    
                }
            }
        }
    }
}
