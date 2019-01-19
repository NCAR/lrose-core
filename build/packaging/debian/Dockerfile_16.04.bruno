## -*- docker-image-name: "nsflrose/lrose-blaze" -*-

FROM ubuntu:16.04 as builder

MAINTAINER Bruno Melli "bpmelli@rams.colostate.edu"
LABEL vendor="Joint NCAR CSU"
LABEL release-date="2019-01-05"

COPY checkout_and_build_auto.py /tmp

# Add libs needed to build lrose and run the lrose components

RUN apt-get update && apt-get install -y  \
    libbz2-dev libx11-dev libpng12-dev libfftw3-dev \
    libjasper-dev qtbase5-dev git \
    gcc g++ gfortran libfl-dev \
    automake make libtool pkg-config libexpat1-dev python \
    cmake libgeographic-dev libeigen3-dev libzip-dev libcurl4-openssl-dev
    
WORKDIR /tmp

RUN ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake /usr/bin/qmake-qt5 && \
    /tmp/checkout_and_build_auto.py --debug --package lrose-blaze --prefix \
         /usr/local/lrose --clean

# Checkout and build fractl

RUN git clone https://github.com/mmbell/fractl.git && \
    cd fractl && \
    git checkout lrose-blaze-20190105 && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local/lrose . && \
    make install

# Checkout and build samurai

WORKDIR /tmp

RUN git clone https://github.com/mmbell/samurai.git && \
    cd samurai && \
    git checkout lrose-blaze-20190105 && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local/lrose . && \
    make install
    
# Start from a fresh ubuntu to not carry over all the build stuff

FROM ubuntu:16.04

# Copy the lrose build

ADD VERSION .
COPY --from=builder /usr/local/lrose/ /usr/local/lrose/

WORKDIR /home/lrose

RUN apt-get update && apt-get install -y \
    libqt5gui5 libqt5core5a qt5-default libx11-6 \
    libfftw3-3 libgeographic14 \
    libfreetype6 libcurl3 && \
    rm -rf /var/lib/apt/lists/* && \
    useradd -G video lrose

ENV PATH /usr/local/lrose/bin:$PATH
