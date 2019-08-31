
import pyserial as serial
import datetime
import logging
import numpy as np
import struct
import pandas as pd

class BertranControl:

    def __init__(self, port = "COM4"):
        self.ser = serial.Serial()
        self.ser.port = port
        self.ser.baudrate = 9600
        self.ser.open()

    def set_voltage_channel_adc(self, channel, adc_pin):
        self.ser.write("V{}{}\n".format(channel, adc_pin).encode())
        return 0

    def set_current_channel_adc(self, channel, adc_pin):
        self.ser.write("C{}{}\n".format(channel, adc_pin).encode())

    def load_changes(self):
        self.ser.write("L\n".encode())

    def power_down(self):
        self.ser.write("X\n".encode())

    def set_voltage(self, channel, voltage):
        self.ser.write("S{}{:09.3f}".format(channel,voltage).encode())

    def get_data(self):
        """
        Retrieves data from all 8 channels of the PMOD DAC
        :return:
        time [datetime object] time of acquisition
        in_data [numpy array, dtype=float] 16 element numpy array with voltage and current measurements from each channel
        index 0 & 8 correspond to channel 0; 1 & 9 correspond to channel 1 and so on...
        """
        time = datetime.datetime.now()
        self.ser.write("R\n".encode())
        in_data = self.ser.read_all()
        in_data = self._interpret_bytes(in_data)
        return time, in_data

    @staticmethod
    def _interpret_bytes(byte_data):
        """
        :param byte_data: From the Arduino, bytes object with a start byte 'S' or 83 that is 16 floats long (16*4).
        :return: numpy array of floats corresponding to the voltage [0-7] and current [8-15] values for channels 0-8
         of the PMOD DAC
        """
        idx = None
        for i in range(len(byte_data)):
            if byte_data[i]==83:
                idx = i+1
                break

        in_data = np.zeros(1,16)

        # If we don't have a start index you will have to return a zeros array
        if idx is None:
            logging.warning("Did Not Find Start Character for Read Data")
            return in_data

        # Move through the bytes array and convert 4 bytes into a float for each of the 16 values
        for i in range(16):
            in_data[0][1]=struct.unpack('f', byte_data[idx+i*4:idx+(i*4+4)])[0]
        return in_data


def create_electropherogram_dataframe(data_time=None,data=None)
    columns = ['Time', 'C0_voltage', 'C1_voltage', 'C2_voltage', 'C3_voltage', 'C4_voltage', 'C5_voltage', 'C6_voltage',
               'C7_voltage', 'C0_current', 'C1_current', 'C2_current', 'C3_current', 'C4_current', 'C5_current',
               'C6_current', 'C7_current']

    if data is None:
        new_df = pd.DataFrame(columns=columns)
    else:
        new_df = pd.DataFrame(data=data,columns = columns[1:])
        new_df['Time']=data_time

    return new_df
