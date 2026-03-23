import fitz  # PyMuPDF
import matplotlib.pyplot as plt
import numpy as np
import cv2 as cv
import os
from pymediainfo import MediaInfo


class Data_synchronization:
    def __init__(self, sync_start=True, sync_stop=True, total_duration_test_s=-1):
        self.sync_start = sync_start
        self.sync_stop = sync_stop
        self.total_duration_test_s = total_duration_test_s

def get_mediainfo(video_path):
    """Get FPS, frame count, and duration using MediaInfo"""
    try:
        media_info = MediaInfo.parse(video_path)
        for track in media_info.tracks:
            if track.track_type == 'Video':
                fps = track.frame_rate
                frame_count = track.frame_count
                duration_ms = track.duration
                if fps and frame_count and duration_ms:
                    duration_s = duration_ms / 1000.0
                    return float(fps), int(frame_count), duration_s
        return None, None, None
    except Exception as e:
        print(f"MediaInfo error: {e}")
        return None, None, None

def create_Acquisition_info_file(video_path, start_task_based_s, end_task_based_s, steps_task_s, start_resting_state_s,
                                 end_resting_state_s, start_impulsion_s, end_impulsion_s, steps_impulsion_s,
                                 Data_synchronisation, comments=""):
    if not Data_synchronisation.sync_start and not Data_synchronisation.sync_stop:
        print("Synchronization problem....")
        return

    # Get video info
    frame_count = []
    for i in video_path:
        fps, frames, duration = get_mediainfo(i)
        if fps is None or frames is None:
            print(f"Could not get video info from MediaInfo for {i}")
            return
        frame_count.append(frames)
        FPS = fps  # from last video
        DURATION = duration # from last video

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

    # Debug
    print("FPS:", FPS)
    print("frame count:", frame_count)
    print("duration:", DURATION)
    total_frame_video = np.sum(frame_count)

    # offset to substract after first rest period
    offset = 0

    # Synchronization is OK for the end of the video but not the beginning
    if not Data_synchronisation.sync_start and Data_synchronisation.sync_stop:
        offset = int(Data_synchronisation.total_duration_test_s * FPS - total_frame_video)

    print("offset", offset)

    # Task-based protocol
    if start_task_based_s != -1 and end_task_based_s != -1:
        for i, step in enumerate(steps_task_s):
            lines.append(f"Step {i} : {int(step * FPS) - offset}")

        # Add the remaining lines
        val = int(start_task_based_s * FPS) - offset
        val = 0 if val < 0 else val
        lines.append(f"Start task-based : {val}")
        if end_task_based_s == "stop":
            lines.append(f"End task-based : {np.sum(frame_count) - 1}")
        else:
            lines.append(f"End task-based : {int(end_task_based_s * FPS) - offset}")

    # Resting state protocol
    if start_resting_state_s != -1 and end_resting_state_s != -1:
        # Add the remaining lines
        val = int(start_resting_state_s * FPS) - offset
        val = 0 if val < 0 else val

        lines.append(f"Start resting-state : {val}")
        if end_resting_state_s == "stop":
            lines.append(f"End resting-state : {np.sum(frame_count) - 1}")
        else:
            lines.append(f"End resting-state : {int(end_resting_state_s * FPS) - offset}")

    if start_impulsion_s != -1 and end_impulsion_s != -1:
        # Add the remaining lines
        val = int(start_impulsion_s * FPS)
        val = 0 if val < 0 else val
        lines.append(f"Start Impulsion : {val}")
        if end_impulsion_s == "stop":
            lines.append(f"End Impulsion : {np.sum(frame_count) - 1}")
        else:
            lines.append(f"End Impulsion : {int(end_impulsion_s * FPS) - offset}")

        for i, step in enumerate(steps_impulsion_s):
            lines.append(f"Impulsion {i} : {int(step * FPS) - offset}")

    for i in range(len(frame_count)):
        # Add the remaining lines
        lines.append(f"Frame count {i + 1} : {frame_count[i]}")

    if comments != "":
        lines.append("Comments : " + comments)

    # Write the lines to a text file
    output_file = os.path.dirname(video_path[0]) + "/Acquisition_infos.txt"
    with open(output_file, "w") as file:
        file.write("\n".join(lines))


# Create acquisition file info resting state
start_task_based_s = -1
end_task_based_s = -1
steps_task_based_s = []

start_resting_state_s = 0
end_resting_state_s = "stop"

comments = ""

video_dir = "/home/kawinkij/Downloads/Temp/2025-06-06-124357/"
file_path = [video_dir + "Segment03.mp4"]

# Data synchro (no synchronization problem)
data_synch = Data_synchronization()  # Synch ok

create_Acquisition_info_file(file_path, start_task_based_s, end_task_based_s, steps_task_based_s, start_resting_state_s,
                             end_resting_state_s, start_impulsion_s=-1, end_impulsion_s=-1, steps_impulsion_s=[],
                             Data_synchronisation=data_synch, comments=comments)