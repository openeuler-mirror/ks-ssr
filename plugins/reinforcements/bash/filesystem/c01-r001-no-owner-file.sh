#!/bin/bash

function get() {
  num1=$(find `df -l | sed -n '2,$p' | awk '{print $6}' ` -xdev -nouser 2>/dev/null | wc -l)
  num2=$(find `df -l | sed -n '2,$p' | awk '{print $6}' ` -xdev -nogroup 2>/dev/null | wc -l)
  # 如果num1或者num2大于0,则设置enabled变量为false，否则为true
  enabled=$([[ $num1 -gt 0 || $num2 -gt 0 ]] && echo false || echo true)
  echo "{\"enabled\":$enabled}"
}

function set() {
    echo "ok"
}


source ../utils/entrypoint.sh
