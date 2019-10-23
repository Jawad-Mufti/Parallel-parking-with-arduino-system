import cv2
import numpy as np                                                              #library that gives you complicated, but fast arrays to store the stuff in
MIN_MATCH_COUNT = 30

#print (cv2.__version__)                                                         #need to download version 2.4 if you wanna use SIFT and SURF

#detector = cv2.SIFT()                                                           #feature extractor created

detector = cv2.xfeatures2d.SIFT_create()

FLANN_INDEX_KDITREE = 0                                                         #define the flag

flannParam = dict(algorithm = FLANN_INDEX_KDITREE,tree = 5)                     #apply correct parameters
flann=cv2.FlannBasedMatcher(flannParam, {})                                     #initialize feature matcher

greenLightImg = cv2.imread('green.png', 0)                              #imread to load the img, make the file gray scale by adding 0
redLightImg = cv2.imread('red.png', 0)
if greenLightImg.all == None or redLightImg.all == None:
    raise Exception("could not load img")
greenKP, greenDesc = detector.detectAndCompute(greenLightImg, None)         #detect and compute the description
redKP, redDesc = detector.detectAndCompute(redLightImg, None)
                                                                                #greenKP stores list of key points (coordinates of features), greenDesc stores list of descriptions for the keypoints

                                                                                #they are needed to find visually similar objects in our live video
                                                                                #kp is the coordinate of a key point on the img
                                                                                # descriptor desc is where you store those coordinates

cam = cv2.VideoCapture(0)                                                       #initialize camera of videocapture object

while True:
    ret, QueryImgBGR = cam.read()                                               #capture a frame from the camera
    #if QueryImgBGR == None:
    #    raise Exception("coulndt load 2nd img")
    
    QueryImg = cv2.cvtColor(QueryImgBGR, cv2.COLOR_BGR2GRAY)                    #turn it into gray scale
    
    queryKP, queryDesc = detector.detectAndCompute(QueryImg, None)              #extract features
    matchesGreen = flann.knnMatch(queryDesc, greenDesc, k = 2)     #match features of both img and stor in matchesGreen
    matchesRed = flann.knnMatch(queryDesc, redDesc, k = 2)
    goodMatchGreen = []                                                              #filter out false matchesGreen 	
    goodMatchRed = []
    for m, n in matchesGreen:
        if(m.distance < 0.75*n.distance):
            goodMatchGreen.append(m)
    for m, n in matchesRed:
        if(m.distance < 0.75*n.distance):
            goodMatchRed.append(m)                                                                             #to make sure we have enough feature matchesGreen to call them a match
    if(len(goodMatchGreen) >= MIN_MATCH_COUNT or len(goodMatchRed) >= MIN_MATCH_COUNT):              
        greenTp = []                                                                 #empty lists to store coordinates of matched features from training image and queryImg
        greenQp = []
        redTp = []
        greenQp = []

        for m in goodMatchGreen: 
            greenTp.append(greenKP[m.trainIdx].pt)
            greenQp.append(queryKP[m.queryIdx].pt)
        for m in goodMatchRed:
            redTp.append(redKP[m.trainIdx].pt)
            redQp.append(queryKP[m.queryIdx].pt)

        greenTp, greenQp = np.float32((greenTp, greenQp))      		#then convert whats inside to numpy lists
        redTp, redQp = np.float32((redTp, redQp))

        G, greenStatus = cv2.findHomography(greenTp,greenQp,cv2.RANSAC,3.0)                    #translate points from training points to queryImg points
        R, redStatus = cv2.findHomography(redTp, redQp, cv2.RANSAC,3.0)

        h1, w1 = greenLightImg.shape
        h2, w2 = redLightImg.shape

        trainBorderGreen = np.float32([[[0,0],[0,h1-1],[w1-1,h1-1],[w1-1,0]]])
        trainBorderRed = np.float32([[[0,0],[0,h2-1],[w2-1,h2-1],[w2-1,0]]])
        queryBorder1 = cv2.perspectiveTransform(trainBorderGreen, G)
        queryBorder2 = cv2.perspectiveTransform(trainBorderRed, R)
        cv2.polylines(QueryImgBGR,[np.int32(queryBorder1)], True,(0,255,0),5)    #draw the lines on the query img
        cv2.polylines(QueryImgBGR,[np.int32(queryBorder2)], True,(0,255,0),5) 

    else:
        print ("Not enough matchesGreen")  #%(len(goodMatchGreen),MIN_MATCH_COUNT)
       # print "Nothing with enough matchesGreen found %d %d" % (len(goodMatchGreen), MIN_MATCH_COUNT)
    cv2.imshow('result', QueryImgBGR)
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break
cam.release()
cv2.destroyAllWindows()







