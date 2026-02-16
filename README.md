Code was written for STM32F303RE / STM32F3xx, but is also compatible with other STM32 microcontrollers.
<br><br>

## 📺 SSD1306 OLED Display (I2C)

The SSD1306 is a compact monochrome OLED display widely used in embedded systems for debugging, telemetry, and user interfaces.  
It communicates via I2C or SPI and provides sharp, high-contrast output with very low power consumption.

This module is ideal for robotics dashboards, STM32 debugging output, and real-time sensor visualization.
<br><br>

## 🔧 Key Features
- 128×64 monochrome OLED display<br>
- SSD1306 controller IC<br>
- I2C interface (default) or SPI support<br>
- High contrast and wide viewing angle<br>
- Low power consumption<br>
- No backlight required (self-emissive pixels)
<br><br>

## ⚙️ Electrical Characteristics
- Supply Voltage: 3.3V – 5V<br>
- Logic Level: 3.3V compatible<br>
- Interface: I2C (SCL, SDA)<br>
- I2C Address: 0x3C or 0x3D (depending on module)<br>
- Resolution: 128×64 pixels
<br><br>

## 📡 STM32 Integration
Fully compatible with STM32 MCUs including:<br>
STM32F303RE, STM32F333xx<br>
Any STM32 with I2C peripheral support
<br><br>

Typical wiring (I2C mode):<br>
VCC → 3.3V<br>
GND → GND<br>
SCL → I2C_SCL (e.g., PB6 / PB8)<br>
SDA → I2C_SDA (e.g., PB7 / PB9)
<br><br>

## 🧠 Software Support
Works with:
- STM32 HAL drivers<br>
- STM32CubeIDE projects<br>
- Simulink generated code (I2C blocks)<br>
- Lightweight SSD1306 libraries
<br><br>

Common features:
- Text rendering (ASCII fonts)<br>
- Pixel drawing<br>
- Lines, rectangles, bitmaps<br>
- Real-time telemetry display
<br><br>

## 🧪 Typical Use Cases
- Drone telemetry display (battery, mode, link status)<br>
- Embedded debugging console<br>
- Sensor dashboards (IMU, temperature, altitude)<br>
- RC transmitter UI<br>
- Menu systems for robotics
<br><br>

## ⚠️ Notes
- Use pull-up resistors on SDA/SCL if not included (4.7kΩ typical)<br>
- Verify I2C address using scanner if display not detected<br>
- Keep I2C wires short to avoid noise issues<br>
- For STM32: enable Fast Mode (400 kHz) for smoother updates<br>
