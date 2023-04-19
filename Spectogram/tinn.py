import wave
import os

# Define the path to your WAV file
wav_dir = 'D:/Final year project/Snoring Dataset'
wav_path = 'D:/Final year project/Snoring Dataset/0_0.wav'

# Define the number of segments to split the WAV file into
num_segments = 5

# Define the number of parts to split each segment into
num_parts = 16

# Open the WAV file for reading
#for wav_path in os.listdir(wav_dir):
with wave.open(wav_path, 'rb') as wav_file:
        # Get the number of frames in the WAV file
        if "1_" in wav_path:
            label = 1
        else:
            label = 0

        num_frames = wav_file.getnframes()

        # Calculate the number of frames per segment
        frames_per_segment = num_frames // num_segments

        # Calculate the number of frames per part
        frames_per_part = frames_per_segment // num_parts

        # Loop through the segments
        for i in range(num_segments):
            # Set the starting and ending frames for this segment
            start_frame = i * frames_per_segment
            end_frame = (i + 1) * frames_per_segment

            # Initialize an array to hold the max amplitude of each part
            max_amplitudes = []

            # Loop through the parts of this segment
            for j in range(num_parts):
                # Set the starting and ending frames for this part
                start_part_frame = start_frame + j * frames_per_part
                end_part_frame = start_frame + (j + 1) * frames_per_part

                # Set the current frame to the starting frame of this part
                current_frame = start_part_frame

                # Initialize a variable to hold the max amplitude of this part
                max_amplitude = 0

                # Loop through the frames of this part
                while current_frame < end_part_frame:
                    # Read in the next frame from the WAV file
                    frame = wav_file.readframes(1)

                    # If we've reached the end of the WAV file, break out of the loop
                    if not frame:
                        break

                    # Convert the frame to an integer
                    amplitude = int.from_bytes(frame, 'little', signed=True)

                    # If this amplitude is greater than the current max amplitude, update the max amplitude
                    if abs(amplitude) > max_amplitude:
                        max_amplitude = abs(amplitude)

                    # Increment the current frame
                    current_frame += 1

                # Add the max amplitude of this part to the array
                max_amplitudes.append(max_amplitude)

            max_amplitudes.append(label)
            # Print out the max amplitude of each part for this segment
            amplitudes_str = ' '.join(map(str, max_amplitudes))
            print(amplitudes_str)

            """ with open('data.txt', 'a') as f:
                # Loop through the amplitude values
                for amplitude in max_amplitudes:
                    # Write each amplitude value to the file, separated by a space
                    f.write(str(amplitude) + ' ')
                # Add a newline character at the end of the file
                f.write('\n') """
