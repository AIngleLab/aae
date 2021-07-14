#!/usr/bin/env bash



set -ex

cd "${0%/*}/../../../.."

VERSION=$(<share/VERSION.txt)

java_tool() {
  java -jar "lang/java/tools/target/aingle-tools-$VERSION.jar" "$@"
}

py_tool() {
  PYTHONPATH=lang/py python3 -m aingle.tool "$@"
}

ruby_tool() {
  ruby -Ilang/ruby/lib lang/ruby/test/tool.rb "$@"
}

tools=( {java,py,ruby}_tool )

proto=share/test/schemas/simple.avpr

portfile="/tmp/interop_$$"

cleanup() {
  rm -rf "$portfile"
  for job in $(jobs -p); do
    kill "$job" 2>/dev/null || true;
  done
}

trap 'cleanup' EXIT

for server in "${tools[@]}"; do
  for msgDir in share/test/interop/rpc/*; do
    msg="${msgDir##*/}"
    for c in "$msgDir/"*; do
      echo "TEST: $c"
      for client in "${tools[@]}"; do
        rm -rf "$portfile"
        "$server" rpcreceive 'http://127.0.0.1:0/' "$proto" "$msg" \
          -file "$c/response.aingle" > "$portfile" &
        count=0
        until [[ -s "$portfile" ]]; do
          sleep 1
          if (( count++ >= 10 )); then
            echo "$server did not start." >&2
            exit 1
          fi
        done
        read -r _ port < "$portfile"
        "$client" rpcsend "http://127.0.0.1:$port" "$proto" "$msg" \
          -file "$c/request.aingle"
      done
    done
  done
done
