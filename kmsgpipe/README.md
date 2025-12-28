## Dev env setup

### Suggested folder structure

```makefile
kmsgpipe/
├── include/
│   └── kmsgpipe.h              # public driver interface (may include kmsgpipe_common.h)
├── shared/
│   └── include/
│       └── kmsgpipe_common.h   # shared between user lib + kernel
│   └── src/
│       └── ring_buffer.c
│
├── lib/
│   ├── src/
│   ├── include/
│   └── tests/                  # google tests for business logic
│
├── labs/
│   ├── lab1/
│   │   └── README.md
│   └── lab2/...
│
└── Makefile
```

### Notes

Short tutorials on various linux driver building and testing concepts

- [KBuild Objects](./labs/KBuild-objects.md): Info about various kbuild targets used in KBuild Makefile
- [Testable drivers](./labs/kernel-driver-design-and-testing.md): Info about organizing your driver code so that it is testable using Kunit framework
- [Kunit setup](./labs/kunit_kmsgpipe_setup.md): A short wiki documenting how to setup your driver code for unit testing with Kunit
