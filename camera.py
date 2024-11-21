from flask import Flask, request
import cv2
import threading

app = Flask(__name__)

def start_camera():
    camera = cv2.VideoCapture(0)  # Use 0 for the default camera
    if not camera.isOpened():
        print("Error: Could not open the camera.")
        return

    print("Camera started. Press 'q' in the camera window to stop.")
    while True:
        ret, frame = camera.read()
        if not ret:
            print("Error: Failed to capture frame.")
            break

        cv2.imshow('Camera Feed', frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    camera.release()
    cv2.destroyAllWindows()

@app.route('/motion', methods=['GET'])
def motion_detected():
    print("Motion detected! Starting camera...")
    camera_thread = threading.Thread(target=start_camera)
    camera_thread.start()
    return "Motion detected! Camera started.", 200

if __name__ == "__main__":
    print("Starting Flask server...")
    app.run(host="0.0.0.0", port=5000)
