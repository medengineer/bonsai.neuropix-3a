#pragma once
using namespace System;
#include <ElectrodePacket.h>

namespace Neuropix3a
{
	namespace Net
	{
		public ref class ElectrodePacket
		{
		internal:
			::ElectrodePacket *packet;
		private:
			~ElectrodePacket() { this->!ElectrodePacket(); }
			!ElectrodePacket();
		public:
			ElectrodePacket();

			property IntPtr StartTrigger {
				IntPtr get() { return (IntPtr)packet->startTrigger; }
			}

			property IntPtr Synchronization {
				IntPtr get() { return (IntPtr)packet->synchronization; }
			}

			property IntPtr Counters {
				IntPtr get() { return (IntPtr)packet->ctrs; }
			}

			property IntPtr LfpData {
				IntPtr get() { return (IntPtr)packet->lfpData; }
			}

			property IntPtr ApData {
				IntPtr get() { return (IntPtr)packet->apData; }
			}
		};
	}
}

