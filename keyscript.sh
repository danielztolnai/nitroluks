#!/bin/sh
echo "*** Trying to unlock the disk $CRYPTTAB_SOURCE ($CRYPTTAB_NAME) by nitrokey, waiting for the USB to be recognized..." >&2

# set exit status to fail so we fall back to default luks prompt if execution of nitro_luks does not work for some reason
exit_status=1
retry_pin=2

# call nitroluks to get the LUKS key
while [ $retry_pin -eq 2 ]; do
    /lib/cryptsetup/askpass "Please unlock nitrokey: " | /bin/nitro_luks
    exit_status=$?
    retry_pin=$exit_status
done

# in case nitroluks fails, fall back to the default LUKS password prompt
if [ $exit_status -eq 1 ]; then
    /lib/cryptsetup/askpass "Please unlock disk ${CRYPTTAB_NAME}: "
else
    echo "*** LUKS setup successful by nitrokey" >&2
fi
