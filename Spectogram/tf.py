import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Conv2D, Flatten, MaxPooling2D
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt
import numpy as np
import os
import librosa
import librosa.display

# Load audio data and convert to spectrograms
data_dir = 'D:/Final year project/Snoring Dataset Old'
snore_dir = os.path.join(data_dir, 'snore')
non_snore_dir = os.path.join(data_dir, 'non_snore')

snore_data = []
for filename in os.listdir(snore_dir):
    audio_file = os.path.join(snore_dir, filename)
    y, sr = librosa.load(audio_file)
    spec = librosa.feature.melspectrogram(y=y, sr=sr)
    spec = librosa.power_to_db(spec, ref=np.max)
    snore_data.append(spec)

non_snore_data = []
for filename in os.listdir(non_snore_dir):
    audio_file = os.path.join(non_snore_dir, filename)
    y, sr = librosa.load(audio_file)
    spec = librosa.feature.melspectrogram(y=y, sr=sr)
    spec = librosa.power_to_db(spec, ref=np.max)
    non_snore_data.append(spec)

# Create labels
snore_labels = np.ones(len(snore_data))
non_snore_labels = np.zeros(len(non_snore_data))

# Combine data and labels
data = np.concatenate((snore_data, non_snore_data), axis=0)
labels = np.concatenate((snore_labels, non_snore_labels), axis=0)

# Split data into training and validation sets
X_train, X_val, y_train, y_val = train_test_split(data, labels, test_size=0.2)

# Reshape data for convolutional neural network
X_train = X_train.reshape(X_train.shape[0], X_train.shape[1], X_train.shape[2], 1)
X_val = X_val.reshape(X_val.shape[0], X_val.shape[1], X_val.shape[2], 1)

# Define the CNN model
model = Sequential()
model.add(Conv2D(16, kernel_size=(3, 3), activation='relu', input_shape=X_train.shape[1:]))
model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Conv2D(16, kernel_size=(3, 3), activation='relu'))
model.add(MaxPooling2D(pool_size=(2, 2)))
model.add(Flatten())
model.add(Dense(32, activation='relu'))
model.add(Dense(1, activation='sigmoid'))

# Compile the model
model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])

# Train the model
history = model.fit(X_train, y_train, batch_size=32, epochs=10, verbose=1, validation_data=(X_val, y_val))

# Evaluate the model
score = model.evaluate(X_val, y_val, verbose=0)
print('Test loss:', score[0])
print('Test accuracy:', score[1])

# Save the model
model.save('D:/Final year project/Spectogram/train/new_detection_model.h5')


model = tf.keras.models.load_model('D:/Final year project/Spectogram/train/new_detection_model.h5')
converter = tf.lite.TFLiteConverter.from_saved_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_quant_model = converter.convert()

open('D:/Final year project/Spectogram/train/new_model.tflite', 'wb').write(tflite_quant_model)
