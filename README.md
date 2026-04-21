# μRTOS

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight, hybrid preemptive/cooperative real‑time operating system for AVR MCUs (ATmega328, etc.).  
Designed for simplicity, minimal overhead, and easy integration into Arduino projects.


## ✨ Features

- **Preemptive + cooperative** – timer‑driven ticks with voluntary `uRTOS_Yield()`.
- **Static task definitions** – all task descriptors reside in PROGMEM, no dynamic allocation.
- **Tiny footprint** – core kernel fits in < 2 kB Flash, ~80 for system + ~5 per task RAM bytes.
- **No linker script hacks** – works with standard AVR GCC / Arduino toolchain.
- **Customizable** – pluggable scheduler, weak timer init, optional flags.
- **MIT licensed** – free for personal and commercial use.


## 🚀 Quick Start

1. **Include the header**
	```cpp
	#include <urtos/urtos.h>
	```

2. **Install the tick ISR**
	```cpp
	uRTOS_INSTALL(TIMER2_OVF_vect);
	```

3. **Define tasks**
	```cpp
	uRTOS_PROC_HANDLE(blink12);
	uRTOS_PROC_HANDLE(blink13);

	void blink12() {
		pinMode(12, OUTPUT);

		while (true) {
			digitalWrite(12, !digitalRead(12));
			delay(1000);
		}
	}

	void blink13() {
		pinMode(13, OUTPUT);

		while (true) {
			digitalWrite(13, !digitalRead(13));
			delay(500);
		}
	}
	```

4. **Configure system & tasks**
	```cpp
	const TaskDesc_t tasks[] PROGMEM = {
		{
			.handle = blink12,
			.stackSize = 128
		},
		{
			.handle = blink13,
			.stackSize = 128
		}
	};

	const SysInitInfo_t initInfo PROGMEM = {
		.timerNo = 2,
		.timerPrescaler = uRTOS_TIM_PRESC_64,
		.scheduler = uRTOS_Sched_RoundRobin
	};
	```

5. **Start the kernel**
	```cpp
	void setup() {
		uRTOS_Run(&initInfo, tasks, 2);
	}

	void loop() {}
	```


## 📚 Documentation

Full API documentation can be generated with [Doxygen](https://doxygen.nl/) using the provided `Doxyfile`.
All public interfaces are documented directly in the source headers.


## ❗ Error Handling

uRTOS defines a set of **fatal errors** that occur during initialization or scheduling. If such an error is detected, the kernel invokes a user‑provided callback (if set in `SysInitInfo_t::errorCallback`) and then halts. **Recovery is not possible** because system integrity cannot be guaranteed.

### Error Codes

| Macro                          | Value | Description                                                                 |
|--------------------------------|-------|-----------------------------------------------------------------------------|
| `uRTOS_INSUFFICIENT_MEMORY`    | 1     | The allocated task stacks overlap with `.data` / `.bss` sections.           |
| `uRTOS_SCHEDULER_NOT_DEFINED`  | 2     | The `scheduler` field in `SysInitInfo_t` is `NULL`.                         |
| `uRTOS_ALL_TASKS_SUSPENDED`    | 3     | No runnable task exists (all tasks have the `uRTOS_TFLAGS_SUSPENDED` flag). |

### Usage Example

```cpp
// Blink an LED or log the error code
void myErrorHandler(Errno_t err) {
	pinMode(13, OUTPUT);

	while (true)
	{
		digitalWrite(13, !digitalRead(13));
		delay(200);
	}
}

const SysInitInfo_t initInfo PROGMEM = {
	.timerNo		= 2,
	.timerPrescaler	= uRTOS_TIM_PRESC_64,

	// Defined as &__heap_start, you can use your own value,
	// or add an offset.
	.stacksStart	= uRTOS_STACK_START,
	.scheduler		= uRTOS_Sched_RoundRobin,
	.errorCallback	= myErrorHandler
};
```

## 🤝 Contributing

Contributions are welcome! Feel free to open an **Issue** for bugs or feature requests, or submit a **Pull Request** with improvements.
Ideas for contributions:

- Porting to **ATmega2560** (handling 24‑bit function pointers).
- Support for other AVR chips.
- Implementing `uRTOS_Delay()` (blocking delay with sleep state).
- Synchronization primitives: **mutexes**, **semaphores**, **barriers**.
- More scheduler algorithms (priority‑based, EDF, etc.).


## 📄 License

MIT License. See LICENSE file for details.

Copyright (c) 2026 Elindorn


## ⚠️ Disclaimer
This is an experimental RTOS intended for learning and prototyping.
While it has been tested and proven stable in many scenarios, it comes with no warranty.
Use at your own risk.
