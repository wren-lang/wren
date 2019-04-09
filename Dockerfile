FROM debian:9.8

RUN apt-get update && apt-get install -y make gcc python
ADD . /wren
WORKDIR /wren
RUN make
CMD bin/wren
