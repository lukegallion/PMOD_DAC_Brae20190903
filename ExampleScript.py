import pandas as pd
import Bertran_Controller as bertran
import time
import seaborn as sns

RUN_TIME = 60  # seconds
SAMPLE_FREQ = 10  # Hertz
FILENAME = "SavedData.csv"

# Create a Bertran High Voltage Controller object
bert = bertran.BertranControl()

start_time = time.time()

# Create new dataframe
run_data = bertran.create_electropherogram_dataframe()

while time.time() - start_time < RUN_TIME:
    # Retreive new data
    data_time, data = bert.get_data()
    # Add new data to the dataframe
    run_data.append(bertran.create_electropherogram_dataframe(data_time, data))
    # Sleep for a little bit
    time.sleep(1 / SAMPLE_FREQ)


sns.lineplot(x='Time', y = ['C1_voltage', 'C1_current'])
run_data.to_csv(FILENAME)
