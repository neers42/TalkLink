#!/bin/bash

# スクリプト自身の絶対パスからプロジェクトルートを取得
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

ENV_FILE="$PROJECT_ROOT/.env"
EXAMPLE_FILE="$PROJECT_ROOT/.env.example"

if [ ! -f "$ENV_FILE" ]; then
    if [ -f "$EXAMPLE_FILE" ]; then
        cp "$EXAMPLE_FILE" "$ENV_FILE"
        chmod 600 "$ENV_FILE"   # .envファイルのパーミッションを600に設定
        echo ".env file created in project root. Please edit it if necessary."
    else
        echo "Error: .env.example not found in project root!"
        exit 1
    fi
else
    echo ".env file already exists in project root."
fi