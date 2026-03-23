#!/usr/bin/env python
# -*-coding:Utf-8 -*


#!/usr/bin/env python
# -*-coding:Utf-8 -*

from sklearn.decomposition import FastICA, PCA
import matplotlib.pyplot as plt
import numpy as np
import os
import sys


if len(sys.argv) > 1:
    path = sys.argv[1]

def Cardiac_infos_Filter(data,Good_FFT):
    #INIT
    NbSamples = np.size(data,2)
    filter        = np.zeros(NbSamples,dtype=int)
    threshVal = 2

    if not Good_FFT:
        #List Empty
        for i in range(0,np.size(data,0)):
            thresh = np.max(np.abs(data[i,0,:]))/threshVal
            for j in range(0,NbSamples):
                if np.abs(data[i,0,j])> thresh :
                    filter[j]=1
    else:
        #List Not Empty
        for i in range(0,len(Good_FFT)):
            thresh = np.max(np.abs(data[Good_FFT[i],0,:]))/threshVal
            for j in range(0,NbSamples):
                if np.abs(data[Good_FFT[i],0,j])> thresh :
                    filter[j]=1

    return filter


# Main
print("1")

#Load temporal mean files
data = data = np.loadtxt(path+"/temporal_mean.txt")
NbSamples = data.shape[0]

# Redress data
x = np.linspace(0, NbSamples-1, NbSamples)

p = np.polyfit(x, data[:,0], 1)
data[:,0] = data[:,0] - (p[0]*x);

p = np.polyfit(x, data[:,1], 1)
data[:,1] = data[:,1] - (p[0]*x);

p = np.polyfit(x, data[:,2], 1)
data[:,2] = data[:,2] - (p[0]*x);

# Standardize datas
data[:,0] = (data[:,0] - np.mean(data[:,0]))/np.std(data[:,0]);
data[:,1] = (data[:,1] - np.mean(data[:,1]))/np.std(data[:,1]);
data[:,2] = (data[:,2] - np.mean(data[:,2]))/np.std(data[:,2]);


# Compute ICA
ica = FastICA(n_components=3)
S_ = ica.fit_transform(data)  # Reconstruct signals


# Process FFT
FFT = np.array([[np.fft.fft(S_[:,0])],[np.fft.fft(S_[:,1])],[np.fft.fft(S_[:,2])]])


print("2")


# Filter ICA 
stdICA = np.zeros(3)
MaxICA = np.zeros(3)
for i in range(0,3):

    MaxICA[i]=np.max(np.abs(FFT[i,0,:]))
    stdICA[i]=np.std(np.abs(FFT[i,0,:]))


Good_FFT =[]
for i in range(0,3):
    if MaxICA[i]/4 >3*stdICA[i] :
        Good_FFT.append(i)


# Process Filter
filter = Cardiac_infos_Filter(FFT,Good_FFT)


# Save filter in .txt file
np.savetxt(path+"/filter.txt",filter,fmt='%i')


print("3")



# 
# from sklearn.decomposition import FastICA, PCA
# import matplotlib.pyplot as plt
# import numpy as np
# 
# 
# 
# 
# 
# # def Cardiac_infos_Filter(data):
# #     NbSamples = np.size(data,2)
# #     filter        = np.zeros(NbSamples,dtype=int)
# #     for i in range(0,np.size(FFT,0)):
# #         thresh = np.max(np.abs(data[i,0,:]))/2
# #         for j in range(0,NbSamples):
# #             if np.abs(data[i,0,j])> thresh :
# #                 filter[j]=1
# #         
# #     return filter
# 
# def Cardiac_infos_Filter(data):
#     NbSamples = np.size(data,2)
#     filter        = np.zeros(NbSamples,dtype=int)
#     for i in range(0,np.size(FFT,0)):
#         thresh = np.max(np.abs(data[i,0,:]))/2
#         for j in range(0,NbSamples):
#             if np.abs(data[i,0,j])> thresh :
#                 filter[j]=1
#         
#     return filter
# 
# 
# # Main
# 
# 
# 
# #Load temporal mean files
# #data = np.loadtxt("/home/caredda/DVP/files/temporal_mean.txt")
# data = np.loadtxt("temporal_mean.txt")
# NbSamples = data.shape[0]
# 
# # Redress data
# x = np.linspace(0, NbSamples-1, NbSamples)
# 
# p = np.polyfit(x, data[:,0], 1)
# data[:,0] = data[:,0] - (p[0]*x);
# 
# p = np.polyfit(x, data[:,1], 1)
# data[:,1] = data[:,1] - (p[0]*x);
# 
# p = np.polyfit(x, data[:,2], 1)
# data[:,2] = data[:,2] - (p[0]*x);
# 
# # Standardize datas
# data[:,0] = (data[:,0] - np.mean(data[:,0]))/np.std(data[:,0]);
# data[:,1] = (data[:,1] - np.mean(data[:,1]))/np.std(data[:,1]);
# data[:,2] = (data[:,2] - np.mean(data[:,2]))/np.std(data[:,2]);
# 
# 
# # Compute ICA
# ica = FastICA(n_components=3)
# S_ = ica.fit_transform(data)  # Reconstruct signals
# 
# 
# # Process FFT
# FFT = np.array([[np.fft.fft(S_[:,0])],[np.fft.fft(S_[:,1])],[np.fft.fft(S_[:,2])]])
# 
# 
# 
# # Process Filter
# filter = Cardiac_infos_Filter(FFT)
# 
# 
# # Plot result
# # dF  = 30/np.size(FFT,2);
# # F   = np.linspace(0,np.size(FFT,2)-1,np.size(FFT,2))
# # F   = F*dF
# # 
# # plt.figure(1)
# # plt.subplot(4,1, 1)
# # plt.plot(F,np.abs(FFT[0,0,:]))
# # plt.subplot(4,1, 2)
# # plt.plot(F,np.abs(FFT[1,0,:]))
# # plt.subplot(4,1, 3)
# # plt.plot(F,np.abs(FFT[2,0,:]))
# # plt.subplot(4,1, 4)
# # plt.plot(F,filter)
# # plt.show()
# 
# # Save filter in .txt file
# #np.savetxt("/home/caredda/DVP/files/filter.txt",filter,fmt='%i')
# np.savetxt("filter.txt",filter,fmt='%i')
