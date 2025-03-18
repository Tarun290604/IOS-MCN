# O-RAN M-plane implementation

## 1. Build V1
```bash
mkdir && cd build
cmake -DMPLANE_PATH=<path-to>/mplane-v1 ..
make
```

# 2. Build V2
```bash
mkdir && cd build
cmake -DMPLANE_PATH=<path-to>/mplane-v2 -DMPLANE_VERSION=V2 ..
make
```

## 3. Run the client
```bash
./my_client <ru-ip-address> ...
cat delay.xml
```

If no `<ru-ip-address>` specified, `nc_listen_ssh()` command is used instead of `nc_connect_ssh()`.
