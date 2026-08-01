# Sky Alarm
The sky alarm from xkcd #2979! It tells you when cool space things are happening. 

<img width="642" height="392" alt="skyalarm_cropped" src="https://github.com/user-attachments/assets/b0c12726-d240-4eca-b810-b1fe5009f9c6" />
<img width="507" height="499" alt="Screenshot 2026-07-31 at 3 04 06 PM" src="https://github.com/user-attachments/assets/096757a7-9d08-4bc9-a55b-3b7e728e6caf" />

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

<img width="507" height="499" alt="Screenshot 2026-07-31 at 3 04 06 PM" src="https://github.com/user-attachments/assets/82a4bff8-f25e-4718-bbe2-3aa34a63140c" />

<img width="572" height="559" alt="Screenshot 2026-07-31 at 2 35 29 PM" src="https://github.com/user-attachments/assets/cbee3306-3d05-4486-9749-b59baf72543c" />

<img width="532" height="539" alt="Screenshot 2026-07-31 at 2 35 40 PM" src="https://github.com/user-attachments/assets/9f06400d-90c6-44ce-98d9-99742f0ec45c" />

<img width="607" height="303" alt="Screenshot 2026-07-31 at 2 35 59 PM" src="https://github.com/user-attachments/assets/b01c22d9-6281-48fd-842c-129c669178f2" />

<img width="1410" height="2000" alt="zine" src="https://github.com/user-attachments/assets/1f0031af-6447-4c77-ad7f-df633335c437" />

