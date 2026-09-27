Note from Dave L Clark about loading the drivers, from an email-conversation around 2009.

The documentation for Sopwith itself does sheds some light on a configuration the game is meant to
be played with, but this can be of use if anyone wants to take use of these to do other simple
networking-things on a group of DOS-based PCs.

-----------------------------------------------------------------------------------------------------------

Hi

I'm sorry, these are from ancient history, and I don't have any documentation on them any more.
If I recall, namedev did not require any parameters, but Serial.sys certainly would (port, baud
rate, etc). Loading it in the config sys with something like -? should force a display of allowable
parameters. Fromm a hex dump of the driver, I would guess:
 
-c<port>  (eg: 1,2,...)
-b<baudrate> (eg: 9600)
-p<parity> (eg e,o,n for even, odd, none) 
-d<databbits> (eg 8,7,6,5)
-s<stopbits> (eg: 1,2)
-i<inputbuffersize> (eg: 512)
-o<outputbuffersize> (eg: 512)
-h<handshake> (eg h (hardware))
-t (no idea what this is)
-k (no idea)
-r<number> (no idea)
-a<device-name> (I believe this was uses to force a driver name, probably not needed)
 
An example from the dump of the driver:
-c1 -b50  -pe -d5 -s1 -i512 -o512 -hh -t -k -r1 -a<device-name>
 
Dave
