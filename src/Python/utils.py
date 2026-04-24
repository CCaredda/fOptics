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

class Distance_metrics:
    def __init__(self):
        self.mean_dist_motor = 0
        self.std_dist_motor = 0

        self.mean_dist_sensory = 0
        self.std_dist_sensory = 0

        self.tot_motor_areas = 0
        self.tot_sensory_areas = 0

        self.mean_practical_sensi_sensory = 0
        self.mean_practical_sensi_motor = 0

        self.mean_dist = 0
        self.std_dist = 0
        
        self.Patient_id = ""
        self.analysis_type = ""

    def init_list(self):
        self.mean_dist_motor = []
        self.std_dist_motor = []

        self.mean_dist_sensory = []
        self.std_dist_sensory = []

        self.mean_practical_sensi_sensory = []
        self.mean_practical_sensi_motor = []

        self.tot_motor_areas = []
        self.tot_sensory_areas = []

        self.mean_dist = []
        self.std_dist = []
        self.Patient_id = []

    def convert_to_np_arrays(self):
        self.mean_dist_motor = np.asarray(self.mean_dist_motor)
        self.std_dist_motor = np.asarray(self.std_dist_motor)

        self.mean_dist_sensory = np.asarray(self.mean_dist_sensory)
        self.std_dist_sensory = np.asarray(self.std_dist_sensory)

        self.mean_practical_sensi_sensory = np.asarray(self.mean_practical_sensi_sensory)
        self.mean_practical_sensi_motor = np.asarray(self.mean_practical_sensi_motor)

        self.tot_motor_areas = np.asarray(self.tot_motor_areas)
        self.tot_sensory_areas = np.asarray(self.tot_sensory_areas)

        self.mean_dist = np.asarray(self.mean_dist)
        self.std_dist = np.asarray(self.std_dist)
        self.Patient_id = np.asarray(self.Patient_id)
        

class Sensitivity_specificity_metrics:
    def __init__(self):
        self.Sensitiviy_motor = 0
        self.Sensitiviy_sensory = 0
        self.Specificity = 0

        self.TP_motor = 0
        self.FN_motor = 0
        self.nb_EBS_motor = 0
        

        self.TP_sensory = 0
        self.FN_sensory = 0
        self.nb_EBS_sensory = 0
        
        self.Patient_id = ""
        self.analysis_type = ""

    def init_list(self):
        self.Sensitiviy_motor = []
        self.Sensitiviy_sensory = []
        self.Specificity = []
        self.Patient_id = []

        self.TP_motor = []
        self.FN_motor = []
        self.nb_EBS_motor = []

        self.TP_sensory = []
        self.FN_sensory = []
        self.nb_EBS_sensory = []

    def convert_to_np_arrays(self):
        self.Sensitiviy_motor = np.asarray(self.Sensitiviy_motor)
        self.Sensitiviy_sensory = np.asarray(self.Sensitiviy_sensory)
        self.Specificity = np.asarray(self.Specificity)
        self.Patient_id = np.asarray(self.Patient_id)

        self.TP_motor = np.asarray(self.TP_motor)
        self.FN_motor = np.asarray(self.FN_motor)
        self.nb_EBS_motor = np.asarray(self.nb_EBS_motor)

        self.TP_sensory = np.asarray(self.TP_sensory)
        self.FN_sensory = np.asarray(self.FN_sensory)
        self.nb_EBS_sensory = np.asarray(self.nb_EBS_sensory)


class contour_brain_functions:
    def __init__(self):
        self.c_motor = ()
        self.c_sensory = ()
        self.c_language = ()       

def get_Patient_list(path):

    folders = glob.glob(path+"/P[0-9]*")

    folders_sorted = sorted(
        folders,
        key=lambda s: int(re.search(r"P(\d+)", s).group(1))
    )
    return folders_sorted

def resize_image(file_names,factor_col=1, factor_row=1 ):
    files = glob.glob(file_names)
    for f in files:
        #Load image
        p = Path(f)
        if p.suffix == ".txt":
            in_img = np.loadtxt(f)
        else:
            in_img = cv2.imread(f)

        #Resize
        out_img = cv2.resize(in_img,(int(in_img.shape[1]*factor_col),int(in_img.shape[0]*factor_row)))

        #Write image
        if p.suffix == ".txt":
            np.savetxt(f, out_img)
        else:
            cv2.imwrite(f,out_img)


def rotate_image(file_names,rotate ):
    files = glob.glob(file_names)
    for f in files:
        #Load image
        p = Path(f)
        if p.suffix == ".txt":
            in_img = np.loadtxt(f)
        else:
            in_img = cv2.imread(f)

        #Resize
        out_img = cv2.rotate(in_img,rotate)

        #Write image
        if p.suffix == ".txt":
            np.savetxt(f, out_img)
        else:
            cv2.imwrite(f,out_img)


def resize_all_image_for_Patient(Patient_dir, factor_col=1, factor_row=1 ):
    new_path = Patient_dir+"_resize"

    factor_col = 1
    factor_row = 2

    #Copy everything
    shutil.copytree(Patient_dir, new_path, dirs_exist_ok=True)


    # #Resize all files
    # resize_image(new_path+"/epicenters*", factor_col, factor_row )
    # resize_image(new_path+"/mask*", factor_col, factor_row )
    # resize_image(new_path+"/ref*", factor_col, factor_row )

    for i in range(1,5):
        
        p = new_path+"/Task_based_auto/SPM_MBLL_"+str(i)
        print(p)
        resize_image(p+"/initial_img*", factor_col, factor_row )
        resize_image(p+"/mask*", factor_col, factor_row )
        resize_image(p+"/0/*", factor_col, factor_row )
        resize_image(p+"/1/*", factor_col, factor_row )
        resize_image(p+"/2/*", factor_col, factor_row )


