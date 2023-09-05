riscv is a computer architecture just like x86 or arm, the only difference is it's open. it's simpler compared to x86 or arm. riscv is just a specification of the isa which is an abstract model that people could use to create a real cpu, or virtual machine that emulate it; so this is what I did, I made a software emulation layer that execute riscv instructions and provide io's to the programs.

## 1. Build

building the project is straight forward:
```bash
make
```

## 2. Run

the virtual machine is ready. the only missing part is the software. so we need a full system with a kernel and user programs to run on the vm, because the kernel will setup the environment, including io communication, and memory protection, then launches user space programs. for demo purposes you can just run the shipped in kernel:

```bash
make run
```

wait for the system to boot and you will get a shell.

## custom image:

if you want to build your own kernel image follow these instructions:

your directory structure should look like after you setup all projects:

```bash
.
├── buildroot
├── linux
├── opensbi
├── config
├── core
├── devices
├── dts
├── helpers
├── include
├── kernel
├── main.c
├── Makefile
├── patches
├── README.md
└── soc
```
means all the cloning is done in the root directory.

#### a. rootfs

the rootfs contains the necessary programs to run in user space. we will use buildroot project for this to build an initramfs for the linux kernel.

download buildroot:

```bash
git clone --depth 1 --branch 2021.02.6 git://git.buildroot.net/buildroot buildroot
```

copy the config:

```bash
cp config/rv64_buildroot_config buildroot/.config
```

build:

```bash
(cd buildroot; make -j$(nproc))
```

#### b. Kernel

first of all we need rv64 cross compiler toolchain, checkout your distro package that has that, in my case it's archlinux:

```bash
doas pacman -S riscv64-linux-gnu-gcc
```

then we need to download the linux kernel source (torvalds tree):

```bash
git clone --depth 1 --branch v6.3 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git linux
```

copy the config:

```bash
cp config/rv64_linux_config linux/.config
```

build the kernel:

```bash
(cd linux; make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc))
```

#### c. opensbi

opensbi is a supervisor interface that run in `machine mode` and provides services for the supervisor (the kernel). it's like the firmware and it helps operating systems to access the `soc` services in a portable way.

download opensbi:

```bash
git clone https://github.com/riscv-software-src/opensbi.git opensbi

(cd opensbi; git checkout v1.2)
```

build:

```bash
(cd opensbi; make CROSS_COMPILE=riscv64-linux-gnu- PLATFORM_RISCV_XLEN=64 PLATFORM_RISCV_ISA=rv64ima_zicsr_zifencei PLATFORM=generic FW_PAYLOAD_PATH=../linux/arch/riscv/boot/Image)
```

#### d. run the image

the final image will have the following structure:

```bash
┌──────────┬───────────┬────────────────────┐
│          │           │                    │
│ opensbi  │   linux   │     buildroot      │
│          │           │                    │
└──────────┴───────────┴────────────────────┘
```

we also need the device tree:

```bash
make ./dts/rv64_dt.dtb
```

let's boot the vm:

```bash
./riscv-vm dts/rv64_dt.dtb ./opensbi/build/platform/generic/firmware/fw_payload.bin
```
