#pragma once
#include "event.h"
#include "eventtype.h"
#include "events_def.h"

class protocol_codec_t
{
public:
    virtual bool encode(iEvent* ev, uint8_t* buffer, uint32_t size) = 0;
    virtual iEvent* decode(uint16_t mid, uint8_t* buffer, uint32_t size) = 0;
};
