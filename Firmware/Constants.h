/* Pin's */
const byte WS2812_Pin = 10;
const byte TeamSelect_R_Pin = 2;
const byte TeamSelect_G_Pin = 3;
const byte TeamSelect_B_Pin = 4;

/*
when changing the trigger pin also update PCICR and PCMSK0
in the EnableTrigger() and DisableTrigger() functions
*/
const byte Trigger_Pin = 8;


const byte WS2812_Leds = 9;

const byte Rex = 0;
const byte Giggle = 1;
const byte Buzz = 2;

const uint32_t RedRGB = 0x00FF0000;
const uint32_t GreenRGB = 0x0000FF00;
const uint32_t BlueRGB = 0x000000FF;
const uint32_t BlackRGB = 0x00000000;
const uint32_t WhiteRGB = 0x00FFFFFF;
