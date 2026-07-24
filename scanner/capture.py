import serial
import serial.tools.list_ports
import time
import os
import keyboard

BAUD_RATE = 115200
SAVE_NAME = 'capture.jpg'

def get_arduino_port():
    print("Scanning for the connected port...")
    ports = serial.tools.list_ports.comports()
    
    # Look for your specific HC-05 unique hardware ID footprint
    for port in ports:
        if "0022120126BA" in port.hwid.upper():
            print(f"Found HC-05 device on {port.device}")
            return port.device
    
    # Fallback to general USB devices if Bluetooth isn't found
    for port in ports:
        desc = port.description.upper()
        if "ARDUINO" in desc or "CH340" in desc or "USB SERIAL" in desc or "CP210" in desc:
            print(f"Found USB device: {port.description} on {port.device}")
            return port.device
            
    raise ConnectionError("Could not automatically find the HC-05 Bluetooth port.")

def run_system():
    try:
        PORT = get_arduino_port() 
        print(f"Attempting connection to {PORT}...")

        # Initialize with a slightly higher timeout for Bluetooth latency
        ser = serial.Serial()
        ser.port = PORT
        ser.baudrate = BAUD_RATE
        ser.timeout = 2
        ser.open() # Explicitly open the port

        print(f"Successfully connected to {PORT}!")
        
        print("Synchronizing with Arduino...")
        ser.reset_input_buffer()
        
        # Give it a brief pause to catch the active telemetry stream
        time.sleep(2) 
        # Clear out any stale startup text trapped in the Bluetooth pipe
        ser.reset_input_buffer()

        # Send a software reset command to clear Arduino states wirelessly
        print("Sending reset command...")
        ser.write(b'Z')
        time.sleep(0.5)
        ser.reset_input_buffer()
        
        def trigger_estop():
            print("\n\n[EMERGENCY STOP] 'ESC' pressed! Halting system...")
            try:
                ser.write(b'\xEE') 
                ser.close()
            except:
                pass
            os._exit(0) 

        keyboard.add_hotkey('esc', trigger_estop)
        print(f"System armed on {PORT}. Press 'ESC' at any time to Emergency Stop.")
        
        while True:
            print("\n--- Waiting for Vehicle ---")
            
            stream_buffer = bytearray()
            
            # 1. HIGH SPEED TEXT LISTENING
            while True:
                if ser.in_waiting > 0:
                    chunk = ser.read(ser.in_waiting)
                else:
                    chunk = ser.read(1)
                    
                if not chunk:
                    continue
                    
                stream_buffer.extend(chunk)
                
                # Check for JPEG start header explicitly
                start_idx = stream_buffer.find(b'\xFF\xD8')
                if start_idx != -1:
                    print("\n[Triggered] Image incoming...")
                    # Keep everything from the JPEG start header onwards
                    img_data = bytearray(stream_buffer[start_idx:])
                    break
                    
                # If no header, parse and print the text lines cleanly
                while b'\n' in stream_buffer:
                    line_end = stream_buffer.find(b'\n')
                    line = stream_buffer[:line_end].decode('ascii', errors='ignore').strip()
                    
                    if line and all(32 <= ord(c) < 127 for c in line):
                        print(f"Arduino: {line}")
                        
                    stream_buffer = stream_buffer[line_end+1:]
            
            # 2. HIGH SPEED IMAGE RECEIVING
            print("Receiving stream...")
            start_time = time.time()
            while True:
                # 1. Primary check: The real JPEG end marker
                end_idx = img_data.find(b'\xFF\xD9', 1000)
                if end_idx != -1:
                    img_data = img_data[:end_idx + 2]
                    break
                    
                # 2. FALLBACK CHECK: Did the text logs bleed into the image buffer?
                fallback_idx = img_data.find(b'Capture success!')
                if fallback_idx != -1:
                    print("\n[Warning] Bluetooth dropped the JPEG end marker, but text boundary detected! Salvaging frame...")
                    # Search backwards to see if any valid end marker survived
                    last_valid = img_data.rfind(b'\xFF\xD9')
                    if last_valid != -1 and last_valid > 1000:
                        img_data = img_data[:last_valid + 2]
                    else:
                        img_data = img_data[:fallback_idx] # Strip the text off the corrupted end
                    break

                # Safety timeout: If Bluetooth hangs for more than 10 seconds, abort corrupt frame
                if time.time() - start_time > 10.0:
                    print("Error: Image transfer timed out over Bluetooth!")
                    img_data = None
                    break

                # Inhale incoming binary data quickly
                if ser.in_waiting > 0:
                    img_data.extend(ser.read(ser.in_waiting))
                else:
                    time.sleep(0.01) # Prevent CPU spinning if waiting for slow Bluetooth packets

            # If the image timed out or is too small to be a valid JPEG (< 100 bytes), skip saving
            if img_data is None or len(img_data) < 100:
                print("Skipping corrupted or empty frame capture.")
                continue

            if not os.path.exists('captures'):
                os.makedirs('captures')

            # If the image timed out or is too small to be a valid JPEG (< 100 bytes), skip saving
            if img_data is None or len(img_data) < 100:
                print("Skipping corrupted or empty frame capture.")
                continue
            
            # --- OVERWRITE BEHAVIOR ---
            new_filename = 'captures/capture.jpg'

            '''
            # Find the next available filename using a counter
            counter = 0
            while os.path.exists(f'captures/capture_{counter}.jpg'):
                counter += 1
            
            new_filename = f'captures/capture_{counter}.jpg'
            '''

            # Save the file (this automatically overwrites the old one)
            with open(new_filename, 'wb') as f:
                f.write(img_data)
            
            print(f"swee la '{new_filename}' saved! Size: {len(img_data)} bytes")

            yield new_filename

    except Exception as e:
        print(f"Connection error: {e}")

if __name__ == "__main__":
    run_system()