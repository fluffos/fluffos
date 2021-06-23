FROM alpine:3.14 as builder

WORKDIR /build
COPY . /build/fluffos

RUN apk add --no-progress --no-cache \
    linux-headers gcc g++ clang-dev make cmake python2 bash \
    mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static\
    libevent-dev libevent-static libexecinfo-dev libexecinfo-static \
    openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
    pcre-dev bison git

RUN wget -O - https://github.com/jemalloc/jemalloc/releases/download/5.2.1/jemalloc-5.2.1.tar.bz2 | tar -xj \
    && cd jemalloc-5.2.1 \
    && ./configure --prefix=/usr \
    && make \
    && make install

RUN cd /build/fluffos \
    && mkdir build \
    && cd build \
    && cmake .. -DMARCH_NATIVE=OFF -DSTATIC=ON \
    && make install

FROM alpine:3.14

WORKDIR /fluffos

COPY --from=builder /build/fluffos/build/bin ./bin

ENTRYPOINT ["/fluffos/bin/driver"]
