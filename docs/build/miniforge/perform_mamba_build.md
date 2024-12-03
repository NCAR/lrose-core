## How to perform an lrose-core build and install using miniforge

Normally lrose-core is built using the system libraries installed with the OS.

This procedure allows you to perform the build using only
the packages installed via miniforge,
which is an open source derivative of anaconda and conda-forge.

### 1. Install miniforge3

The miniforge3 distribution is available in GitHub at:

* [https://github.com/conda-forge/miniforge](https://github.com/conda-forge/miniforge)

Go to the latest release, for example:

* [https://github.com/conda-forge/miniforge/releases/tag/24.9.2-0](https://github.com/conda-forge/miniforge/releases/tag/24.9.2-0)

and download the .sh file for your OS. For example, for LINUX x86_64, I downloaded:

```
  Miniforge3-24.9.2-0-Linux-x86_64.sh
```

Make is executable:

```
  chmod +x Miniforge3-24.9.2-0-Linux-x86_64.sh
```

and then run it:

```
  ./Miniforge3-24.9.2-0-Linux-x86_64.sh
```

Install in the default location which is:

```
  $HOME/miniforge3
```

### 2. Install required packages in ```miniforge3```

Check out ```lrose-bootstrap```:

```
  git clone https://github.com/ncar/lrose-bootstrap
```

Run the script to install the packages required by ```lrose-core```.

For example:

```
  cd lrose-bootstrap/miniforge
  ./install_mamba_packages.linux
```

### 3. Build using conda-build

conda-build will build lrose-code, using the recipe in the meta.yaml file.

```
  cd lrose-bootstrap/miniforge
  ~/mambaforge/bin/conda-build .
```

conda-build will build lrose-code, using the recipe in the meta.yaml file.

### 4. Install the package using the local build

```
   ~/miniforge3/bin/mamba install --use-local lrose-core
```

```lrose-core``` will be installed in:

* ```~/miniforge3/bin```
* ```~/miniforge3/lib```
* ```~/miniforge3/include```

