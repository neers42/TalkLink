
#!/usr/bin/env bash
set -euo pipefail
umask 077

# 実行環境
MODE="${1:-}"

# スクリプトの場所からリポジトリルートを取得
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

case "$MODE" in
    development)
        COMPOSE_FILE="docker-compose.yml"
        ENV_FILE=".env"
        ;;
    production)
        COMPOSE_FILE="compose.prod.yml"
        ENV_FILE=".env.production"
        ;;
    *)
        echo "Usage: $0 {development|production}" >&2
        exit 1
        ;;
esac

BACKUP_DIR="$PROJECT_DIR/backups/$MODE"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="$BACKUP_DIR/talklink_${DATE}.sql.gz"
TEMP_FILE="${BACKUP_FILE}.tmp"

mkdir -p "$BACKUP_DIR"

trap 'rm -f "$TEMP_FILE"' EXIT

echo "[$(date)] Backup started. Mode: $MODE"

cd "$PROJECT_DIR"

# MySQLバックアップ
docker compose \
    --env-file "$ENV_FILE" \
    -f "$COMPOSE_FILE" \
    exec -T mysql \
    sh -c '
        : "${MYSQL_DATABASE:?MYSQL_DATABASE is not set}"
        : "${MYSQL_BACKUP_PASSWORD:?MYSQL_BACKUP_PASSWORD is not set}"

        export MYSQL_PWD="$MYSQL_BACKUP_PASSWORD"

        exec mysqldump \
            --protocol=socket \
            -u talklink_backup \
            --single-transaction \
            --quick \
            --no-tablespaces \
            "$MYSQL_DATABASE"
    ' \
    | gzip > "$TEMP_FILE"

# 圧縮ファイルの整合性確認
gzip -t "$TEMP_FILE"

# 正常終了後に正式ファイル名へ変更
mv "$TEMP_FILE" "$BACKUP_FILE"

echo "[$(date)] Backup completed: $BACKUP_FILE"
