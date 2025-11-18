# CSE 379 / 479 Library
This project is a statically linked library made for the TM4C123GH6PM MCU that handles common functions and intialization for several different modules planned.  The goal is to provide an easy way to both provide a reference implementation for the different projects in UB CSE 379 and 479 for both initialization and implementation (usingthe different modules).  This way you will be able to more easily determine what aspect of your project is incorrect, being how you're using it or how you're initializes it.  Each module will be contained its in own header file.

Additionally, there is additional debugging functions to assist with determining what has gone wrong with your code.  Currently, there are functions that you can add to the `tm4c123gh6pm_startup_ccs.c` in the `FaultISR` or the `IntDefaultHandler` functions that will print the contents of the stack to UART and tell you what instruction caused the hard fault.

This project is on [GitLab](https://gitlab.com/Nfox18212/cse479-library).  This project will be mirrored on GitHub for convenience.

## Usage:
To use this project, clone the repository into your `workspace_v12` directory.  In Code Composer Studio, right click the project and open the project's properties.  Ensure that the build target is a Statically Linked Library.  Then, open the project you want to build with this library.  Go into project properties again.  First, click on CCS Build and go to dependencies.  Add the `cse479-library` project as a dependency.  Then click on Arm Linker -> File Search Path and add `cse479-library.lib` as a library file.

If there are any questions, refer to the documentation pdf (or .odt for now) before asking questions or opening an issue.  I will ensure that the documentation is sufficiently thorough.  If you have concerns about the documenation or believe something is not properly documented, please see [Contributing].

### Modules:

To use a specific module, add `#include <modulename.h>` to your C file, or the following snippet to your assembly file.  More information can be found in the ARM Assembly Language Tools User's Guide in Chapter 13, Page 319.
```arm
.cdecls C,NOLIST  
%{ 
    #include <module1name.h>
    #include <module2name.h>
%}
```


### Debugging
Add `#include <debug.h>` to your `tm4c123gh6pm_startup_ccs.c` file and add `handle_fault();` to the `FaultISR` function.  


## TODO (Sorted in order of priority)
- Implement generic purpose timer initialization for the OneShot and Periodic Modes
    - initialize interrupts
    - intialize both 32 and 64 bit timers, as well as 16/32 bit half width timers
- Implement improved UART/Serial I/O with support for backspace characters
    - Look into using printf, snprintf, fprintf on the tiva.  UART address as a custom stream?
- Implement GPIO initialization and reading/writing to different GPIO pins (includes interrupts)
- Implement basic PWM initialization and changing frequency and pulse width
- Implement basic ADC initialziation
- Implement basic UDMA initialization

## Contributing
If you want to contribute, please make a fork of this project and open a pull request.  Please open issues and pull requests on GitLab rather than GitHub.  I will respond to issues and PRs sooner if they are on GitLab.

Please also open issues under the correct labels.  I will not respond to issues with improper labelling.

### Formatting
Formatting is done using LLVM style C formatting.  Tabs are two spaces.  Additionally, when using the `uptradd` macro, please ensure that all offsets are in hexadecimal and are five characters in length.  For example `0x010` is correct and `16` and `0x10` are not correct.