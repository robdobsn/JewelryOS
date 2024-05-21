import requests
import time
import json
import os

# Break out using ctrl-c

ip_address = "192.168.86.156"
url = "http://" + ip_address + "/api/hrmsamples/get"

output_file = "HRMReadOverWifi.txt"
output_file = os.path.join(os.path.dirname(__file__), output_file)

def decode_content(content):

    output_list = []
    utf_content = content.decode("utf-8")
    lines = utf_content.split("\n")
    for line in lines:
        if line != "":
            line_obj = json.loads(line)
            if "s" in line_obj:
                output_list += line_obj["s"]
    return output_list

if __name__ == "__main__":

    while True:
        try:
            response = requests.get(url)
            # print(response.content)

            with open(output_file, "a") as f:
                read_data = decode_content(response.content)
                for sample_value in read_data:
                    f.write(str(sample_value) + "\n")
                print(f"Captured {len(read_data)} samples")
                # print(read_data)

        except KeyboardInterrupt:
            break

        for i in range(20):
            time.sleep(0.1)
