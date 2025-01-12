FROM gcc:latest
RUN apt-get update && apt-get install -y libzmq3-dev
COPY BS_A2_4_receiver.c /BS_A2_4_receiver.c
RUN gcc -o receiver BS_A2_4_receiver.c -lpthread -lrt -lm -lzmq
ENTRYPOINT ["./receiver"]
