#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Structure for RSA keys
typedef struct {
    long long n;
    long long e;
    long long d;
} RSA_Key;

// Function to compute gcd
long long gcd(long long a, long long b) {
    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// Function to compute modular exponentiation
long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

// Function to compute modular inverse using extended Euclidean algorithm
long long mod_inverse(long long a, long long m) {
    long long m0 = m, t, q;
    long long x0 = 0, x1 = 1;
    if (m == 1) return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;
    return x1;
}

// Function to check if a number is prime (simple trial division)
int is_prime(long long n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

// Function to generate RSA key pair
RSA_Key generate_rsa_key() {
    RSA_Key key;
    long long p, q;
    // Choose small primes for demonstration
    p = 61; // prime
    q = 53; // prime
    key.n = p * q;
    long long phi = (p - 1) * (q - 1);
    key.e = 17; // common choice, coprime with phi
    key.d = mod_inverse(key.e, phi);
    return key;
}

// Function to encrypt a message with RSA
long long rsa_encrypt(long long m, RSA_Key key) {
    return mod_pow(m, key.e, key.n);
}

// Function to decrypt a message with RSA
long long rsa_decrypt(long long c, RSA_Key key) {
    return mod_pow(c, key.d, key.n);
}

int main() {
    // Generate RSA key
    RSA_Key rsa_key = generate_rsa_key();
    printf("RSA Key:\n");
    printf("n = %lld\n", rsa_key.n);
    printf("e = %lld\n", rsa_key.e);
    printf("d = %lld\n", rsa_key.d);

    // Example message
    long long message = 42;
    printf("\nOriginal message: %lld\n", message);

    // Encrypt
    long long ciphertext = rsa_encrypt(message, rsa_key);
    printf("Encrypted: %lld\n", ciphertext);

    // Decrypt
    long long decrypted = rsa_decrypt(ciphertext, rsa_key);
    printf("Decrypted: %lld\n", decrypted);

    if (decrypted == message) {
        printf("RSA encryption/decryption successful!\n");
    } else {
        printf("RSA encryption/decryption failed!\n");
    }

    return 0;
}