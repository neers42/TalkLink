#!/bin/bash

set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 0.1.2"
    exit 1
fi

IMAGE_TAG_VERSION="$1"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_DIR"

# 作業ツリーがクリーンであることを確認する
# (コミットし忘れの変更が混在した状態でリリースしてしまうのを防ぐ)
if [ -n "$(git status --porcelain)" ]; then
    echo "Error: Working tree is not clean. Commit or stash your changes before releasing." >&2
    git status --short >&2
    exit 1
fi

# CMakeLists.txtのproject()バージョンと引数のバージョンが一致することを確認する
# (CMakeLists.txtの更新忘れによるgitタグ/Dockerイメージ/バイナリのバージョンのズレを防ぐ)
CMAKE_VERSION=$(grep -oP 'project\(TalkLink VERSION \K[0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt)
if [ -z "$CMAKE_VERSION" ]; then
    echo "Error: Failed to read version from CMakeLists.txt." >&2
    exit 1
fi
if [ "$CMAKE_VERSION" != "$IMAGE_TAG_VERSION" ]; then
    echo "Error: Version mismatch. CMakeLists.txt has '$CMAKE_VERSION' but '$IMAGE_TAG_VERSION' was requested." >&2
    echo "Update CMakeLists.txt's project() VERSION and commit it before releasing." >&2
    exit 1
fi

CHAT_IMAGE="ghcr.io/neers42/talklink-chat-server:${IMAGE_TAG_VERSION}"
AUTH_IMAGE="ghcr.io/neers42/talklink-auth-server:${IMAGE_TAG_VERSION}"
GIT_TAG="v${IMAGE_TAG_VERSION}"

echo "=== Build TalkLink ${IMAGE_TAG_VERSION} ==="

docker build \
    -f ChatServer/Dockerfile \
    -t "${CHAT_IMAGE}" \
    .

docker build \
    -f AuthenticationServer/Dockerfile \
    -t "${AUTH_IMAGE}" \
    .

echo "=== Push images ==="

docker push "${CHAT_IMAGE}"
docker push "${AUTH_IMAGE}"

echo "=== Tag release ==="

git tag -a "${GIT_TAG}" -m "Release ${GIT_TAG}"
git push origin "${GIT_TAG}"

echo "=== Release images pushed successfully ==="
echo "${CHAT_IMAGE}"
echo "${AUTH_IMAGE}"
echo "${GIT_TAG}"
