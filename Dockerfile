FROM espressif/idf:v5.4
WORKDIR /project
RUN apt-get update && apt-get install -y g++
