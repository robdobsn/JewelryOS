import numpy as np
from scipy.signal import butter, lfilter_zi, filtfilt, lfilter
import csv

# Filter parameters
fs = 25.0  # Sample rate, Hz
lowcut = 0.75  # Desired low cut frequency of the filter, Hz
highcut = 3.0  # Desired high cut frequency of the filter, Hz
order = 2  # Filter order (this results in an 8th-order filter for bandpass)

# Create Butterworth bandpass filter
b, a = butter(order, [lowcut, highcut], fs=fs, btype='band')

# Print coefficients
print("b coefficients:", b)
print("a coefficients:", a)

# Generate initial conditions for the filter
zi = lfilter_zi(b, a) * 0  # Initial conditions (assuming zero initial conditions)

# Print initial conditions
print("Initial conditions (zi):", zi)

np.set_printoptions(suppress=True, formatter={'float_kind': '{:0.8f}'.format})
USE_GENERATED_DATA = True

if USE_GENERATED_DATA:
    # Generate a sample signal for testing
    duration = 10.0  # seconds
    t = np.linspace(0, duration, int(fs*duration), endpoint=False)  # N seconds of data at 25 Hz
    data = np.sin(1.0 * 2 * np.pi * t) + 0.5 * np.sin(2.0 * 2 * np.pi * t)
else:
    # Read from CSV file 20240519_1_ADC_Data.csv second column is the Red ADC data
    data = []
    with open('20240519_1_ADC_Data.csv', newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        for row in reader:
            if row[1] == "Red":
                continue
            data.append(float(row[1]))

# Print data
# print("Data:", data)
    
# Apply the filter
filtered_data = lfilter(b, a, data)  # Using lfilter with initial conditions

zipped = zip(data, filtered_data)
for d, f in zipped:
    print("Data: ", d, "Filtered: ", f)
# print("Filtered data:", filtered_data[0:100])
