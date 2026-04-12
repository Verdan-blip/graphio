#!/bin/bash

PROTO_FILE="contract/protobuf/window_events.proto"

OUT_DIR_C="include/generated"
OUT_DIR_PY="client/scoring/generated"

mkdir -p "$OUT_DIR_C"
mkdir -p "$OUT_DIR_PY"

if command -v protoc-c &> /dev/null; then
    protoc-c --c_out="$OUT_DIR_C" "$PROTO_FILE"
    echo "[SUCCESS] C-files generated in: $OUT_DIR_C"
else
    echo "[ERROR] protoc-c not found. Please install protobuf-c-compiler."
fi

if command -v protoc &> /dev/null; then
    # Добавляем пустой __init__.py, чтобы Python видел папку как пакет
    touch "$OUT_DIR_PY/__init__.py"
    
    protoc --python_out="$OUT_DIR_PY" "$PROTO_FILE"
    echo "[SUCCESS] python-files generated in: $OUT_DIR_PY"
else
    echo "[ERROR] protoc-c not found. Please install protobuf-c-compiler."
fi

echo "Готово."