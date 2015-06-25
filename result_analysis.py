#!/usr/bin/python

import numpy
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

file = open('TrjFile.txt','r')

line = file.readline();

data = []
while line:
    nums = filter(None, line.strip('\n').split(' '))
    nums = [float(item) for item in nums]
    data.append(nums)
    line = file.readline()
print nums
file.close()
dataArr = numpy.array(data)
time = dataArr[:,0]
axis = []
for i in range(1, 19):
    print i
    axis.append(dataArr[:,i])

print len(time)
tplt = time / 1.0

plt.subplot(3,1, 1)
for i in range(3):
    plt.plot(tplt[::], (axis[i]))
    plt.hold(True)
plt.hold(False)
#plt.axis([1, 10, -40000, 40000])

plt.subplot(3,1, 2)
for i in range(3):
    plt.plot(tplt[:-1:], numpy.diff( (axis[i]))/0.001)
    plt.hold(True)
#plt.axis([1, 10, -40000, 40000])

plt.subplot(3,1, 3)
for i in range(3):
    plt.plot(tplt[:-2:], numpy.diff(numpy.diff( (axis[i]))/0.001)/0.001)
    plt.hold(True)
plt.hold(False)
plt.grid(True)
plt.axis([0, 30, -2, 2])
plt.show()
