"""
    Example code to interface from ESP32/Badge with the blaster
"""

from machine import Pin
from time import ticks_us
import esp32

class DataPacket():
    COMMAND = ["NONE","SHOOT","HEAL","CHANNEL","FIRETYPE","GAMEMODE","GOTHIT","ANIMATION","TEAMSWITCH","CHATTER","PULLTRIGGER","SETFLAGSA","RESERVED","RESERVED","RESERVED","ACK"]
    TEAM = ["NONE","REX","GIGGLES","YELLOW","BUZZ","MAGENTA","AZURE","WHITE"]
    def __init__(self, raw = 0):
        self._raw = raw
  
    @property
    def raw(self) -> int:
        return self._raw
   
    # TEAM
    @property
    def team(self) -> int:
        return (self._raw & 0b0000000000000111) >> 0
    
    @team.setter
    def team(self, value:Union[str,int]) -> None:
        if isinstance(value, str):
            value = DataPacket.TEAM.index(value.upper())
        if isinstance(value, int):
            self._raw &= ~0b111
            self._raw |= (value & 0b111)
    
    @property
    def team_str(self) -> str:
        return DataPacket.TEAM[self.team]
    
    # TRIGGER
    @property
    def trigger(self) -> bool:
        return True if (self._raw & 0b0000000000001000) >> 3 else False
    
    @trigger.setter
    def trigger(self, value:bool) -> None:
        value = 1 if value else 0
        self._raw &= ~(0b1 << 3)
        self._raw |= value << 3
    
    # COMMAND
    @property
    def command(self) -> int:
        return (self._raw & 0b0000000011110000) >> 4
    
    @command.setter
    def command(self, value:Union[str,int]) -> None:
        if isinstance(value, str):
            value = DataPacket.COMMAND.index(value.upper())
        if isinstance(value, int):
            self._raw &= ~(0b111 << 4)
            self._raw |= (value & 0b111) << 4
    
    @property
    def command_str(self) -> str:
        return DataPacket.COMMAND[self.command]
    
    # PARAMETER
    @property
    def parameter(self) -> int:
        return (self._raw & 0b0000111100000000) >> 8
    
    @parameter.setter
    def parameter(self, value:int) -> None:
        self._raw &= ~(0b1111 << 8)
        self._raw |= (value & 0b1111) << 8
    
    # CRC
    @property
    def crc(self): return (self._raw & 0b1111000000000000) >> 12
    
    @crc.setter
    def crc(self, value):
        self._raw &= 0b0000111111111111
        self._raw |= (value << 12)
    
    def calculate_crc(self, apply:bool=False):
        """
        Calculate CRC based on first 12 bits and return value.
        if apply is True then set the CRC in this packet.
        """
        crc = [0, 0, 0, 0]
        # makes computing the checksum a litle bit faster
        d0 = (self._raw >> 0) & 1
        d1 = (self._raw >> 1) & 1
        d2 = (self._raw >> 2) & 1
        d3 = (self._raw >> 3) & 1
        d4 = (self._raw >> 4) & 1
        d5 = (self._raw >> 5) & 1
        d6 = (self._raw >> 6) & 1
        d7 = (self._raw >> 7) & 1
        d8 = (self._raw >> 8) & 1
        d9 = (self._raw >> 9) & 1
        d10 = (self._raw >> 10) & 1
        d11 = (self._raw >> 11) & 1
        crc[0] = d11 ^ d10 ^ d9 ^ d8 ^ d6 ^ d4 ^ d3 ^ d0 ^ 0
        crc[1] = d8 ^ d7 ^ d6 ^ d5 ^ d3 ^ d1 ^ d0 ^ 1
        crc[2] = d9 ^ d8 ^ d7 ^ d6 ^ d4 ^ d2 ^ d1 ^ 1
        crc[3] = d10 ^ d9 ^ d8 ^ d7 ^ d5 ^ d3 ^ d2 ^ 0
        calc_crc = (crc[3] << 3) + (crc[2] << 2) + (crc[1] << 1) + crc[0]
        if apply: self.crc ^= calc_crc
        return calc_crc
        
    
    def __repr__(self): return(f"{self.team_str}, {self.trigger=}, {self.command_str}, {self.parameter=}, {self.crc=}, {self.calculate_crc()=}")


class Reader():
    def __init__(self, pin:int, can_transmit:bool = False) -> None:
        self._can_transmit = can_transmit
        self._pin_nr = pin
        self._reset()
        self._pin = None
    
    def _reset(self) -> None:
        self._raw = 0
        self._bits_read = 0
        self._ref_time = ticks_us()
    
    def _handle_irq(self, e: Pin) -> None:
        t = ticks_us()
        delta = t - self._ref_time
        self._ref_time = t
        if self._bits_read == 16: return
        if self._bits_read > 16: self._reset()
        if delta > (12600 * 0.8) and delta < (12600 / 0.8):
            self._reset()
        elif delta > (2100 * 0.8) and delta < (2100 / 0.8):
            self._raw = (1 << 15) | (self._raw >> 1)
            self._bits_read += 1
        elif delta > (1050 * 0.8) and delta < (1050 / 0.8):
            self._raw = self._raw >> 1
            self._bits_read += 1
        else:
            self._reset()
            
    def start(self) -> None:
        self._pin = Pin(self._pin_nr, Pin.IN, Pin.PULL_UP)
        self._reset()
        self._pin.irq(trigger=Pin.IRQ_RISING, handler=self._handle_irq)
        
    def stop(self) -> None:
        if self._pin:
            self._pin.irq(handler=None)
        
    def read_packet(self) -> Optional[DataPacket]:
        if self._bits_read == 16:
            packet = DataPacket(self._raw)
            self._reset()
            return packet
        return None
    
    def transmit_packet(self, packet: DataPacket) -> None:
        if not self._can_transmit: return
        self.stop()
        link = esp32.RMT(0, pin=Pin(self._pin_nr, Pin.OUT), idle_level=False, clock_div=200)
        packet.crc = 0
        packet.calculate_crc(apply=True)
        #time.sleep(.1)
        pulse_train = [210*16,210*8] #start
        for i in range(16):
            if packet.raw & (1 << i):
                pulse_train.extend([210*1,210*3])
            else:
                pulse_train.extend([210*1,210*1])
        pulse_train.extend([210,210]) #stop
        link.write_pulses(pulse_train,1)
        while not link.wait_done():
            ...
        link.deinit()
        self.start()
        
        
def go():
    b = Reader(04, can_transmit=True)
    r = Reader(25) #25=ir     4=blaster
    r.start()
    while (True):
        if p := r.read_packet():
            if p.calculate_crc() == p.crc:
                print(p)
                b.transmit_packet(p)
            else:
                print("CRC fail")

from time import sleep
def go_tx():
    b = Reader(04, can_transmit=True)
    p = DataPacket(0)
    p.team = "BUZZ"
    p.trigger = False
    p.command = "SHOOT"
    p.parameter = 15
    while (True):
        b.transmit_packet(p)
        print(p)
        sleep(1)
