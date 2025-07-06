FROM ubuntu:22.04

# 依存パッケージインストール
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    libssl-dev \
    libpq-dev \
    libhiredis-dev \
    pkg-config \
    libglib2.0-dev \
    libsecret-1-dev \
    libwolfssl-dev \
    && rm -rf /var/lib/apt/lists/*

    
# 作業ディレクトリを作成
WORKDIR /app

# プロジェクトファイルを全てコピー（.dockerignore で不要ファイルは除外）
COPY . .

# 外部依存の取得とCMakeビルド
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j$(nproc)

 # デフォルトの実行コマンド
CMD ["/bin/bash"]