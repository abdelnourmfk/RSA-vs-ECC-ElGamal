import tkinter as tk
import time
import hashlib

# RSA Implementation
def gcd(a, b):
    while b != 0:
        a, b = b, a % b
    return a

def mod_pow(base, exp, mod):
    return pow(base, exp, mod)

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

class RSA:
    def __init__(self):
        self.p = 61
        self.q = 53
        self.n = self.p * self.q
        self.phi = (self.p - 1) * (self.q - 1)
        self.e = 17
        self.d = mod_inverse(self.e, self.phi)

    def encrypt(self, m):
        return mod_pow(m, self.e, self.n)

    def decrypt(self, c):
        return mod_pow(c, self.d, self.n)

    def encrypt_text(self, text):
        encrypted = []
        for char in text:
            m = ord(char)
            if m >= self.n:
                raise ValueError("Character too large for RSA key")
            c = self.encrypt(m)
            encrypted.append(str(c))
        return ','.join(encrypted)

    def decrypt_text(self, encrypted_str):
        decrypted = []
        for c_str in encrypted_str.split(','):
            c = int(c_str)
            m = self.decrypt(c)
            decrypted.append(chr(m))
        return ''.join(decrypted)

# ECC Implementation
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __repr__(self):
        return f"({self.x}, {self.y})"

def mod_inverse_ecc(a, m):
    return mod_inverse(a, m)

def point_add(P, Q, a, p):
    if P == Q:
        return point_double(P, a, p)
    if P.x == Q.x:
        return None  # Infinity
    dx = (Q.x - P.x) % p
    dy = (Q.y - P.y) % p
    lam = (dy * mod_inverse_ecc(dx, p)) % p
    x3 = (mod_pow(lam, 2, p) - P.x - Q.x) % p
    y3 = (lam * ((P.x - x3) % p) % p - P.y) % p
    return Point(x3, y3)

def point_double(P, a, p):
    if P.y == 0:
        return None
    lam = ((3 * mod_pow(P.x, 2, p) + a) * mod_inverse_ecc(2 * P.y, p)) % p
    x3 = (mod_pow(lam, 2, p) - 2 * P.x) % p
    y3 = (lam * ((P.x - x3) % p) % p - P.y) % p
    return Point(x3, y3)

def scalar_mult(k, P, a, p):
    result = None
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

class ECC_ElGamal:
    def __init__(self):
        self.p = 97
        self.a = 2
        self.b = 3
        self.G = Point(3, 6)
        self.dA = 7
        self.QA = scalar_mult(self.dA, self.G, self.a, self.p)

    def encrypt(self, M, k):
        C1 = scalar_mult(k, self.G, self.a, self.p)
        kQA = scalar_mult(k, self.QA, self.a, self.p)
        C2 = point_add(M, kQA, self.a, self.p)
        return C1, C2

    def decrypt(self, C1, C2):
        dC1 = scalar_mult(self.dA, C1, self.a, self.p)
        # Subtract: add negative
        neg_dC1 = Point(dC1.x, (-dC1.y) % self.p)
        return point_add(C2, neg_dC1, self.a, self.p)

    def encrypt_text(self, text, k):
        # Encode text to point: use hash for x, compute y
        h = int(hashlib.sha256(text.encode('utf-8')).hexdigest(), 16)
        x = h % self.p
        # Compute y^2 = x^3 + a x + b mod p
        y2 = (mod_pow(x, 3, self.p) + self.a * x + self.b) % self.p
        # Find y such that y^2 = y2 mod p (simple case, assume exists)
        y = 0
        for i in range(self.p):
            if (i * i) % self.p == y2:
                y = i
                break
        M = Point(x, y)
        return self.encrypt(M, k)

    def decrypt_text(self, C1, C2):
        M = self.decrypt(C1, C2)
        # Cannot reverse hash, so return the point
        return M

