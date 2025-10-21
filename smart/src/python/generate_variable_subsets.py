import random
import math
import os

from maximal_universal_subset import run_get_mus

if __name__ == "__main__":
   random.seed(42)
   underspecified = sorted(run_get_mus())
   print(f"Underspecified variables size: {len(underspecified)}")

   num_cores = os.cpu_count()

   V = len(underspecified)
   k = round(2.7+1.6*math.log(V,10))
   n = max(int(0.5*V**0.9), num_cores)

   for i in range(int(n)):
       variable_set = random.sample(underspecified, k)
    #    print(f"variable_set_{i}: {variable_set}")
       with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
           f.write('\n'.join(variable_set))