import easyocr
import cv2
import os

print("Loading EasyOCR Model into GPU... Please wait.")
# Initialize reader once so it doesn't bottleneck the fast-capture loop
reader = easyocr.Reader(['en'], gpu=True)

def read_hubodometer_ocr(image_path, output_file="ocr_log.txt"):
    print(f"Scanning '{image_path}' for digits using EasyOCR...")
    
    if not os.path.exists(image_path):
        print(f"Error: '{image_path}' not found.")
        return

    image = cv2.imread(image_path)
    if image is None:
        print("Error: OpenCV could not read the image.")
        return

    # Apply your predefined hubodometer bounding box
    roi = image[222:326, 0:640] 
    
    # Restrict the AI to only look for numbers
    results = reader.readtext(roi, allowlist='0123456789', detail=1)

    if not results:
        print("No digits detected in the specified ROI.")
        with open(output_file, 'a') as file:
            file.write("0,0.00\n")
        return

    with open(output_file, 'a') as file:
        for (bbox, text, prob) in results:
            top_left = tuple(map(int, bbox[0]))
            bottom_right = tuple(map(int, bbox[2]))
            
            # Draw green bounding boxes and text on the cropped image
            cv2.rectangle(roi, top_left, bottom_right, (0, 255, 0), 2)
            cv2.putText(roi, text, (top_left[0], top_left[1] - 10), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            
            print(f"Detected Digits: {text} (Confidence: {prob:.2f})")
            file.write(f"{text},{prob:.2f}\n")

    # Save the annotated ROI crop so you can visually verify the AI's accuracy
    annotated_path = image_path.replace('.jpg', '_annotated.jpg')
    cv2.imwrite(annotated_path, roi)
    print(f"--- OCR Data saved to {output_file} & {annotated_path} ---")