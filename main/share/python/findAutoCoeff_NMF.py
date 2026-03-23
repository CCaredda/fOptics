#!/usr/bin/env python
# -*-coding:Utf-8 -*


import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backend_bases import MouseButton
import os
from nn_fac.nmf import nmf
import sys


if len(sys.argv) > 1:
    path = sys.argv[1]


def get_Gaussian(Half_width,mean_val,x):
    sig = Half_width/2.3548
    out = (1/(sig*np.sqrt(2*np.pi)))*np.exp(-np.power(x - mean_val,2)/(2*np.power(sig,2)))


    out = out/np.max(out)
    return out

#true matrix shape(lambda,Frequencies)
def nmf_sep(truematrix, k=5, type='mean'):
    # only for rank 2
    # normalize data with l1 if not done already
    truematrix_n = truematrix/np.sum(truematrix,axis=0)
    # find largest norm column
    # find its index in the array
    norms = np.linalg.norm(truematrix_n,axis=0)
    idx_best = np.argmax(norms)
    w0_n = truematrix_n[:,idx_best]
    # find closest to the best for mean
    norms1 = np.linalg.norm(w0_n[:,None] - truematrix_n,axis=0)
    idx_sorted = np.argsort(norms1)
    best_k = idx_sorted[:k] #taking k closest
    # median/mean in original space
    if type=='mean':
        w1 = np.mean(truematrix[:,best_k], axis=1)
    else:
        w1 = np.median(truematrix[:,best_k], axis=1)
    w1_n = w1/np.sum(w1)
    # find furthest column from the best one picked previously
    norms2 = np.linalg.norm(w1_n[:,None] - truematrix_n,axis=0)
    idx_sorted2 = np.argsort(norms2)
    best_k2 = idx_sorted2[-k:]
    # median/mean in original space
    if type=='mean':
        w2 = np.mean(truematrix[:,best_k2], axis=1) # bad cause norms are all over the place
    else:
        w2 = np.median(truematrix[:,best_k2], axis=1)
    w2_n = w2/np.sum(w2)
    # Compute the coefficients from the data
    W = np.array([w1_n,w2_n]).T
    H = np.linalg.pinv(W)@truematrix # ok not to use nnls if data is nonnegative
    # return estimated sources and their position in the data
    return W, H, np.array([w1,w2]).T #idx_bord1, idx_bord2





def invertMatrix(in_mat):

    # E_RGB_inv_A = inv(E_RGB_detectA'*E_RGB_detectA)*E_RGB_detectA' ;
    out = np.dot(np.linalg.pinv(np.dot(np.transpose(in_mat),in_mat)),np.transpose(in_mat))
    return out

def getAbsorbanceChanges(I,start_ref,end_ref):

    dA = np.zeros(I.shape)
    for k in range(I.shape[1]):
        dA[:,k] =np.log10(np.divide(np.mean(I[start_ref:end_ref,k]),I[:,k]))
    return dA

def FFT(I):
    FFT_signal = np.zeros(I.shape)

    for i in range(0,I.shape[1]):
        FFT_signal[:,i] = np.abs(np.fft.fftshift(np.fft.fft(I[:,i])))

    return FFT_signal


#Create Blackman filter
def create_filter(F,idF1,idF2):
    filter = np.zeros(F.shape)
    filter[idF1:idF2] = 1
    return filter



#Add filter
def onKeyPress(event):
    global filter, filter_plot, F, Full_F, Delta_F, Delta_F_min,F1,F2, id_f, FFT_signal, id1, id2, Full_id1, Full_id2

    #delta f when using arrows
    dF = 0.01

    #Delete last filter if delete key
    if(event.key == "delete"):
        if(id_f>0):
            F1.pop()
            F2.pop()
            id1.pop()
            id2.pop()
            Full_id1.pop()
            Full_id2.pop()
            filter.pop()
            filter_plot[id_f].remove()
            filter_plot.pop()
            id_f = id_f-1


    #Change id_f
    if(event.key == "+"):
        if(id_f<(len(filter)-1)):
            id_f = id_f+1

    if(event.key == "-"):
        if(id_f>0):
            id_f = id_f-1

    #Increase frequency range
    if(event.key == "up"):
        F2[id_f] = F2[id_f]+dF
        F1[id_f] = F1[id_f]-dF
        Delta_F = F2[id_f]-F1[id_f]
    #Reduce frequency range
    if(event.key == "down"):
        if(((F2[id_f]-dF)- (F1[id_f]+dF))>Delta_F_min ):
            F2[id_f] = F2[id_f]-dF
            F1[id_f] = F1[id_f]+dF
            Delta_F = F2[id_f]-F1[id_f]
    #Move frequency range on left
    if(event.key == "left"):
        F1[id_f] = F1[id_f]-dF
        F2[id_f] = F2[id_f]-dF
    #Move frequency range on right
    if(event.key == "right"):
        F1[id_f] = F1[id_f]+dF
        F2[id_f] = F2[id_f]+dF

    #Get indexes in Frequency vector
    id1[id_f] = np.argmin(np.abs(F-F1[id_f]))
    id2[id_f] = np.argmin(np.abs(F-F2[id_f]))
    Full_id1[id_f] = np.argmin(np.abs(Full_F-F1[id_f]))
    Full_id2[id_f] = np.argmin(np.abs(Full_F-F2[id_f]))


    #compute filter
    filter[id_f] = create_filter(F,id1[id_f],id2[id_f])

    #Plot
    for i in range(len(filter_plot)):
        #remove plots
        filter_plot[i].remove()
        #plot and scale filters
        filter_plot[i] = plt.axvspan(F1[i],F2[i],color="g",alpha=0.3)

    plt.grid()
    plt.draw()


