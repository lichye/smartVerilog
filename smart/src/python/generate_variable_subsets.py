import random

from maximal_universal_subset import run_get_mus


# bench is a str representing the benchmark name (without text extention)
def generate_variable_subsets(n, k):
    random.seed(42)
    underspecified = sorted(run_get_mus())
    print(f"Underspecified variables: {underspecified}")

    for i in range(n):
       variable_set = random.sample(underspecified, k)
    #    print(f"variable_set_{i}: {variable_set}")
       with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
           f.write('\n'.join(variable_set))
        

# print("Generate "+str(5)+" variable subsets with "+str(4)+" variables each.")
generate_variable_subsets(5, 4)