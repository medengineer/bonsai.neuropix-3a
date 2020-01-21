using Bonsai.IO;
using OpenCV.Net;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing.Design;
using System.Linq;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;
using Neuropix3a.Net;

namespace Bonsai.Neuropix3a
{
    public class Probe : Source<NeuropixDataFrame>
    {
        public Probe()
        {
            StreamingFileName = "datalog.npx";
        }

        [Category("Acquisition")]
        [Description("Indicates whether to stream all acquisition data to a file.")]
        public bool FileStreaming { get; set; }

        [Category("Acquisition")]
        [FileNameFilter("NPX Files (*.npx)|*.npx|All files (*.*)|*.*")]
        [Description("The path to the NPX file used to record all acquisition data.")]
        [Editor("Bonsai.Design.SaveFileNameEditor, Bonsai.Design", typeof(UITypeEditor))]
        public string StreamingFileName { get; set; }

        [Category("Acquisition")]
        [Description("The optional suffix used to generate streaming file names.")]
        public PathSuffix StreamingSuffix { get; set; }

        [Category("Configuration")]
        [Description("The high-pass filter bandwidth used during acquisition of AP data.")]
        public FilterBandwidth FilterBandwidth { get; set; }

        [Category("Configuration")]
        [Description("Indicates whether the headstage LEDs should be turned off.")]
        public bool LedOff { get; set; }

        [Category("Configuration")]
        [Description("The gain setting used for all AP channels.")]
        public GainSetting GainAP { get; set; }

        [Category("Configuration")]
        [Description("The gain setting used for all LFP channels.")]
        public GainSetting GainLFP { get; set; }

        [Category("Calibration")]
        [FileNameFilter("CSV Files (*.csv)|*.csv|All files (*.*)|*.*")]
        [Description("The path to a CSV file containing ADC comparator calibration data.")]
        [Editor("Bonsai.Design.OpenFileNameEditor, Bonsai.Design", typeof(UITypeEditor))]
        public string ComparatorCalibration { get; set; }

        [Category("Calibration")]
        [FileNameFilter("CSV Files (*.csv)|*.csv|All files (*.*)|*.*")]
        [Description("The path to a CSV file containing ADC offset calibration data.")]
        [Editor("Bonsai.Design.OpenFileNameEditor, Bonsai.Design", typeof(UITypeEditor))]
        public string AdcOffsetCalibration { get; set; }

        [Category("Calibration")]
        [FileNameFilter("CSV Files (*.csv)|*.csv|All files (*.*)|*.*")]
        [Description("The path to a CSV file containing ADC slope calibration data.")]
        [Editor("Bonsai.Design.OpenFileNameEditor, Bonsai.Design", typeof(UITypeEditor))]
        public string AdcSlopeCalibration { get; set; }

        [Category("Calibration")]
        [FileNameFilter("CSV Files (*.csv)|*.csv|All files (*.*)|*.*")]
        [Description("The path to a CSV file containing gain correction data.")]
        [Editor("Bonsai.Design.OpenFileNameEditor, Bonsai.Design", typeof(UITypeEditor))]
        public string GainCorrection { get; set; }

        public override IObservable<NeuropixDataFrame> Generate()
        {
            return Observable.Create<NeuropixDataFrame>((observer, cancellationToken) =>
            {
                return Task.Factory.StartNew(() =>
                {
                    try
                    {
                        using (var basestation = new NeuropixBasestation())
                        {
                            var version = basestation.GetAPIVersion();
                            Console.WriteLine("Neuropix API version: {0}.{1}", version.Major, version.Minor);
                            Console.WriteLine("Opening basestation data and configuration link...");
                            basestation.Open();

                            version = basestation.GetHardwareVersion();
                            Console.WriteLine("Neuropix FPGA bootcode version: {0}.{1}", version.Major, version.Minor);

                            version = basestation.GetBSVersion();
                            Console.WriteLine("Neuropix Basestation connect board version: {0}.{1}", version.Major, version.Minor);

                            var probeID = basestation.ReadID();
                            Console.WriteLine("Neuropix probe S/N: {0} Option {1}", probeID.SerialNumber, probeID.ProbeType);

                            Console.Write("Neuropix ADC calibration... ");
                            var comparatorCalibration = ComparatorCalibration;
                            var adcOffsetCalibration = AdcOffsetCalibration;
                            var adcSlopeCalibration = AdcSlopeCalibration;
                            if (!string.IsNullOrEmpty(comparatorCalibration) &&
                               !string.IsNullOrEmpty(adcOffsetCalibration) &&
                               !string.IsNullOrEmpty(adcSlopeCalibration))
                            {
                                basestation.ApplyAdcCalibrationFromCsv(comparatorCalibration, adcOffsetCalibration, adcSlopeCalibration);
                            }
                            else basestation.ApplyAdcCalibrationFromEeprom();
                            Console.WriteLine("OK");

                            Console.WriteLine("Neuropix setting gain and filter bandwidth... ");
                            basestation.WriteAllAPGains(GainAP);
                            basestation.WriteAllLFPGains(GainLFP);
                            basestation.SetFilter(FilterBandwidth);

                            Console.Write("Neuropix Gain correction... ");
                            var gainCorrection = GainCorrection;
                            if (!string.IsNullOrEmpty(gainCorrection))
                            {
                                basestation.ApplyGainCalibrationFromCsv(gainCorrection);
                            }
                            //else basestation.ApplyGainCalibrationFromEeprom();
                            Console.WriteLine("OK");

                            basestation.LedOff(LedOff);
                            basestation.Mode = AsicMode.Recording;
                            basestation.DataMode = true;
                            basestation.TriggerMode = false;
                            basestation.SetNrst(false);
                            basestation.ResetDatapath();
                            var fileName = StreamingFileName;
                            if (!string.IsNullOrEmpty(fileName) && FileStreaming)
                            {
                                PathHelper.EnsureDirectory(fileName);
                                fileName = PathHelper.AppendSuffix(fileName, StreamingSuffix);
                                basestation.StartRecording(fileName);
                            }
                            Console.WriteLine("Neuropix recording armed.");

                            basestation.SetNrst(true);
                            basestation.NeuralStart();
                            Console.WriteLine("Neuropix recording start...");

                            int packetCounter = 0;
                            float bufferCapacity = 0;
                            var packet = new ElectrodePacket();
                            while (!cancellationToken.IsCancellationRequested)
                            {
                                if (packetCounter == 0) bufferCapacity = basestation.FifoFilling;
                                basestation.ReadElectrodeData(packet);
                                var result = new NeuropixDataFrame(packet, bufferCapacity);
                                observer.OnNext(result);
                                packetCounter = (packetCounter + 1) % 50;
                            }

                            Console.WriteLine("Neuropix stop recording...");
                            if (!string.IsNullOrEmpty(fileName) && FileStreaming)
                            {
                                basestation.StopRecording();
                            }
                            basestation.Close();
                        }
                    }
                    catch(System.Runtime.InteropServices.SEHException ex)
                    {
                        Console.WriteLine(ex);
                        throw;
                    }
                },
                cancellationToken,
                TaskCreationOptions.LongRunning,
                TaskScheduler.Default);
            });
        }
    }
}