def rotate_all_image_for_Patient(Patient_dir, rotate):
    new_path = Patient_dir+"_rotate"


    #Copy everything
    shutil.copytree(Patient_dir, new_path, dirs_exist_ok=True)


    rotate_image(new_path+"/epicenters.png", rotate)
    rotate_image(new_path+"/mask_activation.png", rotate)
    rotate_image(new_path+"/ref.png", rotate)
    rotate_image(new_path+"/ref_mask.png", rotate)

    for i in range(1,5):
        
        p = new_path+"/Task_based_auto/SPM_MBLL_"+str(i)
        print(p)
        rotate_image(p+"/initial_img*", rotate)
        rotate_image(p+"/mask*", rotate)
        rotate_image(p+"/0/*", rotate )
        rotate_image(p+"/1/*", rotate )
        rotate_image(p+"/2/*", rotate)


def generate_square_ROI_inside_contour(mask, tile=5, min_frac=0.5):
 
    
    H, W = mask.shape

    # 2) Integral image (note the +1 in both dims)
    sat = cv2.integral(mask, sdepth=cv2.CV_32S)  # shape (H+1, W+1)

    # 3) Grid of full 5x5 tiles (drop partial tiles at image borders)
    nx = W // tile
    ny = H // tile
    thr = int(np.ceil(min_frac * (tile * tile)))  # >=13 for 5x5 and 0.5

    # keep_mask = np.zeros((ny, nx), dtype=bool)
    boxes = []

    for j in range(ny):
        y0 = j * tile
        y1 = y0 + tile
        for i in range(nx):
            x0 = i * tile
            x1 = x0 + tile

            # Fast sum over [y0:y1, x0:x1] using integral image
            s = sat[y1, x1] - sat[y0, x1] - sat[y1, x0] + sat[y0, x0]

            if s >= thr:
                # keep_mask[j, i] = True
                boxes.append((x0, y0, x1, y1))

    return boxes
    # return keep_mask, boxes
    



def get_ROI_center(ROI):
    return (int(ROI[0]+ (ROI[2] - ROI[0])/2),
            int(ROI[1]+ (ROI[3] - ROI[1])/2))


def get_square_ROI_idx_functional_areas(mask_function, ROIs, Thresh_activity):

    array_fOptics = np.zeros((len(ROIs),), dtype=np.uint8)

    #Normalize mask_function to get max value =1
    if mask_function.sum() != 0:
        mask_function = mask_function/mask_function.max()

    for i in range(len(ROIs)):

        (x0, y0, x1, y1) = ROIs[i]

        ROI_fOptics = mask_function[y0:y1,x0:x1]

        if ROI_fOptics.sum()>=Thresh_activity:
            array_fOptics[i] = 1
 
    return array_fOptics


def visualize_Square_ROI(img, ROIs, idx_function = np.array([]), colors_function = (255,0,0), color_ROI=-1, thickness=1):
    vis = img.copy()
    if color_ROI != -1:
        for (x0, y0, x1, y1) in ROIs:
            cv2.rectangle(vis, (x0, y0), (x1-1, y1-1), color_ROI, thickness)

    #Draw function
    alpha = 0.3
    if idx_function.shape[0]>0:
        overlay = vis.copy()
        for i in range(idx_function.shape[0]):
            #Draw filled ROI
            (x0, y0, x1, y1) = ROIs[idx_function[i]]
            cv2.rectangle(overlay, (x0, y0), (x1, y1), colors_function, thickness=-1)
            cv2.rectangle(overlay, (x0, y0), (x1, y1), (255,255,255), thickness=3)
            

        # Blend it with the original image
        vis = cv2.addWeighted(overlay, alpha, img, 1 - alpha, 0)

    return vis




def get_square_ROI_idx_EBS_epicenters(epicenter_file, ref_img, color_function, ROIs, f=1):

    idx_ROI = []

    #Get the center of the epicenters
    centers = get_epicenter(epicenter_file, ref_img, f, color_function )
    
    #Get center of mass of the epicenters
    for center in centers:
        
        for i in range(len(ROIs)):
            (x0, y0, x1, y1) = ROIs[i]
            if center[0]<=x1 and center [0]>=x0 and center[1]<=y1 and center[1]>=y0:
                idx_ROI.append(i)
                break
    return np.asarray(idx_ROI)




def create_EBS_maps(img, ROIs, idx_ROI_motor=np.array([]), idx_ROI_sensory = np.array([])):
    
    
    #init output
    EBS_map = np.zeros((len(ROIs),), dtype=np.uint8)
    EBS_map_motor = np.zeros(EBS_map.shape, dtype=np.uint8)
    EBS_map_sensory = np.zeros(EBS_map.shape, dtype=np.uint8)

    if idx_ROI_motor.shape[0]>0:
        EBS_map_motor[idx_ROI_motor] = 1
    
    if idx_ROI_sensory.shape[0]>0:
        EBS_map_sensory[idx_ROI_sensory] = 1

    EBS_map = np.bitwise_or(EBS_map_motor,EBS_map_sensory)
    return EBS_map, EBS_map_motor, EBS_map_sensory


