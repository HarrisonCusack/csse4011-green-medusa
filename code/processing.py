import serial
import json
import time
import re
import math
import sys
import threading
import tago


import numpy as np

my_device = tago.Device('ff867015-eef4-41ab-8261-95c8b0fd758f')
device_information = my_device.info()

CLEAR_CHAR = chr(0x1b) + "[J"
ESCAPE_CHAR = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

serialPort = serial.Serial("/dev/serial/by-id/usb-Wilfred_MK_USB_Console_Tute_EC10EDEC405209D1-if00", 115200)

def send_serial():
    while True:
        x = input()
        y = x + '\n'
        serialPort.write(y.encode('ascii', 'ignore'))

class Kalman:
    def __init__(self, x_init, cov_init, meas_err, proc_err):
        self.ndim = len(x_init)
        self.A = np.eye(ndim)        #state transition model
        self.H = np.eye(ndim)        #observation model
        self.x_hat =  x_init
        self.cov = cov_init
        self.Q_k = np.eye(ndim)*proc_err #covariance matrix of process noise
        self.R = np.eye(len(self.H))*meas_err   #covariance matrix of observation noise
        
    def update(self, obs):

        # Make prediction
        self.x_hat_est = np.dot(self.A,self.x_hat) #+ np.array([0.5 * math.cos(math.radians(heading)), 
                                                    #           0.5 * math.sin(math.radians(heading))])
        self.cov_est = np.dot(self.A,np.dot(self.cov,np.transpose(self.A))) + self.Q_k

        # Update estimate
        self.error_x = obs - np.dot(self.H,self.x_hat_est)
        self.error_cov = np.dot(self.H,np.dot(self.cov_est,np.transpose(self.H))) + self.R
        self.K = np.dot(np.dot(self.cov_est,np.transpose(self.H)),np.linalg.inv(self.error_cov))
        self.x_hat = self.x_hat_est + np.dot(self.K,self.error_x)
        if ndim>1:
            self.cov = np.dot((np.eye(self.ndim) - np.dot(self.K,self.H)),self.cov_est)
        else:
            self.cov = (1-self.K)*self.cov_est 


def begin():

    currentTime = time.time()

    x = threading.Thread(target=send_serial)
    x.start()
    previous_sensor = ""

    k_temperature = Kalman(np.array([24]), np.eye(ndim), 1, 1)
    k_humidity = Kalman(np.array([35]), np.eye(ndim), 1, 1)
    k_pressure = Kalman(np.array([101]), np.eye(ndim), 1, 1)
    k_voc = Kalman(np.array([10]), np.eye(ndim), 1, 1)

    while True:
    
        rawInput = serialPort.readline().decode("utf-8").strip()
        processingInput = ESCAPE_CHAR.sub('', rawInput.split(CLEAR_CHAR)[-1])
        processedInput = processingInput.split(" ")

        if (len(processedInput) == 4):
            if(processedInput[2] != previous_sensor):
                filter = {'qty':50000}
                result = my_device.remove(filter)
            previous_sensor = processedInput[2]
            if (processedInput[0] == '3'):
                if (processedInput[2] == '1'):
                    k_pressure.update(np.array([float(processedInput[3])]))
                    print("Node:", processedInput[0], ", Time:", time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)), ", Sensor:", processedInput[2], ", Data:", k_pressure.x_hat[0])
                elif (processedInput[2] == '3'):
                    k_temperature.update(np.array([float(processedInput[3])]))
                    print("Node:", processedInput[0], ", Time:", time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)), ", Sensor:", processedInput[2], ", Data:", k_temperature.x_hat[0])
                elif (processedInput[2] == '4'):
                    k_humidity.update(np.array([float(processedInput[3])]))
                    print("Node:", processedInput[0], ", Time:", time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)), ", Sensor:", processedInput[2], ", Data:", k_humidity.x_hat[0])
                elif (processedInput[2] == '5'):
                    k_voc.update(np.array([float(processedInput[3])]))
                    print("Node:", processedInput[0], ", Time:", time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)), ", Sensor:", processedInput[2], ", Data:", k_voc.x_hat[0])
                else:
                    print("Node:", processedInput[0], ", Time:", time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)), ", Sensor:", processedInput[2], ", Data:", processedInput[3])    
            else:
                print("Node:", processedInput[0], ", Time:", time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)), ", Sensor:", processedInput[2], ", Data:", processedInput[3])
            
            unit = ""
            if (processedInput[2] == '1'):
                unit = "kPa"
            elif (processedInput[2] == '2'):
                unit = "index"
            elif (processedInput[2] == '3'):
                unit = "C"
            elif (processedInput[2] == '4'):
                unit = "RH%"
            elif (processedInput[2] == '5'):
                unit = "index"
            elif (processedInput[2] == '6'):
                unit = "ug/m^3"
            elif (processedInput[2] == '7'):
                unit = "ug/m^3"
            elif (processedInput[2] == '8'):
                unit = "ug/m^3"
            elif (processedInput[2] == '9'):
                unit = "ug/m^3"
            elif (processedInput[2] == '10'):
                unit = "index"

            data = []
            data.append(
                {
                'variable': 'node' + processedInput[0],
                'value': processedInput[3],
                'time' : time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(float(processedInput[1]) / 1000.0 + currentTime)),
                'unit': unit
                }
            )

            result = my_device.insert(data)

            
        
        elif (len(processedInput) == 1 and len(processedInput[0]) > 1):
            if (processedInput[0][-2] == "n"):
                currentTime = time.time()
        

        # try:
        #     newValues = [float(i) for i in newValues2]
        # except:
        #     newValues = oldValues

        #print("Node:", processedInput[0], ", Time:", processedInput[1], ", Sensor:", processedInput[2], ", Data:", processedInput[3])


if __name__ == '__main__':
    ndim = 1
    begin()