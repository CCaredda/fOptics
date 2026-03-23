## Extract first frame of the video

import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt
import os

file_path = "/home/caredda/Videos/RGB_videos/Already_processed/Patient_20230905/System_003_RGB_LEICA_OHX_090316001/Sample_001/Condition_Task-based_Autonomous_Right_hand-opening__Resting-state__Hand-dirac/Segment01.mp4"
dir_path = os.path.dirname(file_path)

start_id = 0

cap = cv.VideoCapture(file_path)
fourcc = cv.VideoWriter_fourcc(*'mp4v')
len_video = int(cap.get(cv.CAP_PROP_FRAME_COUNT))

print("FPS", cap.get(cv.CAP_PROP_FPS))
print("Image (width height) ", int(cap.get(cv.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv.CAP_PROP_FRAME_HEIGHT)))
print("Len video", len_video)



# ret, frame = cap.read()
# cv.imwrite(dir_path+"/initial_img.png",frame)



## Convert video into frames

import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt
import os

path = "/home/caredda/Videos/RGB_videos/Already_processed/Patient_20230908/"

video_filename = "Segment01"
file_path = path + video_filename + ".mp4"

os.mkdir(path+"img")


start_id = 0

cap = cv.VideoCapture(file_path)
fourcc = cv.VideoWriter_fourcc(*'mp4v')
len_video = int(cap.get(cv.CAP_PROP_FRAME_COUNT))

print("FPS", cap.get(cv.CAP_PROP_FPS))
print("Image (width height) ", int(cap.get(cv.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv.CAP_PROP_FRAME_HEIGHT)))
print("Len video", len_video)


for i in range(len_video):
  ret, frame = cap.read()
  cv.imwrite(path+"img/RGB_"+str(start_id+i)+".png",frame)


cap.release()

## Correct mp4 files

import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt
import tkinter.filedialog


path = "/home/caredda/Videos/RGB_videos/Already_processed/Patient_20230830/System_003_RGB_LEICA_OHX_090316001/Sample_001/Condition_Task-based_Autonomous_Right_hand-opening__Resting-state__Hand-dirac/"

video_filename = "Segment01"

file_path = path + video_filename + ".mp4"
cap = cv.VideoCapture(file_path)
fourcc = cv.VideoWriter_fourcc(*'mp4v')

print("FPS", cap.get(cv.CAP_PROP_FPS))
print("Image (width height) ", int(cap.get(cv.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv.CAP_PROP_FRAME_HEIGHT)))
print("Len video", int(cap.get(cv.CAP_PROP_FRAME_COUNT)))


out = cv.VideoWriter(path + video_filename +'_corrected.mp4',fourcc, cap.get(cv.CAP_PROP_FPS), (int(cap.get(cv.CAP_PROP_FRAME_WIDTH)),int(cap.get(cv.CAP_PROP_FRAME_HEIGHT))))

len_video = int(cap.get(cv.CAP_PROP_FRAME_COUNT))

# Read until video is completed
for i in range(len_video):
  # if i== 9367:
  #   break

  ret, frame = cap.read()
  if ret:
    out.write(frame)


# When everything done, release the video capture object
cap.release()
out.release()

## get paradigm times

time_steps = np.array([20.21, 19.99, 19.88, 19.94, 20.10, 19.95, 20, 20.05, 20.22, 2*60+59.88, 5, 30.94, 30.11 ])

frame_steps = np.cumsum(time_steps*cap.get(cv.CAP_PROP_FPS)).astype(int)


##
import cv2 as cv

cap = cv.VideoCapture("/home/caredda/Charly/Presentation/21_01_25/Visualization1.avi")
ret, frame = cap.read()


width = int(cap.get(cv.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv.CAP_PROP_FRAME_HEIGHT)/2)
out = cv.VideoWriter("/home/caredda/Charly/Presentation/21_01_25/test.mp4",cv.VideoWriter_fourcc(*'mp4v'), cap.get(cv.CAP_PROP_FPS),(width,height))

print(width,height)
len_video = int(cap.get(cv.CAP_PROP_FRAME_COUNT))

for i in range(1, len_video):
  ret, frame = cap.read()
  if ret:
    frame = frame[0:height,:,:]
    out.write(frame)

out.release()
cap.release()
