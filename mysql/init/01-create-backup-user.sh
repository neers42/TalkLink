
#!/usr/bin/env bash
set -euo pipefail

# 必須環境変数の確認
: "${MYSQL_DATABASE:?MYSQL_DATABASE is not set}"
: "${MYSQL_BACKUP_PASSWORD:?MYSQL_BACKUP_PASSWORD is not set}"

BACKUP_USER="talklink_backup"

# DB名に使用できる文字を制限
if [[ ! "$MYSQL_DATABASE" =~ ^[a-zA-Z0-9_]+$ ]]; then
    echo "Invalid MYSQL_DATABASE" >&2
    exit 1
fi

# SQLに安全に埋め込める文字だけを許可
if [[ ! "$MYSQL_BACKUP_PASSWORD" =~ ^[a-zA-Z0-9_-]+$ ]]; then
    echo "Invalid MYSQL_BACKUP_PASSWORD" >&2
    exit 1
fi

# MySQL公式Dockerイメージの初期化関数を確認
if ! declare -F docker_process_sql >/dev/null; then
    echo "docker_process_sql is not available" >&2
    exit 1
fi

# バックアップ専用ユーザーの作成と権限付与
docker_process_sql <<SQL
CREATE USER IF NOT EXISTS '${BACKUP_USER}'@'localhost'
IDENTIFIED BY '${MYSQL_BACKUP_PASSWORD}';

GRANT SELECT, SHOW VIEW, TRIGGER
ON \`${MYSQL_DATABASE}\`.*
TO '${BACKUP_USER}'@'localhost';
SQL

echo "Backup user initialized successfully."
