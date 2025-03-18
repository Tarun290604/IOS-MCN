ROOT :=$(shell pwd)
OAI_DIR := $(ROOT)/openairinterface5g
OAI_BUILD_DIR := $(OAI_DIR)/cmake_targets/ran_build/build
OAI_XRAN_PATCH_DIR := $(OAI_DIR)/cmake_targets/tools/oran_fhi_integration_patches/E
XRAN_DIR := $(ROOT)/phy/fhi_lib
XRAN_LIB_DIR := $(XRAN_DIR)/lib
RTE_SDK := $(ROOT)/dpdk-stable-20.11.9

PREFIX := /usr/local        # default install location
BINDIR := /usr/local/bin     # default binary install location
LIBDIR := /usr/local/lib     # default library install location
XRAN_INSTALL_DIR := $(LIBDIR)/xran

all: git build-dirs dependency xran ran o1-adapter 
	@echo "Built everything successfully"

### Install part of the make is not complete yet ###
install: ./build/lib ./build/bin
	install -D ./build/bin/* $(BINDIR)
	install -D ./build/lib/* $(LIBDIR)
	install -D ./build/xran/lib/build/libxran.so /usr/local/xran/lib/build/libxran.so
	ldconfig
	@echo "install done"

uninstall:
	rm -rf /usr/local/bin/nr-* /usr/local/bin/gnb-adapter
	cd /usr/local/lib && rm -rf libcoding.so libldpc_optim.so liboai_device.so liboran_fhlib_5g.so \
		librfsimulator.so libtelnetsrv_ci.so libtelnetsrv_o1.so libdfts.so libldpc_orig.so \
		liboai_transpro.so libparams_libconfig.so libtelnetsrv_5Gue.so libtelnetsrv_ciUE.so \
		libtelnetsrv_rrc.so libldpc_optim8seg.so libldpc.so liboai_usrpdevif.so libparams_yaml.so \
		libtelnetsrv_bearer.so libtelnetsrv_enb.so libtelnetsrv.so
	ldconfig
	rm -rf /usr/local/xran
	ldconfig
	@echo "Uninstall successful!" 


git:
	git submodule init
	git submodule update

dependency: dep-ran dep-o1-adapter dep-dpdk

	@echo Done!

dep-ran: 
	# Install ran depencies using build_oai
	cd openairinterface5g/cmake_targets && ./build_oai -I
	apt install libxml2-dev -y #temporarily installing it here to fix the build issue, should be moved into ./build_oai script

dep-o1-adapter:
	# build and install depenencies
	rm -rf /opt/dev/install/* ## Temporarily added this here to remove the o1-adapter dependecy libraries before building. Should change this later. !!
	apt install libpcre2-dev -y
	./o1-adapter/docker/scripts/netconf_dep_install.sh
	ldconfig

dep-dpdk:
	# installing packages depend on the Linux version; here Ubuntu 22.02 is used as reference, but packages can change
	apt-get update -y
	apt-get install -y meson
	apt install -y libnuma-dev
	apt-get install -y tzdata build-essential git cmake pkg-config unzip wget libpcre2-dev zlib1g-dev libssl-dev autoconf libtool
	apt-get install -y --no-install-recommends psmisc unzip wget openssl openssh-client vsftpd openssh-server

ran: ran-fhi72 ran-fhi8
	@echo Built RAN components successfully!

ran-fhi72:
	cd openairinterface5g/cmake_targets && ./build_oai --gNB --ninja  -t oran_fhlib_5g --noavx512 --cmake-opt -Dxran_LOCATION=/usr/local/xran/lib --build-lib "telnetsrv" 
	install -D openairinterface5g/cmake_targets/ran_build/build/nr-* ./build/bin/
	install -D openairinterface5g/cmake_targets/ran_build/build/*.so ./build/lib/
	@echo Done!

xran:
	cd phy && git restore . 
	cd phy && git checkout oran_e_maintenance_release_v1.0 && git apply $(OAI_XRAN_PATCH_DIR)/oaioran_E.patch && git apply $(OAI_XRAN_PATCH_DIR)/iisc_oaioran_E.patch
	cd phy/fhi_lib/lib && XRAN_DIR=$(XRAN_DIR) RTE_SDK=$(RTE_SDK) make clean && XRAN_DIR=$(XRAN_DIR) RTE_SDK=$(RTE_SDK) make XRAN_LIB_SO=1
	@echo Built XRAN successfully.
	sudo rm -rf /usr/local/xran
	install -D $(XRAN_LIB_DIR)/build/libxran.so /usr/local/xran/lib/build/libxran.so
	install -D $(XRAN_LIB_DIR)/build/libxran.so ./build/xran/lib/build/libxran.so
	cd phy/fhi_lib/lib && find . -name '*.h' -exec install -D {} /usr/local/xran/lib/{} \;
	cd phy/fhi_lib/app && find . -name '*.h' -exec install -D {} /usr/local/xran/app/{} \;
	@echo Installed XRAN successfully.

	@echo Done!

dpdk:
	wget http://fast.dpdk.org/rel/dpdk-20.11.9.tar.xz
	tar -xvf dpdk-20.11.9.tar.xz
	cd dpdk-stable-20.11.9 && meson build && ninja -C build && ninja install -C build

	@echo Done!

ran-fhi8: uhd
	cd openairinterface5g/cmake_targets && ./build_oai --gNB -w  USRP --noavx512
	install -D openairinterface5g/cmake_targets/ran_build/build/liboai_usrpdevif.so ./build/lib/
	install -D openairinterface5g/cmake_targets/ran_build/build/liboai_device.so ./build/lib/

.PHONY: uhd

uhd: dep-uhd
	[ -d "uhd" ] || git clone https://github.com/EttusResearch/uhd.git
	cd uhd && git checkout v4.6.0.0 && cd host && mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr/local ../ && make && make test && make install && sudo ldconfig

dep-uhd:
	apt install git cmake g++ libboost-all-dev libgmp-dev swig python3-numpy \
	python3-mako python3-sphinx python3-lxml doxygen libfftw3-dev \
	libsdl1.2-dev libgsl-dev libqwt-qt5-dev libqt5opengl5-dev python3-pyqt5 \
	liblog4cpp5-dev libzmq3-dev python3-yaml python3-click python3-click-plugins \
	python3-zmq python3-scipy python3-gi python3-gi-cairo gir1.2-gtk-3.0 \
	libcodec2-dev libgsm1-dev libusb-1.0-0 libusb-1.0-0-dev libudev-dev -y
	apt install python3-pip -y
	pip install mako
	pip install numpy

phy-test:
	cd openairinterface5g/cmake_targets && ./build_oai -P --nrUE --noavx512
	# Install commands to install phy-test binaries yet to implemented

o1-adapter:
	# build the binary
	cd ./o1-adapter/src && ./build.sh
	cp -r ./o1-adapter/docker/config conf/o1-config
	install -D ./o1-adapter/src/gnb-adapter ./build/bin/

	@echo Done!

.PHONY: o1-adapter

build-dirs:
	mkdir -p build
	mkdir -p build/bin
	mkdir -p build/lib

help:
	@echo "Available targets:"
	@echo "  all           - Build everything"
	@echo "  dependency    - Install dependencies"
	@echo "  xran          - Build XRAN libraries"
	@echo "  ran           - Build OAI RAN components"
	@echo "  clean         - Remove build artifacts"
	@echo "  install       - Install binaries and libraries"
	@echo "  uninstall     - Remove installed files"
