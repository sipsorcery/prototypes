using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using SciChart.Charting.Visuals;

namespace DspTests
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private const string KEY_CODE = "oarxoB/v+1PVisQ5Tv/BIS0KoBGPhM3UYHNWvqTQnh2NwospvdIUcMtMjtOsIcUWlUzV+0KO+AwHD5Fb4tflKILnFlUKdFQXe4Ntc8/UgAXy9PgNjyCI6fr9s9y9aTsFJo1k73NzzGe+bLIRAkmlIe0a673l6cDnMKsV5LvugeWGqrgt3A073HUh50Pjw363vVrrFQmzJXmDczdPwTPAxU9pSkFiL0c9W9ZMAc8xzDm8OWPT76y8SIfJZDXQkPxu00hawbxgc2Cdvzd8HXpt1a+J3gT7lgGowmPwVsIByeoG0FMsREn2FZeIFGJoPX2u4QFJbBoXFi9vwhlxu4RoH+JNOKTh5U4xV+a2NSubulR522UHE53MsDbQacyKjLDmx3So2wnS8KyYhAxNNcNBOpvEtxBNARr0cynLadVCpuc+zsljG6xGI24hOz7IGvE2ouInCexzNfAoPmJno2w2v+QbRtif0lLpkqE/g4yZ7Hs2OFZtP4BP1CHOtC4SiWnQLmlmvqXM8CbmEPTN4pUm6DxKdJgvjDjzz0lKd33Uu8yrIin8HWEur/tTRQwOj7jvyPPxarA=";
        
        public App()
        {
            SciChartSurface.SetRuntimeLicenseKey(KEY_CODE);
        }

    }
}
