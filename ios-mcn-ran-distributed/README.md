# ios-mcn-ran-distributed

**IOS MCN RAN Distributed Sub Project Release**

This repository contains the necessary Git submodules for setting up a Radio Access Network (RAN) based on OpenAirInterface5G (OAI) and to connect with a Service Management and Orchestration (SMO) component using an O1 adapter, as well as a fronthaul interface library for PHY layer connectivity.

 ### **Overview**

This project aims to provide seamless integration between the RAN, SMO, and fronthaul interfaces to enable efficient 5G network operations. The repository includes:

- **OAI RAN (OpenAirInterface5G)**: Implements RAN functionality.
- **O1 Adapter**: An adapter module enabling communication between the RAN and SMO over the O1 interface.
- **Fronthaul Interface Library (PHY)**: A library supporting the PHY layer fronthaul interface, allowing connection between the RAN and remote units.

### **Usage**

**Prerequisites:**
> **_NOTE:_** Please run all the below commands mentioned in root mode or using sudo. <!-- Need to think of which commands absolutely need root --> 

Ensure that the required dependencies are installed for building the project. You can install them by running:

```
make dependency
````
**Building the Project**

Once dependencies are installed, you can build the entire project using the following command:
> **_NOTE:_**  This step can be avoided if the built package is downloaded directly from the releases.
```
make all
```
This will build all the components, including the RAN, O1 adapter, and fronthaul interface. It will also configure the necessary directories for the output binaries and libraries.

**Installing the Project**

To install the built binaries and libraries to the default system locations, run the following:
```
make install
```
This will install the binaries and libraries into /usr/local/bin and /usr/local/lib respectively, and update the system's dynamic linker cache.

**Uninstalling the Project**

If you need to uninstall the project, you can run:
```
make uninstall
```
This will remove the installed binaries and libraries from the system.

**Available Targets**

The following targets are available in the Makefile:

- all: Build all components (RAN, O1 adapter, and fronthaul interface).
- dependency: Install all necessary dependencies.
- install: Install the built binaries and libraries to system directories.
- uninstall: Uninstall the previously installed binaries and libraries.
- xran: Build the XRAN libraries.
- ran: Build the OpenAirInterface5G RAN components.
- o1-adapter: Build the O1 adapter binary.
- clean: Remove all build artifacts.
- help: Display available targets and usage information.

For a complete list of available Makefile targets, run:
```
make help
```
