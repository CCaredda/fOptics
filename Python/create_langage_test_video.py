import fitz  # PyMuPDF
import matplotlib.pyplot as plt
import numpy as np
import cv2 as cv
import os

def read_pdf_as_images(pdf_path):
    """
    Reads pages from a PDF file, converts them to images.

    Args:
        pdf_path (str): Path to the PDF file.
    Returns:
        list of images
    """
    # Open the PDF file
    pdf_document = fitz.open(pdf_path)
    num_pages = pdf_document.page_count

    list_img = []

    for page_number in range(num_pages):
        page = pdf_document[page_number]

        # Convert the page to an image
        pix = page.get_pixmap()  # Default resolution
        img = np.frombuffer(pix.samples, dtype=np.uint8).reshape(pix.height, pix.width, pix.n)
        list_img.append(img)

    # Close the document
    pdf_document.close()

    return list_img

## Create video file
path = "/home/caredda/DVP/C++/imagerie_fonctionnelle_cerveau/git/intraoperative-functional-brain-mapping/Python/data_test_langage/"
# Read pdf files
activity_slices = read_pdf_as_images(path+"test_langage.pdf")
rest_slice = np.zeros(activity_slices[0].shape,activity_slices[0].dtype)

#Draw White cross
length = np.minimum(2*int(rest_slice.shape[0]/8),2*int(rest_slice.shape[1]/8))
rest_slice[int(rest_slice.shape[0]/2),int(rest_slice.shape[1]/2-length/2):int(rest_slice.shape[1]/2+length/2),0] = 255
rest_slice[int(rest_slice.shape[0]/2),int(rest_slice.shape[1]/2-length/2):int(rest_slice.shape[1]/2+length/2),1] = 255
rest_slice[int(rest_slice.shape[0]/2),int(rest_slice.shape[1]/2-length/2):int(rest_slice.shape[1]/2+length/2),2] = 255

rest_slice[int(rest_slice.shape[0]/2-length/2):int(rest_slice.shape[0]/2+length/2),int(rest_slice.shape[1]/2),0] = 255
rest_slice[int(rest_slice.shape[0]/2-length/2):int(rest_slice.shape[0]/2+length/2),int(rest_slice.shape[1]/2),1] = 255
rest_slice[int(rest_slice.shape[0]/2-length/2):int(rest_slice.shape[0]/2+length/2),int(rest_slice.shape[1]/2),2] = 255


# Adapt these values
output_path ="/home/caredda/temp/"
FPS = 5
rest_in_s = 30
langage_test_in_s = 30
time_per_speech_in_s = 4.5
end_video_in_s = 180
Nb_repetitions = 3



#These values are automatically calculated

#Limit to 4.5 per image of langage test
if time_per_speech_in_s<4.5:
    time_per_speech_in_s = 4.5

nb_image_per_task = int(langage_test_in_s/time_per_speech_in_s) #nb image per langage task

if Nb_repetitions*nb_image_per_task>len(activity_slices):
    print("Nb of test slices: ",len(activity_slices))
    print("Nb of required slices: ",Nb_repetitions*nb_image_per_task)
    raise SystemExit("The number of repetitions is to high or time_per_speech_in_s is too fast")

#udpate time_per_speech_in_s
time_per_speech_in_s = langage_test_in_s/nb_image_per_task

#Get number of frame for rest, tasks and resting state
nb_image_per_task_fps = int(time_per_speech_in_s*FPS)
nb_image_rest_fps = int(rest_in_s*FPS)
nb_image_end_fps = int(end_video_in_s*FPS)

#Get frame id

print("nb images rest",nb_image_rest_fps)
print("nb images task",nb_image_per_task*nb_image_per_task_fps)


#Init video
img_width = rest_slice.shape[1]
img_height = rest_slice.shape[0]
out_video = cv.VideoWriter(output_path+"test_langage.mp4", cv.VideoWriter_fourcc(*'mp4v'), FPS, (img_width, img_height))

id_langage_img = 0
for i in range(Nb_repetitions):
    #Rest
    for t in range(nb_image_rest_fps):
        out_video.write(rest_slice)

    #Langage tests
    for j in range(nb_image_per_task):
        for t in range(nb_image_per_task_fps):
            out_video.write(activity_slices[id_langage_img])
        id_langage_img +=1

#Rest for langage test
for t in range(nb_image_rest_fps):
    out_video.write(rest_slice)

#Resting-state
for t in range(nb_image_end_fps):
    out_video.write(rest_slice)

out_video.release()
