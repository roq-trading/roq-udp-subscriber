#!/usr/bin/env bash

NAME="subscriber"

CONFIG_FILE="config/$NAME.toml"

UDP_PORT=6789

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
  --udp_port $UDP_PORT \
  $@
