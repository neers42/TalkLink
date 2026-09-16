# ===============================
# Build Stage
# ===============================
# syntax=docker/dockerfile:1.4
FROM ubuntu:26.04 AS builder

WORKDIR /app

# パッケージインストール
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    ninja-build \
    libssl-dev \
    libpq-dev \
    pkg-config \
    libwolfssl-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# キャッシュを有効にするため、先に依存ファイルだけコピー
COPY CMakeLists.txt ./
COPY CommonLib ./CommonLib
COPY AuthenticationServer ./AuthenticationServer
COPY ThirdParty ./ThirdParty

# ビルド
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_AUTH_SERVER=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && \
    cmake --build build --target AuthenticationServer

# ===============================
# Runtime Stage
# ===============================
FROM ubuntu:26.04

WORKDIR /app

ENV TZ=Asia/Tokyo
RUN apt-get update && \
    apt-get install -y tzdata && \
    ln -sf /usr/share/zoneinfo/Asia/Tokyo /etc/localtime && \
    echo "Asia/Tokyo" > /etc/timezone && \
    DEBIAN_FRONTEND=noninteractive dpkg-reconfigure -f noninteractive tzdata && \
    apt-get install -y \
        libssl3 \
        libpq-dev \
        libcurl4 \
        dbus \
        ca-certificates \
        python3 \
        python3-pip \
        python3-cryptography \
        curl && \
    rm -rf /var/lib/apt/lists/*

# ビルド成果物と必要ファイルコピー
COPY --from=builder /app/build/bin/AuthenticationServer ./AuthenticationServer/bin/AuthenticationServer
COPY AuthenticationServer/conf ./AuthenticationServer/conf
COPY /certs/auth-server ./certs/auth-server
COPY AuthenticationServer/scripts ./AuthenticationServer/scripts

# 実行権限付与及びスクリプト実行
RUN chmod +x ./AuthenticationServer/scripts/*.sh 

# Firebaseの公開鍵を取得し、JWTをPEMファイルに変換するスクリプトを実行
ENTRYPOINT ["/app/AuthenticationServer/scripts/entrypoint.sh"]
# サーバー起動はスクリプト内で実施
CMD ["./AuthenticationServer/bin/AuthenticationServer"]