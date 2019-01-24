#include "event.h"

iEvent::iEvent(uint32_t eid, uint32_t sn) : eid_(eid), sn_(sn)
{
}

iEvent::~iEvent()
{
}

uint32_t iEvent::generateSeqNo()
{
    static uint32_t sn = 0;
	return sn++;
}
