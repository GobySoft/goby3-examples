#!/bin/bash
# MOOS applications cannot read a process substitution, so write the files they need to /tmp
# and remove them when goby_launch stops this script
set -e
role=$1
platform=$role
[[ "$role" == "auv" ]] && platform=auv${trail_auv_index}

dir=/tmp/goby3_example_trail
mkdir -p $dir
config=$(dirname $0)/trail.pb.cfg.py
$config $role moos > $dir/$platform.moos
$config $role bhv > $dir/$platform.bhv
trap "rm -f $dir/$platform.moos $dir/$platform.bhv" EXIT
while true; do sleep 1; done
