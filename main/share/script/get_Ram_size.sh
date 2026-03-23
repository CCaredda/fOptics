#!/bin/sh
grep MemTotal /proc/meminfo | awk '{print $2}'
exit $?
