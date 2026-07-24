import os
from pyzbar.pyzbar import decode
from PIL import Image
from datetime import datetime

def decode_qr_from_image(image_path, text_file="scanned_data.txt"):
    print(f"Scanning '{image_path}' for QR codes...")
    
    if not os.path.exists(image_path):
        print(f"Error: The file '{image_path}' was not found.")
        return False

    try:
        img = Image.open(image_path)
        decoded_objects = decode(img)
        
        # If no QR is found, return False immediately (DO NOT write to log)
        if not decoded_objects:
            print("No QR code detected in the image. Frame discarded.")
            return False

        # If a QR is found, write it to the log and return True
        with open(text_file, 'a') as file:
            for i, obj in enumerate(decoded_objects):
                qr_data = obj.data.decode('utf-8')
                print("\n--- QR Code Found! ---")
                print(f"Data: {qr_data}")
                file.write(f"{qr_data}\n")
                
        print(f"--- Data successfully saved to {text_file} ---")
        return True

    except Exception as e:
        print(f"An error occurred while processing the image: {e}")
        return False

if __name__ == "__main__":
    TARGET_IMAGE = 'capture.jpg' 
    OUTPUT_FILE = 'qr_log.txt'
    
    decode_qr_from_image(TARGET_IMAGE, OUTPUT_FILE)