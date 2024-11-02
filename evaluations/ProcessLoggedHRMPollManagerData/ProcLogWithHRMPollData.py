import re
import csv

# Define the byte structure and constants
BYTE_GROUP_LENGTH = 51  # Length of each data group in bytes
TIME_INCREMENT = 0.04   # 40 ms increment for each sample in seconds

def parse_read_data(line):
    """Extracts the timestamp and readData field from a given line and converts to bytes."""
    time_match = re.search(r'I \((\d+)\)', line)
    data_match = re.search(r'readData\s([0-9a-fA-F]+)', line)
    
    if time_match and data_match:
        timestamp = int(time_match.group(1)) / 1000.0  # Convert milliseconds to seconds
        hex_data = data_match.group(1)
        return timestamp, bytes.fromhex(hex_data)
    return None, None

def extract_data_groups(data, initial_timestamp):
    """Parses the 51 bytes of data according to the specified algorithm."""
    parsed_data = []
    timestamp = initial_timestamp  # Start with the initial timestamp
    
    for i in range(0, len(data), BYTE_GROUP_LENGTH):
        group = data[i:i + BYTE_GROUP_LENGTH]
        if len(group) < BYTE_GROUP_LENGTH:
            continue  # Ignore incomplete groups

        # Calculate N and initialize k and i outside the loop
        N = (group[0] + 32 - group[2]) % 32
        k = 3
        i = 0

        while i < N and k + 5 < BYTE_GROUP_LENGTH:
            # Extract Red and IR values
            red_value = (group[k] << 16) | (group[k + 1] << 8) | group[k + 2]
            ir_value = (group[k + 3] << 16) | (group[k + 4] << 8) | group[k + 5]
            parsed_data.append({"Time (s)": round(timestamp, 2), "Red": red_value, "IR": ir_value})
            
            # Update for the next sample
            timestamp += TIME_INCREMENT
            k += 6
            i += 1

    return parsed_data

def main(input_file, output_file):
    with open(input_file, 'r') as file, open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Time (s)", "Red", "IR"])  # Write header

        for line in file:
            if 'readData' in line:
                initial_timestamp, data = parse_read_data(line)
                if data:
                    groups = extract_data_groups(data, initial_timestamp)
                    for group in groups:
                        writer.writerow([f"{group['Time (s)']:.2f}", group["Red"], group["IR"]])

# Run the script with your file
# input_file = "20241031-221727.log"
# input_file = "20241031-230713.log"
# input_file = "20241031-233740.log"
# input_file = "20241031-234442.log"
input_file = "20241101-004359.log"
output_file = input_file + ".csv"
main(input_file, output_file)
print("CSV file created:", output_file)