def get_full_size_EBS_map(img, ROIs, idx_ROI_motor=np.array([]), idx_ROI_sensory = np.array([])):
    
    
    #init output
    EBS_map = np.zeros((img.shape[0], img.shape[1]), dtype=np.uint8)


    for i in idx_ROI_motor:
        #Draw filled ROI
        (x0, y0, x1, y1) = ROIs[i]
        cv2.rectangle(EBS_map, (x0, y0), (x1, y1), color=1, thickness=-1)
    
    for i in idx_ROI_sensory:
        #Draw filled ROI
        (x0, y0, x1, y1) = ROIs[i]
        cv2.rectangle(EBS_map, (x0, y0), (x1, y1), color=1, thickness=-1)

    return EBS_map




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

def get_mean_std_distance_to_EBS(ROIs, idx_epicenter, fOptics_mask, reso_mm, dist_vec, contour_function):
    
    # Create mask motor
    mask_function = np.zeros(fOptics_mask.shape,dtype=np.uint8)
    cv2.drawContours(mask_function,contour_function,-1,255,cv2.FILLED)


    fOptics_mask_in_eloquent_cortex = np.bitwise_and(fOptics_mask,mask_function)

    dst_vec = np.array([])
    for id in range(idx_epicenter.shape[0]):
        pt = get_ROI_center(ROIs[idx_epicenter[id]])

        c, hierarchy = cv2.findContours(fOptics_mask_in_eloquent_cortex.copy(),cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)

        dst = np.array([])
        for i in range(len(c)):
            dst = np.append(dst,cv2.pointPolygonTest(c[i], pt, True)) #0: on contour, pos: inside contour, neg: outside contour 

        
        if dst.shape[0]>0:
            dst[dst>0] = 0 #set 0 pix if point is inside contour
            dst_vec = np.append(dst_vec, dst.max()*reso_mm) #min distance in pixel is max value
        
    dst_vec = np.abs(dst_vec)
    if dst_vec.shape[0]>0:
        m=int(100*dst_vec.mean())/100
        s=int(100*dst_vec.std())/100
    else:
        m=np.nan
        s=np.nan

    dist_vec = np.append(dist_vec,dst_vec)
    return dist_vec, m, s

def get_mean_std_distance_to_EBS2(ROIs, idx_epicenter, fOptics_mask, reso_mm, Thresh_activity, contour_function):
    
    #Compute EBS and fOptics grid
    fOptics_grid = get_square_ROI_idx_functional_areas(fOptics_mask, ROIs, Thresh_activity)

    # Create mask function
    mask_function = np.zeros(fOptics_mask.shape,dtype=np.uint8)
    cv2.drawContours(mask_function,contour_function,-1,255,cv2.FILLED)

    #Only keep fOptics mask in eloquent cortex
    fOptics_mask_in_eloquent_cortex = np.bitwise_and(fOptics_mask,mask_function)

    #Remove small blobs (smaller than 25% of EBS reso)
    fOptics_mask_in_eloquent_cortex = remove_small_contour(fOptics_mask_in_eloquent_cortex,Thresh_activity)

    dst_vec = np.array([])

    #Loop over EBS epicenters
    for id in range(idx_epicenter.shape[0]):

        
        #Check if fOptics grid detected the epicenter
        if fOptics_grid[idx_epicenter[id]]:
            dst_vec = np.append(dst_vec, 0)
            continue
        
        pt = get_ROI_center(ROIs[idx_epicenter[id]])
        c, hierarchy = cv2.findContours(fOptics_mask_in_eloquent_cortex.copy(),cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)

        dst = np.array([])
        for i in range(len(c)):
            dst = np.append(dst,cv2.pointPolygonTest(c[i], pt, True)) #0: on contour, pos: inside contour, neg: outside contour 

        
        if dst.shape[0]>0:
            dst[dst>0] = 0 #set 0 pix if point is inside contour
            dst_vec = np.append(dst_vec, dst.max()*reso_mm) #min distance in pixel is max value

    dst_vec = np.abs(dst_vec)
    
    
    if dst_vec.shape[0]>0:
        #Get mean distance
        m=int(100*dst_vec.mean())/100
        
        #Get the number of detected epicenters
        nb = dst_vec.shape[0]

    else:
        m=np.nan
        nb = 0

    return m, nb


def get_practical_sensitivity(nb_areas_detected_in_eloquent_cortex, nb_epicenters):
    if nb_epicenters==0:
        return np.nan
    return nb_areas_detected_in_eloquent_cortex/nb_epicenters

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



def get_epicenter(fmask_filename, input_img, f, color_function):
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

    #Calculate centroid of each contour
    contour_centers = []
    for cnt in c:
        M = cv2.moments(cnt)
        if M["m00"] != 0:  
            cx = int(M["m10"] / M["m00"])
            cy = int(M["m01"] / M["m00"])
            contour_centers.append((cx, cy))

    return contour_centers
    

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


def calculate_epicenter_sensitivity(mask_stat, epicenters):
    Sensitivity = 0
    
    for epicenter in epicenters:
        if mask_stat[epicenter[1],epicenter[0]] > 0:
            Sensitivity += 100
        
    if len(epicenters) == 0:
        return np.nan

    return Sensitivity/len(epicenters)


