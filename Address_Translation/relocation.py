import random

def relocation_sim(seed, n, base, bound):
    random.seed(seed)

    print(f"Base = {base}")
    print(f"Bound = {bound}")
    print("-" * 30)

    for i in range(n):
        virtual = random.randint(0, bound * 2)  # 일부는 일부러 범위 초과

        if virtual < bound:
            physical = virtual + base
            print(f"VA: {virtual} → PA: {physical}")
        else:
            print(f"VA: {virtual} → Fault (Out of Bound)")

relocation_sim(seed=1, n=5, base=16000, bound=4000)