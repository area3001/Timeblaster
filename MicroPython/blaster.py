from machine import Pin
from time import ticks_us, sleep, ticks_diff
from array import array
import esp32
import micropython

micropython.alloc_emergency_exception_buf(100)

class Enum():
    """
    Poor man's Enum
    """
    _lookup_table:Dict[int,str] = None
    @classmethod
    def lookup(cls,number):
        if not cls._lookup_table:
            cls._lookup_table = {getattr(cls,x):x for x in dir(cls) if not x.startswith("_") and not callable(getattr(cls,x))}
        return cls._lookup_table.get(number)

class Command(Enum):
    """
    Implemented commands
    """
    none = 0
    shoot = 1
    heal = 2
    channel = 3
    trigger_action = 4
    game_mode = 5
    animation = 7
    team_change = 8
    chatter = 9
    settings = 10
    ack = 15
    
class Team(Enum):
    """
    Available teams
    """
    none = 0
    rex = 1
    giggle = 2
    yellow = 3
    buzz = 4
    magenta = 5
    azure = 6
    white = 7
    
class GameMode(Enum):
    timeout = 0
    zombie = 1
    sudden_death = 2
    
class Animation(Enum):
    blaster_start = 1
    error = 2
    crash = 3
    fireball = 4
    one_up = 5
    coin = 6
    voice = 7
    wolfWhistle = 8
    chatter = 9
    
    blink_team_led = 15


class DataPacket():
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
    def team(self, value:int) -> None:
        if isinstance(value, int):
            self._raw &= ~0b111
            self._raw |= (value & 0b111)
    
    @property
    def team_str(self) -> str:
        return Team.lookup(self.team)
    
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
    def command(self, value:int) -> None:
        if isinstance(value, int):
            self._raw &= ~(0b1111 << 4)
            self._raw |= (value & 0b1111) << 4
    
    @property
    def command_str(self) -> str:
        return Command.lookup(self.command)
    
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
        #Reader properties
        self._can_transmit : bool = can_transmit
        self._pin_nr : int = pin
        #Current incommint packet info
        self._raw : int = 0
        self._ref_time : int = ticks_us()
        self._bits_read : int = 0 
        #ringbuffer to store raw data before it's processed
        self._buffer_size : int = 10
        self._buffer = array('I', (0 for _ in range(self._buffer_size)))
        self._buffer_writer : int = 0
        self._buffer_reader : int = 0
        #Validated messages
        self._messages : List[DataPacket] = []
        #Counters
        self._message_buffer_overflow = 0
        self._crc_fails = 0
        self._rx_count = 0
        self._rx_buffer_overflow = 0
        self._tx_count = 0
        #True if an ack was received
        self._ack : bool = False
        #reference to the irq handler, see https://docs.micropython.org/en/latest/reference/isr_rules.html#creation-of-python-objects
        self._irq_handler_ref = self._handle_irq
        #pin object used by this instance
        self._pin : Pin = Pin(self._pin_nr, Pin.IN, Pin.PULL_UP)
        #default state is listening for data
        self._start_listening()
  
    def ack_state(self) -> bool:
        """
        Returns true if an ACK packet has been received.
        Resets the ACK status 
        """
        self._process_buffer()
        if self._ack:
            self._ack = False
            return True
        return False
    
    @property
    def status(self):
        return {"tx_count":self._tx_count,
                "rx_count":self._rx_count,
                "rx_crc_fails":self._crc_fails,
                "rx_buffer_overflow":self._rx_buffer_overflow,
                "rx_message_buffer_overflow":self._message_buffer_overflow}
    
    def read_packet(self) -> Optional[DataPacket]:
        """
        Gets a DataPacket from the message buffer, None if the buffer is empty
        """
        self._process_buffer()        
        if len(self._messages) > 0:
            return self._messages.pop(0)
  
    def transmit_packet(self, packet: DataPacket) -> None:
        if not self._can_transmit: return
        self._tx_count += 1
        self._stop_listening()
        self._pin.init(self._pin.OUT)
        link = esp32.RMT(0, pin=self._pin, idle_level=False, clock_div=200)
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
        self.ack_state() #clear the ack state
        link.write_pulses(pulse_train,1)
        while not link.wait_done():
            ...
        link.deinit()
        self._start_listening()  
  
    def _handle_irq(self, e: Pin) -> None:
        t = ticks_us()
        delta = ticks_diff(t,self._ref_time)
        self._ref_time = t
        
        if delta > 1680 and delta < 2625: #2100 µs +/- 20%
            self._raw = (1 << 15) | (self._raw >> 1)
            self._bits_read += 1
        elif delta > 840 and delta < 1313: #1050 µs +/- 20%
            self._raw = self._raw >> 1
            self._bits_read += 1
        else:
            self._bits_read = 0
            
        #Push raw data to a ring buffer if we counted 16 bits.
        if self._bits_read == 16:
            self._rx_count += 1
            self._buffer[self._buffer_writer] = self._raw
            self._buffer_writer += 1
            if self._buffer_writer >= self._buffer_size:
                self._buffer_writer = 0
            if self._buffer_writer == self._buffer_reader: #push the reader forward if the buffer is full
                self._rx_buffer_overflow += 1
                self._buffer_reader += 1
                if self._buffer_reader >= self._buffer_size:
                    self._buffer_reader = 0
            self._bits_read = 0     
    

    
    def _process_buffer(self):
        """
        Process the raw buffer and store valid messages in the message buffer.
        Also sets the ACK flag if an ack message was found
        """
        while self._buffer_reader != self._buffer_writer:
            packet = DataPacket(self._buffer[self._buffer_reader])
            self._buffer_reader += 1
            if self._buffer_reader >= self._buffer_size:
                self._buffer_reader = 0 
            if packet.calculate_crc() == packet.crc:
                if packet.command == Command.ack:
                    self._ack = True
                else:
                    self._messages.append(packet)
                    while len(self._messages) >= self._buffer_size:
                        self._message_buffer_overflow += 1
                        self._messages.pop(0)
            else:
                self._crc_fails += 1
    
        
            
    def _start_listening(self) -> None:
        """
        Start listening on the defined hardware pin for data
        """
        self._bits_read = 0 
        self._pin.init(self._pin.IN, self._pin.PULL_UP)
        self._pin.irq(trigger=Pin.IRQ_RISING, handler=self._handle_irq)
        
    def _stop_listening(self) -> None:
        if self._pin:
            self._pin.irq(handler=None)
        
   
    

        
