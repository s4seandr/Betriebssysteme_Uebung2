FROM gcc:latest
RUN apt-get update && apt-get install -y libzmq3-dev
COPY BS_A2_4_sender.c /BS_A2_4_sender.c
RUN gcc -o sender BS_A2_4_sender.c -lpthread -lrt -lm -lzmq
ENTRYPOINT ["./sender"]
