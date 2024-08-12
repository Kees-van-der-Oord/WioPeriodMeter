# Wio Period Meter
The Wio Period Meter shows the period of CMOS/TTL pulses. The console is a [Seeed Wio Terminal](https://wiki.seeedstudio.com/Wio_Terminal_Intro/) programmed with the [Arduino](https://www.arduino.cc/en/software) sketch [WioPeriodMeter.ino](WioPeriodMeter/WioPeriodMeter.ino).

![WiePeriodMeter](https://github.com/user-attachments/assets/55607441-9674-44df-a68b-a0ad4a5af4ed)

The terminal gets power through the USB-C connector. The BNC cable is connected to white cable of the right [Grove](https://wiki.seeedstudio.com/Grove_System) socket (D1, pin 15).

When pulses are detected (falling edge), the fan on the bottom starts rotating and the period between the pulses is shown on the screen.

The USB-C link virtual COM port (baud-rate 115200 bps) allows reading the period from software (sending any character will return the last recorded period in microseconds). This requires [the driver](https://github.com/Seeed-Studio/Signed_USB_Serial_Driver) to be installed first.