def calculate_sensitivity_specificity(mask_stat, mask_function, TP, FN, TN, FP):
    
    Sensitivity = np.nan
    Specificity = np.nan
    Sensitivity_relaxed = np.nan
    #Global sensitivity and specificity
    if(mask_function.sum() > 0):
        #Sensitivity
        Sensitivity = TP.sum() /  (TP.sum() + FN.sum())
        Sensitivity = int(Sensitivity*10000)
        Sensitivity = Sensitivity/100
        
        #Specificity
        Specificity = TN.sum()/ (TN.sum() + FP.sum())
        Specificity = int(Specificity*10000)
        Specificity = Specificity/100

        if mask_stat.sum() == 0:
            Sensitivity = 0
            Specificity = 100

    return Sensitivity, Specificity




def get_sensitivity_specificity_contours(mask_stat,mask, contours=contour_brain_functions()):
    # Sensi = TP / (TP +FN)
    # Speci = TN/ (TN + FP)



    # Create mask motor
    mask_motor = np.zeros(mask.shape,dtype=np.uint8)
    cv2.drawContours(mask_motor,contours.c_motor,-1,1,cv2.FILLED)

    # Create mask sensory
    mask_sensory = np.zeros(mask.shape,dtype=np.uint8)
    cv2.drawContours(mask_sensory,contours.c_sensory,-1,1,cv2.FILLED)

    # Get functional masks of all functional areas
    mask_functional = np.bitwise_or(mask_motor,mask_sensory)


    # TP: true positives: number of pixels of mask_stats inside mask_motor or mask_sensi
    TP_motor = np.bitwise_and(mask_stat,mask_motor)
    TP_sensory = np.bitwise_and(mask_stat,mask_sensory)
    TP = np.bitwise_and(mask_stat,mask_functional)

    # FN: False negatives: number of pixels for which the ground truth detected activated brain areas but not mask_stat
    FN_motor = np.bitwise_and(mask_motor,np.bitwise_not(mask_stat))
    FN_sensory = np.bitwise_and(mask_sensory,np.bitwise_not(mask_stat))
    FN = np.bitwise_and(mask_functional,np.bitwise_not(mask_stat))


    # TN: True negatives: number of pixels for which mask_stats and the gold standard did not detect activated brain areas
    TN = np.bitwise_and(mask, np.bitwise_and(np.bitwise_not(mask_functional), np.bitwise_not(mask_stat)))
    #Apply openning to remove isolated blobs
    TN = cv2.morphologyEx(TN, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5)),iterations=2)


    # FP: False positives: number of pixels for which the functional optics detected activated brain areas but not the ground truth
    FP = np.bitwise_and(mask, np.bitwise_not(mask_functional))
    #Apply openning to remove isolated blobs
    FP = cv2.morphologyEx(FP, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5)),iterations=2)
    FP = np.bitwise_and(FP, mask_stat)











    # Calculate sensitivity and specificity for each functional area
    results = Sensitivity_specificity_metrics()
    results.Sensitiviy_motor, results.Specificity = calculate_sensitivity_specificity(mask_stat, mask_motor, TP_motor, FN_motor, TN, FP)
    results.Sensitiviy_sensory, _ = calculate_sensitivity_specificity(mask_stat, mask_sensory, TP_sensory, FN_sensory, TN, FP)

    return results





def get_sensitivity_specificity_epicenters(mask_stat, mask_EBS_tot, mask_EBS_motor, mask_EBS_sensory):

    # Sensi = TP / (TP +FN)
    # Speci = TN/ (TN + FP)



    # TP: true positives: number of pixels of mask_stats inside mask_motor or mask_sensi
    TP_motor = np.bitwise_and(mask_stat,mask_EBS_motor)
    TP_sensory = np.bitwise_and(mask_stat,mask_EBS_sensory)

    # FN: False negatives: number of pixels for which the ground truth detected activated brain areas but not mask_stat
    FN_motor = np.bitwise_and(mask_EBS_motor,np.bitwise_not(mask_stat))
    FN_sensory = np.bitwise_and(mask_EBS_sensory,np.bitwise_not(mask_stat))



    # Calculate sensitivity and specificity for each functional area
    results = Sensitivity_specificity_metrics()
    results.Sensitiviy_motor = TP_motor.sum() /  (TP_motor.sum() + FN_motor.sum())
    results.TP_motor = TP_motor.sum()
    results.FN_motor = FN_motor.sum()
    results.nb_EBS_motor = mask_EBS_motor.sum()
    
    results.Sensitiviy_sensory = TP_sensory.sum() /  (TP_sensory.sum() + FN_sensory.sum())
    results.TP_sensory = TP_sensory.sum()
    results.FN_sensory = FN_sensory.sum()
    results.nb_EBS_sensory = mask_EBS_sensory.sum()


    return results












