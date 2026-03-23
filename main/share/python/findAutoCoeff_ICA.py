#!/usr/bin/env python
# -*-coding:Utf-8 -*

# Current version

import numpy as np
import matplotlib.pyplot as plt
from sklearn.decomposition import  FastICA
import os
import sys


if len(sys.argv) > 1:
    data_path = sys.argv[1]

def apply_filter(in_data,filter):

    spectra = np.fft.fft(in_data)
    spectra = np.multiply(spectra,filter)

    return np.fft.ifft(spectra).real

def get_spectral_Sources(in_data,nb_IC):
    #Normalize data
    # X = normalize_data(in_data)
    X = in_data
    #Process fastICA
    transformer = FastICA(n_components=nb_IC,random_state=0)
    return transformer.fit_transform(X)

def normalize_data(data) :
    out = np.zeros(data.shape)
    for i in range(data.shape[1]):
        out[:,i] = (data[:,i] - np.mean(data[:,i]))/np.std(data[:,i])

    out[np.isinf(out)] = 0
    out[np.isnan(out)] = 0

        # data[i,:] = data[i,:]/ - np.std(data[i,:])

    return out


def invertMatrix(in_mat):

    # E_RGB_inv_A = inv(E_RGB_detectA'*E_RGB_detectA)*E_RGB_detectA' ;
    out = np.dot(np.linalg.pinv(np.dot(np.transpose(in_mat),in_mat)),np.transpose(in_mat))
    return out

def getAbsorbanceChanges(I,start_ref,end_ref):

    dA = np.zeros(I.shape)
    for k in range(I.shape[1]):
        dA[:,k] =np.log10(np.divide(np.mean(I[start_ref:end_ref,k]),I[:,k]))
    return dA



print("1")
#Load mean intensity
I = np.loadtxt(data_path+"/Mean_Intensity_Values.txt")
#Load cardiac filter
cardiac_filter = np.loadtxt(data_path+"/Cardiac_Filter.txt") #frequencies not centered



infos = np.loadtxt(data_path+"/infos.txt")
Fe = infos[2]
start_ref = int(infos[0])
end_ref = int(infos[1])


#get absorbance changes
dA = getAbsorbanceChanges(I,start_ref,end_ref)

#Filter data
dA_filtered = np.zeros(dA.shape)
for k in range(dA_filtered.shape[1]):
    dA_filtered[:,k] = apply_filter(dA[:,k] ,cardiac_filter)

#normalize
dA_filtered = normalize_data(dA_filtered)

#Compute FastICA
transformer = FastICA(n_components=2,random_state=0, max_iter=1000, tol=1e-8)
ICA_base =  transformer.fit_transform(dA_filtered.T) #return (n_samples,n_components)
ICA_base_inv= invertMatrix(ICA_base)


print("2")
#Write results
np.savetxt(data_path+"/ICA_base.txt",ICA_base)
np.savetxt(data_path+"/ICA_base_inv.txt",ICA_base_inv)

plt.close('all')
plt.figure()
plt.subplot(121)
plt.plot(dA)
plt.subplot(122)
plt.plot(dA_filtered)
plt.show()

F = np.linspace(-Fe/2,Fe/2,dA.shape[0])
plt.figure()
plt.subplot(121)
plt.plot(F,np.abs(np.fft.fftshift(np.fft.fft(dA[:,0]))),'r')
plt.plot(F,np.abs(np.fft.fftshift(np.fft.fft(dA[:,1]))),'g')
plt.plot(F,np.abs(np.fft.fftshift(np.fft.fft(dA[:,2]))),'b')

plt.subplot(122)
plt.plot(F,np.abs(np.fft.fftshift(np.fft.fft(dA_filtered[:,0]))),'r')
plt.plot(F,np.abs(np.fft.fftshift(np.fft.fft(dA_filtered[:,1]))),'g')
plt.plot(F,np.abs(np.fft.fftshift(np.fft.fft(dA_filtered[:,2]))),'b')
plt.show()

print("3")
##




