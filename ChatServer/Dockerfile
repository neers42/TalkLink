# ===============================
# Build Stage
# ===============================
# syntax=docker/dockerfile:1.4
FROM ubuntu:22.04 AS builder

WORKDIR /app

# パッケージインストール
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    ninja-build \
    libssl-dev \
    libpq-dev \
    libhiredis-dev \
    pkg-config \
    libglib2.0-dev \
    libsecret-1-dev \
    libwolfssl-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# キャッシュを有効にするため、先に依存ファイルだけコピー
COPY CMakeLists.txt ./
COPY AuthenticationServer ./AuthenticationServer
COPY CommonLib ./CommonLib
COPY ChatServer ./ChatServer


# ビルド
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build


# ===============================
# Runtime Stage
# ===============================
FROM ubuntu:22.04

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libssl-dev \
    libpq-dev \
    libhiredis-dev \
    libwolfssl-dev \
    libglib2.0-0 \
    libsecret-1-0 \
    libcurl4 \
    dbus \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ビルド成果物と必要ファイルコピー
COPY --from=builder /app/build/bin/ChatServer ./ChatServer/bin/ChatServer
COPY ChatServer/conf ./ChatServer/conf
COPY ChatServer/certs ./ChatServer/certs
COPY CommonLib/external_lib/lib/mysql-connector-c++-9.1.0/linux/lib64/libmysqlcppconn.so.10 /usr/local/lib/libmysqlcppconn.so.10

# certs ディレクトリの確認用ログを出すようにしてもよい
RUN ls -l ./ChatServer/certs

# 共有ライブラリのリンクを更新
RUN ldconfig

CMD ["./ChatServer/bin/ChatServer"]