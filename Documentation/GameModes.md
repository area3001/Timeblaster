# Game Modes currently supported by the blaster

The blaster has 3 games modes that can be selected by sending the [Game Mode](DataFormat.md#game-mode) message frm the badge.

| Mode | ID | |
|---|---|---|
| Timeout | 0x1 | Default |
| Zombie | 0x2 | |
| Sudden Death | 0x3 | |

## Timeout
In this mode the player chooses its team by using the hardware team switch.
Teams that can be slected are: Rex, Giggle, Buzz. These and other teams can also be set with the [Team Change](DataFormat.md#team-change) command.

You can change teams mid game unless the team was set by the badge. This is to allow you to control the IRA2020 lighting on the camp site.

When you get shot by a enemy team (team with other color than your own) your blaster will start a timeout period of 5 seconds. In that time you can not shoot or be shot.   
When the timeout passed you can play as usual.

## Zombie
Zombie mode can be set with the [Team Change](DataFormat.md#team-change) command only.

In zombie mode your team is set by the blaster or message from the badge. If you set your team using the hardware switch it gets locked when shooting for the first time.

When you get shot by an enemy team (team with other color than your own) your blaster gets infected and becomes a zombie blaster. When you shoot you shoot using the color of the enemy team that infected you.   

Some Players can be designated as healers by a message from the badge. When a team mate with healing powers shoots you, you are cured.   
Healers can off course also be infected, when a healer gets infected they loose the healing power.

The game is over when all blaster are infected with the same team color.

## Sudden Death
In sudden death mode all blasters have one life. Once you get shot by an enemy blaster your blaster will stop working.

The game is over when only one team or blaster is left.