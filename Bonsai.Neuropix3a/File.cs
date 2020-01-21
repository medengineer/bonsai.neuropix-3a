using Bonsai.IO;
using OpenCV.Net;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing.Design;
using System.Linq;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Neuropix3a.Net;

namespace Bonsai.Neuropix3a
{
    public class File : Source<NeuropixDataFrame>
    {
        public File()
        {
            BufferSize = 1;


            Frequency = 30000;
        }

        [FileNameFilter("NPX Files (*.npx)|*.npx|All files (*.*)|*.*")]
        [Description("The path to the NPX file containing acquisition data.")]
        [Editor("Bonsai.Design.OpenFileNameEditor, Bonsai.Design", typeof(UITypeEditor))]
        public string Path { get; set; }

        [Description("The number of electrode packets in each output buffer.")]
        public int BufferSize { get; set; }

        [Description("The frequency of the output signal.")]
        public int Frequency { get; set; }

        public override IObservable<NeuropixDataFrame> Generate()
        {
            return Observable.Create<NeuropixDataFrame>((observer, cancellationToken) =>
            {
                return Task.Factory.StartNew(() =>
                {
                    var stopwatch = new Stopwatch();
                    using (var basestation = new NeuropixBasestation())
                    using (var sampleSignal = new ManualResetEvent(false))
                    {
                        basestation.Open(Path);
                        try
                        {
                            basestation.Mode = AsicMode.Recording;
                            basestation.DataMode = true;

                            var packets = new ElectrodePacket[BufferSize];
                            for (int i = 0; i < packets.Length; i++)
                            {
                                packets[i] = new ElectrodePacket();
                            }

                            while (!cancellationToken.IsCancellationRequested)
                            {
                                stopwatch.Restart();
                                for (int i = 0; i < packets.Length; i++)
                                {
                                    if (!basestation.ReadElectrodeData(packets[i]))
                                    {
                                        observer.OnCompleted();
                                        return;
                                    }
                                }
                                var result = new NeuropixDataFrame(packets, 0);
                                observer.OnNext(result);

                                var frequency = Frequency;
                                var sampleInterval = frequency > 0 ? 1000.0 / Frequency : 0;
                                var dueTime = Math.Max(0, (sampleInterval * packets.Length * NeuropixDataFrame.SampleCount) - stopwatch.Elapsed.TotalMilliseconds);
                                if (dueTime > 0)
                                {
                                    sampleSignal.WaitOne(TimeSpan.FromMilliseconds(dueTime));
                                }
                            }
                        }
                        finally { basestation.Close(); }
                    }
                },
                cancellationToken,
                TaskCreationOptions.LongRunning,
                TaskScheduler.Default);
            });
        }
    }
}
