# Wio Period Meter
The Wio Period Meter shows the period of CMOS/TTL pulses. The console is a [Seeed Wio Terminal(https://wiki.seeedstudio.com/Wio_Terminal_Intro/)] programmed with the [Arduino(https://www.arduino.cc/en/software)] sketch WioPeriodMeter.ino.

![WiePeriodMeter](https://github.com/user-attachments/assets/55607441-9674-44df-a68b-a0ad4a5af4ed)

The terminal gets power through the USB-C connector. The BNC cable is connected to red line of the right Groove socket (D1, pin 15).

When pulses are detected (falling edge), the fan on the bottom starts rotating and the period between the pulses is shown on the screen.
