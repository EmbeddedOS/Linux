# Linux

- This repository develops various entities in Embedded Linux Systems, from Hardware -> Kernel -> User-Space applications.
- For the hardware, we focus on QEMU emulator to emulate hardware components.

- The repository structure:

```text
.
├── docs                        # General documentations.
│   ├── *.md
│   ├── video                   # Documentations from videos, Youtube, etc.
│   │   └── *.md
│   └── wiki                    # https://wiki.osdev.org/ or Wikipedia, etc.
│       └── *.md
├── hw                          # Hardware code, emulator etc.
│   └── qemu                    # QEMU emulator hardware components.
│       └── *.c, *.h, */
├── kernel                      # Kernel modules, drivers, sub-systems. 
│   ├── examples                # Example code of basic features.
│   │   └── *.c, *.h, */
│   └── *.c, *.h, */
├── Makefile                    # Global make file to build system.
├── README.md                   # README for the repository.
└── usr                         # User application codes.
    ├── examples                # Example code for basic features.
    │   └── *.c, *.h, */
    └── *.c, *.h, */
```
