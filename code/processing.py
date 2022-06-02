import serial
import json
import time
import re
import math
import sys

import numpy as np


CLEAR_CHAR = chr(0x1b) + "[J"
ESCAPE_CHAR = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

serialPort = serial.Serial("/dev/serial/by-id/usb-ZEPHYR_USB_Console_Tute_EC10EDEC405209D1-if00", 115200)

class Kalman:
    def __init__(self, x_init, cov_init, meas_err, proc_err):
        self.ndim = len(x_init)
        self.A = np.eye(ndim)        #state transition model
        self.H = np.eye(ndim)        #observation model
        self.x_hat =  x_init
        self.cov = cov_init
        self.Q_k = np.eye(ndim)*proc_err #covariance matrix of process noise
        self.R = np.eye(len(self.H))*meas_err   #covariance matrix of observation noise
        
    def update(self, obs, heading):

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

    while True:
        
        rawInput = serialPort.readline().decode("utf-8").strip()
        
        processingInput = ESCAPE_CHAR.sub('', rawInput.split(CLEAR_CHAR)[-1])
        processedInput = processingInput.split(" ")

        # try:
        #     newValues = [float(i) for i in newValues2]
        # except:
        #     newValues = oldValues

        print("Node:", processedInput[0], ", Time:", processedInput[1], ", Sensor:", processedInput[2], ", Data:", processedInput[3])


if __name__ == '__main__':
    begin()