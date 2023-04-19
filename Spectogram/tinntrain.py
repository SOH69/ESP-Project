import os
import librosa

# Set directory containing wave files
wave_dir = 'D:/Final year project/Snoring Dataset'

# Set output file name and path
output_file = 'D:/Final year project/Spectogram/data.txt'

# Open output file for writing
with open(output_file, 'w') as f:

    # Loop through all wave files in directory
    for filename in os.listdir(wave_dir):
        if filename.endswith('.wav'):

            # Load wave file
            audio_path = os.path.join(wave_dir, filename)
            audio_data, sr = librosa.load(audio_path, sr=None)

            # Calculate amplitude and write to output file
            amplitude = int(audio_data.max() * 32767)
            if "1_" in filename:
                label = 1
            else:
                label = 0
            f.write(f'{amplitude} {label}\n')
