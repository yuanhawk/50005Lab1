#!/bin/zsh
ipcs -m | grep `whoami` | awk '{ print $2 }' | xargs -n1 ipcrm -m
ipcs -s | grep `whoami` | awk '{ print $2 }' | xargs -n1 ipcrm -s
ipcs -q | grep `whoami` | awk '{ print $2 }' | xargs -n1 ipcrm -q