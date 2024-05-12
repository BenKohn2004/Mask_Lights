# Mask Lights #

This project is a simple and inexpensive method of relaying the Red and Green scoring lights from a [Favero Full-Arm-05](https://www.favero.com/en2_fencing_sport_electronic_scoreboard_fencing_signalling_equipment_apparatus_for_foil_sword_epee_sabre-29-17.html) scoring machine to LED lights attached to the interior sides of a fencing mask.

## Parser and Transmitter ##

The output data from the Favero using an [RJ11](https://www.aliexpress.us/item/2251832602517821.html) Connector to an [Arduino ESP8266 WeMos D1 Mini](https://www.aliexpress.us/item/3256805831695231.html) using the wiring [Favero Serial Parser schematic](Schematic_Favero-Light-Relays.pdf) with the exception that the LED Relay Control was left off for this build. The Arduino code [8266 Parser and Transmitter Favero](8266_Parser_and_Transmitter_Favero) was then uploaded the Wemos. The only change that should be needed to the code is line 4, "Favero_One", which is the scoring machine's name. The code should function either way, but it will be easier to use if each scoring machine has a unique name. The code uses the Wemos' MAC address to differentiate between scoring machines, but the human interface uses the name defined by line 4.

## Receiver ##

The [circuit for the LED Lights Receiver](Schematic_Mask_Lights_PCB.pdf) was then designed and the [PCB](PCB_Top.PNG) ordered. Some of the parts used where an [OLED SSD1306](https://www.aliexpress.us/item/3256805784586367.html), a [MH-CD42 Charging Module](https://www.aliexpress.us/item/2255799917197675.html), [stout 15mm tall push buttons](https://www.aliexpress.us/item/2251832725948381.html), [3mm LEDs](https://www.aliexpress.us/item/3256805857472652.html), a [power switch](https://www.aliexpress.us/item/3256803752541650.html), a [LiPo 3.7V battery](https://www.aliexpress.us/item/3256806371239803.html) and an [8 by 8 LED Matrix](https://www.aliexpress.us/item/3256801079332896.html). A couple of notes on the PCB design, it is functional but by no means optimized and there are a couple of errors. Specifically the Green_Red_Button connects to D0/GPIO16 which is internally pulled down so the button, which is connected to GND, should instead be connected to 3.3V and therefore the button will not function. The Arduino code is adapted to not use the left Green_Red_Button. Also, the Battery connection markings are reversed and the + and - are marked backwards from what they actually are on the circuit. If you connect the battery as indicated, as one would reasonably do, the PCB will start to heat up and smoke. There are also slightly different designs of the SSD1306, one of the has the Vcc and GND reversed from the one used in the PCB design, I would recommend using the SSD1306 that lines up with the PCB. Similarly, the pins for the power switch also line up poorly, so they required mechanical inducement to fit, though it does make switch feel more snug in the PCB.

![PCB_Top.PNG](https://github.com/BenKohn2004/Mask_Lights/blob/main/PCB_Top.PNG)

The PCB is stored in a [3D printed enclosure](https://cad.onshape.com/documents/9127cf46288eeb6dc56328e9/w/f2f425a857582dd788254ce1/e/b9644c77b763d9ce140f913f?renderMode=0&uiState=6640f95af08a9a25bbff4b10) and secured to the mask using [velcro straps](https://www.aliexpress.us/item/3256806755948519.html). The LED matrix was connected to the mask using [lockwire](https://www.aliexpress.us/item/3256803467852184.html), though there may be better alternatives and I worry about sharp edges despite dulling the edge afterwards. I use [magnetic usb chargers](https://www.aliexpress.us/item/3256805633947608.html) to protect the Wemos USB connection and hopefully prolong its life.

## Relay Receiver ##

I included the code [8266_Relay_Receiver](8266_Relay_Receiver). The code acts similarly to the mask receiver except a High/Low signal is passed using the same receiver PCB at the Green and Red LED +/- connections. This lets someone to connect relays to the PCB and actuate repeater lights using the same platform, enclosures and boards.

## Future Development ##

Some thoughts going forward, all the data from the box, mostly, is transmitted and the 8 by 8 matrix uses individually addressable LEDs, so more can be conveyed than solely the score lights. For example there could be indication of the score or a count down when the timer is about to run expire.

This can of course can be adapted to other scoring machines as well. It would be reasonable to use the other repositories on this Github to transmit both St. George Machines and the Virtual Scoring Machine.
