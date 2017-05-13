FROM ubuntu:latest
MAINTAINER marius.rieder@nine.ch

RUN apt-get -qq update && \
    apt-get -qq install -y autoconf libzookeeper-mt-dev check bats build-essential libconfig-dev && \
    rm -rf /var/lib/apt/lists/*

# Copy the main application.
COPY . /zksh

WORKDIR /zksh

RUN autoreconf && ./configure && make
