#!/bin/bash
fetch_and_convert() {
  echo "[INFO] Firebase公開鍵を取得"
  MAX_AGE=$(/app/AuthenticationServer/scripts/fetch_firebase_keys.sh | tail -1)
  if [ $? -ne 0 ]; then
      echo "[ERROR] Firebase鍵の取得に失敗"
      exit 1
  fi

  echo "[INFO] JWTをPEMファイルに変換"
  python3 /app/AuthenticationServer/scripts/convert.py || {
      echo "[ERROR] JWT to PEM 変換に失敗"
      exit 1
  }
}

monitor_keys() {
  while true; do
    echo "[INFO] $MAX_AGE 秒スリープしてから再取得"
    sleep "$MAX_AGE"
    fetch_and_convert
  done
}

# 初回取得＆変換を実行
fetch_and_convert

# 非同期で監視開始
monitor_keys &

# サーバー起動
echo "[INFO] Starting AuthenticationServer"
exec "$@"