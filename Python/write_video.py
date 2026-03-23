# Convert avi video with correct codecs
import numpy as np
import matplotlib.pyplot as plt
import cv2 as cv


delay_in_s = 0
last_frame_in_s = -1

#Read video
file_in = '/home/caredda/Videos/Acquisition_Leica_OHX_Iphone_01_10_21/Acquisition_Iphone/Acquisition_corrected_reg.avi'
file_out = '/home/caredda/Videos/Acquisition_Leica_OHX_Iphone_01_10_21/Acquisition_Iphone/Iphone_reg2.avi'
video_in = cv.VideoCapture(file_in)

#Get video properties
FPS = video_in.get(cv.CAP_PROP_FPS)
tot_frame = video_in.get(cv.CAP_PROP_FRAME_COUNT)

#Compute delays in frames
delay_in_frames = delay_in_s * FPS
if(last_frame_in_s==-1):
    last_frame_in_frames = tot_frame
else:
    last_frame_in_frames = last_frame_in_s * FPS

if(video_in.isOpened()):

    ret, frame = video_in.read()

    #init videowriter
    video_out = cv.VideoWriter(file_out,cv.VideoWriter_fourcc('M','J','P','G'),FPS,(frame.shape[1],frame.shape[0]))

    for i in range(0,np.int(video_in.get(cv.CAP_PROP_FRAME_COUNT))):
        ret, frame = video_in.read()
        if(ret==False):
            break
        if(i>=delay_in_frames and i<last_frame_in_frames):
            video_out.write(frame)
        print(100*i/np.int(video_in.get(cv.CAP_PROP_FRAME_COUNT)))



video_out.release()