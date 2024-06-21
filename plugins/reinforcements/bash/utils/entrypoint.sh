#!/bin/bash

while [[ $# -gt 0 ]]; do
  case "$1" in
    set)
      op=set
      shift 1
      ;;
    get)
      op=get
      shift 1
      ;;
  esac
done

if [[ "$op" == "set" ]]; then
  set
elif [[ "$op" == "get" ]]; then
  get
fi

