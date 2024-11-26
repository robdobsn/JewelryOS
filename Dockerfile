FROM espressif/idf:v5.3.1
WORKDIR /project
RUN apt-get update && apt-get install -y g++
