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
    libhiredis-dev \
    pkg-config \
    libglib2.0-dev \
    libsecret-1-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# キャッシュを有効にするため、先に依存ファイルだけコピー
COPY CMakeLists.txt ./
COPY CommonLib ./CommonLib
COPY ChatServer ./ChatServer
COPY ThirdParty ./ThirdParty
COPY Tests ./Tests


# ビルド
# -s : ソースディレクトリ（CMakeLists.txt がある場所）を指定
# -B : ビルドディレクトリを指定
# -G : ビルドシステムを指定（Ninjaを使用）
# -DCMAKE_BUILD_TYPE=Release : リリースビルドを指定
# -DBUILD_CHAT_SERVER=ON : ChatServerをビルドするオプションを有効化
# -DWITH_MYSQL=ON : MySQLサポートを有効化
# -DCMAKE_POLICY_VERSION_MINIMUM=3.5 : CMakeのポリシーバージョンを指定
# cmake --build build --target ChatServer : 指定したターゲット（ChatServer）をビルド
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CHAT_SERVER=ON \
    -DBUILD_TESTS=ON \
    -DWITH_MYSQL=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && \
    cmake --build build --target ChatServer domain_tests && \
    ctest --test-dir build -L domain --output-on-failure


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
        libpq-dev \
        libhiredis-dev \
        libssl3 \
        libglib2.0-0 \
        libsecret-1-0 \
        libcurl4 \
        dbus \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# ビルド成果物と必要ファイルコピー
COPY --from=builder /app/build/bin/ChatServer ./ChatServer/bin/ChatServer
RUN test -s ./ChatServer/bin/ChatServer
COPY ChatServer/conf ./ChatServer/conf
COPY ChatServer/certs ./ChatServer/certs
COPY ThirdParty/lib/mysql-connector-c++-9.1.0/linux/lib64/libmysqlcppconn.so.10 /usr/local/lib/libmysqlcppconn.so.10

# certs ディレクトリの確認用ログを出すようにしてもよい
RUN ls -l ./ChatServer/certs

# 共有ライブラリのリンクを更新
RUN ldconfig

CMD ["./ChatServer/bin/ChatServer"]