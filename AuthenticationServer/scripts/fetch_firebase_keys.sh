#!/bin/bash

# CERTディレクトリ
CERT_DIR="/app/AuthenticationServer/certs"
# Firebaseからダウンロードした公開鍵のJSONファイルのパス
JSON_PATH="${CERT_DIR}/firebase/pubkeys.json"
# Firebaseの公開鍵を取得するURL
KEYS_URL="https://www.googleapis.com/robot/v1/metadata/x509/securetoken@system.gserviceaccount.com"

mkdir -p "${CERT_DIR}/firebase"

#/firebaseの中にファイルが存在する場合は削除する
if find "${CERT_DIR}/firebase" -mindepth 1 | read; then
  echo "[INFO] Removing existing files in ${CERT_DIR}/firebase"
  rm -rf ${CERT_DIR}/firebase/*
fi

# 公開鍵を保存する
echo "[INFO] Fetching Firebase public keys from ${KEYS_URL}"
# curlでFirebaseの公開鍵を取得
# headerとbodyをそれぞれ取得する。headerはキャッシュのmax-ageを取得するために必要、bodyはJSON形式の公開鍵を保存するために必要
response=$(curl -s -i "$KEYS_URL")

# ヘッダー部分と本文部分を分割（空行で区切り）
header=$(echo "$response" | sed -n '/^\r*$/q;p')
body=$(echo "$response" | sed -n '/^\r*$/,$p' | sed '1d')

# max-ageをヘッダーから抽出（小文字大文字区別なし）
#s/正規表現/置換文字列/
max_age=$(echo "$header" | grep -i "Cache-Control" | sed -n 's/.*max-age=\([0-9]*\).*/\1/p')

# JSONをファイルに保存
echo "$body" > "$JSON_PATH"

# ダウンロード成功判定
if [ $? -eq 0 ] && [ -n "$max_age" ]; then
  echo "[INFO] Firebase keys saved to ${JSON_PATH}"
  echo "[INFO] Cache-Control max-age: ${max_age} seconds"
  # max-ageを標準出力で返す（呼び出し元で取得可能）
  echo "$max_age"
  exit 0
else
  echo "[ERROR] Failed to fetch Firebase keys or parse max-age"
  exit 1
fi
