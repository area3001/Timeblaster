# Micropython interface

This is a high level overview of the Blaster class.
This class hides the lower level Reader interface and makes communication a bit easier.

1) create an instance of the blaster class.    
   This will "use" the badge and IR pins. You should avoid making more than one instance of this class.
```python
blaster = Blaster()
```

2) Set the IR channel
   channel_id is an integer between 0 and 15. Only blasters on the same IR channel can communicate over IR. Default=0   
           
   Returns True if success else False.

```python
blaster.set_channel(channel_id)
```   

3) Set trigger action   
   This command allows you to set 4 flags   
   * stealth  
     LED's go to lowest brightness until being shot or fired
   * single_shot   
     The blaster can shoot 1 time before being disabled.
   * healing   
     When this is set the blaster sends out a healing pulse instead of a damage pulse. 
   * disable   
     disables the blaster trigger button   

   All options are false by default

```python
blaster.set_trigger_action(stealth=True|False, single_shot=True|False, healing=True|False, disable=True|False)
```   

4) Set Team color.   
   The blaster has 7 teams to play with. 3 of them can be set by the hardware swithch.
   All 7 can be set via the set team color command

```python
blaster.set_team(Team.rex)
#team can be rex, giggle, buz, yellow, magenta, azure and white
```

5) Set Game Mode.

6) got hit

7) got healed

8) forward ir packet

9) play animation

10) start chatter