def onclick(event):
    global filter, filter_plot, F, Full_F, Delta_F,F1,F2,id_f, FFT_signal, id1, id2, Full_id1, Full_id2

    if event.button is not MouseButton.LEFT:
        return

    #Compute min and max frequencies
    ix, iy = event.xdata, event.ydata
    F1.append(ix-Delta_F/2)
    F2.append(ix+Delta_F/2)

    #increase id_f
    id_f = id_f+1

    #Get indexes in Frequency vector
    id1.append(np.argmin(np.abs(F-F1[id_f])))
    id2.append(np.argmin(np.abs(F-F2[id_f])))
    Full_id1.append(np.argmin(np.abs(Full_F-F1[id_f])))
    Full_id2.append(np.argmin(np.abs(Full_F-F2[id_f])))
    #compute filter
    filter.append(create_filter(F,id1[id_f],id2[id_f]))

    #Plot
    filter_plot.append(plt.axvspan(F1[id_f],F2[id_f],color="g",alpha=0.3))


    plt.grid()
    plt.draw()


def on_close(event):

    global FFT_signal, filter, F,Full_F, Full_id1, Full_id2

    # Compute final window
    final_filter = np.zeros(filter[0].shape)
    for i in range(len(filter)):
        final_filter = final_filter + filter[i]


    # Frequency_idx
    idx = np.zeros((len(Full_id1),2))
    for i in range(len(Full_id1)):
        idx[i,0] = Full_id1[i]
        idx[i,1] = Full_id2[i]


    np.savetxt(path+"/Frequency_idx.txt",idx)
    np.savetxt(path+"/filter.txt",final_filter)
    np.savetxt(path+"/FFT_signal.txt",FFT_signal)
    np.savetxt(path+"/F.txt",F)
    np.savetxt(path+"/Full_F.txt",Full_F)


    # Apply final window
    for i in range(FFT_signal.shape[1]):
        FFT_signal[:,i] = FFT_signal[:,i]*final_filter

    # Remove null values
    FFT_signal = FFT_signal[final_filter>0,:]
    F = F[final_filter>0]
    final_filter = final_filter[final_filter>0]




    # Separable NMF on data without projecting on span of E
    # 1st: l1 normalisation --> done in the function
    # 2nd: finding extreme rays and scores
    # median is in the data points
    # mean is not
    W_sep, H_sep, W_unnorm = nmf_sep(FFT_signal.T, k=5, type='mean')


    out = nmf(FFT_signal.T, rank=2, n_iter_max = 5, init="custom", U_0 = W_sep, V_0 = H_sep, return_costs=True)
    W_sep_nmf = out[0]
    H_sep_nmf = out[1]
    W_sep_nmf = W_sep_nmf/np.sum(W_sep_nmf,axis=0)



    # normalisation
    W_inv_sep_nmf= invertMatrix(W_sep_nmf)

    np.savetxt(path+"/NMF_base.txt",W_sep_nmf)
    np.savetxt(path+"/NMF_base_inv.txt",W_inv_sep_nmf)
    np.savetxt(path+"/FFT_signal_reduced.txt",FFT_signal)
    np.savetxt(path+"/F_reduced.txt",F)


    #plt.figure()
    #plt.plot(W_NMF)
    #plt.show()




plt.close('all')


print("1")
in_data = np.loadtxt(path+"/Mean_Intensity_Values.txt")
infos = np.loadtxt(path+"/infos.txt")

Fe = infos[2]
start_ref = int(infos[0])
end_ref = int(infos[1])


#get absorbance changes
dA = getAbsorbanceChanges(in_data,start_ref,end_ref)
np.savetxt(path+"/Absorbance_changes.txt",dA)


#Process FFT
FFT_signal = FFT(dA)

# define frequencies
Full_F = np.linspace(-Fe/2,Fe/2,FFT_signal.shape[0])


#Reduce frequency range  [0;Fe/2]
FFT_signal = FFT_signal[np.argmin(np.abs(Full_F)):,:]
F = Full_F[np.argmin(np.abs(Full_F)):]


#Select frequencies between F1 and F2
#Delta F
Delta_F_min = 0.1
Delta_F = 0.5

print("2")
#Create filter
id_f = 0
filter = []
F1 = []
F2 = []
id1 = []
id2 = []
Full_id1 = []
Full_id2 = []


F1.append(0.85)
F2.append(1.25)
id1.append(np.argmin(np.abs(F-F1[id_f])))
id2.append(np.argmin(np.abs(F-F2[id_f])))
Full_id1.append(np.argmin(np.abs(Full_F-F1[id_f])))
Full_id2.append(np.argmin(np.abs(Full_F-F2[id_f])))
filter.append(create_filter(F,id1[0],id2[0]))

print("3")
#Create figure
fig = plt.figure()
plt.title("$|FFT(\Delta A)|$")
plt.plot(F,FFT_signal)

#Plot filter
filter_plot = []
# filter_plot.append(plt.plot(F,filter[id_f]*np.max(FFT_signal[np.argmax(filter[id_f]),:]),'r',linewidth=3))
filter_plot.append(plt.axvspan(F1[id_f],F2[id_f],color="g",alpha=0.3))
fig.canvas.mpl_connect('button_press_event', onclick)
fig.canvas.mpl_connect('close_event', on_close)
fig.canvas.mpl_connect('key_press_event', onKeyPress)


plt.grid()
#plt.xlim(0,Fe/2)
plt.xlim(0,2)
plt.xlabel("Frequency (Hz)")
plt.show()

print("4")