def convert_metric_list_to_np(metric, metric_dist):
    #Create array for saving xlsx file
    
    np_metric_dist = Distance_metrics()
    np_metric_dist.mean_dist = np.array([])
    np_metric_dist.std_dist = np.array([])

    np_metric_dist.mean_dist_motor = np.array([])
    np_metric_dist.std_dist_motor = np.array([])

    np_metric_dist.mean_dist_sensory = np.array([])
    np_metric_dist.std_dist_sensory = np.array([])

    np_metric_dist.Patient_id = np.array([])
    np_metric_dist.analysis_type = np.array([])
    
    for data in metric_dist:
        np_metric_dist.mean_dist = np.append(np_metric_dist.mean_dist, data.mean_dist)
        np_metric_dist.std_dist = np.append(np_metric_dist.std_dist, data.std_dist)

        np_metric_dist.mean_dist_motor = np.append(np_metric_dist.mean_dist_motor, data.mean_dist_motor)
        np_metric_dist.std_dist_motor = np.append(np_metric_dist.std_dist_motor, data.std_dist_motor)

        np_metric_dist.mean_dist_sensory = np.append(np_metric_dist.mean_dist_sensory, data.mean_dist_sensory)
        np_metric_dist.std_dist_sensory = np.append(np_metric_dist.std_dist_sensory, data.std_dist_sensory)

        np_metric_dist.Patient_id = np.append(np_metric_dist.Patient_id, data.Patient_id)
        np_metric_dist.analysis_type = np.append(np_metric_dist.analysis_type, data.analysis_type)

        
    
    np_metric = Sensitivity_specificity_metrics()
    np_metric.Patient_id = np.array([])
    np_metric.analysis_type = np.array([])

    np_metric.Sensitiviy_motor = np.array([])
    np_metric.Sensitiviy_sensory = np.array([])

    # All
    np_metric.Specificity = np.array([])

    for data in metric:

        np_metric.Sensitiviy_sensory = np.append(np_metric.Sensitiviy_sensory, data.Sensitiviy_sensory)
        np_metric.Sensitiviy_motor = np.append(np_metric.Sensitiviy_motor, data.Sensitiviy_motor)
        np_metric.Specificity = np.append(np_metric.Specificity, data.Specificity)
        np_metric.Patient_id = np.append(np_metric.Patient_id, data.Patient_id)
        np_metric.analysis_type = np.append(np_metric.analysis_type, data.analysis_type)

    return np_metric, np_metric_dist


def convert_vector_into_str(in_data):
    data_out = np.array([])
    for d in in_data:
      v = "-" if np.isnan(d) else "$"+str(d)+"$"
      data_out = np.append(data_out,v)
    return data_out

def convert_dist_sensi_values_in_str(dist, nb_detec, nb_tot, sensi):
    # if np.all(np.isnan(arr)):
    #     v = "(0/"+str(arr.shape[0])+")"
    # else:
    #     v = f"${np.nanmean(arr):.2f} \pm " + f"${np.nanstd(arr):.2f}$ (" + str(np.count_nonzero(~np.isnan(arr)))+"/"+str(arr.shape[0])+")"
    if np.all(np.isnan(dist)):
        v = "-\n("+str(0)+"/"+str(nb_tot)+")"
    else:
        v = f"${np.nanmean(sensi):.2f} \% \pm " + f"{np.nanstd(sensi):.2f}\%$ ("+str(nb_detec)+"/"+str(nb_tot)+")\n"+f"${np.nanmean(dist):.2f}$ mm $\pm " + f"{np.nanstd(dist):.2f}$ mm"
    return v

def convert_sensi_values_in_str(sensi, nb_detec, nb_tot):
    # if np.all(np.isnan(arr)):
    #     v = "(0/"+str(arr.shape[0])+")"
    # else:
    #     v = f"${np.nanmean(arr):.2f} \pm " + f"${np.nanstd(arr):.2f}$ (" + str(np.count_nonzero(~np.isnan(arr)))+"/"+str(arr.shape[0])+")"
    if np.all(np.isnan(sensi)):
        v = "-\n("+str(0)+"/"+str(nb_tot)+")"
    else:
        v = f"${np.nanmean(sensi):.2f} \pm " + f"{np.nanstd(sensi):.2f}$ ("+str(nb_detec)+"/"+str(nb_tot)+")"
    return v


def convert_speci_values_in_str(arr):
    # if np.all(np.isnan(arr)):
    #     v = "(0/"+str(arr.shape[0])+")"
    # else:
    #     v = f"${np.nanmean(arr):.2f} \pm " + f"${np.nanstd(arr):.2f}$ (" + str(np.count_nonzero(~np.isnan(arr)))+"/"+str(arr.shape[0])+")"
    if np.all(np.isnan(arr)):
        v = "-"
    else:
        v = f"${np.nanmean(arr):.2f} \pm " + f"{np.nanstd(arr):.2f}$"
    
    return v


def convert_dist_values_in_str(dist, tot):

    dist2 = dist[dist>0]
            
    # if dist2.shape[0]>0:
    #     practical_sensi = "$"+str(non_null_values)+"$/$"+str(tot)+"$"
    #     v = f"${dist2.mean():.2f} \pm "+f"{dist2.std():.2f}$ ("+practical_sensi+")"
    # else:
    #     v = "(0/"+str(tot)+")"
    if dist2.shape[0]>0:
        practical_sensi = "$"+str(dist2.shape[0])+"$/$"+str(tot)+"$"
        v = f"${dist2.mean():.2f} \pm "+f"{dist2.std():.2f}$ ("+practical_sensi+")"
    else:
        v = "(0/"+str(tot)+")"
    return v

