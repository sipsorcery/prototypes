using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using MathNet.Numerics;
using NAudio.Utils;
using NAudio.Wave;
using SciChart.Charting.Visuals;
using SciChart.Charting.Model.DataSeries;
using SciChart.Data.Model;
using SciChart.Examples.ExternalDependencies.Data;

namespace DspTests
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow :System.Windows.Window
    {
        private const int SAMPLE_COUNT = 250;
        private const int AUDIO_SAMPLE_PERIOD_MILLISECONDS = 30;
        public const int CIRCULAR_BUFFER_SAMPLES = 3;

        private static readonly WaveFormat _waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(8000, 1);

        private WaveInEvent _waveInEvent;
        private CircularBuffer _audioInBuffer;

        private readonly float[] _xSeries = new float[SAMPLE_COUNT];
        private readonly IXyDataSeries<float, float> _audioSeriesSource;

        public MainWindow()
        {
            InitializeComponent();

            _audioSeriesSource = new XyDataSeries<float, float>
            {
                FifoCapacity = SAMPLE_COUNT
            };

            for (int i = 0; i < _xSeries.Length; i++) _xSeries[i] = i;

            StartAudio();
        }

        private void StartAudio()
        {
            _audioInBuffer = new CircularBuffer(SAMPLE_COUNT * _waveFormat.BlockAlign * CIRCULAR_BUFFER_SAMPLES);

            _waveInEvent = new WaveInEvent();
            _waveInEvent.BufferMilliseconds = AUDIO_SAMPLE_PERIOD_MILLISECONDS;
            _waveInEvent.NumberOfBuffers = 3;
            _waveInEvent.DeviceNumber = 0;
            _waveInEvent.WaveFormat = _waveFormat;
            _waveInEvent.DataAvailable += AudioDataAvailable;
            _waveInEvent.StartRecording();
        }

        private void AudioDataAvailable(object sender, WaveInEventArgs args)
        {
            _audioInBuffer.Write(args.Buffer, 0, args.BytesRecorded);

            while (_audioInBuffer.Count > (SAMPLE_COUNT * 4))
            {
                int bytesPerSample = _waveFormat.BlockAlign;

                byte[] buffer = new byte[SAMPLE_COUNT * bytesPerSample];
                _audioInBuffer.Read(buffer, 0, SAMPLE_COUNT * bytesPerSample);

                List<float> samples = new List<float>();
                for (int i = 0; i < SAMPLE_COUNT * bytesPerSample; i += bytesPerSample)
                {
                    samples.Add(BitConverter.ToSingle(buffer, i));
                }

                ProcessTimeSeriesAudioSample(samples.ToArray());
            }
        }

        /// <summary>
        /// Displays raw time series data.
        /// </summary>
        /// <param name="samples">Audio input samples.</param>
        private void ProcessTimeSeriesAudioSample(float[] samples)
        {
            using (sciChart.SuspendUpdates())
            {
                _audioSeriesSource.Clear();
                _audioSeriesSource.Append(_xSeries, samples);
            }
        }

        /// <summary>
        /// Displays the FFT of time series data.
        /// </summary>
        /// <param name="samples">Audio input samples.</param>
        private void ProcessSpectrumAudioSample(float[] samples)
        {
            var complexSamples = samples.Select(x => new Complex(x, 0)).ToArray();

            MathNet.Numerics.IntegralTransforms.Fourier.Forward(complexSamples);

            using (sciChart.SuspendUpdates())
            {
                _audioSeriesSource.Clear();
                _audioSeriesSource.Append(_xSeries, samples);
            }
        }

        private void LineChartExampleView_OnLoaded(object sender, RoutedEventArgs e)
        {
            //var dataSeries = new XyDataSeries<double, double>();
            //var dataSeries = new SciChart.Charting.Model.DataSeries.

            //lineRenderSeries.DataSeries = dataSeries;

            audioSeries.DataSeries = _audioSeriesSource;

            //float[] xDummyData = new float[] { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
            //float[] yDummyData = new float[] { 12.0f, 22.0f, 3.0f, 7.0f, 15.0f };

            //double[] xSeries = new double[SAMPLE_COUNT];
            ////double[] ySeries = new double[SAMPLE_COUNT];
            //for (int i = 0; i < SAMPLE_COUNT; i++)
            //{
            //    xSeries[i] = i;
            //    //[i] = i % 2 == 0 ? 1.0 : 0.0;
            //}

            //var sinusoid = MathNet.Numerics.Generate.Sinusoidal(SAMPLE_COUNT, 8000, 261.6, 1.0);

            //var data = DataManager.Instance.GetFourierSeries(1.0, 0.1);
            //dataSeries.Append(data.XData, data.YData);

            //dataSeries.Append(xSeries, sinusoid);
            //dataSeries.Append(xSeries, ySeries);


        }
    }
}
