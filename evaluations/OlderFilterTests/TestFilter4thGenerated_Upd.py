import numpy as np
from scipy.signal import butter, lfilter_zi, lfilter

# Filter parameters
fs = 25.0  # Sample rate, Hz
lowcut = 0.75  # Desired low cut frequency of the filter, Hz
highcut = 3.0  # Desired high cut frequency of the filter, Hz
order = 2  # Filter order (4th-order for bandpass)

# Create Butterworth bandpass filter
b, a = butter(order, [lowcut, highcut], fs=fs, btype='band')

# Zero initial conditions for the filter
zi = lfilter_zi(b, a)

# Print coefficients
print("b coefficients:", b)
print("a coefficients:", a)
print("Initial conditions (zi):", zi)

# Generate a sample signal for testing
duration = 100.0  # seconds
t = np.linspace(0, duration, int(fs*duration), endpoint=False)
data = np.sin(1.0 * 2 * np.pi * t) + 0.5 * np.sin(2.0 * 2 * np.pi * t)

# Apply the filter
filtered_data, _ = lfilter(b, a, data, zi=zi*data[0])

# Print the first few filtered values
for d, f in zip(data, filtered_data):
    print(f"x1: {d:.8f}, y1: {f:.8f}")
