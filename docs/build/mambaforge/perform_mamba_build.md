## How to perform an lrose-core build and install using mambaforge

Normally lrose-core is built using the system libraries installed with the OS.

This procedure allows you to perform the build using only
the packages installed via mamba-forge,
which is a derivative of anaconda and conda-forge.

### 1. Install mambaforge

The miniforge distribution is available in GitHub at:

* [https://github.com/conda-forge/miniforge](https://github.com/conda-forge/miniforge)

Go to the latest release, for example:

* [https://github.com/conda-forge/miniforge/releases/tag/24.7.1-0](https://github.com/conda-forge/miniforge/releases/tag/24.7.1-0)

and download the .sh file for your OS. For example, I downloaded:

```
  Mambaforge-24.7.1-0-Linux-x86_64.sh
```

Make is executable:

```
  chmod +x Mambaforge-24.7.1-0-Linux-x86_64.sh
```

and then run it:

```
  ./Mambaforge-24.7.1-0-Linux-x86_64.sh
```

Install in the default location which is:

```
  $HOME/mambaforge
```

### 2. Install required packages in ```mambaforge```

Check out ```lrose-bootstrap```:

```
  git clone https://github.com/ncar/lrose-bootstrap
```

Run the script to install the packages required by ```lrose-core```:

```
  cd lrose-bootstrap/conda-forge/mambaforge
  ./install_mamba_packages
```

### 3. Build using conda-build

conda-build will build lrose-code, using the recipe in the meta.yaml file.

```
  cd lrose-bootstrap/conda-forge/mambaforge
  ~/mambaforge/bin/conda-build .
```

### 4. Install the package using the local build

conda-build will build lrose-code, using the recipe in the meta.yaml file.

```
   ~/mambaforge/bin/mamba install --use-local lrose-core
```

```lrose-core``` will be installed in:

* ```~/mambaforge/bin```
* ```~/mambaforge/lib```
* ```~/mambaforge/include```

