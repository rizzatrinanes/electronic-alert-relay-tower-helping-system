# Electronic Alert Relay Tower Helping System (EART-HS)

EART-HS is an ESP32-based emergency alert prototype.

The current repository contains an ESP32-CAM program that connects to
Wi-Fi, sends an emergency email, captures images, stores them on a
microSD card, and controls indicator LEDs during the process.

## Current Implementation

The main implemented firmware is:

``` text
EARTHS_ESP32_CAM.ino
```

The repository also contains:

``` text
EARTHS_Transmitter.ino
EARTHS_Receiver.ino
```

These two files are currently empty in this version of the repository.

## Features

-   ESP32-CAM initialization
-   Wi-Fi connection
-   Emergency email notification
-   Camera image capture
-   MicroSD storage
-   Image numbering using EEPROM
-   Flash and status LED control
-   Email-sending status indication
-   Automatic restart after the operation is completed

## Hardware / Libraries Used

The firmware uses the ESP32 camera stack and Arduino libraries/APIs for:

-   ESP32-CAM (AI Thinker camera pin configuration)
-   Wi-Fi
-   Camera
-   MicroSD
-   EEPROM
-   SPI
-   SMTP email
-   GPIO / LEDs

## How It Works

When the ESP32-CAM starts:

1.  It initializes the GPIO pins.
2.  It connects to the configured Wi-Fi network.
3.  It initializes the SMTP session.
4.  It restores the picture counter from EEPROM.
5.  It initializes the camera.
6.  It mounts the microSD card in 1-bit mode.
7.  It sends an emergency email.
8.  It captures pictures for approximately 20 seconds.
9.  The pictures are saved to the microSD card.
10. The flash LED is turned off.
11. After the operation is complete, the device waits for approximately
    one minute and restarts.

## Email Alert

The firmware sends an email with the subject:

``` text
EMERGENCY ALERT
```

The message identifies the emergency location configured in the
firmware.

SMTP is configured for Gmail on port `465`.

## Image Storage

Captured images are stored on the microSD card using filenames such as:

``` text
/picture1.jpg
/picture2.jpg
/picture3.jpg
```

The picture number is stored in EEPROM so the counter can continue after
a restart.

## Camera

The code uses the pin configuration for the:

``` text
AI Thinker ESP32-CAM
```

When PSRAM is available, the firmware uses a higher camera frame size
and two frame buffers. Otherwise, it falls back to a lower frame size.

## Project Files

``` text
electronic-alert-relay-tower-helping-system-main/
├── EARTHS_ESP32_CAM.ino
├── EARTHS_Transmitter.ino
└── EARTHS_Receiver.ino
```

## Setup

Open `EARTHS_ESP32_CAM.ino` in Arduino IDE and select the appropriate
ESP32-CAM board configuration.

Before uploading, configure the Wi-Fi and email settings for your own
environment.

Do not commit Wi-Fi passwords, email passwords, SMTP credentials, or
other private credentials to a public repository.

## Security Note

The original project file contains hard-coded Wi-Fi and SMTP
credentials. These should be removed or replaced before publishing the
source code to a public GitHub repository.

If the credentials in the uploaded version are real and still active,
rotate/change them before making the repository public.

## Project Status

Prototype / Academic Project
