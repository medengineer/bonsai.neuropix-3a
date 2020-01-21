using Neuropix3a.Net;
using OpenCV.Net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bonsai.Neuropix3a
{
    public class NeuropixDataFrame
    {
        const int ChannelCount = 384;
        internal const int SampleCount = 12;

        public NeuropixDataFrame(ElectrodePacket packet, float bufferCapacity)
        {
            var startTrigger = new Mat(1, SampleCount, Depth.U8, 1, packet.StartTrigger);
            var synchronization = new Mat(1, SampleCount, Depth.U16, 1, packet.Synchronization);
            var counters = new Mat(SampleCount, SampleCount + 1, Depth.S32, 1, packet.Counters);
            var lfpData = new Mat(ChannelCount, 1, Depth.F32, 1, packet.LfpData);
            var apData = new Mat(SampleCount, ChannelCount, Depth.F32, 1, packet.ApData);

            StartTrigger = startTrigger.Clone();
            Synchronization = synchronization.Clone();
            Counters = Transpose(counters);
            LfpData = lfpData.Clone();
            ApData = Transpose(apData);
            BufferCapacity = bufferCapacity;
        }

        public NeuropixDataFrame(ElectrodePacket[] packets, float bufferCapacity)
        {
            var sampleCount = SampleCount * packets.Length;
            var startTrigger = new Mat(1, sampleCount, Depth.U8, 1);
            var synchronization = new Mat(1, sampleCount, Depth.U16, 1);
            var counters = new Mat(SampleCount + 1, sampleCount, Depth.S32, 1);
            var lfpData = new Mat(ChannelCount, packets.Length, Depth.F32, 1);
            var apData = new Mat(ChannelCount, sampleCount, Depth.F32, 1);

            using (var startTriggerHeader = new Mat(1, SampleCount, Depth.U8, 1, IntPtr.Zero))
            using (var synchronizationHeader = new Mat(1, SampleCount, Depth.U16, 1, IntPtr.Zero))
            using (var countersHeader = new Mat(SampleCount, SampleCount + 1, Depth.S32, 1, IntPtr.Zero))
            using (var lfpDataHeader = new Mat(ChannelCount, 1, Depth.F32, 1, IntPtr.Zero))
            using (var apDataHeader = new Mat(SampleCount, ChannelCount, Depth.F32, 1, IntPtr.Zero))
            {
                for (int i = 0; i < packets.Length; i++)
                {
                    startTriggerHeader.SetData(packets[i].StartTrigger, Mat.AutoStep);
                    synchronizationHeader.SetData(packets[i].Synchronization, Mat.AutoStep);
                    countersHeader.SetData(packets[i].Counters, Mat.AutoStep);
                    lfpDataHeader.SetData(packets[i].LfpData, Mat.AutoStep);
                    apDataHeader.SetData(packets[i].ApData, Mat.AutoStep);
                    CV.Copy(startTriggerHeader, startTrigger.GetSubRect(new Rect(i * SampleCount, 0, startTriggerHeader.Cols, startTriggerHeader.Rows)));
                    CV.Copy(synchronizationHeader, synchronization.GetSubRect(new Rect(i * SampleCount, 0, synchronizationHeader.Cols, synchronizationHeader.Rows)));
                    CV.Transpose(countersHeader, counters.GetSubRect(new Rect(i * SampleCount, 0, countersHeader.Rows, countersHeader.Cols)));
                    CV.Copy(lfpDataHeader, lfpData.GetSubRect(new Rect(i, 0, lfpDataHeader.Cols, lfpDataHeader.Rows)));
                    CV.Transpose(apDataHeader, apData.GetSubRect(new Rect(i * SampleCount, 0, apDataHeader.Rows, apDataHeader.Cols)));
                }
            }

            StartTrigger = startTrigger;
            Synchronization = synchronization;
            Counters = counters;
            LfpData = lfpData;
            ApData = apData;
            BufferCapacity = bufferCapacity;
        }

        public Mat StartTrigger { get; private set; }

        public Mat Synchronization { get; private set; }

        public Mat Counters { get; private set; }

        public Mat LfpData { get; private set; }

        public Mat ApData { get; private set; }

        public float BufferCapacity { get; private set; }

        static Mat Transpose(Mat source)
        {
            var result = new Mat(source.Cols, source.Rows, source.Depth, source.Channels);
            CV.Transpose(source, result);
            return result;
        }
    }
}
