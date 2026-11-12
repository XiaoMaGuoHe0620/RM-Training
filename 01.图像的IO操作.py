import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt

# 1 读取图像
#img = cv.imread("/home/xiaoma/Downloads/1.jpeg") 
img = cv.imread("1.jpeg",0) 

# 2 显示图像
# 2.1 利用opencv展示图像
# cv.imshow("UAV",img)
# k = cv.waitKey(0)

# 2.2 在matplotlib中展示图像
#plt.imshow(img[:,:,::-1])
plt.imshow(img,cmap=plt.cm.gray)
plt.show()

# # 3 保存图像
cv.imwrite("UAV.png",img)
