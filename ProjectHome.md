This is an [Arduino library](http://www.arduino.cc/en/Reference/Libraries) created to allow the user to easily use Zigduino's built-in 2.4 GHz radio transceiver. The [Zigduino](http://www.logos-electro.com/zigduino) is a Arduino compatible microcontroller platform that uses the [ATmega128RFA1](http://www.atmel.com/dyn/products/product_card.asp?part_id=4692), so it has a 2.4 GHz [IEEE 802.15.4](http://en.wikipedia.org/wiki/IEEE_802.15.4) radio transceiver built right in.

It is created with [µracoli](http://www.nongnu.org/uracoli/). µracoli is open source, unlike [Atmel's 802.15.4 MAC](http://www.atmel.com/dyn/products/tools_card.asp?tool_id=4675&category_id=163&family_id=676&subfamily_id=2124), which is closed source (thus not re-distributable). So this library has the advantage of being more easily installed and freely distributed.

All usage documentation is within the source code, please see [ZigduinoRadio.cpp](http://code.google.com/p/zigduino-radio/source/browse/trunk/ZigduinoRadio/ZigduinoRadio.cpp) and read the comments.

This library is designed to bare some resemblance to Arduino's [Serial class](http://arduino.cc/en/Reference/Serial) and Arduino's [Wire library](http://arduino.cc/en/Reference/Wire).

Please add bug reports and enhancement requests to the ["Issues" tab above](http://code.google.com/p/zigduino-radio/issues/list).

See the ["Wiki" tab above](http://code.google.com/p/zigduino-radio/wiki) to read various documentation.