def write_metric_results_Latex(filename, np_metric_auto, np_metric_SPM,
                               np_metric_dist_auto, np_metric_dist_SPM):

    dist_auto = np.array([])
    dist_SPM = np.array([])

    dist_motor_auto = np.array([])
    dist_sensory_auto = np.array([])

    dist_motor_SPM = np.array([])
    dist_sensory_SPM = np.array([])

    for i in range(np_metric_dist_auto.mean_dist_motor.shape[0]):

        # if np.isnan(np_metric_dist_auto.mean_dist[i]):
        #     dist_auto = np.append(dist_auto,"-")
        # else:
        #     dist_auto = np.append(dist_auto,"$"+str(np_metric_dist_auto.mean_dist[i])+" \pm "+str(np_metric_dist_auto.std_dist[i])+"$")
        
        # if np.isnan(np_metric_dist_SPM.mean_dist[i]):
        #     dist_SPM = np.append(dist_SPM,"-")
        # else:
        #     dist_SPM = np.append(dist_SPM,"$"+str(np_metric_dist_SPM.mean_dist[i])+" \pm "+str(np_metric_dist_SPM.std_dist[i])+"$")

    
        if np.isnan(np_metric_dist_auto.mean_dist_motor[i]):
            dist_motor_auto = np.append(dist_motor_auto,"-")
        else:
            dist_motor_auto = np.append(dist_motor_auto,"$"+str(np_metric_dist_auto.mean_dist_motor[i])+"$")
        
        if np.isnan(np_metric_dist_auto.mean_dist_sensory[i]):
            dist_sensory_auto = np.append(dist_sensory_auto,"-")
        else:
            dist_sensory_auto = np.append(dist_sensory_auto,"$"+str(np_metric_dist_auto.mean_dist_sensory[i])+"$")


        if np.isnan(np_metric_dist_SPM.mean_dist_motor[i]):
            dist_motor_SPM = np.append(dist_motor_SPM,"-")
        else:
            dist_motor_SPM = np.append(dist_motor_SPM,"$"+str(np_metric_dist_SPM.mean_dist_motor[i])+"$")
        
        if np.isnan(np_metric_dist_SPM.mean_dist_sensory[i]):
            dist_sensory_SPM = np.append(dist_sensory_SPM,"-")
        else:
            dist_sensory_SPM = np.append(dist_sensory_SPM,"$"+str(np_metric_dist_SPM.mean_dist_sensory[i])+"$")


    df = pd.DataFrame(
    {
    "ID subjects": np_metric_dist_auto.Patient_id,
    # "Analysis type": np_metric_dist_auto.analysis_type,

    # "Dist auto (mm)": dist_auto,
    # "Dist SPM (mm)": dist_SPM,

    "Sensitivity motor auto": convert_vector_into_str(100*np_metric_auto.Sensitiviy_motor),
    "Sensitivity motor SPM": convert_vector_into_str(100*np_metric_SPM.Sensitiviy_motor),

    "Dist motor auto (mm)": dist_motor_auto,   
    "Dist motor SPM (mm)": dist_motor_SPM,

    "Sensitivity sensory auto": convert_vector_into_str(100*np_metric_auto.Sensitiviy_sensory),
    "Sensitivity sensory SPM": convert_vector_into_str(100*np_metric_SPM.Sensitiviy_sensory),
    
    "Dist sensory auto (mm)": dist_sensory_auto,
    "Dist sensory SPM (mm)": dist_sensory_SPM,

    "Specificity auto": convert_vector_into_str(np_metric_auto.Specificity),
    "Specificity SPM": convert_vector_into_str(np_metric_SPM.Specificity),

    })


    #Add mean and std values
    mean_std_values = {
    "ID subjects": "Mean +- Std",
    "Analysis type": "",
    
    "Sensitivity motor auto": convert_sensi_values_in_str(100*np_metric_auto.Sensitiviy_motor,
                                                               np_metric_auto.TP_motor.sum(),
                                                               np_metric_auto.nb_EBS_motor.sum()),

    "Dist motor auto (mm)": convert_dist_sensi_values_in_str(np_metric_dist_auto.mean_dist_motor,
                                                             np_metric_dist_auto.tot_motor_areas.sum(),
                                                             np_metric_auto.nb_EBS_motor.sum(),
                                                             100*np_metric_dist_auto.mean_practical_sensi_motor),


    "Sensitivity motor SPM": convert_sensi_values_in_str(100*np_metric_SPM.Sensitiviy_motor,
                                                               np_metric_SPM.TP_motor.sum(),
                                                               np_metric_auto.nb_EBS_motor.sum()),
    "Dist motor SPM (mm)": convert_dist_sensi_values_in_str(np_metric_dist_SPM.mean_dist_motor,
                                                             np_metric_dist_SPM.tot_motor_areas.sum(),
                                                             np_metric_auto.nb_EBS_motor.sum(),
                                                             100*np_metric_dist_SPM.mean_practical_sensi_motor),



    "Sensitivity sensory auto": convert_sensi_values_in_str(100*np_metric_auto.Sensitiviy_sensory,
                                                               np_metric_auto.TP_sensory.sum(),
                                                               np_metric_auto.nb_EBS_sensory.sum()),

    "Dist sensory auto (mm)": convert_dist_sensi_values_in_str(np_metric_dist_auto.mean_dist_sensory,
                                                             np_metric_dist_auto.tot_sensory_areas.sum(),
                                                             np_metric_auto.nb_EBS_sensory.sum(),
                                                             100*np_metric_dist_auto.mean_practical_sensi_sensory),


    "Sensitivity sensory SPM": convert_sensi_values_in_str(100*np_metric_SPM.Sensitiviy_sensory,
                                                               np_metric_SPM.TP_sensory.sum(),
                                                               np_metric_auto.nb_EBS_sensory.sum()),

    "Dist sensory SPM (mm)": convert_dist_sensi_values_in_str(np_metric_dist_SPM.mean_dist_sensory,
                                                             np_metric_dist_SPM.tot_sensory_areas.sum(),
                                                             np_metric_auto.nb_EBS_sensory.sum(),
                                                             100*np_metric_dist_SPM.mean_practical_sensi_sensory),

    "Specificity auto": convert_speci_values_in_str(np_metric_auto.Specificity),
    "Specificity SPM": convert_speci_values_in_str(np_metric_SPM.Specificity),

    }


    # mean_std_values = {
    # "ID subjects": "Mean +- Std",
    # "Analysis type": "",
    
        
    # "Sensitivity motor auto": "" if np.all(np.isnan(np_metric_auto.Sensitiviy_motor)) else "$"+str(int(100*np.nanmean(np_metric_auto.Sensitiviy_motor))/100) + " \pm " + str(int(100*np.nanstd(np_metric_auto.Sensitiviy_motor))/100) +"$",
    # "Dist motor auto (mm)": "" if np.all(np.isnan(np_metric_dist_auto.mean_dist_motor)) else "$"+str(int(100*np.nanmean(np_metric_dist_auto.mean_dist_motor))/100)+" \pm "+str(int(100*np.nanmean(np_metric_dist_auto.std_dist_motor))/100)+"$",

    # "Sensitivity motor SPM": "" if np.all(np.isnan(np_metric_SPM.Sensitiviy_motor)) else "$"+str(int(100*np.nanmean(np_metric_SPM.Sensitiviy_motor))/100) + " \pm " + str(int(100*np.nanstd(np_metric_SPM.Sensitiviy_motor))/100) +"$",
    # "Dist motor SPM (mm)": "" if np.all(np.isnan(np_metric_dist_SPM.mean_dist_motor)) else "$"+str(int(100*np.nanmean(np_metric_dist_SPM.mean_dist_motor))/100)+" \pm "+str(int(100*np.nanmean(np_metric_dist_SPM.std_dist_motor))/100)+"$",

    # "Sensitivity sensory auto": "" if np.all(np.isnan(np_metric_auto.Sensitiviy_sensory)) else "$"+str(int(100*np.nanmean(np_metric_auto.Sensitiviy_sensory))/100) + " \pm " + str(int(100*np.nanstd(np_metric_auto.Sensitiviy_sensory))/100) +"$",
    # "Dist sensory auto (mm)": "" if np.all(np.isnan(np_metric_dist_auto.mean_dist_sensory)) else "$"+str(int(100*np.nanmean(np_metric_dist_auto.mean_dist_sensory))/100)+" \pm "+str(int(100*np.nanmean(np_metric_dist_auto.std_dist_sensory))/100)+"$",

    # "Sensitivity sensory SPM": "" if np.all(np.isnan(np_metric_SPM.Sensitiviy_sensory)) else "$"+str(int(100*np.nanmean(np_metric_SPM.Sensitiviy_sensory))/100) + " \pm " + str(int(100*np.nanstd(np_metric_SPM.Sensitiviy_sensory))/100) +"$",
    # "Dist sensory SPM (mm)": "" if np.all(np.isnan(np_metric_dist_SPM.mean_dist_sensory)) else "$"+str(int(100*np.nanmean(np_metric_dist_SPM.mean_dist_sensory))/100)+" \pm "+str(int(100*np.nanmean(np_metric_dist_SPM.std_dist_sensory))/100)+"$",

    # "Specificity auto": "" if np.all(np.isnan(np_metric_auto.Specificity)) else "$"+str(int(100*np.nanmean(np_metric_auto.Specificity))/100) + " \pm " + str(int(100*np.nanstd(np_metric_auto.Specificity))/100) +"$",
    # "Specificity SPM": "" if np.all(np.isnan(np_metric_SPM.Specificity)) else "$"+str(int(100*np.nanmean(np_metric_SPM.Specificity))/100) + " \pm " + str(int(100*np.nanstd(np_metric_SPM.Specificity))/100) +"$",

    # }
    df.loc[len(df)] = mean_std_values

    #Add popution nb
    # Pop_number = {
    # "ID subjects": "Pop",
    # "Analysis type": "",
    
    # "Sensitivity motor auto": str((~np.isnan(np_metric_auto.Sensitiviy_motor)).sum()),
    # "Dist motor auto (mm)": str((~np.isnan(np_metric_dist_auto.mean_dist_motor)).sum()),

    # "Sensitivity motor SPM": str((~np.isnan(np_metric_SPM.Sensitiviy_motor)).sum()),
    # "Dist motor SPM (mm)": str((~np.isnan(np_metric_dist_SPM.mean_dist_motor)).sum()),

    # "Sensitivity sensory auto": str((~np.isnan(np_metric_auto.Sensitiviy_sensory)).sum()),
    # "Dist sensory auto (mm)": str((~np.isnan(np_metric_dist_auto.mean_dist_sensory)).sum()),

    # "Sensitivity sensory SPM": str((~np.isnan(np_metric_SPM.Sensitiviy_sensory)).sum()),
    # "Dist sensory SPM (mm)": str((~np.isnan(np_metric_dist_SPM.mean_dist_sensory)).sum()),

    # "Specificity auto": str((~np.isnan(np_metric_auto.Specificity)).sum()),
    # "Specificity SPM": str((~np.isnan(np_metric_SPM.Specificity)).sum()),

    # # "Dist auto (mm)": "$"+str(int(100*np.nanmean(np_metric_dist_auto.mean_dist))/100)+" \pm "+str(int(100*np.nanmean(np_metric_dist_auto.std_dist))/100)+"$",
    # # "Dist SPM (mm)": "$"+str(int(100*np.nanmean(np_metric_dist_SPM.mean_dist))/100)+" \pm "+str(int(100*np.nanmean(np_metric_dist_SPM.std_dist))/100)+"$",
    
    # }
    # df.loc[len(df)] = Pop_number


    df.to_excel(filename)
            

