import cv2
import os

file_path = r'C:\Users\liewh\Desktop\SIT\Y1T3\CEG1010 - ITP\OptiTrack\captures\capture.jpg'

if not os.path.exists(file_path):
    print(f"Error: The file '{file_path}' does not exist.")
    print("Please check the filename and path, or ensure the file extension (e.g., .jpg, .png) is correct.")
else:
    image = cv2.imread(file_path)
    
    if image is None:
        print("Error: OpenCV could not read the image. It might be corrupted or in an unsupported format.")
    else:
        print("Image loaded successfully. Please select the digits with your mouse.")
        print("Press SPACE or ENTER to confirm, or 'c' to cancel.")
        
        roi_rect = cv2.selectROI("Select the digits", image, fromCenter=False, showCrosshair=True)
        
        x, y, w, h = roi_rect
        if w > 0 and h > 0:
            print("\nSelection successful!")
            print(f"Update your code to: roi = image[{y}:{y+h}, {x}:{x+w}]")
        else:
            print("\nSelection cancelled or invalid.")
            
        cv2.destroyAllWindows()