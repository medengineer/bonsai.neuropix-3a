#include "stdafx.h"
#include <string.h>
#include "ElectrodePacket.h"
using namespace System::Runtime::InteropServices;

Neuropix3a::Net::ElectrodePacket::ElectrodePacket():
	packet(new ::ElectrodePacket())
{
}

Neuropix3a::Net::ElectrodePacket::!ElectrodePacket()
{
	delete packet;
}

