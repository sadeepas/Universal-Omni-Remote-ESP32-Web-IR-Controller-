# Universal-Omni-Remote-ESP32-Web-IR-Controller-
A modern, glassmorphic web interface hosted on an ESP32 to control TVs and media devices via Infrared. Features multi-room support, brand memory, and an automated code scanner. No app required—just a browser.


# Universal Omni-Remote 📺✨

A stylish, web-based universal remote control powered by an ESP32. This project hosts a responsive web app (React-style UI without the bloat) directly on the microcontroller, allowing you to control TVs, Soundbars, and Media Players from any smartphone connected to the network.


## 🌟 Features

*   **Glassmorphism UI:** A beautiful, dark-mode interface built with Tailwind CSS.
*   **Auto Scan:** Automatically cycles through common power codes to find the right frequency for your TV.
*   **Multi-Room Support:** Save different brands for different rooms (Living Room, Bedroom, Office). The ESP32 remembers your settings even after a reboot.
*   **Comprehensive Controls:** Power, Volume, Channel, D-Pad, Numpad, and Media controls.
*   **Broad Compatibility:** Supports Samsung, LG, Sony, Panasonic, Philips, Sharp, TCL, Vizio, and generic NEC protocols.
*   **Haptic Feedback:** Vibrates your phone on button press (mobile only).

## 🛠️ Hardware Required

1.  **ESP32 Development Board** (NodeMCU-32S or similar).
2.  **IR LED** (940nm).
3.  **NPN Transistor** (2N2222 or BC547) - *Recommended for better range.*
4.  **Resistor** (220Ω).
5.  Breadboard and jumper wires.

### Wiring Diagram
*   **IR LED Anode (+):** Connect to ESP32 **GPIO 4**.
*   **IR LED Cathode (-):** Connect to GND (via resistor/transistor circuit).

> **Note:** Direct connection to the GPIO pin works for short range, but a transistor driver circuit is highly recommended for room-wide coverage.

## 💾 Software Dependencies

This project is built using **PlatformIO** or **Arduino IDE**. You will need the following libraries:

1.  **[IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266)** - For sending IR signals.
2.  **[ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)** - To serve the UI.
3.  **[AsyncTCP](https://github.com/me-no-dev/AsyncTCP)** - Dependency for the web server.

## 🚀 Installation & Setup

1.  **Clone the Repo:**
    ```bash
    git clone https://github.com/yourusername/universal-omni-remote.git
    ```

2.  **Configure Network:**
    Open the `.ino` file and configure your WiFi settings.
    *   *Default Mode:* Access Point (AP).
    *   *SSID:* `Universal-Remote`
    *   *Password:* `12345678`
    
    *Tip: For the best UI experience (loading Tailwind/FontAwesome icons), change the code to connect to your home WiFi router (Station Mode) instead of creating its own hotspot.*

3.  **Upload the Code:**
    Flash the firmware to your ESP32 using Arduino IDE or PlatformIO.

4.  **Upload the Interface:**
    *   **Option A (SPIFFS):** Upload the `uni_remote.html` to the ESP32 file system and ensure your code routes `/` to serve this file.
    *   **Option B (Inline):** Convert the HTML content into a raw C++ string and serve it directly in the `server.on("/", ...)` handler.

## 📱 Usage

1.  Power up the ESP32.
2.  Connect your phone to the WiFi network (AP or Home Router).
3.  Navigate to the IP address displayed in the Serial Monitor (Default AP IP: `192.168.4.1`).
4.  **Select a Room** (e.g., Living Room).
5.  **Select a Brand** manually, OR click **AUTO SCAN**.
    *   *Auto Scan:* Point the IR LED at the TV. The remote will cycle through power codes. Press **STOP** immediately when the TV turns on/off. The code is now saved for that room.

## ⚠️ Important Note on UI Assets

The HTML file uses **CDN links** for Tailwind CSS and FontAwesome icons to keep the file size small. 
*   If you run the ESP32 in **AP Mode** (Hotspot) and your phone disconnects from mobile data, the styling and icons might not load. 
*   **Recommendation:** Connect the ESP32 to your home WiFi router so the client device retains internet access to fetch the styles.

## 🤝 Contributing

Pull requests are welcome! If you want to add more IR protocols or refine the UI, feel free to fork the project.

## 📄 License

Distributed under the MIT License.
