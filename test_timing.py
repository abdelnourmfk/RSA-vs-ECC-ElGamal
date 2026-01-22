import time

# Test timing precision
print("Testing timing precision...")

# Old method (time.time())
start = time.time()
for i in range(1000):
    pass  # Simple operation
end = time.time()
print(f"time.time(): {end - start:.8f}s for 1000 iterations")

# New method (time.perf_counter())
start = time.perf_counter()
for i in range(1000):
    pass  # Simple operation
end = time.perf_counter()
print(f"time.perf_counter(): {end - start:.8f}s for 1000 iterations")

# Test with actual crypto operations
from crypto_gui import RSA, ECC_ElGamal, Point

rsa = RSA()
ecc = ECC_ElGamal()

print("\nTesting RSA timing:")
start = time.perf_counter()
for _ in range(100):
    c = rsa.encrypt(42)
    d = rsa.decrypt(c)
end = time.perf_counter()
print(f"RSA 100 operations: {(end - start):.8f}s (avg: {(end - start)/100:.8f}s per op)")

print("\nTesting ECC timing:")
M = Point(10, 20)
k = 3
start = time.perf_counter()
for _ in range(100):
    C1, C2 = ecc.encrypt(M, k)
    decrypted = ecc.decrypt(C1, C2)
end = time.perf_counter()
print(f"ECC 100 operations: {(end - start):.8f}s (avg: {(end - start)/100:.8f}s per op)")

print("\nTiming test completed!")