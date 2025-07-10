---
layout: doc
title: Docker (Ubuntu 22.04+)
---

# Building FluffOS using Docker on Ubuntu 22.04+

This page shows how to build and install FluffOS inside a Docker container
based on Ubuntu 22.04. The produced image contains the `driver` binary
ready to run.

## Dockerfile

Create a file named `Dockerfile.ubuntu` with the following content:

```Dockerfile
FROM ubuntu:22.04 AS builder

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential git bison cmake \
        libmysqlclient-dev libpcre3-dev libpq-dev \
        libsqlite3-dev libssl-dev libz-dev \
        libjemalloc-dev libicu-dev

WORKDIR /build

COPY . /build/fluffos

RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) install

FROM ubuntu:22.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        libjemalloc2 libicu70 libssl3 libpcre3 \
        libmysqlclient-dev libpq5 libsqlite3-0 zlib1g && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /fluffos

COPY --from=builder /build/fluffos/build/bin ./bin

ENTRYPOINT ["/fluffos/bin/driver"]
```

## Build the image

```bash
docker build -t fluffos:ubuntu -f Dockerfile.ubuntu .
```

## Run FluffOS

```bash
docker run -it --rm fluffos:ubuntu
```

You can mount your own mudlib when starting the container:

```bash
docker run -it --rm -v /path/to/mudlib:/mudlib fluffos:ubuntu \
  /fluffos/bin/driver /mudlib
```

Prebuilt images are also available from [Docker Hub](https://hub.docker.com/r/fluffos/fluffos).
