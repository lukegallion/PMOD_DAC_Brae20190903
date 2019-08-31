import pandas as pd
import Bertran_Controller as bertran
import time
import seaborn as sns

RUN_TIME = 60  # seconds
SAMPLE_FREQ = 10  # Hertz
FILENAME = "SavedData.csv"

# Create a Bertran High Voltage Controller object
bert = bertran.BertranControl()

# Initialize electrodes + ADC channels (This could probably be a function, since it won't change much
bert.set_voltage_channel_adc(1, 5)
bert.set_voltage_channel_adc(2, 2)
bert.set_current_channel_adc(1, 3)
bert.set_current_channel_adc(2, 4)
bert.set_voltage_channel_adc(3, 1)
bert.set_voltage_channel_adc(3, 0)

# Create new dataframe
run_data = bertran.create_electropherogram_dataframe()

# Set voltages and start voltage
bert.set_voltage(1, 425)
bert.set_voltage(2, 312)
start_time = time.time()
bert.load_changes()

while time.time() - start_time < RUN_TIME:
    # Retreive new data
    data_time, data = bert.get_data()
    # Add new data to the dataframe
    run_data.append(bertran.create_electropherogram_dataframe(data_time, data))
    # Sleep for a little bit
    time.sleep(1 / SAMPLE_FREQ)

bert.power_down()

sns.lineplot(x='Time', y=['C1_voltage', 'C1_current'])
run_data.to_csv(FILENAME)
