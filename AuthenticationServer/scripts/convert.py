import json
import os

# CERTの入出力パスの指定
CERT_JSON_PATH = "./../certs/firebase/pubkeys.json"
CERT_JSON_PATH = "/app/AuthenticationServer/certs/firebase/pubkeys.json"
OUTPUT_DIR = "/app/AuthenticationServer/certs/firebase"

# JSONからPEM形式のファイルを生成する関数
def convert_json_to_pem(json_path, output_dir):
    with open(json_path, "r") as f:
        data = json.load(f)

    for kid, pem_str in data.items():
        pem_path = os.path.join(output_dir, f"{kid}.pem")
        with open(pem_path, "w") as pem_file:
            pem_file.write(pem_str.replace("\\n", "\n"))
        print(f"[INFO] Saved PEM for kid {kid} to {pem_path}")

# メイン処理
if __name__ == "__main__":
    if not os.path.exists(CERT_JSON_PATH):
        print(f"[ERROR] JSON file not found at {CERT_JSON_PATH}")
        exit(1)

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    convert_json_to_pem(CERT_JSON_PATH, OUTPUT_DIR)