FROM python:3.11.7-slim

RUN apt update && apt install -y git

# Install PlatformIO Core
RUN python3 -m pip install -U platformio

WORKDIR /home/platformio
RUN git clone https://github.com/rusty-labs/lora-mqtt-gateway.git

WORKDIR /home/platformio/lora-mqtt-gateway
RUN platformio run --environment ir-temperature-node

CMD ["sleep","3600"]
