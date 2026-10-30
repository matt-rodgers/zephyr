podman run \
    -it \
    --rm \
    -w /workdir \
    --volume .:/workdir \
    --user root \
    ghcr.io/zephyrproject-rtos/zephyr-build:main
