FROM espressif/idf:v5.1.2
WORKDIR /project
RUN apt-get update && apt-get install -y g++
