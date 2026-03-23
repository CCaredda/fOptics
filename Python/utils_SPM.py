import numpy as np
import cv2 as cv


#Procedure
#1) split your images into squares so that having edges defined by ksize:
#Choose FWHM value: my default value is 7
#ksize = int(3 * FWHM)
#ksize = ksize + 1 if ksize % 2 == 0 else ksize

#2) count the number of resels (resolutions elements). It corresponds to the number of squares inside the surgical window area: variable nb_resel

#3) Call the function get_Random_Field_Theory_Threshold to get the z-stats threshold:
# z_th_5_percent = get_Random_Field_Theory_Threshold(nb_resel)

#4) Call the function process_SPM to get the statistical inferences (binary map and the Z-stats map)
#the argument of the function are:
#Delta_C_map: (N,M,T) matrix of concentration changes. N and M stand for the number of row and column and T the temporal dimension
#Delta_C_theo (T), reference signal or seed
#z_th_5_percent: z_stats threshold
#FWHM: FWHM value used to define the numbers of resels and for the Gaussian smoothing

#4.1) Apply GLM to get Z_stats (shape (N,M)
#4.2) Apply Gaussian smoothing on Z_stat
#4.3) Apply thresholding to get the statistical inferences

# Extract variance from Matrix
# in_Mat size(T,N)
# out size(N)
def extractVarianceFromMat(in_Mat):
    out = np.zeros((in_Mat.shape[1],))
    for i in range(in_Mat.shape[1]):
        out[i] = np.var(in_Mat[:,i])
    return out

#Get Euler characteristics
def EC_5_percent(x,nb_resel):
    # print(nb_resel)
    return np.power((2*np.pi),-1.5)*(4*np.log(2))*nb_resel*x*np.exp(-(np.power(x,2)/2)) - 0.05


#Random Field Theory (get threshold)
def get_Random_Field_Theory_Threshold(nb_resel):

    z_th = np.arange(1,5,0.0001)
    y = EC_5_percent(z_th,nb_resel)
    id_min = np.argmin(np.abs(y))
    z_th_5_percent = z_th[id_min]

    return z_th_5_percent

# GLM T statistics
# invert_X = (X'X)-1 size (S=2,S=2)
# B size(S=2,N)
# var_e size (1,N)
# T_map size(1,N)
# stimulus_id : 0 (activation) 1 (non activation)
def GLM_t_stats(invert_X,B,var_e):

    stimulus_id = 0
    #Build contrast vector
    c   = np.zeros((2,1))
    c_t = np.zeros((1,2,))

    c[stimulus_id,0]      = 1
    c_t[0,stimulus_id]    = 1

    #size (1,N)
    T_map = np.dot(c_t,B)

    # Size (1,1)
    denom = np.squeeze(np.dot(np.dot(c_t,invert_X),c))


    for i in range(T_map.shape[1]):
        val = np.sqrt(denom*var_e[i])
        T_map[0,i] = 0 if val==0 else T_map[0,i]/val

    return T_map

# General linear model
# Y = X.B + e
# Y: Concentration changes) Size: (time,nb pixels)
# X: Design matrix (Bold signal with physiological a priori on amplitude) Size: (T,S).
def GLM(Y,X):
    #B = (X'X)-1 X'Y
    tr_X = X.T

    invert_X = cv.mulTransposed(X,True)
    cv.invert(invert_X,invert_X,cv.DECOMP_SVD)

    #Spatial pattern of responses
    B = np.dot(np.dot(invert_X,tr_X),Y)

    # errors
    e = Y - np.dot(X,B)
    var_e = extractVarianceFromMat(e)


    #Residuals forming matrix
    R = np.dot(np.dot(X,invert_X),tr_X)
    R = np.eye(R.shape[0]) - R
    # R = np.eye(R.shape[0],R.dtype()) - R

    T_map = GLM_t_stats(invert_X,B,var_e)

    return T_map

#Process SPM
# Delta_C_map size(nb_pxel_x,nb_pixels_y,Time)
# Delta_C_theo size(Time)
def process_SPM(Delta_C_map,Delta_C_theo,z_th_5_percent,FWHM):

    #Design matrix
    Design_Matrix = np.zeros((Delta_C_theo.shape[0],2))
    Design_Matrix[:,0] = Delta_C_theo

    #Format Delta C map data in line
    #Y size (Time,nb_pixels)
    Y = np.zeros((Delta_C_map.shape[2],Delta_C_map.shape[0]*Delta_C_map.shape[1]))
    id = 0
    for x in range(Delta_C_map.shape[0]):
        for y in range(Delta_C_map.shape[1]):
            Y[:,id] = Delta_C_map[x,y,:]
            id = id+1

    #Compute GLM
    T_map = GLM(Y,Design_Matrix)

    #Get Z map
    Z_map = (T_map-50)/10

    #reconconstruct Z and T map
    Z_map_rec = np.zeros((Delta_C_map.shape[0],Delta_C_map.shape[1]))


    id = 0
    for x in range(Delta_C_map.shape[0]):
        for y in range(Delta_C_map.shape[1]):
            Z_map_rec[x,y] = Z_map[0,id]
            id = id+1


    #Process RFT
    sigma    = FWHM/2.355
    ksize = int(3 * FWHM)
    ksize = ksize + 1 if ksize % 2 == 0 else ksize

    Z_map_rec = cv.GaussianBlur(Z_map_rec,(ksize,ksize),sigma)
    mask_SPM = np.zeros(Z_map_rec.shape,dtype=np.uint8)
    mask_SPM[Z_map_rec>z_th_5_percent] = 1

    return Z_map_rec,mask_SPM