def write_metric_results2(filename, np_metric_auto, np_metric_SPM, np_metric_dist_auto, np_metric_dist_SPM):

    df = pd.DataFrame(
    {
    "ID subjects": np_metric_dist_auto.Patient_id,
    # "Analysis type": np_metric_dist_auto.analysis_type,

    # "Dist auto (mm)": dist_auto,
    # "Dist SPM (mm)": dist_SPM,

    "Sensitivity motor auto": np_metric_auto.Sensitiviy_motor,
    "Sensitivity motor SPM": np_metric_SPM.Sensitiviy_motor,

    "Dist motor auto (mm)": np_metric_dist_auto.mean_dist_motor,   
    "Dist motor SPM (mm)": np_metric_dist_SPM.mean_dist_motor,

    "Sensitivity sensory auto": np_metric_auto.Sensitiviy_sensory,
    "Sensitivity sensory SPM": np_metric_SPM.Sensitiviy_sensory,
    
    "Dist sensory auto (mm)": np_metric_dist_auto.mean_dist_sensory,
    "Dist sensory SPM (mm)": np_metric_dist_SPM.mean_dist_sensory,

    "Specificity auto": np_metric_auto.Specificity,
    "Specificity SPM": np_metric_SPM.Specificity,

    })

    df.to_excel(filename)
   


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

