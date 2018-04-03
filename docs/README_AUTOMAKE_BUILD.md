## Building using AUTOMAKE and CONFIGURE

### Choose your install directory (prefix)

The default is: `${HOME}/lrose`

### Check out lrose core

```
  git clone https://github.com/NCAR/lrose-core
```

### Get the usage

```
  cd lrose-core
  ./build/checkout_and_build_autp.py --help
```

`package` defaults to `lrose`

`prefix` defaults to `${HOME}/lrose`

Available packages are:

```
  lrose lrose-blaze radx titan cidd
```

### Run the auto checkout and build

The following example is for the lrose-blaze release:

```
  cd lrose-core
  ./build/checkout_and_build_autp.py --debug --prefix ~/lrose --package lrose-blaze
```

Another example - build lrose and install in /usr/local/lrose:

```
  cd lrose-core
  ./build/checkout_and_build_autp.py --debug --prefix /usr/local/lrose --package lrose
```

