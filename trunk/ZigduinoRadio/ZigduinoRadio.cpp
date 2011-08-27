/* Copyright (c) 2011 Frank Zhao
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */
   
   #include "ZigduinoRadio.h"
#include "ZigduinoRadioEvents.h"

uint8_t cZigduinoRadio::rxFrameBuffer[MAX_FRAME_SIZE];
uint8_t cZigduinoRadio::rxRingBuffer[ZR_FIFO_SIZE];
uint8_t cZigduinoRadio::rxRingBufferHead = 0;
uint8_t cZigduinoRadio::rxRingBufferTail = 0;
uint8_t cZigduinoRadio::txTmpBuffer[ZR_TXTMPBUFF_SIZE];
uint8_t cZigduinoRadio::txTmpBufferLength = 0;
uint8_t cZigduinoRadio::lastLqi = 0;
uint8_t cZigduinoRadio::lastRssi = VOID_RSSI;
uint8_t cZigduinoRadio::hasAttachedRxEvent = 0;
uint8_t cZigduinoRadio::hasAttachedTxEvent = 0;
uint8_t cZigduinoRadio::usedBeginTransmission = 0;
volatile uint8_t cZigduinoRadio::txIsBusy = 0;
uint8_t* (*cZigduinoRadio::zrEventReceiveFrame)(uint8_t, uint8_t*, uint8_t, int8_t, uint8_t);
void (*cZigduinoRadio::zrEventTxDone)(radio_tx_done_t);

cZigduinoRadio::cZigduinoRadio()
{
	// default event handlers
	user_radio_error = 0;
	user_radio_irq = 0;
	zr_attach_receive_frame(onReceiveFrame);
	zr_attach_tx_done(onTxDone);
}

void cZigduinoRadio::begin(channel_t chan)
{
	radio_init(rxFrameBuffer, MAX_FRAME_SIZE);
	
	// fixed frame header
	txTmpBuffer[0] = 0x01; txTmpBuffer[1] = 0x80; txTmpBuffer[2] =  0; txTmpBuffer[3] = 0x11; txTmpBuffer[4] = 0x22; txTmpBuffer[5] = 0x33; txTmpBuffer[6] = 0x44;
	
	// set the channel
	radio_set_param(RP_CHANNEL(chan));
	
	// default to receiver
	radio_set_state(STATE_RX);
}

void cZigduinoRadio::attachError(void (*funct)(radio_error_t))
{
	user_radio_error = funct;
}

void cZigduinoRadio::attachIrq(void (*funct)(uint8_t))
{
	user_radio_irq = funct;
}

void cZigduinoRadio::attachReceiveFrame(uint8_t* (*funct)(uint8_t, uint8_t*, uint8_t, int8_t, uint8_t))
{
	zrEventReceiveFrame = funct;
	hasAttachedRxEvent = (funct == 0) ? 0 : 1;
}

void cZigduinoRadio::attachTxDone(void (*funct)(radio_tx_done_t))
{
	zrEventTxDone = funct;
	hasAttachedTxEvent = (funct == 0) ? 0 : 1;
}

