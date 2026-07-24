import keyboard
import os
from scanner.capture import run_system
from scanner.read_qr import decode_qr_from_image
from scanner.read_ocr import read_hubodometer_ocr

def main():
    print("--- Master Traffic Cam Controller Initialized ---")
    
    # Create the logs directory if it doesn't already exist
    if not os.path.exists('logs'):
        os.makedirs('logs')
        print("Created 'logs' directory.")
    
    try:
        # The capture script acts as a continuous generator. 
        for new_image_path in run_system():
            print("\n========================================")
            print(f"Processing new capture: {new_image_path}")
            
            # 1. Scan the image for QR Codes (Point to logs folder)
            qr_success = decode_qr_from_image(new_image_path, 'logs/qr_log.txt')
            
            # 2. ONLY run OCR if the QR scan was successful (Point to logs folder)
            if qr_success:
                read_hubodometer_ocr(new_image_path, 'logs/ocr_log.txt')
            else:
                print("Skipping OCR parsing to maintain log parity.")
            
            print("========================================\n")
            
    except Exception as e:
        print(f"System Error: {e}")

if __name__ == "__main__":
    main()