# Sky Alarm
The sky alarm from xkcd #2979! It tells you when cool space things are happening. 

<img width="642" height="392" alt="skyalarm_cropped" src="https://github.com/user-attachments/assets/b0c12726-d240-4eca-b810-b1fe5009f9c6" />
<img width="558" height="530" alt="SS 2026-10-10 at 1 56 14 PM" src="https://github.com/user-attachments/assets/0cc05467-5621-480e-9ff6-c47808edeb64" />


## Why 
<img width="664" height="605" alt="Sky Alarm" src="https://imgs.xkcd.com/comics/sky_alarm_2x.png" />
I also wanted this device. There are alert apps and telegram groups and RSS feeds out there, but I wanted something not tied to my phone or computer. I also thought it would be fun to make a large physical device in the style of the original comic. 

## Features 
- Siren turns on (sound and light) when a cool space thing is happening in your location 
- Only alerts for phenomena observable with the naked eye 
- Cool space things:
  - Lunar eclipses
  - Solar eclipses
  - Supermoons
  - ISS passes
- Configurable notification settings 

## Usage 
2. Strip a cable (USB-C to USB-C or USB-A to USB-C; if using the latter, strip the USB-C end).
3. Locate the power and ground cables.
4. Secure the power and ground cables to their corresponding screw terminals (labeled on the PCB).
5. Get two lengths of wire. Connect one end of each wire to the battery terminals of the siren. (You may need to disassemble or drill into the casing to access this)
6. Connect the remaining end of each wire to the siren screw terminals (labeled on the PCB).
7. Put in separate batteries for the siren. Make sure the switch is turned ON.
8. Plug in any USB-C to USB-C wall charger to the XIAO ESP32C3.
9. Plug in the external power cable. 

## Specifications 
- XIAO ESP32C3 microcontroller - edit firmware without any extra equipment
- MAX98357A - cheap and widely available amplifier 
- Any toy siren ~5V with an on-off switch and batteries should work
- 61.4 x 65.5 mm PCB - casing can be made smaller with different parts without adjusting the board
- Runs on any wall power supply 1A-3A (suggested) 
- bom can be found as bom.csv 

## Upcoming features 
- Weather checks - no alerts if overcast
- Description page for details about alerted event 
- Custom audio 
- Switching from API calls to local calculations for more predictable events 
- More cool space things!

## Images 
<img width="1284" height="784" alt="skyalarm_cropped" src="https://github.com/user-attachments/assets/b0c12726-d240-4eca-b810-b1fe5009f9c6" />

<img width="558" height="530" alt="SS 2026-10-10 at 1 56 14 PM" src="https://github.com/user-attachments/assets/832735fd-bd39-4410-8ffe-481fa62378b7" />
<img width="528" height="536" alt="SS 2026-10-10 at 1 57 04 PM" src="https://github.com/user-attachments/assets/7d4e4dd9-544d-42c8-a6ca-31db8c653091" />
<img width="689" height="717" alt="SS 2026-10-10 at 1 57 40 PM" src="https://github.com/user-attachments/assets/46ea8e88-c944-40ae-b213-f3a2e58b16d7" />



<img width="607" height="303" alt="Screenshot 2026-07-31 at 2 35 59 PM" src="https://github.com/user-attachments/assets/b01c22d9-6281-48fd-842c-129c669178f2" />

<img width="1409" height="2000" alt="NEW zine - skyalarm" src="https://github.com/user-attachments/assets/080a9e01-a821-4185-9e3c-daf91458945d" />


## Credits 
Inspiration from [https://xkcd.com/2979/](https://xkcd.com/2979/) 

Font from [https://github.com/ipython/xkcd-font](https://github.com/ipython/xkcd-font) 

<img width="264" height="227" alt="Screenshot 2026-07-31 at 7 51 07 PM" src="https://github.com/user-attachments/assets/29e8c750-c40a-464b-afc5-70a1428c2aa7" />
