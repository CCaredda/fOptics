
import sys
from tkinter import filedialog
from tkinter import *
from tkinter.filedialog import askopenfilename
import os.path
import cv2 as cv
import numpy as np
import scipy.io
import matplotlib.pyplot as plt
import shutil

sys.path.append('/home/caredda/DVP/C++/imagerie_fonctionnelle_cerveau/git/optical-fMRI-projection/functions')
from drawing_functions import create_colorbar, drawTextOnImg, drawColorbarLegend, get_SPM_colormap

def get_Centered_colormap(in_data,min,max,colormap):

    temp = np.zeros(in_data.shape)
    #Values >0 are normalized between 127-255
    temp[in_data>0] = (in_data[in_data>0] *((255-127)/max)) + 127

    #Values <0 are normalized between 0-127
    temp[in_data<=0] = (in_data[in_data<=0] - min) *((127)/(-min))


    temp[temp>255] = 255
    temp[temp<0] =0
    temp = temp.astype(np.uint8)

    # temp = cv2.medianBlur(temp,15)
    temp = cv.GaussianBlur(temp,(31,31),5)

    #Apply colormap
    res = cv.applyColorMap(temp, colormap)

    return res




# colormap = cv.COLORMAP_WINTER
colormap = cv.COLORMAP_JET

if len( sys.argv ) <2:
    print("Error in input args at least 1 args is required: main path")
    print("Optionnal args are : ")
    print("int (0/1) for plotting all colorbars (default 1)")
    print("float coeff for mutliplying data (default 1.0)")
    print("float alpha for results transparency (default 0.5)")

    exit()
# get main path
args = sys.argv
main_path = args[1]
if len( args ) >= 3:
    plot_colorbar_each_plot = int(args[2])
else:
    plot_colorbar_each_plot = 1

if len( args ) >= 4:
    coeff_multi = np.double(args[3])
else:
    coeff_multi = 1

if len( args ) >= 5:
    alpha = np.double(args[4])
else:
    alpha = 0.5

# main_path = "/home/caredda/temp/HSI_25_10_19/675_975nm/Mean_C_MBLL"
# plot_colorbar_each_plot = 1
# coeff_multi = 1e6
# alpha = 0.5


max_val = np.array([3,3,1,3])
min_val = - max_val


#Load initial image and mask
if not os.path.isfile(main_path+"/initial_img.png"):
    print ("initial_img.png does not exist")
    exit()
if not os.path.isfile(main_path+"/mask.png"):
    print ("mask.png does not exist")
    exit()


initial_img = cv.imread(main_path+"/initial_img.png")
coeff_img = 4
initial_img = cv.resize(initial_img,(coeff_img*initial_img.shape[1],coeff_img*initial_img.shape[0]))

mask = cv.imread(main_path+"/mask.png",cv.IMREAD_ANYDEPTH)
mask = mask/mask.max()
mask = cv.resize(mask,(initial_img.shape[1],initial_img.shape[0]))

#Select files
root = Tk()
root.withdraw()
filez = filedialog.askopenfilenames(initialdir = main_path, title='Choose files to identify neuronavigation points in RGB images')

if(len(filez)==0):
    exit()

#Load data and resize
data = []

for i in range(len(filez)):
    temp = coeff_multi*np.loadtxt(filez[i])
    temp = cv.resize(temp,(initial_img.shape[1],initial_img.shape[0]))
    data.append(temp)


#Apply colormap and merge with initial img
out = []

for i in range(len(data)):
    #Apply colormap
    colormap_res = get_Centered_colormap(data[i],min_val[i],max_val[i],colormap)
    temp = initial_img.copy()

    #Merge with initial img
    temp[mask>0] = (1-alpha)*temp[mask>0] + alpha*colormap_res[mask>0]

    if(plot_colorbar_each_plot):

        colorbar_Optics = create_colorbar(initial_img,colormap)
        output = np.zeros((initial_img.shape[0],initial_img.shape[1]+colorbar_Optics.shape[1],3))
        output[:,0:initial_img.shape[1],:] = temp
        output[:,initial_img.shape[1]:initial_img.shape[1]+colorbar_Optics.shape[1],:] = colorbar_Optics
    else:
        output = temp
    out.append(output)




# Concatenate results
if(len(out)<=3):
    if(plot_colorbar_each_plot):
        output = np.zeros((out[0].shape[0],len(out)*out[0].shape[1],3))
    else:
        colorbar_Optics = create_colorbar(initial_img,colormap)
        output = np.zeros((out[0].shape[0],len(out)*out[0].shape[1]+colorbar_Optics.shape[1],3))
        output[:,len(out)*out[0].shape[1]:,:] = colorbar_Optics
    for i in range(len(out)):
        output[:,i*out[i].shape[1]:(i+1)*out[i].shape[1],:] = out[i]

if(len(out)==4):
    if(plot_colorbar_each_plot):
        output = np.zeros((2*out[0].shape[0],2*out[0].shape[1],3))
    else:
        colorbar_Optics =  create_colorbar(np.zeros((2*out[0].shape[0],2*out[0].shape[1])),colormap)
        output = np.zeros((2*out[0].shape[0],2*out[0].shape[1]+colorbar_Optics.shape[1],3))
        output[:,2*out[0].shape[1]:,:] = colorbar_Optics
    for i in range(2):
        output[0:out[0].shape[0],i*out[0].shape[1]:(i+1)*out[0].shape[1],:] = out[i]
        output[out[0].shape[0]:2*out[0].shape[0],i*out[0].shape[1]:(i+1)*out[0].shape[1],:] = out[i+2]

cv.imwrite(main_path+"/results.png",output)

np.savetxt(main_path+"/max_values.txt",max_val)


# ##
#
# i = 2
#
# plt.close('all')
# plt.subplot(121)
# plt.imshow(data[i],vmin=min_val[i],vmax=max_val[i])
# plt.subplot(122)
# plt.imshow(get_Centered_colormap(data[i],min_val[i],max_val[i],colormap))
#
# plt.show()

