#!/usr/bin/env bash

set -euo pipefail

CERT_DAYS=365
KEY_BITS=4096

COUNTRY="JP"
STATE="Kanagawa"
LOCALITY="Yokohama"
ORGANIZATION="TalkLink"
ORG_UNIT="Development"
COMMON_NAME="localhost"


generate_cert() {
    local dir="$1"
    local common_name="$2"

    mkdir -p "$dir"

    echo "Generating certificate for ${common_name}..."

    openssl genpkey \
        -algorithm RSA \
        -out "${dir}/server-key.pem" \
        -pkeyopt "rsa_keygen_bits:${KEY_BITS}"

    openssl req \
        -new \
        -x509 \
        -key "${dir}/server-key.pem" \
        -out "${dir}/server-cert.pem" \
        -sha256 \
        -days "${CERT_DAYS}" \
        -subj "/C=${COUNTRY}/ST=${STATE}/L=${LOCALITY}/O=${ORGANIZATION}/OU=${ORG_UNIT}/CN=${COMMON_NAME}" \
        -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
        
    chmod 600 "${dir}/server-key.pem"
    chmod 644 "${dir}/server-cert.pem"
}

generate_cert "./certs/chat-server" "localhost"
generate_cert "./certs/auth-server" "localhost"

echo "Certificates generated."