FROM alpine:latest
MAINTAINER Byeonggon Lee (gonny952@gmail.com)

RUN apk update && apk add git cmake automake autoconf libtool make gcc musl-dev cmocka-dev jansson-dev uthash-dev

RUN mkdir -p /usr/src
RUN git clone https://github.com/vinbero/libgenc /usr/src/libgenc
RUN git clone https://github.com/vinbero/libgaio /usr/src/libgaio
RUN git clone https://github.com/vinbero/libfastdl /usr/src/libfastdl
RUN git clone https://github.com/vinbero/libvinbero_common /usr/src/libvinbero_common
RUN git clone https://github.com/vinbero/vinbero-interfaces /usr/src/vinbero-interfaces
RUN git clone https://github.com/vinbero/vinbero /usr/src/vinbero

RUN mkdir /usr/src/libgenc/build; cd /usr/src/libgenc/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install

RUN mkdir /usr/src/libgaio/build; cd /usr/src/libgaio/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install

RUN mkdir /usr/src/libfastdl/build; cd /usr/src/libfastdl/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install

RUN mkdir /usr/src/libvinbero_common/build; cd /usr/src/libvinbero_common/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Debug ..; make; make test; make install

RUN mkdir /usr/src/vinbero-interfaces/build; cd /usr/src/vinbero-interfaces/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Debug ..; make; make test; make install

RUN mkdir /usr/src/vinbero/build; cd /usr/src/vinbero/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Debug ..; make; make test; make install
