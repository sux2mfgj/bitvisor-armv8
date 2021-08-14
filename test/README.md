

```shell
$ docker run --rm --privileged multiarch/qemu-user-static:register --reset
```

```
$ docker run -it multiarch/ubuntu-core:arm64-hirsute /bin/bash
(docker) $ apt update && apt upgrade -y && apt install -y cgreen1 libcgreen1 libcgreen1-dev build-essential
```

Link with built bitvisor code. You should run `make` after bitvisor built.
```
$ make -C ../ ARCH=aarch64 && make  && ./test
```

