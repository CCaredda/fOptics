#!/usr/bin/env python
# -*-coding:Utf-8 -*

import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate
import os
import sys


if len(sys.argv) > 2:
    data_path = sys.argv[1]
    res_path =  sys.argv[2]


print("1")
#Load
data = np.loadtxt(res_path+"/infos.txt")
Bold = np.loadtxt(data_path+"/Bold_Signal.txt")


Times = np.zeros((data.shape[0]-2,2))
for i in range(0,Times.shape[0]):
    Times[i,:] = data[i,:]

Fs    = data[data.shape[0]-1 -1,0];
Size  = data[data.shape[0]-1 ,0];

# Define experimental paradigm
t_paradigm = np.arange( 0, Size/Fs, 1/Fs)
y_paradigm = np.zeros(np.size(t_paradigm))

for i in range(0,Times.shape[0]):
    y_paradigm[int( Times[i,0] ): int(Times[i,1])] =1

print("2")
# Define time vector of BOLD signal
t_bold   = np.arange( 0, 20, 20/np.size(Bold));
t_bold_new = np.arange(0,20,1/Fs)

# Interpolation (according Fs value)
tck = interpolate.splrep(t_bold, Bold, s=0)
y_bold_interp = interpolate.splev(t_bold_new, tck, der=0)

# Convolve Bold with pardigme signal
#res_convolve = np.convolve(y_bold_interp,y_paradigm,mode='same')
res_convolve = np.convolve(y_bold_interp,y_paradigm)


#Metabolic response
H_cyt = (t_bold_new**2)*np.exp(-t_bold_new)
Cyt_convolve = np.convolve(H_cyt,y_paradigm)



# Save res in .txt file
np.savetxt(res_path+"/Bold_convolved.txt",res_convolve,fmt='%10.8f')
np.savetxt(res_path+"/Metabolic_response_convolved.txt",Cyt_convolve,fmt='%10.8f')

print("3")



