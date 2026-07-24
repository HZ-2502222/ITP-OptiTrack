# ITP-OptiTrack

ITP-OptiTrack is a hardware and software integration project for automated image capture, OCR, and QR code scanning. It combines an Arduino-based capture system with a Python processing backend and a local web dashboard for viewing logs and database entries.

## Features

- Hardware image capture with the ArduCAM Mini 5MP (OV5642)
- Proximity triggering using an ultrasonic sensor
- Wireless communication through an HC-05 Bluetooth module
- Automated image processing through Python scripts for capture, ROI handling, OCR, and QR decoding
- A web dashboard for viewing system status, logs, and stored records

## Repository Structure

```text
ITP-OptiTrack/
├── Arduino/
│   ├── ArduCAM_Mini_5MP_OV5642_Plus_Functions/
│   │   └── ArduCAM_Mini_5MP_OV5642_Plus_Functions.ino
│   └── HC05_Bluetooth_Module/
│       └── HC05_Bluetooth_Module.ino
├── captures/
├── itp_dashboard/
│   ├── app.py
│   ├── database.json
│   ├── guide.txt
│   ├── static/
│   │   └── style.css
│   └── templates/
│       └── index.html
├── logs/
│   ├── ocr_log.txt
│   └── qr_log.txt
├── scanner/
│   ├── capture.py
│   ├── read_ocr.py
│   ├── read_qr.py
│   └── roi.py
├── main.py
└── requirements.txt
```

## Hardware Requirements

- Arduino Uno R3 with USB-C, or a compatible microcontroller
- ArduCAM Mini 5MP Plus (OV5642) SPI camera module
- Ultrasonic sensor such as HC-SR04
- HC-05 Bluetooth module
- 3 LEDs: green, yellow, and red
- Appropriate resistors and wiring components
- Breadboard, jumper wires, and a suitable power supply or USB-C cable

## Software Setup

### Arduino

Before uploading the sketches, make sure your Arduino IDE is configured with the required libraries.

- Install the ArduCAM library, version 4.0.0 or later.
- Check that the ArduCAM `memorysaver.h` settings match the OV5642 module.
- Confirm that `Wire`, `SPI`, and `SoftwareSerial` are available.

#### ArduCAM and Sensor Sketch

1. Open `Arduino/ArduCAM_Mini_5MP_OV5642_Plus_Functions/ArduCAM_Mini_5MP_OV5642_Plus_Functions.ino`.
2. Verify the wiring:
   - Camera CS to digital pin 7.
   - Ultrasonic sensor Trig to digital pin 3 and Echo to digital pin 2.
   - Green LED to pin 4, yellow LED to pin 5, and red LED to pin 6.
3. Select the correct board and COM port in the Arduino IDE.
4. Upload the sketch.
5. Open Serial Monitor at 115200 baud to view initialization messages and distance readings.

#### HC-05 Bluetooth Sketch

1. Open `Arduino/HC05_Bluetooth_Module/HC05_Bluetooth_Module.ino`.
2. Verify the wiring:
   - HC-05 TXD to Arduino pin 9.
   - HC-05 RXD to Arduino pin 8.
3. Upload the sketch.
4. For AT mode, open Serial Monitor at 9600 baud with Both NL & CR selected.
5. The default AT baud rate is 38400, and the default password is 1234.

### Python

1. Install Python 3.x.
2. Clone this repository.
3. Open a terminal in the project root.
4. Install the Python dependencies:

```bash
pip install -r requirements.txt
```

You may also need OCR system dependencies such as Tesseract-OCR, depending on your operating system.

## Usage

### Run the Core System

From the repository root:

```bash
python main.py
```

This starts the scanner pipeline, processes captures, and writes QR and OCR output to the log files.

### Run the Dashboard

Start the dashboard from the `itp_dashboard` directory so the relative paths resolve correctly:

```bash
cd itp_dashboard
python app.py
```

Open the local address shown in the terminal, usually `http://localhost:5000`.

## Notes

- QR and OCR reads are stored in `logs/`.
- The dashboard reads from the log files and updates `database.json`.