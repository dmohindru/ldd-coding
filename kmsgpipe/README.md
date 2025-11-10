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
│   │   ├── driver/
│   │   ├── user_tests/
│   │   ├── kunit_tests/
│   │   └── README.md
│   └── lab2/...
│
└── Makefile
```
