import numpy as np
from scipy.signal import butter, lfilter_zi, lfilter

# Filter parameters
fs = 25.0  # Sample rate, Hz
lowcut = 0.75  # Desired low cut frequency of the filter, Hz
highcut = 3.0  # Desired high cut frequency of the filter, Hz
order = 2  # Filter order (4th-order for bandpass)

# Create Butterworth bandpass filter
b, a = butter(order, [lowcut, highcut], fs=fs, btype='band')

# Print coefficients
print("b coefficients:", b)
print("a coefficients:", a)

# Zero initial conditions for the filter
zi = lfilter_zi(b, a) * 0

# Print initial conditions
print("Initial conditions (zi):", zi)

# Generate a sample signal for testing
duration = 10.0  # seconds
t = np.linspace(0, duration, int(fs*duration), endpoint=False)  # N seconds of data at 25 Hz
data = np.sin(1.0 * 2 * np.pi * t) + 0.5 * np.sin(2.0 * 2 * np.pi * t)
    
# Apply the filter
filtered_data = lfilter(b, a, data)

np.set_printoptions(suppress=True, formatter={'float_kind': '{:0.8f}'.format})
zipped = zip(data, filtered_data)
for d, f in zipped:
    print("Data: ", d, "Filtered: ", f)
