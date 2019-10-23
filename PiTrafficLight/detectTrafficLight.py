# USAGE
# python detectTrafficLight.py

# import the necessary packages
from imutils.video import VideoStream
from imutils.video import FPS
import numpy as np
import argparse
import imutils
import time
import cv2
import serial

time.sleep(2.0)

	#This is a method used in the main loop to "mask out" rectangles and detect 
	#if any of those rectangles are in fact a "red traffic light" or a "green traffic light" 
	#by comparing vertices of shapes, and checking the amount of certain pixels
	#in a detected rectangle
def shape_compare(c, frame):

	#turn curves into lines
	shape = cv2.arcLength(c, True)
	
	#Approximate polygon curves to get better idea on straight lines
	approx = cv2.approxPolyDP(c, 0.04 * shape, True)

	#Compare vertices of approximated shapes
	if len(approx) == 4:

		(x, y, w, h) = cv2.boundingRect(approx)
		ar = w / float(h)

		if ar >= 0.20 and ar <= 0.80:
			cropped_img = frame[x: x + w, y: y+h]

			maskGreen = cv2.inRange(cropped_img, lower_green_bound, upper_green_bound)
			green = cv2.countNonZero(maskGreen)

			maskRed = cv2.inRange(cropped_img, lower_red_bound, upper_red_bound)
			red = cv2.countNonZero(maskRed)

			maskBlack = cv2.inRange(cropped_img, lower_black_bound, upper_black_bound)
			black = cv2.countNonZero(maskBlack)
			
			if(red > green and red >= 30): #this was 50 before
				print("RED LIGHT")
				arduino_serial.write(b'0')
				return "RedLight"
			
			elif(green >= 30 or(black >= 1000 and red < 20)):
				print("GREEN LIGHT")
				arduino_serial.write(b'1')	
				return "GreenLight"

			return "rectangle"

print("[INFO] loading model...")
print("[INFO] starting video stream...")

#comment first line and uncomment second one to use the PI camera module instead of laptop webcam
#vs = VideoStream(src=0).start()
vs = VideoStream(usePiCamera=True).start()

#serial connection to arduino
try:
	arduino_serial = serial.Serial('/dev/ttyACM0', 9600)
except Exception:
	arduino_serial = serial.Serial('/dev/ttyACM1', 9600)

#Colors to find	
#lower_green_bound = np.array([0, 150, 0], dtype = "uint8")
#upper_green_bound = np.array([100, 255, 100], dtype = "uint8")

#lower_red_bound = np.array([0, 0, 100], dtype = "uint8")
#upper_red_bound = np.array([100, 100, 255], dtype = "uint8")

#Other colors values if the ones above dont work
lower_green_bound = np.array([29, 100, 21], dtype = "uint8") #mid was 130, then 100 before!
upper_green_bound = np.array([83, 250, 106], dtype = "uint8")

lower_red_bound = np.array([15, 15, 100], dtype = "uint8")
upper_red_bound = np.array([50, 50, 200], dtype = "uint8")

lower_black_bound = np.array([0, 0, 0], dtype = "uint8")
upper_black_bound = np.array([51, 51, 51], dtype = "uint8")

time.sleep(2.0)
fps = FPS().start()

#start the car driving forward
arduino_serial.write(b'1')	

#loop over the frames from the video stream to create a video
#for every frame we capture we check for rectangles, and if any rectangles are detected we then 
#try to detect the traffic lights by color detection
while True:
	# grab the frame from the threaded video stream and resize it
	# to have a maximum width of 400 pixels
	frame = vs.read()
	frame = imutils.resize(frame, width=400) #I used 600 here and it worked, but I guess it should work with 400 too

	# grab the frame dimensions and convert it to a blob
	(h, w) = frame.shape[:2]
	blob = cv2.dnn.blobFromImage(cv2.resize(frame, (300, 300)),
		0.007843, (300, 300), 127.5)

	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	blurred = cv2.GaussianBlur(gray, (5, 5), 0)
	thresh = cv2.threshold(blurred, 60, 255, cv2.THRESH_BINARY)[1]

	_, contours, _ = cv2.findContours(thresh.copy(), cv2.RETR_TREE,
		cv2.CHAIN_APPROX_SIMPLE)

	#Draw each contour we find
	for c in contours:

		#Calculate center of contours
		moment = cv2.moments(c)
		if (moment["m00"] == 0):
			moment["m00"]=1
		cX = int(moment["m10"] / moment["m00"])
		cY = int(moment["m01"] / moment["m00"])

		#Call the method we made above to decide what the shape of a contour is
		thishape = shape_compare(c, frame)

		if(thishape == "RedLight" or thishape == "GreenLight" or thishape == "rectangle"):
			cv2.drawContours(frame, [c], -1, (0, 255, 0), 2)

			cv2.putText(frame, thishape, (cX - 20, cY - 20),
				cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)



	# show the output frame
	cv2.imshow("Frame", frame)
	key = cv2.waitKey(1) & 0xFF

	# if the `q` key was pressed, break from the loop
	if key == ord("q"):
		break

	# update the FPS counter
	fps.update()

# stop the timer and display FPS information
fps.stop()
print("[INFO] elapsed time: {:.2f}".format(fps.elapsed()))
print("[INFO] approx. FPS: {:.2f}".format(fps.fps()))

#stop the car when quitting the program, incase it's still running
arduino_serial.write(b'0')

# do a bit of cleanup
cv2.destroyAllWindows()
vs.stop()
