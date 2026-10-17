#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-}"
RETENTION_DAYS=7

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

case "$MODE" in
    development|production)
        ;;
    *)
        echo "Usage: $0 {development|production}" >&2
        exit 1
        ;;
esac

BACKUP_DIR="$PROJECT_DIR/backups/$MODE"

if [[ ! -d "$BACKUP_DIR" ]]; then
    echo "Backup directory not found: $BACKUP_DIR" >&2
    exit 1
fi

echo "Checking backups older than $RETENTION_DAYS days..."

find "$BACKUP_DIR" \
    -maxdepth 1 \
    -type f \
    -name 'talklink_*.sql.gz' \
    -mtime +6 \
    -print

echo "Rotation dry-run completed. No files deleted."