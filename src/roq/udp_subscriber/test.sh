#!/usr/bin/env bash

NAME="subscriber"

CONFIG_FILE="config/$NAME.toml"

UDP_SNAPSHOT_PORT=5678
UDP_INCREMENTAL_PORT=6789

# debug?

if [ "$1" == "debug" ]; then
  KERNEL="$(uname -a)"
  case "$KERNEL" in
    Linux*)
      PREFIX="gdb --args"
      ;;
    Darwin*)
      PREFIX="lldb --"
      ;;
  esac
  shift 1
else
	PREFIX=
fi

# launch

$PREFIX "./roq-udp-subscriber" \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --metrics_listen_address "$HOME/run/${NAME}_metrics.sock" \
  --udp_snapshot_port $UDP_SNAPSHOT_PORT \
  --udp_incremental_port $UDP_INCREMENTAL_PORT \
  $@
