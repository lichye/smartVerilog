import random
import math
import json
import os
import sys

from minimal_satisfiable_assignment import get_mus

if __name__ == "__main__":
   random.seed(42)

   Config = sys.argv[1]

   with open(Config) as f:
      config = json.load(f)
      Blockified_settings = config.get("Blockified_settings")

   if(Blockified_settings["MSA"]== True and Blockified_settings["Random"]== False):
      underspecified,model = get_mus("runtime/variables.txt", "runtime/SygusResult.sl", timeout=300)
      print(f"Underspecified variables size: {len(underspecified)}")
      underspecified = list(underspecified)

      num_cores = os.cpu_count()

      V = len(underspecified)
      k = round(2.7+1.6*math.log(V,10))
      n = max(int(0.5*V**0.9), num_cores)

      for i in range(int(n)):
         variable_set = random.sample(underspecified, k)
      #    print(f"variable_set_{i}: {variable_set}")
         with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
            f.write('\n'.join(variable_set))
   
   if(Blockified_settings["Random"]== True and Blockified_settings["MSA"]== False):
      with open("runtime/variables.txt") as f:
         variables = f.read().splitlines()
      variables = list(set(variables))
      V = len(variables)
      k = round(2.7+1.6*math.log(V,10))
      n = max(int(0.5*V**0.9), os.cpu_count())
      print("Underspecified variable size is "+str(V))
      print("Subset size k is "+str(k))
      print("Number of threads n is "+str(n))

      underspecified = list(variables)

      for i in range(int(n)):
         variable_set = random.sample(underspecified, k)
      #    print(f"variable_set_{i}: {variable_set}")
         with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
            f.write('\n'.join(variable_set))

   if(Blockified_settings["Random"]== True and Blockified_settings["MSA"]== True):
      # Mix 50% MSA and 50% Random
      
      # Do 50% MSA
      underspecified,model = get_mus("runtime/variables.txt", "runtime/SygusResult.sl", timeout=300)
      print(f"Underspecified variables size from MSA: {len(underspecified)}")
      underspecified = list(underspecified)
      V = len(underspecified)
      k = round(2.7+1.6*math.log(V,10))
      n = max(int(0.5*V**0.9), os.cpu_count())
      msa_n = int(n/2)
      for i in range(int(msa_n)):
         variable_set = random.sample(underspecified, k)
         with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
            f.write('\n'.join(variable_set))
      
      # Do 50% Random
      with open("runtime/variables.txt") as f:
         variables = f.read().splitlines()
      variables = list(set(variables))
      V = len(variables)
      k = round(2.7+1.6*math.log(V,10))
      random_n = n - msa_n
      print("Underspecified variable size from Random is "+str(V))
      print("Subset size k is "+str(k))
      print("Number of threads n is "+str(random_n))
      underspecified = list(variables)
      for i in range(int(random_n)):
         variable_set = random.sample(underspecified, k)
         with open(f"runtime/variables/thread_{i+msa_n}.txt", 'w') as f:
            f.write('\n'.join(variable_set))