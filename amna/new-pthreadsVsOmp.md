| Feature           | OpenMP                | pthreads                   |
| ----------------- | --------------------- | -------------------------- |
| Thread creation   | Automatic             | Manual                     |
| Work splitting    | Automatic (`omp for`) | Manual slicing             |
| Barriers          | Built-in              | Custom implemented         |
| Histogram merging | critical section      | main thread merge          |
| Thread lifetime   | region-based          | persistent                 |
| Complexity        | Simple                | High                       |
| Debug difficulty  | Low                   | Very high                  |
| Best for          | quick parallelism     | fine control + performance |
