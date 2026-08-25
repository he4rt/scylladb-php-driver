## On a clean CentOS 7

Set up the EPEL, install the toolchain and `libuv`:
```
wget https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/e/epel-release-7-13.noarch.rpm
sudo rpm -Uvh epel-release*rpm
sudo yum install -y libuv-devel openssl-devel cmake3 make g++ git
```

Now clone the source code, checkout particular revision if needed:
```
git clone https://github.com/scylladb/cpp-driver.git
cd cpp-driver/
```

Packaging:
```
cat licenses/* > LICENSE.txt
cd packaging/
./build_rpm.sh
```

## On a clean Ubuntu 18

```
sudo apt-get update
sudo apt-get install -y libuv1-dev openssl cmake make g++ git devscripts debhelper dh-exec libssl-dev zlib1g-dev
git clone https://github.com/scylladb/cpp-driver.git
cd cpp-driver/packaging
./build_deb.sh
```
