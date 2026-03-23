#!/usr/bin/env python
# -*-coding:Utf-8 -*

from pathlib import Path
import numpy as np
from sklearn.decomposition import FastICA
import matplotlib.pyplot as plt
# from sklearn import preprocessing
import cv2 as cv
from scipy import stats
from skimage.metrics import structural_similarity as ssim
from skimage.metrics import mean_squared_error




fontscale = 3
fontthickness = 5

def reconstructMask(in_data,img_out,point_pos):
    for i in range(point_pos.shape[0]):
        img_out[point_pos[i,0],point_pos[i,1]] = in_data[i]
    
    return img_out
    
    
def normalize_data(data) :
    for i in range(data.shape[1]):
        data[:,i] = (data[:,i] - np.mean(data[:,i]))/np.std(data[:,i])
        
    data[np.isinf(data)] = 0
    data[np.isnan(data)] = 0
        
        # data[i,:] = data[i,:]/ - np.std(data[i,:])
    return data
    
    
def merge_img(undersample_img_shape, data, pixel_pos, input_img, mask, alpha, median_ksize, morpho_ksize):
    temp = np.zeros((undersample_img_shape[0],undersample_img_shape[1]))
    temp = reconstructMask(data,temp,pixel_pos)
    
    
    
    max = 1
    min = -1
    
    temp = cv2.resize(temp,(input_img.shape[1],input_img.shape[0]))
    
    
    #Normalize
    res = np.zeros(temp.shape)
    res[temp>0]= (temp[temp>0] * ((255-127)/(max))) + 127;
    res[temp<=0]= (temp[temp<=0] -min)* (127/(-min));
    res = res.astype(np.uint8)
    
    res = cv2.medianBlur(res,median_ksize)
    
    contours = thresh_img(res,mask,morpho_ksize)

    
    #Apply colormap
    res = cv2.applyColorMap(res, cv2.COLORMAP_JET)
        
    #Merge
    out = np.zeros(input_img.shape)
    # res = cv2.resize(res,(input_img.shape[1],input_img.shape[0]))
    for i in range(input_img.shape[0]):
        for j in range(input_img.shape[1]):
            if mask[i,j] == 0 :
                out[i,j,:] = input_img[i,j,:]
            if mask[i,j] != 0 : 
                out[i,j,:] = alpha*res[i,j,:] + (1-alpha)* input_img[i,j,:]
    out = out.astype(np.uint8)
    
    return out,contours
    
    
    
    

def thresh_img(data, mask, k_size, mod='no mod',val = 0.75):
    
    mean_val = np.mean(data[mask>0])
    std_val = np.std(data[mask>0])
        
    thresh = np.zeros(mask.shape)
    
    mod_val = mod
    
    if(mod=='no mod'):
        if(mean_val>127):
            mod_val = 'pos'
            thresh_val = mean_val + val*std_val
            thresh[data>thresh_val] = 1
        else:
            mod_val = 'neg'
            thresh_val = mean_val - val*std_val
            thresh[data<thresh_val] = 1
    else:
        if(mod=='pos'):
            thresh_val = mean_val + val*std_val
            thresh[data>thresh_val] = 1
        if(mod=='neg'):
            thresh_val = mean_val - val*std_val
            thresh[data<thresh_val] = 1
            
    print('Thresh mod: '+str(mod_val)+' thresh_val: '+str(thresh_val))
    

    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(k_size,k_size))
    thresh = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel)
    thresh = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel)
    thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)
    thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)
    
    
    contours, hierarchy = cv2.findContours(thresh.astype(np.uint8), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    

    return contours
        

    

    


    
## Main
alpha = 0.5

# video 25/10/19
path = "/home/caredda/temp/P26/"
M_pos = (1004,668)
M_txt = (984,648)

#Load data
c = 0
data = np.loadtxt(path+"Filtered_Contrast_resting-state/MBLL_"+str(c)+".txt") #shape Time ,Pixels
pixel_pos = np.loadtxt(path+"pixel_pos.txt").astype(int) #Shape Pixels, 2
img_shape = (191,265)
input_img = cv.imread(path+"initial_img.png")
mask = cv.imread(path+"mask.png",cv.IMREAD_GRAYSCALE)


#normalize data for each pixel
data = normalize_data(data)

#Process fastICA
K = 5
transformer = FastICA(n_components=K, random_state=0, whiten='unit-variance')
transformer.fit_transform(data)

#Get mixing matrix
Mixing_Mat = transformer.mixing_  # Get estimated mixing matrix, shape Pixels, K

#unmixing matrix
unMixing_Mat = np.linalg.pinv(Mixing_Mat) #Shape K, Pixels

#Normalize mixing and unmixing matrices
for k in range(K):
    Mixing_Mat[:,k] = Mixing_Mat[:,k]/np.abs(np.max(Mixing_Mat[:,k]))
    unMixing_Mat[k,:] = unMixing_Mat[k,:]/np.abs(np.max(unMixing_Mat[k,:]))


morpho_ksize = 41
median_ksize = 31
#reconstruct mixing mat
r_Mixing_Mat = np.zeros((input_img.shape[0],input_img.shape[1],3,K),dtype=np.uint8)
c_mixing = []
r_unMixing_Mat = np.zeros((input_img.shape[0],input_img.shape[1],3,K),dtype=np.uint8)
c_unmixing = []

for k in range(K):
    r_Mixing_Mat[:,:,:,k],c = merge_img(img_shape, Mixing_Mat[:,k], pixel_pos, input_img, mask, alpha, median_ksize, morpho_ksize)
    c_mixing.append(c)
    
    r_unMixing_Mat[:,:,:,k],c = merge_img(img_shape, unMixing_Mat[k,:], pixel_pos, input_img, mask, alpha, median_ksize, morpho_ksize)
    c_unmixing.append(c)
        

    
    
#Merge with initial image

plt.close('all')
plt.figure()
for k in range(K):
    plt.subplot(2,3,k+1)
    display_img = cv.cvtColor(r_Mixing_Mat[:,:,:,k],cv.COLOR_BGR2RGB)
    cv.drawContours(display_img,c_mixing[k],-1,(0,0,0),2)
    
    plt.imshow(display_img)
    plt.title("Mixing matrix "+str(k+1))
plt.show()

plt.figure()
for k in range(K):
    plt.subplot(2,3,k+1)
    display_img = cv.cvtColor(r_unMixing_Mat[:,:,:,k],cv.COLOR_BGR2RGB)
    cv.drawContours(display_img,c_unmixing[k],-1,(0,0,0),2)
    
    plt.imshow(display_img)
    plt.title("Unmixing matrix "+str(k+1))
plt.show()


