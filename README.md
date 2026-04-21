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
	```c
	#include <urtos/urtos.h>
	```

2. **Install the tick ISR**
	```c
	uRTOS_INSTALL(TIMER2_OVF_vect);
	```

3. **Define tasks**
	```c
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
	```c
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
	```c
	void setup() {
		uRTOS_Run(&initInfo, tasks, 2);
	}

	void loop() {}
	```


## 📚 Documentation

Full API documentation can be generated with [Doxygen](https://doxygen.nl/) using the provided `Doxyfile`.
All public interfaces are documented directly in the source headers.


## 🤝 Contributing

Contributions are welcome! Feel free to open an **Issue** for bugs or feature requests, or submit a **Pull Request** with improvements.
Ideas for contributions:

- Porting to **ATmega2560** (handling 24‑bit function pointers and larger RAM).
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
