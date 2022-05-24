# Building CSU FRACTL

## Checkout fractl from CSU

```
  git clone https://github.com/mmbell/fractl.git 
```

## Install required packages

```fractl``` is dependent on ```lrose-core```.

You need to install the dependencies for both. See:

* [lrose-core package dependencies](./lrose_package_dependencies.md)

## Do the build - uses cmake

```
  cd fractl
  mkdir build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local/lrose ..
  make install
```

Note: this assumes lrose is installed in:

```
 /usr/local/lrose.
```

## Install in a different location?

If you want to install in a different location, change ```CMAKE_INSTALL_PREFIX```.

For example:

```
  cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local/lrose ..
  make install
```

to install in:

```
  /opt/local/lrose
```


