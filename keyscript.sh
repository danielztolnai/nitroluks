#!/bin/sh
echo "[keyscript] Unlocking $CRYPTTAB_SOURCE ($CRYPTTAB_NAME) by nitrokey, waiting for device..." >&2

# Try to read the password from Nitrokey
exit_status=1
if /bin/nitro_luks -d; then
    /lib/cryptsetup/askpass "* Please unlock disk ${CRYPTTAB_NAME}: " | /bin/nitro_luks
    exit_status=$?
fi

# In case reading from Nitrokey fails, fall back to a LUKS password prompt
if [ $exit_status -eq 1 ]; then
    /lib/cryptsetup/askpass "Please unlock disk ${CRYPTTAB_NAME}: "
fi
