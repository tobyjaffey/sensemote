#!/bin/sh

make -C cctl/cctl-prog
make -C config

for APP in `cd apps && ls`
do
	make APP=$APP PLATFORM=CC1110 || exit 1
	make APP=$APP PLATFORM=POSIX || exit 1
done

