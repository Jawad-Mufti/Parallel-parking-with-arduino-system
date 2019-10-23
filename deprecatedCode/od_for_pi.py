import cv2
import numpy as np                                                              #library that gives you complicated, but fast arrays to store the stuff in
import serial
import time
from picamera.array import PiRGBArray
from picamera import PiCamera

MIN_MATCH_COUNT = 30

#print (cv2.__version__)                                                         #need to download version 2.4 if you wanna use SIFT and SURF
                                                         #feature extractor created
try:
    arduino_serial = serial.Serial('/dev/ttyACM0',9600)
except Exception:
    arduino_serial = serial.Serial('/dev/ttyACM1',9600)

time.sleep(2)

#detector = cv2.SIFT()  
detector = cv2.xfeatures2d.SIFT_create()

FLANN_INDEX_KDITREE = 0                                                         #define the flag

flannParam = dict(algorithm = FLANN_INDEX_KDITREE,tree = 5)                     #apply correct parameters
flann=cv2.FlannBasedMatcher(flannParam, {})                                     #initialize feature matcher

#set up the imgs
trainingImg = cv2.imread("TrainingData/1.jpg", 0)                              #imread to load the img, make the file gray scale by adding 0
img2 = cv2.imread("TrainingData/2.jpg",0)
if trainingImg.all == None: #also add an or for the other img
    raise Exception("could not load img")

#setup detection on those imgs
trainingKP, trainingDesc = detector.detectAndCompute(trainingImg, None)         #detect and compute the description                                                                                #trainingKP stores list of key points (coordinates of features), trainingDesc stores list of descriptions for the keypoints
kp2,desc2 = detector.detectAndCompute(img2, None)
                                                                                #they are needed to find visually similar objects in our live video
#setup the video stream                                                                                #kp is the coordinate of a key point on the img                                                                             # descriptor desc is where you store those coordinates
camera = PiCamera()
camera.resolution=(640,480)
camera.framerate = 30
camera_array=PiRGBArray(camera, size=(640,480))
time.sleep(0.1)
#cam = cv2.VideoCapture(0)                                                       #initialize camera of videocapture object
for frame in camera.capture_continuous(camera_array, format="bgr", use_video_port=True):
    while True:
        #ret, QueryImgBGR = cam.read()                                               #capture a frame from the camera
        #if QueryImgBGR == None:
        #    raise Exception("coulndt load 2nd img")
        image = frame.array
        QueryImg = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)                    #turn it into gray scale 
        queryKP, queryDesc = detector.detectAndCompute(QueryImg, None)              #extract features

        matches = flann.knnMatch(queryDesc, trainingDesc, k = 2)                    #match features of both img and stor in matches
        matches2 = flann.knnMatch(queryDesc, desc2, k = 2)         

        goodMatch = []                                                              #filter out false matches
        goodMatch2 = []

        for m, n in matches:
            if(m.distance < 0.75*n.distance):
                goodMatch.append(m)

        for m, n in matches2:
            if(m.distance < 0.75*n.distance):
                goodMatch2.append(m)        
                                                                           #to make sure we have enough feature matches to call them a match
        #by this method of having an if and an else if statement, the program will not notice both red and green at the same time, which is desireable
        if(len(goodMatch) >= MIN_MATCH_COUNT):         #IF YOU FIND A RED LIGHT                             # check if num of matches more than smallest allowed
            tp = []                                                                 #empty lists to store coordinates of matched features from training image and queryImg
            qp = []

            for m in goodMatch: 
                tp.append(trainingKP[m.trainIdx].pt)
                qp.append(queryKP[m.queryIdx].pt)

            tp, qp = np.float32((tp, qp))                                           #then convert whats inside to numpy lists

            H, status = cv2.findHomography(tp,qp,cv2.RANSAC,3.0)                    #translate points from training points to queryImg points

            h, w = trainingImg.shape

            trainBorder = np.float32([[[0,0],[0,h-1],[w-1,h-1],[w-1,0]]])
            queryBorder = cv2.perspectiveTransform(trainBorder, H)
            cv2.polylines(image,[np.int32(queryBorder)], True,(0,255,0),5)    #draw the lines on the query img

            #arduino_serial.write(d'0') #maybe use a boolean to make sure we only send it once and not every time we go into here?
            #time.sleep(2)
            #even if you remove the red light the car shouldnt start moving again until we place a green light in front of it

        elif(len(goodMatch2) >= MIN_MATCH_COUNT):     #IF YOU FIND A GREEN LIGHT                                 # check if num of matches more than smallest allowed
            tp2 = []                                                                 #empty lists to store coordinates of matched features from training image and queryImg
            qp2 = []

            for m in goodMatch2: 
                tp2.append(kp2[m.trainIdx].pt)
                qp2.append(queryKP[m.queryIdx].pt)

            tp2, qp2 = np.float32((tp2, qp2))                                           #then convert whats inside to numpy lists

            H, status = cv2.findHomography(tp2,qp2,cv2.RANSAC,3.0)                    #translate points from training points to queryImg points

            h, w = img2.shape

            trainBorder = np.float32([[[0,0],[0,h-1],[w-1,h-1],[w-1,0]]])
            queryBorder = cv2.perspectiveTransform(trainBorder, H)
            cv2.polylines(image,[np.int32(queryBorder)], True,(0,255,0),5)  
     
            #arduino_serial.write(d'1') #- for tex if it gets one it stops for a red light, sleep
            #time.sleep(2)

        else:
            print ("Not enough matches")  #%(len(goodMatch),MIN_MATCH_COUNT)
           # print "Nothing with enough matches found %d %d" % (len(goodMatch), MIN_MATCH_COUNT)
        cv2.imshow('result', image)
        camera_array.truncate(0)
        if cv2.waitKey(10) & 0xFF == ord('q'):
            break
cam.release()
cv2.destroyAllWindows()







