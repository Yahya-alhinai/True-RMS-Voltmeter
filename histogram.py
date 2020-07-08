import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import time
import serial
import tkinter
import statistics as stats

COM_PORT = '/dev/ttyUSB0' # eg 'COM7' for windows, '/dev/ttyXXXX' for unix
BAUDRATE = 128000
READ_DELAY = 1 #delay in seconds before reading
READ_WORDS = 500 #number of data words (3 bytes) to read before plotting

Y_MAX = READ_WORDS

#serial setup
ser=serial.Serial(COM_PORT,BAUDRATE)
"""
#EXAMPLE DATA
mu = 100 # mean of distribution
sigma = 2 # standard deviation of distribution
for x in range(0,10):
    p=(mu + sigma*np.random.randn(READ_WORDS*3+2))
    print(ser.write(p))
    ser.write(b'0x00')
"""
#**Main Code**
print("Beginning Main")
xar=[]
fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
while(1):
    #basically fast forwarding until sync byte
    if int.from_bytes(ser.read(1),byteorder='big')==0:
        print("Recording Data Frame\n")
        time.sleep(READ_DELAY)
        xar.clear()
        for i in range(0,READ_WORDS-1):
            xar.append(int.from_bytes(ser.read(3),byteorder='big'))
        ax1.clear()
        ax1.hist(xar)
        print("Mean:"+str(stats.mean(xar))+" Var:"+str(stats.variance(xar)))
        plt.ylim(0,Y_MAX)
        #plt.xlim(80, 120)
        plt.pause(0.1)
plt.show()