def create_json_patient(json_file, metric_auto = Sensitivity_specificity_metrics(),
                        metric_SPM = Sensitivity_specificity_metrics(),
                        dist_auto = Distance_metrics(),
                        dist_SPM = Distance_metrics(),
                        nb_c_motor=1, nb_c_sensory=0):

    df = pd.DataFrame(
    {
    "Patient ID": [dist_auto.Patient_id],
    "Analysis type": [dist_auto.analysis_type],

    "Sensitivity motor auto": [metric_auto.Sensitiviy_motor],
    "Sensitivity motor SPM": [metric_SPM.Sensitiviy_motor],

    "Sensitivity sensory auto": [metric_auto.Sensitiviy_sensory],
    "Sensitivity sensory SPM": [metric_SPM.Sensitiviy_sensory],

    "Specificity auto": [metric_auto.Specificity],
    "Specificity SPM": [metric_SPM.Specificity],

    "TP_motor_auto":[metric_auto.TP_motor],
    "TP_motor_SPM":[metric_SPM.TP_motor],
    "FN_motor_auto":[metric_auto.FN_motor],
    "FN_motor_SPM":[metric_SPM.FN_motor],

    "TP_sensory_auto":[metric_auto.TP_sensory],
    "TP_sensory_SPM":[metric_SPM.TP_sensory],
    "FN_sensory_auto":[metric_auto.FN_sensory],
    "FN_sensory_SPM":[metric_SPM.FN_sensory],


    "Mean Dist motor auto (mm)": [dist_auto.mean_dist_motor],
    "Mean Dist motor SPM (mm)": [dist_SPM.mean_dist_motor],
    "Mean Dist sensory auto (mm)": [dist_auto.mean_dist_sensory],
    "Mean Dist sensory SPM (mm)": [dist_SPM.mean_dist_sensory],

    "Mean practical sensi motor auto": [dist_auto.mean_practical_sensi_motor],
    "Mean practical sensi motor SPM": [dist_SPM.mean_practical_sensi_motor],

    "Mean practical sensi sensory auto": [dist_auto.mean_practical_sensi_sensory],
    "Mean practical sensi sensory SPM": [dist_SPM.mean_practical_sensi_sensory],

    "nb EBS motor": [nb_c_motor], 
    "nb EBS sensory": [nb_c_sensory],

    "nb detected motor auto": [dist_auto.tot_motor_areas],
    "nb detected motor SPM": [dist_SPM.tot_motor_areas],
    "nb detected sensory auto": [dist_auto.tot_sensory_areas],
    "nb detected sensory SPM": [dist_SPM.tot_sensory_areas],
    
        


    # "Mean Dist auto (mm)": [dist_auto.mean_dist],
    # "Mean Dist SPM (mm)": [dist_SPM.mean_dist],

    # "Std Dist motor auto (mm)": [dist_auto.std_dist_motor],
    # "Std Dist motor SPM (mm)": [dist_SPM.std_dist_motor],
    # "Std Dist sensory auto (mm)": [dist_auto.std_dist_sensory],
    # "Std Dist sensory SPM (mm)": [dist_SPM.std_dist_sensory],
    # "Std Dist auto (mm)": [dist_auto.std_dist],
    # "Std Dist SPM (mm)": [dist_SPM.std_dist],
    })

    # Save json
    df.to_json(json_file, orient="records", indent=2)



def get_sample_values(path_json, key):
    json_files = glob.glob(path_json+"/*.json")
    sample = []

    for f in json_files:
        #read json and store results in objects
        df = pd.read_json(f, orient="records")
        if np.isnan(df[key][0]):
            continue
        sample.append(df[key][0])

    return sample