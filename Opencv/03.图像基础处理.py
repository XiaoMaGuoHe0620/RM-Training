import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt

img = np.zeros((256,256,3),np.uint8)

print(img[100,100])
print(img[100,100,0])

img[100,100] = (0,0,255)

print(img[100,100])
print(img.shape)
print(img.dtype)
print(img.size)

plt.imshow(img[:,:,::-1])
plt.show()
