#!/usr/bin/env python
# -*-coding:Utf-8 -*

import numpy as np
import matplotlib.pyplot as plt
import cv2 as cv
import tensorflow as tf
import os
import sys


if len(sys.argv) > 2:
    data_path = sys.argv[1]
    res_path =  sys.argv[2]


print("In script")

in_img = cv.imread(res_path+"/img_in.png",cv.IMREAD_GRAYSCALE)

# Load model
reloaded = tf.keras.models.load_model(data_path+"/model_v1.h5")


# Prepare img
temp_in = cv.resize(in_img,(256,256))
img = np.zeros((1,256,256,1))
img[0,:,:,0] = temp_in
img = img.astype(float)
img = img/255

#Predict ROI
# prediction = reloaded.predict(img)
prediction = reloaded.predict(img)
prediction[prediction<0.5] = 0
prediction[prediction>=0.5] = 255
prediction = prediction.astype(np.uint8)

#Clear prediction

#Closing
kernel = cv.getStructuringElement(cv.MORPH_ELLIPSE,(3,3))
prediction[0,:,:,0] = cv.morphologyEx(prediction[0,:,:,0], cv.MORPH_OPEN, kernel,iterations=6)
prediction[0,:,:,0] = cv.morphologyEx(prediction[0,:,:,0], cv.MORPH_CLOSE, kernel,iterations=6)


#Find ROI contour
contours, hierarchy = cv.findContours(prediction[0,:,:,0], cv.RETR_EXTERNAL, cv.CHAIN_APPROX_NONE)
#Find largest contour
id_c = 0
if(len(contours)>1):
  area = cv.contourArea(contours[0])
  for c in range(0,len(contours)):
    if(cv.contourArea(contours[c])>area):
      area = cv.contourArea(contours[c])
      id_c = c
# Draw ROI on original image size
res = np.zeros((256,256),dtype=np.uint8)
cv.drawContours(res,contours,id_c,255,cv.FILLED)
res = cv.resize(res,(in_img.shape[1],in_img.shape[0]))




#Write ROI contour
res[res>0] = 255
cv.imwrite(res_path+"/ROI_mask.png",res)


print("End script")
