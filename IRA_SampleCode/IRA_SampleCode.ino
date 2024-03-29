/*
   IRremoteESP8266: IRrecvDumpV2 - dump details of IR codes with IRrecv
   An IR detector/demodulator must be connected to the input kRecvPin.

   Copyright 2009 Ken Shirriff, http://arcfn.com
   Copyright 2017-2019 David Conran

   Example circuit diagram:
    https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-receiving

   Changes:
     Version 1.2 October, 2020
       - Enable easy setting of the decoding tolerance value.
     Version 1.0 October, 2019
       - Internationalisation (i18n) support.
       - Stop displaying the legacy raw timing info.
     Version 0.5 June, 2019
       - Move A/C description to IRac.cpp.
     Version 0.4 July, 2018
       - Minor improvements and more A/C unit support.
     Version 0.3 November, 2017
       - Support for A/C decoding for some protocols.
     Version 0.2 April, 2017
       - Decode from a copy of the data so we can start capturing faster thus
         reduce the likelihood of miscaptures.
   Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
*/

#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = 25;

// The Serial connection baud rate.
// i.e. Status message will be sent to the PC at this baud rate.
// Try to avoid slow speeds like 9600, as you will miss messages and
// cause other problems. 115200 (or faster) is recommended.
// NOTE: Make sure you set your Serial Monitor to the same speed.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;

// How much percentage lee way do we give to incoming signals in order to match
// it?
// e.g. +/- 25% (default) to an expected value of 500 would mean matching a
//      value between 375 & 625 inclusive.
// Note: Default is 25(%). Going to a value >= 50(%) will cause some protocols
//       to no longer match correctly. In normal situations you probably do not
//       need to adjust this value. Typically that's when the library detects
//       your remote's message some of the time, but not all of the time.
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

// Legacy (No longer supported!)
//
// Change to `true` if you miss/need the old "Raw Timing[]" display.
#define LEGACY_TIMING_INFO false
// ==================== end of TUNEABLE PARAMETERS ====================

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

// This section of code runs only once at start-up.
void setup() {
#if defined(ESP8266)
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);
#endif  // ESP8266
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  // Perform a low level sanity checks that the compiler performs bit field
  // packing as we expect and Endianness is as we expect.
  assert(irutils::lowLevelSanityCheck() == 0);

  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver
}

// The repeating section of the code
void loop() {
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    uint16_t packet = (results.command & 0xff);
    packet <<= 8;
    packet += results.address & 0xff;
    Serial.print("Bits: ");
    Serial.println(packet, BIN);
    if (validate_crc(packet)) Serial.println("CRC OK");
    switch (results.address & 0b111) {
      case 1:
        Serial.println("REX");
        break;
      case 2:
        Serial.println("GIGGLE");
        break;
      case 4:
        Serial.println("BUZZ");
        break;
    }
  }
}

bool validate_crc(uint16_t packet)
{
  bool crc[] = {0, 0, 0, 0};
  // makes computing the checksum a litle bit faster
  bool d0 = bitRead(packet, 0);
  bool d1 = bitRead(packet, 1);
  bool d2 = bitRead(packet, 2);
  bool d3 = bitRead(packet, 3);
  bool d4 = bitRead(packet, 4);
  bool d5 = bitRead(packet, 5);
  bool d6 = bitRead(packet, 6);
  bool d7 = bitRead(packet, 7);
  bool d8 = bitRead(packet, 8);
  bool d9 = bitRead(packet, 9);
  bool d10 = bitRead(packet, 10);
  bool d11 = bitRead(packet, 11);

  crc[0] = d11 ^ d10 ^ d9 ^ d8 ^ d6 ^ d4 ^ d3 ^ d0 ^ 0;
  crc[1] = d8 ^ d7 ^ d6 ^ d5 ^ d3 ^ d1 ^ d0 ^ 1;
  crc[2] = d9 ^ d8 ^ d7 ^ d6 ^ d4 ^ d2 ^ d1 ^ 1;
  crc[3] = d10 ^ d9 ^ d8 ^ d7 ^ d5 ^ d3 ^ d2 ^ 0;

  bitWrite(packet, 12, crc[0] ^ bitRead(packet, 12));
  bitWrite(packet, 13, crc[1] ^ bitRead(packet, 13));
  bitWrite(packet, 14, crc[2] ^ bitRead(packet, 14));
  bitWrite(packet, 15, crc[3] ^ bitRead(packet, 15));
  return ((packet & 0xf000) == 0);
}
