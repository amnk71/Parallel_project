| Cause                      | Simple Explanation                                 |
| -------------------------- | -------------------------------------------------- |
| **Small input**            | Threads take longer to start than the work itself. |
| **Thread creation/join**   | Setup and cleanup time kills performance.          |
| **Barriers**               | Threads spend more time waiting than sorting.      |
| **macOS scheduling**       | OS delays thread wakeups to save power.            |
| **Cache/memory**           | Threads fight over cache and memory bandwidth.     |
| **Malloc calls**           | Extra memory setup each time.                      |
| **Amdahl’s Law**           | Some parts can’t be parallel, limiting speedup.    |
| **Compiler optimizations** | Sequential code is simpler and faster to optimize. |
| **Tiny timers**            | Microsecond measurements are noisy and inaccurate. |


what ill do:
-run it on windows and check the new times




- input size can we increase
  
