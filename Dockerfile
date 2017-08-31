FROM ubuntu:17.04

WORKDIR /root

RUN apt-get update
RUN apt-get install cmake -y
RUN apt-get install g++ -y
RUN apt-get install g++-multilib -y
RUN apt-get install g++-5 -y
RUN apt-get install libboost-all-dev -y
RUN apt-get install unzip -y
RUN apt-get install wget -y

ADD src hlcup/src/
ADD tests hlcup/tests/
ADD CMakeLists.txt hlcup/

RUN cd hlcup && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make

EXPOSE 80

CMD echo 1 > /proc/sys/net/ipv4/tcp_low_latency
CMD cd hlcup/build && unzip /tmp/data/data.zip && ./server