uint8_t* cZigduinoRadio::onReceiveFrame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t rssi, uint8_t crc)
{
	lastLqi = lqi;
	lastRssi = rssi;
	if (hasAttachedRxEvent == 0)
	{
		// no event handler, so write it into the FIFO
		
		if (len >= 8)
		{
			for (uint8_t i = 7; i < len - 2; i++)
			{
				uint16_t j = ((uint16_t)((uint16_t)rxRingBufferHead + 1)) % ZR_FIFO_SIZE;
				if (j != rxRingBufferTail)
				{
					rxRingBuffer[rxRingBufferHead] = frm[i];
					rxRingBufferHead = j;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			for (uint8_t i = 0; i < len; i++)
			{
				uint16_t j = ((uint16_t)((uint16_t)rxRingBufferHead + 1)) % ZR_FIFO_SIZE;
				if (j != rxRingBufferTail)
				{
					rxRingBuffer[rxRingBufferHead] = frm[i];
					rxRingBufferHead = j;
				}
				else
				{
					break;
				}
			}
		}
	}
	else
	{
		zrEventReceiveFrame(len, frm, lqi, rssi, crc);
	}
}

void cZigduinoRadio::flush()
{
	rxRingBufferHead = rxRingBufferTail;
}

int16_t cZigduinoRadio::read()
{
	// if the head isn't ahead of the tail, we don't have any characters
	if (rxRingBufferHead == rxRingBufferTail)
	{
		return -1;
	}
	else
	{
		uint8_t c = rxRingBuffer[rxRingBufferTail];
		rxRingBufferTail = (rxRingBufferTail + 1) % ZR_FIFO_SIZE;
		return c;
	}
}

int16_t cZigduinoRadio::peek()
{
	// if the head isn't ahead of the tail, we don't have any characters
	if (rxRingBufferHead == rxRingBufferTail)
	{
		return -1;
	}
	else
	{
		uint8_t c = rxRingBuffer[rxRingBufferTail];
		return c;
	}
}

int8_t cZigduinoRadio::available()
{
	return ((int16_t)((int16_t)ZR_FIFO_SIZE + (int16_t)rxRingBufferHead - (int16_t)rxRingBufferTail)) % ZR_FIFO_SIZE;
}

// warning, no frame header or FCS added
void cZigduinoRadio::txFrame(uint8_t* frm, uint8_t len)
{
	while (txIsBusy);
	txIsBusy = 1;
	radio_set_state(STATE_TX);
	radio_send_frame(len, frm, 0);
}

void cZigduinoRadio::beginTransmission()
{
	usedBeginTransmission = 1;
	
	// add frame header
	txTmpBufferLength = 7;
}

void cZigduinoRadio::endTransmission()
{
	usedBeginTransmission = 0;
	
	// empty FCS field
	txTmpBufferLength += 2;
	
	while (txIsBusy);
	txIsBusy = 1;
	radio_set_state(STATE_TX);
	radio_send_frame(txTmpBufferLength, txTmpBuffer, 0);
}

void cZigduinoRadio::send(uint8_t c)
{
	write(c);
}

void cZigduinoRadio::write(uint8_t c)
{
	if (usedBeginTransmission)
	{
		if (txTmpBufferLength < ZR_TXTMPBUFF_SIZE - 2)
		{
			txTmpBuffer[txTmpBufferLength] = c;
			txTmpBufferLength++;
		}
	}
	else
	{
		// frame header, 1 byte payload, then 2 byte FCS
		uint8_t cb[10] = {0x01, 0x80, 0, 0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00};
		cb[7] = c; // set payload
		
		while (txIsBusy);
		txIsBusy = 1;
		radio_set_state(STATE_TX);
		radio_send_frame(10, cb, 0);
	}
}

void cZigduinoRadio::write(char* str)
{
	while (*str)
		write(*str++);
}

void cZigduinoRadio::write(uint8_t* arr, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++)
		write(arr[i]);
}

void cZigduinoRadio::onTxDone(radio_tx_done_t x)
{
	if (hasAttachedTxEvent)
	{
		zrEventTxDone(x);
	}
	
	txIsBusy = 0;
	radio_set_state(STATE_RX);
}

void cZigduinoRadio::setParam(radio_attribute_t attr, radio_param_t parm)
{
	radio_set_param(attr, parm);
}

radio_cca_t cZigduinoRadio::doCca()
{
	return radio_do_cca();
}

void cZigduinoRadio::setState(radio_state_t state, uint8_t force)
{
	if (force)
		radio_force_state(state);
	else
		radio_set_state(state);
}

void cZigduinoRadio::setState(radio_state_t state)
{
	radio_set_state(state);
}

void cZigduinoRadio::forceState(radio_state_t state)
{
	radio_force_state(state);
}

void cZigduinoRadio::setChannel(channel_t chan)
{
	radio_set_param(RP_CHANNEL(chan));
}

uint8_t cZigduinoRadio::getRssi()
{
	return lastRssi;
}

uint8_t cZigduinoRadio::getLqi()
{
	return lastLqi;
}

cZigduinoRadio ZigduinoRadio = cZigduinoRadio();