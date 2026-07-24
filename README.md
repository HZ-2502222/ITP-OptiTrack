ITP-OptiTrack
Welcome to the ITP-OptiTrack project repository. This project is a comprehensive hardware and software integration designed for automated image capture, Optical Character Recognition (OCR), and QR code scanning. It leverages Arduino microcontrollers equipped with camera and Bluetooth modules, pairing them with a Python-based processing backend and a web dashboard for data visualization.

🚀 Features
Hardware Image Capture: Utilizes the ArduCAM Mini 5MP (OV5642) for high-quality image capture.

Wireless Communication: Integrates an HC-05 Bluetooth module for wireless data transmission/triggering.

Automated Image Processing: Python scripts dedicated to capturing images, defining Regions of Interest (ROI), and processing visual data.

OCR & QR Decoding: Built-in support for extracting text (OCR) and decoding QR codes from captured images.

Web Dashboard: A local web application (likely Flask-based) to view logs, database entries, and system status in real-time.

📁 Repository Structure
Plaintext
ITP-OptiTrack/
├── Arduino/
│   ├── ArduCAM_Mini_5MP_OV5642_Plus_Functions/
│   │   └── ArduCAM_Mini_5MP_OV5642_Plus_Functions.ino  # Camera control logic
│   └── HC05_Bluetooth_Module/
│       └── HC05_Bluetooth_Module.ino                   # Bluetooth communication logic
├── captures/
│   └── capture.jpg                                     # Temporary/latest image capture storage
├── itp_dashboard/
│   ├── app.py                                          # Web dashboard application entry point
│   ├── database.json                                   # Local JSON database for dashboard data
│   ├── guide.txt                                       # Dashboard usage guidelines
│   ├── static/
│   │   └── style.css                                   # Dashboard styling
│   └── templates/
│       └── index.html                                  # Dashboard frontend template
├── logs/
│   ├── ocr_log.txt                                     # OCR processing history
│   └── qr_log.txt                                      # QR code scanning history
├── scanner/
│   ├── capture.py                                      # Image capture interfacing script
│   ├── read_ocr.py                                     # OCR processing module
│   ├── read_qr.py                                      # QR code decoding module
│   └── roi.py                                          # Region of Interest (ROI) extraction
├── main.py                                             # Main Python application runner
└── requirements.txt                                    # Python dependencies
🛠️ Hardware Requirements
To fully utilize this repository, you will need the following hardware components:

Arduino Board (e.g., Uno, Mega, or Nano)

ArduCAM Mini 5MP Plus (OV5642) SPI Camera Module

HC-05 Bluetooth Module

Connecting wires and appropriate power supply

💻 Software Setup & Installation
1. Hardware Setup (Arduino)
Open the Arduino IDE.

Navigate to the Arduino/ directory in this repository.

Flash the ArduCAM_Mini_5MP_OV5642_Plus_Functions.ino and HC05_Bluetooth_Module.ino sketches to your respective Arduino setups. Ensure you have the necessary ArduCAM libraries installed in your Arduino IDE.

2. Software Setup (Python)
Ensure you have Python 3.x installed on your system.

Clone this repository to your local machine.

Open a terminal and navigate to the root directory of the project.

Install the required dependencies using pip:

Bash
pip install -r requirements.txt
(Note: You may also need to install system-level dependencies for OCR, such as Tesseract-OCR, depending on your OS).

⚙️ Usage
Running the Core System
To start the main tracking and scanning application, run:

Bash
python main.py
This will initialize the scanner, listen for incoming captures, process ROIs, and execute the QR/OCR reading modules.

Launching the Dashboard
To view the captured data and logs via the web interface, navigate to the itp_dashboard directory and start the web app (assuming standard Python web framework execution):

Bash
cd itp_dashboard
python app.py
Open your web browser and navigate to http://localhost:5000 (or the port specified in the terminal output) to view the dashboard. All QR and OCR reads are logged in the logs/ directory and visualized here.