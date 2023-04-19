import os
import librosa
import numpy as np
from matplotlib import pyplot as plt

# Define the path to your audio files
snore_dir = 'D:/Final year project/Snoring Dataset/snore'
non_snore_dir = 'D:/Final year project/Snoring Dataset/non_snore'

# Define the output directory for the spectrograms
output_dir = 'D:/Final year project/Spectogram/data'

# Define the parameters for the STFT
n_fft = 1024  # size of the FFT window
hop_length = 256  # number of samples between consecutive STFT columns
win_length = n_fft  # size of the STFT window

# Loop through the snore files
for filename in os.listdir(snore_dir):
    # Load the audio file
    audio, sr = librosa.load(os.path.join(snore_dir, filename))
    
    # Compute the spectrogram using the STFT
    spec = np.abs(librosa.stft(audio, n_fft=n_fft, hop_length=hop_length, win_length=win_length))
    
    # Convert the power spectrogram to decibels (dB)
    spec_db = librosa.amplitude_to_db(spec, ref=np.max)
    
    # Save the spectrogram to disk
    output_filename = os.path.splitext(filename)[0] + '.png'
    output_path = os.path.join(output_dir, output_filename)
    librosa.display.specshow(spec_db, sr=sr, hop_length=hop_length)
    plt.savefig(output_path)
    plt.close()
    
# Loop through the non-snore files
for filename in os.listdir(non_snore_dir):
    # Load the audio file
    audio, sr = librosa.load(os.path.join(non_snore_dir, filename))
    
    # Compute the spectrogram using the STFT
    spec = np.abs(librosa.stft(audio, n_fft=n_fft, hop_length=hop_length, win_length=win_length))
    
    # Convert the power spectrogram to decibels (dB)
    spec_db = librosa.amplitude_to_db(spec, ref=np.max)
    
    # Save the spectrogram to disk
    output_filename = os.path.splitext(filename)[0] + '.png'
    output_path = os.path.join(output_dir, output_filename)
    librosa.display.specshow(spec_db, sr=sr, hop_length=hop_length)
    plt.savefig(output_path)
    plt.close()
