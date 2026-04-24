import fitz  # PyMuPDF
import matplotlib.pyplot as plt
import numpy as np
import cv2 as cv
import os

class Data_synchronization:
    def __init__(self, sync_start=True, sync_stop=True, total_duration_test_s=-1):

        self.sync_start = sync_start
        self.sync_stop = sync_stop
        self.total_duration_test_s = total_duration_test_s


def create_Acquisition_info_file(video_path, start_task_based_s, end_task_based_s, steps_task_s, start_resting_state_s, end_resting_state_s, start_impulsion_s, end_impulsion_s, steps_impulsion_s, Data_synchronisation,comments=""):

    if not Data_synchronisation.sync_start and not Data_synchronisation.sync_stop:
        print("Synchronization problem....")
        return


    #Get video info
    frame_count = []
    for i in video_path:
        cap = cv.VideoCapture(i)
        frame_count.append(int(cap.get(cv.CAP_PROP_FRAME_COUNT)))
        FPS = cap.get(cv.CAP_PROP_FPS)
    frame_count = np.asarray(frame_count)

    # Create the content of the file
    lines = [
        f"Camera Frame Rate (FPS) : {np.ceil(FPS)}",
        f"Theorical Frame Rate (FPS) : {FPS}",
        f"Exposure time (us) : -1",
        f"Camera Gain : -1",
        f"Red ratio : -1",
        f"Green ratio : -1",
        f"Blue ratio : -1",
    ]

    print("frame count",frame_count)
    total_frame_video = np.sum(frame_count)

    #offset to substract after first rest period
    offset = 0

    #Synchronization is OK for the end of the video but not the beginning
    if not Data_synchronisation.sync_start and Data_synchronisation.sync_stop:
        offset = int(Data_synchronisation.total_duration_test_s*FPS - total_frame_video)

    print("offset",offset)

    # Task-based protocol
    if start_task_based_s != -1 and end_task_based_s != -1:
        for i, step in enumerate(steps_task_s):
            lines.append(f"Step {i} : {int(step*FPS)-offset}")

        # Add the remaining lines
        val = int(start_task_based_s*FPS)-offset
        val = 0 if val<0 else val
        lines.append(f"Start task-based : {val}")
        if end_task_based_s == "stop":
            lines.append(f"End task-based : {np.sum(frame_count)-1}")
        else:
            lines.append(f"End task-based : {int(end_task_based_s*FPS)-offset}")




    #Resting state protocol
    if start_resting_state_s != -1 and end_resting_state_s != -1:

        # Add the remaining lines
        val = int(start_resting_state_s*FPS) - offset
        val = 0 if val<0 else val

        lines.append(f"Start resting-state : {val}")
        if end_resting_state_s == "stop":
            lines.append(f"End resting-state : {np.sum(frame_count)-1}")
        else:
            lines.append(f"End resting-state : {int(end_resting_state_s*FPS)-offset}")


    if start_impulsion_s != -1 and end_impulsion_s != -1:
        # Add the remaining lines
        val = int(start_impulsion_s*FPS)
        val = 0 if val<0 else val
        lines.append(f"Start Impulsion : {val}")
        if end_impulsion_s == "stop":
            lines.append(f"End Impulsion : {np.sum(frame_count)-1}")
        else:
            lines.append(f"End Impulsion : {int(end_impulsion_s*FPS)-offset}")


        for i, step in enumerate(steps_impulsion_s):
            lines.append(f"Impulsion {i} : {int(step*FPS)-offset}")


    for i in range(len(frame_count)):
        # Add the remaining lines
        lines.append(f"Frame count {i+1} : {frame_count[i]}")


    if comments != "":
        lines.append("Comments : "+comments)

    # Write the lines to a text file
    output_file = os.path.dirname(file_path[0])+"/Acquisition_infos.txt"
    with open(output_file, "w") as file:
        file.write("\n".join(lines))

## Create acquisition file info task-based + resting state + impulsion


#langage or task-based (named task-based in file, change it if motor and langage are done in the same acquisition)
task_rest_in_s = 30
task_test_in_s = 30
Nb_repetitions = 3

