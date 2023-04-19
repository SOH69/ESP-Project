import os
import numpy as np
from PIL import Image
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression

# Define the path to your spectrogram files
spec_dir = 'D:/Final year project/Spectogram/data'

# Define the size of the spectrograms
img_height, img_width = 128, 128

# Initialize the arrays to hold the spectrograms and labels
X = []
y = []

# Loop through the spectrogram files
for filename in os.listdir(spec_dir):
    # Load the spectrogram image
    img = Image.open(os.path.join(spec_dir, filename))
    
    # Resize the spectrogram image to the desired size
    img = img.resize((img_width, img_height))
    
    # Convert the spectrogram image to a NumPy array
    spec = np.array(img)
    
    # Add the spectrogram and label to the arrays
    X.append(spec)
    y.append('snore' if '1_' in filename else 'non-snore')

# Convert the arrays to NumPy arrays
X = np.array(X)
y = np.array(y)

# Split the data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Flatten the spectrograms into 1D arrays
X_train_flat = X_train.reshape(X_train.shape[0], -1)
X_test_flat = X_test.reshape(X_test.shape[0], -1)

# Create a logistic regression model
model = LogisticRegression(max_iter=1000)

# Train the model on the training data
model.fit(X_train_flat, y_train)

# Evaluate the model on the testing data
accuracy = model.score(X_test_flat, y_test)
print('Accuracy:', accuracy)

""" # Load a new spectrogram image
try:
    new_spec = Image.open('D:/Final year project/Spectogram/train')
except PermissionError:
    print("Permission denied to access the image file.")
    exit()

# Resize the spectrogram image to the desired size
new_spec = new_spec.resize((img_width, img_height))

# Convert the spectrogram image to a NumPy array
new_X = np.array(new_spec)

# Reshape the spectrogram array to a 1D array
new_X_flat = new_X.reshape(1, -1)

# Make a prediction using the trained model
prediction = model.predict(new_X_flat)

# Print the prediction
print('Prediction: ', prediction) """
