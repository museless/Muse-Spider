#! /bin/bash
# when date was changed, outbug should kill himself and wake himself again

pkill -SIGINT ^outbug$
sleep 64
/MuseSp/bin/outbug &