class Blaster():
    def __init__(self):
        self._blaster_link = Reader(4, can_transmit=True)
        self._ir_link = Reader(25)
        self._team = Team.none #None is not frozen
    
    def set_channel(self, channel_id:int):
        """
        Sets the IR channel (0..15).
        Only blasters on the same IR channel can communicate via IR.
        """
        if channel_id < 0 or channel_id > 15: return False
        p = DataPacket(0)
        p.team = Team.none
        p.trigger = False
        p.command = Command.channel
        p.parameter = channel_id
        self._blaster_link.transmit_packet(p)
        
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def set_trigger_action(self, stealth=False, single_shot=False, healing=False, disable=False):
        """
        Sets various trigger action flags
        """
        p = DataPacket(0)
        p.team = Team.none
        p.trigger = False
        p.command = Command.trigger_action
        
        param = 0
        if disable: param += 1
        if healing: param += 2
        if single_shot: param += 4
        if stealth: param += 8
        p.parameter = param
        
        self._blaster_link.transmit_packet(p)
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def set_team(self, team:int):
        """
        Sets and locks the blaster team
        """
        self._team = team
        p = DataPacket(0)
        p.team = self._team
        p.trigger = False
        p.command = Command.team_change
        p.parameter = 0
        self._blaster_link.transmit_packet(p)
        
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def set_game_mode(self, mode:int, team: int = 0):
        """
        Set game mode and optionally the team.
        team:0 means let the blaster decide
        """
        self._team = team
        p = DataPacket(0)
        p.team = self._team
        p.trigger = False
        p.command = Command.game_mode
        p.parameter = mode
        
        self._blaster_link.transmit_packet(p)
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def got_hit(self, team: int):
        """
        Test function: pretend that the blaster got hit by team
        """
        p = DataPacket(0)
        p.team = team
        p.trigger = True
        p.command = Command.shoot
        p.parameter = 0
        
        self._blaster_link.transmit_packet(p)
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def got_healed(self, team: int):
        """
        Test function: pretend that the blaster got healing by team
        """
        p = DataPacket(0)
        p.team = team
        p.trigger = True
        p.command = Command.heal
        p.parameter = 0
        
        self._blaster_link.transmit_packet(p)
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def play_animation(self, animation:int):
        p = DataPacket(0)
        p.team = Team.none
        p.trigger = False
        p.command = Command.animation
        p.parameter = animation
        
        self._blaster_link.transmit_packet(p)
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def start_chatter(self):
        p = DataPacket(0)
        p.team = Team.none
        p.trigger = False
        p.command = Command.chatter
        p.parameter = 9
        
        self._blaster_link.transmit_packet(p)
        sleep(.1)
        return self._blaster_link.ack_state()
        
    def settings(self, mute:bool=False, brightness:int=7):
        ...
        
    def forward_ir_shot(self):
        p = self._ir_link.read_packet()
        if not p: return
        if not p.command in[Command.heal, Command.shoot]: return
        self._blaster_link.transmit_packet(p)
        
    def get_blaster_shot(self):
        p = self._blaster_link.read_packet()
        if not p: return
        if not p.command in[Command.heal, Command.shoot]: return
        return p
        
    def log(self):
        while(True):
            if p := self._blaster_link.read_packet():
                print("BLASTER:", p)
            if p := self._ir_link.read_packet():
                print("IR:", p)                 
        
blaster = Blaster()


        



