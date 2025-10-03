FROM ubuntu:22.04 AS builder

RUN apt-get update && \
    apt-get install -y \
    cmake \
    g++ \
    git \
    libjsoncpp-dev \
    uuid-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    wget && \
    rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git submodule update --init && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

WORKDIR /app
COPY . .
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make &&\
    [ -f backend ] || (echo "Build failed!" && exit 1)

FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y \
    libjsoncpp25 \
    uuid-dev \
    openssl \
    libssl-dev \
    libssl3 \
    zlib1g \
    zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/backend .
COPY config.json .

EXPOSE 80
CMD ["./backend", "-c", "config.json"]