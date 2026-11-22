FROM gcc:4.9

COPY . /sockets

WORKDIR /sockets

RUN make

ENTRYPOINT ["./client"]