# Elliptic Curve ElGamal Implementation in Python for testing

def mod_pow(base, exp, mod):
    result = 1
    base %= mod
    while exp > 0:
        if exp % 2 == 1:
            result = (result * base) % mod
        base = (base * base) % mod
        exp //= 2
    return result

def mod_inverse(a, m):
    m0 = m
    y = 0
    x = 1
    if m == 1:
        return 0
    while a > 1:
        q = a // m
        t = m
        m = a % m
        a = t
        t = y
        y = x - q * y
        x = t
    if x < 0:
        x += m0
    return x

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __repr__(self):
        return f"({self.x}, {self.y})"

def is_on_curve(P, a, b, p):
    left = mod_pow(P.y, 2, p)
    right = (mod_pow(P.x, 3, p) + a * P.x + b) % p
    return left == right

def point_add(P, Q, a, p):
    if P == Q:
        return point_double(P, a, p)
    if P.x == Q.x:
        # Vertical line, infinity
        return None  # Represent infinity as None
    dx = (Q.x - P.x) % p
    dy = (Q.y - P.y) % p
    lam = (dy * mod_inverse(dx, p)) % p
    x3 = (mod_pow(lam, 2, p) - P.x - Q.x) % p
    y3 = (lam * ((P.x - x3) % p) % p - P.y) % p
    return Point(x3, y3)

def point_double(P, a, p):
    if P.y == 0:
        return None  # Infinity
    lam = ((3 * mod_pow(P.x, 2, p) + a) * mod_inverse(2 * P.y, p)) % p
    x3 = (mod_pow(lam, 2, p) - 2 * P.x) % p
    y3 = (lam * (P.x - x3) % p - P.y) % p
    return Point(x3, y3)

def scalar_mult(k, P, a, p):
    result = None  # Infinity
    temp = P
    while k > 0:
        if k % 2 == 1:
            if result is None:
                result = temp
            else:
                result = point_add(result, temp, a, p)
        temp = point_double(temp, a, p)
        k //= 2
    return result

# Example
p = 97
a = 2
b = 3
G = Point(3, 6)
print("G on curve:", is_on_curve(G, a, b, p))

dA = 7
QA = scalar_mult(dA, G, a, p)
print("QA:", QA)
print("QA on curve:", is_on_curve(QA, a, b, p))

M = Point(10, 20)
print("M on curve:", is_on_curve(M, a, b, p))

k = 3
C1 = scalar_mult(k, G, a, p)
C2_temp = scalar_mult(k, QA, a, p)
C2 = point_add(M, C2_temp, a, p)
print("C1:", C1)
print("C2:", C2)

# Decryption: M = C2 - dA * C1
dA_C1 = scalar_mult(dA, C1, a, p)
# To subtract, add the negative
neg_dA_C1 = Point(dA_C1.x, (-dA_C1.y) % p)
decrypted = point_add(C2, neg_dA_C1, a, p)
print("Decrypted:", decrypted)

print("Success:", decrypted == M)