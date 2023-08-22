FROM alpine:3.18 as builder

RUN apk add --no-progress --no-cache \
    linux-headers gcc g++ clang-dev make cmake bash \
    mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static\
    openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
    pcre-dev bison git musl-dev libelf-static elfutils-dev zstd-static bzip2-static xz-static

WORKDIR /build

RUN wget -O - https://github.com/jemalloc/jemalloc/releases/download/5.3.0/jemalloc-5.3.0.tar.bz2 | tar -xj

WORKDIR /build/jemalloc-5.3.0

RUN ./configure --prefix=/usr \
    && make \
    && make install

COPY . /build/fluffos
RUN mkdir /build/fluffos/build

WORKDIR /build/fluffos/build
RUN cmake .. -DMARCH_NATIVE=OFF -DSTATIC=ON \
    && make install

FROM alpine:3.18

WORKDIR /fluffos

COPY --from=builder /build/fluffos/build/bin ./bin

ENTRYPOINT ["/fluffos/bin/driver"]
