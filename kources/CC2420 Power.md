## Conversion map between PA_LEVEL and output power in dBm
###(from table 9 in CC2420 specification).

### Source File:
dev/cc2420/cc2420.c:105
 ___________________________________
|			|		|				|
|	dBm		| Level | Register Val	|
|___________|_______|_______________|
|	  0		|	31	|	0xff		|
|	 -1		|	27	|	0xfb		|
|	 -3		|	23	|	0xf7		|
|	 -5		|	19	|	0xf3		|
|	 -7		|	15	|	0xef		|
|	-10		|	11	|	0xeb		|
|	-15		|	 7	|	0xe7		|
|	-25		|	 3	|	0xe3		|
|___________________________________|


|  dBm |  Level |  Register Val |
|---|---|---|
|   |  31 |  0xff |
|   |  27 |  0xfb |
|   |  23 |  0xf7 |
|   |  19 |  0xf3 |
|   |  15 |  0xef |
|   |  11 |  0xeb |
|   |  7 |  0xe7 |
|   |  3 |  0xe3 |