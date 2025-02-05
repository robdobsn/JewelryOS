import os
import csv
import re
import json
import argparse
from pathlib import Path

# Define constants
BYTE_GROUP_LENGTH = 51  # Length of each data group in bytes
TIME_INCREMENT = 0.04   # 40 ms increment for each sample in seconds

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

def parse_read_data(line):
    """Extracts data from log lines containing readData or addSample."""
    groups = []

    # Match timestamps and hex data from readData lines
    time_match = re.search(r'I \((\d+)\)', line)
    data_match = re.search(r'readData\s([0-9a-fA-F]+)', line)
    
    if time_match and data_match:
        timestamp = int(time_match.group(1)) / 1000.0  # Convert milliseconds to seconds
        hex_data = data_match.group(1)
        groups = extract_data_groups(bytes.fromhex(hex_data), timestamp)
        return groups

    # Match addSample JSON data
    json_data_match = re.search(r'addSample\s(.+)', line)
    if json_data_match:
        json_data = json_data_match.group(1)
        extracted_data = json.loads(json_data)

        time_series = extracted_data["t"]
        red_series = extracted_data["r"]
        ir_series = extracted_data["i"]

        for i in range(len(time_series)):
            groups.append({"Time (s)": round(time_series[i] / 1000.0, 2), "Red": red_series[i], "IR": ir_series[i]})

    return groups

def process_log_file(input_file, output_file):
    """Processes the log file and writes to CSV."""
    try:
        with open(input_file, 'r') as file, open(output_file, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["Time (s)", "Red", "IR"])  # Write header

            for line in file:
                groups = parse_read_data(line)
                for group in groups:
                    writer.writerow([f"{group['Time (s)']:.2f}", group["Red"], group["IR"]])

        print(f"CSV file created: {output_file}")
        return 0  # Success exit code

    except Exception as e:
        print(f"Error processing file: {e}")
        return 1  # Failure exit code

def main():
    """Parses command-line arguments and runs the script."""
    parser = argparse.ArgumentParser(description="Process log file and extract HRM data to CSV.")
    parser.add_argument("input_file", type=str, help="Path to the input log file")
    parser.add_argument("output_file", type=str, nargs="?", help="Path to the output CSV file (optional)")

    args = parser.parse_args()

    # Resolve absolute paths
    input_path = Path(args.input_file).resolve()
    output_path = Path(args.output_file).resolve() if args.output_file else input_path.with_suffix(".csv")

    # Check if input file exists
    if not input_path.is_file():
        print(f"Error: Input file '{input_path}' not found!")
        exit(1)

    # Process the file and exit with the appropriate status code
    exit_code = process_log_file(input_path, output_path)
    exit(exit_code)

if __name__ == "__main__":
    main()
