import tensorflow as tf
import numpy as np


model = tf.keras.models.load_model('D:/Final year project/Spectogram/train/new_detection_model.h5')
converter = tf.lite.TFLiteConverter.from_saved_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_quant_model = converter.convert()

open('D:/Final year project/Spectogram/train/new_model.tflite', 'wb').write(tflite_quant_model)