start_task_based_s = 29.23
end_task_based_s = start_task_based_s + 4*task_rest_in_s + 3*task_test_in_s

steps_task_based_s = []
steps_task_based_s.append(task_rest_in_s)
for i in range(Nb_repetitions):
    steps_task_based_s.append(task_test_in_s)
    steps_task_based_s.append(task_rest_in_s)

#Remove last rest step
steps_task_based_s = steps_task_based_s[0:-1]
steps_task_based_s = start_task_based_s + np.cumsum(np.asarray(steps_task_based_s))


# #Task based
# start_task_based_s = 40.39
#
#
# steps_task_based_s = np.array([21.40, 21.98, 22.06, 22, 22.28, 21.73, 21.46, 20.93])
# steps_task_based_s = start_task_based_s + np.cumsum(np.asarray(steps_task_based_s))
#
# # end_task_based_s = "stop"
# end_task_based_s = steps_task_based_s[-1] + 21.28


#Resting state
start_resting_state_s = end_task_based_s
# end_resting_state_s = "stop"
end_resting_state_s = end_task_based_s + 3*60+30.11

comments = ""

#Impulsions
start_impulsion_s=-1
end_impulsion_s=-1
steps_impulsion_s=[]

#Data synchro (no synchronization problem)
data_synch = Data_synchronization() #Synch ok



#Create acquisition file
video_dir = "/home/caredda/Videos/RGB_videos/tb_To_process/P90/Condition_Langage/"
file_path = [video_dir+"Segment01.mp4",video_dir+"Segment02.mp4"]




create_Acquisition_info_file(file_path, start_task_based_s, end_task_based_s, steps_task_based_s, start_resting_state_s, end_resting_state_s, start_impulsion_s=-1, end_impulsion_s=-1, steps_impulsion_s=[],Data_synchronisation = data_synch,comments=comments)

## Create acquisition file info resting state



start_task_based_s = -1
end_task_based_s = -1

steps_task_based_s = []

start_resting_state_s = 5*60
end_resting_state_s = "stop"

comments = ""

video_dir = "/home/caredda/temp/To_upload/video/"
file_path = [video_dir+"1.mp4",video_dir+"2.mp4"]


#Data synchro (no synchronization problem)
data_synch = Data_synchronization() #Synch ok

create_Acquisition_info_file(file_path, start_task_based_s, end_task_based_s, steps_task_based_s, start_resting_state_s, end_resting_state_s, start_impulsion_s=-1, end_impulsion_s=-1, steps_impulsion_s=[], Data_synchronisation = data_synch, comments=comments)

## Create acquisition file (tb-only)


#Task based
start_task_based_s = 51.07


# steps_task_based_s = np.array([21.21, 20.75, 21.93, 21.47, 22.12, 21.83, 22.40, 21.71])
steps_task_based_s = np.array([21.21, 20.75, 21.93, 21.47, 22.12, 21.83])
steps_task_based_s = start_task_based_s + np.cumsum(np.asarray(steps_task_based_s))

# end_task_based_s = "stop"
end_task_based_s = steps_task_based_s[-1] + 22.40


#Resting state
start_resting_state_s = -1
end_resting_state_s = -1

#Impulsions
start_impulsion_s=-1
end_impulsion_s=-1
steps_impulsion_s=[]


#Data synchro (problem in data synchronization) (adapt values)
# data_synch = Data_synchronization(sync_start = False, sync_stop=True,
#                                   total_duration_test_s=steps_task_based_s[-1]+30.42)

#Data synchro (synch ok)
data_synch = Data_synchronization()


#Create acquisition file
video_dir = "/home/caredda/Videos/RGB_videos/tb_To_process/P90/Condition_Task_based_Right_Hand_autonomous/"
file_path = [video_dir+"Segment03.mp4"]

create_Acquisition_info_file(file_path, start_task_based_s, end_task_based_s, steps_task_based_s, start_resting_state_s, end_resting_state_s, start_impulsion_s=-1, end_impulsion_s=-1, steps_impulsion_s=[], Data_synchronisation = data_synch)
