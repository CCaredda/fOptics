import os
import numpy as np
import cv2
import sys
import os
import shutil
import pandas as pd
import glob
from pathlib import Path
import matplotlib.pyplot as plt
import re
import json


#color of functional areas
color_motor = [0,0,255]
color_pre_motor = [0,255,0]
color_sensory = [0,255,255]
color_language = [255,0,0]

#contour width
width_cnt = 4

#Constants
alpha = 0.3
colormap = cv2.COLORMAP_JET



class contour_brain_functions:
    def __init__(self):
        self.c_motor = ()
        self.c_sensory = ()
        self.c_language = ()       


def create_colorbar(in_img,colorbar):

    colorbar_img = np.zeros((in_img.shape[0],int(in_img.shape[1]/32)))
    for i in range(colorbar_img.shape[1]):
        colorbar_img[:,i] = np.linspace(255,0,colorbar_img.shape[0])

    colorbar_img = colorbar_img.astype(np.uint8)
    colorbar_img = cv2.applyColorMap(colorbar_img,colorbar)

    return colorbar_img

def create_colorbar_N_values(img, n_values, colormap):
    #Create colorbar with n_values colors
    values = np.linspace(255, 0, n_values, dtype=int)
    colorbar_img = np.zeros((img.shape[0],int(img.shape[1]/32)))
    for i in range(colorbar_img.shape[1]):
        arr = np.repeat(values, (colorbar_img.shape[0] // 4))

        # Add remainder values
        if (colorbar_img.shape[0] % 4) > 0:
            arr = np.append(arr, values[:colorbar_img.shape[0] % 4])

        colorbar_img[:,i] = arr
    # np.savetxt("/home/caredda/temp/test.txt",colorbar_img)
    
    colorbar_img = cv2.applyColorMap(colorbar_img.astype(np.uint8),colormap)
    
    
    colors = cv2.applyColorMap((np.linspace(0, 255, n_values, dtype=int)).astype(np.uint8),colormap)
    return colorbar_img, np.squeeze(np.asarray(colors))

def get_SPM_colormap_SPM(in_data,min,max,colormap):

    res  = in_data.copy()
    res = res - res.min()
    res = 255*res/res.max()
    res = res.astype(np.uint8)

    #Apply colormap
    res = cv2.applyColorMap(res, colormap)

    return res


def remove_small_contour(fOptics_mask, Thresh_activity):

    out = np.zeros(fOptics_mask.shape,dtype=np.uint8)

    c, hierarchy = cv2.findContours(fOptics_mask.copy(),cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)

    for i in range(len(c)):
        temp = np.zeros(fOptics_mask.shape,dtype=np.uint8)
        temp = cv2.drawContours(temp,c,i,1,-1)
        if temp.sum()>Thresh_activity:
            out = cv2.drawContours(out,c,i,255,-1)
    return out





def get_functional_mask(fmask_filename, input_img, f, color_function):

    #Find functional mask
    file = glob.glob(fmask_filename)

    if len(file) == 0:
        c = ()
        return c

    f_mask = cv2.imread(file[0])

    #Get red (motor) and yellow (sensory) contours
    mask = cv2.inRange(f_mask, np.asarray(color_function), np.asarray(color_function))

    #resize masks
    mask = cv2.resize(mask,(input_img.shape[1]*f,input_img.shape[0]*f))

    #get contours
    c, hierarchy = cv2.findContours(mask.copy(),cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)


    return c



 

def get_functional_img_task_based(main_path,id_result,auto_thresh,f, ref_img, mask_ref, min_area_mm2, reso_mm):


    # color SPM
    color_SPM = (0,0,0)

    # color auto
    color_auto = (255,0,255)

    # Load initial image
    img = cv2.imread(main_path+"/initial_img.png")
    rows = np.array([0,img.shape[0]])
    cols = np.array([0,img.shape[1]])

    # Load mask
    mask = (cv2.imread(main_path+"/mask.png",cv2.IMREAD_GRAYSCALE)/255).astype(np.uint8)


    #get registration transform
    affine_matrix = find_registration_transform(mask_ref, mask)



    # Load input image
    input_img = cv2.imread(main_path+"/initial_img.png")


    #Load Z stat matrix
    Z_Stat_Optical = np.loadtxt(main_path+"/"+str(id_result)+"/Z_Stat_blured.txt")
    Z_Stat_Optical = cv2.resize(Z_Stat_Optical,(input_img.shape[1],input_img.shape[0]))

    #z thresh SPM
    z_thresh_SPM = np.genfromtxt(main_path+"/info_SPM.txt",dtype='str')
    z_thresh_SPM = float(z_thresh_SPM[0,1])

    #z thresh auto
    z_thresh_auto = Z_Stat_Optical[mask==1].mean() + auto_thresh*Z_Stat_Optical[mask==1].std()

    # Load SPM mask
    SPM_mask = cv2.imread(main_path+"/"+str(id_result)+"/SPM.png",cv2.IMREAD_GRAYSCALE)
    SPM_mask = cv2.resize(SPM_mask,(input_img.shape[1],input_img.shape[0]))
    SPM_mask = SPM_mask.astype(np.uint8)



    # Compute auto mask
    auto_mask = np.zeros(Z_Stat_Optical.shape)
    auto_mask[Z_Stat_Optical>z_thresh_auto] = 1
    auto_mask = auto_mask.astype(np.uint8)



    #Bitwise and between auto mask and mask
    auto_mask = np.bitwise_and(auto_mask,mask)




    #Apply affine transform
    SPM_mask = cv2.warpAffine(SPM_mask, affine_matrix, (ref_img.shape[1], ref_img.shape[0]))
    auto_mask = cv2.warpAffine(auto_mask, affine_matrix, (ref_img.shape[1], ref_img.shape[0]))
    Z_Stat_Optical = cv2.warpAffine(Z_Stat_Optical, affine_matrix, (ref_img.shape[1], ref_img.shape[0]))


    # resize all image
    # mask = cv2.resize(mask,(input_img.shape[1]*f,input_img.shape[0]*f))
    # SPM_mask = cv2.resize(SPM_mask,(input_img.shape[1]*f,input_img.shape[0]*f))
    # auto_mask = cv2.resize(auto_mask,(input_img.shape[1]*f,input_img.shape[0]*f))
    # Z_Stat_Optical = cv2.resize(Z_Stat_Optical,(input_img.shape[1]*f,input_img.shape[0]*f))
    # input_img = cv2.resize(input_img,(input_img.shape[1]*f,input_img.shape[0]*f))
    mask = cv2.resize(mask_ref,(ref_img.shape[1]*f,ref_img.shape[0]*f))
    SPM_mask = cv2.resize(SPM_mask,(ref_img.shape[1]*f,ref_img.shape[0]*f))
    auto_mask = cv2.resize(auto_mask,(ref_img.shape[1]*f,ref_img.shape[0]*f))
    Z_Stat_Optical = cv2.resize(Z_Stat_Optical,(ref_img.shape[1]*f,ref_img.shape[0]*f))
    input_img = cv2.resize(ref_img,(ref_img.shape[1]*f,ref_img.shape[0]*f))


    #get min max Tstats
    max = np.max(Z_Stat_Optical[mask>0])
    min = np.min(Z_Stat_Optical[mask>0])

    #Morpho math on auto mask
    auto_mask = cv2.morphologyEx(auto_mask, cv2.MORPH_CLOSE, cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(30*f,30*f)))
    auto_mask = cv2.morphologyEx(auto_mask, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(30*f,30*f)),iterations=1)



    #Bitwise and between auto mask and mask
    auto_mask = np.bitwise_and(auto_mask,mask)



    # Remove contours of fOptics map having area lower than min_area_mm2
    # SPM_mask =  remove_small_contour(SPM_mask, reso_mm, min_area_mm2)
    # auto_mask =  remove_small_contour(auto_mask, reso_mm, min_area_mm2)




    #Find contours
    contours_SPM, hierarchy = cv2.findContours(SPM_mask.copy(),cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    contours_auto, hierarchy = cv2.findContours(auto_mask.copy(),cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)


    #Fill contours
    cv2.drawContours(SPM_mask,contours_SPM,-1,1,cv2.FILLED)
    cv2.drawContours(auto_mask,contours_auto,-1,1,cv2.FILLED)


    #Apply colormap on Z stat
    colormap_res = get_SPM_colormap_SPM(Z_Stat_Optical,min,max,colormap)

    #Z-stats with contour SPM in black and auto contour in magenta
    out = input_img.copy()
    out[mask>0] = (1-alpha)*out[mask>0] + alpha*colormap_res[mask>0]

    #draw contours
    cv2.drawContours(out,contours_SPM,-1,color_SPM,width_cnt)
    cv2.drawContours(out,contours_auto,-1,color_auto,width_cnt)

    # Crop image
    out = out[rows[0]:rows[1],cols[0]:cols[1],:]

    #Create colorbar
    colorbar_Optics = create_colorbar(out,colormap)

    #Add colorbar
    out2 = np.zeros((out.shape[0],out.shape[1]+colorbar_Optics.shape[1],out.shape[2]),dtype=np.uint8)
    out2[:,0:out.shape[1],:] = out
    out2[:,out.shape[1]:,:] = colorbar_Optics


    #Draw threshold on colorbar
    drawThreshold_onColorbar(out2,colorbar_Optics, min,max,z_thresh_auto,color_auto)
    drawThreshold_onColorbar(out2,colorbar_Optics, min,max,z_thresh_SPM,color_SPM)
    colorbar_Optics = drawTextOnImg(out2,str(float("{:.1f}".format(max))),1)
    colorbar_Optics = drawTextOnImg(out2,str(float("{:.1f}".format(min))),0)


    #Compute z-stats
    Z_Stats_display = Z_Stat_Optical*((mask/mask.max()).astype(np.uint8))


    #Adapt dynamic for writing the mask
    auto_mask[auto_mask>0] = 255

    return out2, mask, SPM_mask, auto_mask, Z_Stats_display


def drawThreshold_onColorbar(img,colorbar,min,max,thresh,color):

    offset = 3
    res = (max - min)/colorbar.shape[0]

    Thresh_pix = int((max - thresh)/res)

    if Thresh_pix >= colorbar.shape[0] or Thresh_pix <= 0:
        return img

    img[Thresh_pix-offset:Thresh_pix+offset,img.shape[1]-colorbar.shape[1]:] = color
    img = drawTextOnImg(img,str(float("{:.1f}".format(thresh))),Thresh_pix-offset,color)

    return img

def drawTextOnImg(img,text,pos,font_color = (255, 255, 255)):
    font = cv2.FONT_HERSHEY_COMPLEX_SMALL

    thick = 1
    font_size = 0.9
    (text_width, text_height) = cv2.getTextSize(text, font, font_size, thick)[0]
    text_height += 5
    if text_height%2==1:
        text_height = text_height +1

    if pos == 0: # lower pos
        img = cv2.putText(img,text,(img.shape[1]-text_width,int(img.shape[0]-3)),font,font_size,font_color,thick,cv2.LINE_AA)
        return img

    if pos == 1: #upper pos
        img = cv2.putText(img,text,(img.shape[1]-text_width,text_height),font,font_size,font_color,thick,cv2.LINE_AA)
        return img

    img = cv2.putText(img,text,(img.shape[1]-text_width,int(pos-text_height/4)),font,font_size,font_color,thick,cv2.LINE_AA)
    return img


def find_registration_transform(mask_fixed, mask_moving):

    #Find contours
    contours_fixed, _ = cv2.findContours(mask_fixed, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours_moving, _ = cv2.findContours(mask_moving, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Find the bounding rectangles or use moments to get centroids
    rect_moving = cv2.minAreaRect(contours_moving[0])
    rect_fixed = cv2.minAreaRect(contours_fixed[0])

    # Get the box points for both rectangles (corners of the rectangles)
    box_moving = cv2.boxPoints(rect_moving)
    box_fixed = cv2.boxPoints(rect_fixed)

    # Convert them to integer coordinates
    box_moving = box_moving.astype(int)
    box_fixed = box_fixed.astype(int)


    # Calculate the affine transformation matrix using three points
    # We only need three pairs of corresponding points to calculate the affine transformation
    points_moving = np.float32(box_moving[:3])  # Take 3 points from the moving image
    points_fixed = np.float32(box_fixed[:3])    # Take 3 corresponding points from the fixed image


    # Compute the affine transformation matrix
    affine_matrix = cv2.getAffineTransform(points_moving, points_fixed)

    return affine_matrix
