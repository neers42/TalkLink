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
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

# キャッシュを有効にするため、先に依存ファイルだけコピー
COPY CMakeLists.txt ./
COPY CommonLib ./CommonLib
COPY Proto ./Proto
COPY AuthenticationService ./AuthenticationService
COPY ThirdParty ./ThirdParty
COPY Tests ./Tests
COPY config ./config

# ビルドタイプ(dockerコンポーズ等から --build-arg で上書き可能、デフォルトは本番用のRelease)
ARG BUILD_TYPE=Release

# ビルド(テストも実行し、失敗したらイメージを作らない)
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DBUILD_AUTH_SERVICE=ON \
    -DBUILD_TESTS=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && \
    cmake --build build --target AuthenticationService auth_tests && \
    ctest --test-dir build -L auth --output-on-failure

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
        curl \
        libgrpc++1.51t64 \
        libprotobuf32t64 && \
    rm -rf /var/lib/apt/lists/*

# ビルド成果物と必要ファイルコピー
COPY --from=builder /app/build/bin/AuthenticationService ./AuthenticationService/bin/AuthenticationService
COPY config/auth-service ./AuthenticationService/conf
COPY /certs/auth-service ./certs/auth-service
COPY AuthenticationService/scripts ./AuthenticationService/scripts

# 実行権限付与及びスクリプト実行
RUN chmod +x ./AuthenticationService/scripts/*.sh 

# Firebaseの公開鍵を取得し、JWTをPEMファイルに変換するスクリプトを実行
ENTRYPOINT ["/app/AuthenticationService/scripts/entrypoint.sh"]
# サーバー起動はスクリプト内で実施
CMD ["./AuthenticationService/bin/AuthenticationService"]