#!/bin/bash
set -e

TALKLINK_DIR="/home/ubuntu/talklink/certs"

# 証明書をコピーし配置
cp -L /etc/letsencrypt/live/chat.talklink.jp/fullchain.pem \
    "$TALKLINK_DIR/chat-server/server-cert.pem"

cp -L /etc/letsencrypt/live/chat.talklink.jp/privkey.pem \
    "$TALKLINK_DIR/chat-server/server-key.pem"

cp -L /etc/letsencrypt/live/auth.talklink.jp/fullchain.pem \
    "$TALKLINK_DIR/auth-server/server-cert.pem"

cp -L /etc/letsencrypt/live/auth.talklink.jp/privkey.pem \
    "$TALKLINK_DIR/auth-server/server-key.pem"

chmod 644 "$TALKLINK_DIR/chat-server/server-cert.pem"
chmod 600 "$TALKLINK_DIR/chat-server/server-key.pem"
chmod 644 "$TALKLINK_DIR/auth-server/server-cert.pem"
chmod 600 "$TALKLINK_DIR/auth-server/server-key.pem"

echo "$(date): TalkLink TLS certificates deployed successfully"