# GUI
class CryptoComparator:
    def __init__(self, root):
        self.root = root
        self.root.title("RSA vs ECC ElGamal Comparator")
        self.root.configure(bg='#f0f0f0')
        self.rsa = RSA()
        self.ecc = ECC_ElGamal()

        # RSA Section
        tk.Label(root, text="RSA Encryption", font=("Arial", 14, 'bold'), bg='#f0f0f0', fg='blue').grid(row=0, column=0, columnspan=2)
        tk.Label(root, text="Message (integer):", bg='#f0f0f0').grid(row=1, column=0)
        self.rsa_message = tk.Entry(root)
        self.rsa_message.grid(row=1, column=1)
        self.rsa_message.insert(0, "42")

        tk.Button(root, text="RSA Encrypt/Decrypt", command=self.rsa_test, bg='lightblue').grid(row=2, column=0, columnspan=2)

        self.rsa_result = tk.Label(root, text="", bg='#f0f0f0', fg='green')
        self.rsa_result.grid(row=3, column=0, columnspan=2)

        # ECC Section
        tk.Label(root, text="ECC ElGamal Encryption", font=("Arial", 14, 'bold'), bg='#f0f0f0', fg='blue').grid(row=4, column=0, columnspan=2)
        tk.Label(root, text="Message Point (x,y):", bg='#f0f0f0').grid(row=5, column=0)
        self.ecc_message = tk.Entry(root)
        self.ecc_message.grid(row=5, column=1)
        self.ecc_message.insert(0, "10,20")

        tk.Label(root, text="Random k:", bg='#f0f0f0').grid(row=6, column=0)
        self.k_value = tk.Entry(root)
        self.k_value.grid(row=6, column=1)
        self.k_value.insert(0, "3")

        tk.Button(root, text="ECC Encrypt/Decrypt", command=self.ecc_test, bg='lightgreen').grid(row=7, column=0, columnspan=2)

        self.ecc_result = tk.Label(root, text="", bg='#f0f0f0', fg='green')
        self.ecc_result.grid(row=8, column=0, columnspan=2)

        # ASCII Section
        tk.Label(root, text="ASCII Encryption (RSA)", font=("Arial", 14, 'bold'), bg='#f0f0f0', fg='purple').grid(row=9, column=0, columnspan=2)
        tk.Label(root, text="Message (text):", bg='#f0f0f0').grid(row=10, column=0)
        self.ascii_message = tk.Entry(root)
        self.ascii_message.grid(row=10, column=1)
        self.ascii_message.insert(0, "Hello World")

        tk.Button(root, text="ASCII Encrypt/Decrypt", command=self.ascii_test, bg='lightyellow').grid(row=11, column=0, columnspan=2)

        self.ascii_result = tk.Label(root, text="", bg='#f0f0f0', fg='green')
        self.ascii_result.grid(row=12, column=0, columnspan=2)

        # Comparison
        tk.Button(root, text="Compare Performance", command=self.compare_performance, bg='orange').grid(row=13, column=0, columnspan=2)
        self.comp_result = tk.Label(root, text="", bg='#f0f0f0', fg='black')
        self.comp_result.grid(row=14, column=0, columnspan=2)

    def animate_label(self, label, color1, color2, count=5):
        if count > 0:
            current_color = label.cget('fg')
            new_color = color2 if current_color == color1 else color1
            label.config(fg=new_color)
            self.root.after(200, self.animate_label, label, color1, color2, count-1)

    def rsa_test(self):
        try:
            m = int(self.rsa_message.get())
            # Run multiple iterations for accurate timing
            iterations = 1000
            start = time.perf_counter()
            for _ in range(iterations):
                c = self.rsa.encrypt(m)
                d = self.rsa.decrypt(c)
            end = time.perf_counter()
            avg_time = (end - start) / iterations
            self.rsa_result.config(text=f"Encrypted: {c}\nDecrypted: {d}\nAvg Time: {avg_time:.8f}s per operation", fg='green')
            self.animate_label(self.rsa_result, 'green', 'blue')
        except ValueError:
            self.rsa_result.config(text="Invalid RSA message", fg='red')
            self.animate_label(self.rsa_result, 'red', 'orange')

    def ecc_test(self):
        try:
            coords = self.ecc_message.get().split(',')
            M = Point(int(coords[0]), int(coords[1]))
            k = int(self.k_value.get())
            # Run multiple iterations for accurate timing
            iterations = 1000
            start = time.perf_counter()
            for _ in range(iterations):
                C1, C2 = self.ecc.encrypt(M, k)
                decrypted = self.ecc.decrypt(C1, C2)
            end = time.perf_counter()
            avg_time = (end - start) / iterations
            self.ecc_result.config(text=f"C1: {C1}\nC2: {C2}\nDecrypted: {decrypted}\nAvg Time: {avg_time:.8f}s per operation", fg='green')
            self.animate_label(self.ecc_result, 'green', 'blue')
        except Exception:
            self.ecc_result.config(text="Invalid ECC message or k", fg='red')
            self.animate_label(self.ecc_result, 'red', 'orange')

    def ascii_test(self):
        try:
            text = self.ascii_message.get()
            c = self.rsa.encrypt_text(text)
            d = self.rsa.decrypt_text(c)
            # Show symbols with ASCII values
            symbol_vals = [f"{char}({ord(char)})" for char in text]
            self.ascii_result.config(text=f"Original: {text}\nSymbols: {' '.join(symbol_vals)}\nEncrypted: {c}\nDecrypted: {d}", fg='green')
            self.animate_label(self.ascii_result, 'green', 'purple')
        except Exception as e:
            self.ascii_result.config(text=f"Error: {str(e)}", fg='red')
            self.animate_label(self.ascii_result, 'red', 'orange')

    def compare_performance(self):
        # Performance comparison with higher precision timing
        rsa_times = []
        ecc_times = []

        iterations = 100  # More iterations for better accuracy

        for _ in range(10):
            # RSA
            m = 42
            start = time.perf_counter()
            for _ in range(iterations):
                c = self.rsa.encrypt(m)
                self.rsa.decrypt(c)
            end = time.perf_counter()
            rsa_times.append((end - start) / iterations)

            # ECC
            M = Point(10, 20)
            k = 3
            start = time.perf_counter()
            for _ in range(iterations):
                C1, C2 = self.ecc.encrypt(M, k)
                self.ecc.decrypt(C1, C2)
            end = time.perf_counter()
            ecc_times.append((end - start) / iterations)

        avg_rsa = sum(rsa_times) / len(rsa_times)
        avg_ecc = sum(ecc_times) / len(ecc_times)

        ratio = avg_ecc / avg_rsa if avg_rsa != 0 else 0
        self.comp_result.config(text=f"Average RSA time: {avg_rsa:.8f}s per operation\n"
                                   f"Average ECC time: {avg_ecc:.8f}s per operation\n"
                                   f"Key sizes: RSA n={self.rsa.n} ({len(str(self.rsa.n))} digits)\n"
                                   f"ECC key size: p={self.ecc.p} ({len(str(self.ecc.p))} digits)\n"
                                   f"RSA is {ratio:.1f}x faster than ECC", fg='black')
        self.animate_label(self.comp_result, 'black', 'orange')

if __name__ == "__main__":
    root = tk.Tk()
    app = CryptoComparator(root)
    root.mainloop()