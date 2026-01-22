// Travail Pratique : Chiffrement ElGamal ECC
// Implémentation en C des fonctions de base ECC et du chiffrement ElGamal

// Headers nécessaires pour l'implémentation
#include <stdio.h>   // Pour printf, fprintf, stderr
#include <stdlib.h>  // Pour exit()
#include <stdint.h>  // Pour les types entiers standard
#include <stdbool.h> // Pour le type bool

// Structure représentant un point sur la courbe elliptique
// infinity = true indique le point à l'infini (élément neutre du groupe)
typedef struct {
    long long x;      // Coordonnée x du point
    long long y;      // Coordonnée y du point
    bool infinity;    // Indicateur de point à l'infini
} Point;

// Fonction utilitaire pour le modulo positif
// Assure que le résultat est toujours dans [0, p-1]
static long long mod(long long a, long long p) {
    long long r = a % p;
    if (r < 0) r += p;
    return r;
}

// Algorithme d'Euclide étendu pour calculer le PGCD et les coefficients de Bézout
// Utilisé pour trouver l'inverse modulaire
static long long egcd(long long a, long long b, long long *x, long long *y) {
    if (b == 0) { *x = 1; *y = 0; return a; }
    long long x1, y1;
    long long g = egcd(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return g;
}

// Calcul de l'inverse modulaire utilisant l'algorithme d'Euclide étendu
// Nécessaire pour la division dans les opérations sur les courbes elliptiques
static long long modinv(long long a, long long p) {
    long long x, y;
    long long g = egcd(mod(a, p), p, &x, &y);
    if (g != 1) { fprintf(stderr, "No inverse for %lld mod %lld\n", a, p); exit(1); }
    return mod(x, p);
}

// TP Question 1: Vérification qu'un point appartient à la courbe : bool Is_on_curve(Point P, int a, int b, int p);
// Vérifie si le point P satisfait l'équation de la courbe elliptique y² = x³ + ax + b mod p
// Le point à l'infini est considéré comme appartenant à la courbe par définition
bool is_on_curve(Point P, long long a, long long b, long long p) {
    if (P.infinity) return true;  // Point à l'infini toujours valide
    long long lhs = mod(P.y * P.y, p);  // Côté gauche: y² mod p
    long long rhs = mod(P.x * P.x % p * P.x + a * P.x + b, p);  // Côté droit: x³ + ax + b mod p
    return lhs == rhs;  // Vérification de l'égalité
}

// TP Question 2: Addition de deux points : Point Point_add(Point P, Point Q, int a, int p);
// Implémente l'addition de points sur une courbe elliptique
// Gère tous les cas spéciaux: infini, points opposés, points identiques (doublement)
Point point_add(Point P, Point Q, long long a, long long p) {
    if (P.infinity) return Q;  // P + ∞ = P
    if (Q.infinity) return P;  // ∞ + Q = Q
    if (P.x == Q.x) {
        if (mod(P.y + Q.y, p) == 0) { // P == -Q (points opposés)
            Point R = {0, 0, true};  // Résultat = ∞
            return R;
        } else {
            // Cas du doublement: P == Q
            // Formule: λ = (3x² + a) / (2y) mod p
            long long num = mod(3 * P.x % p * P.x + a, p);  // 3x² + a
            long long den = mod(2 * P.y, p);                // 2y
            long long inv = modinv(den, p);                 // Inverse de 2y
            long long lambda = mod(num * inv, p);           // λ
            long long x3 = mod(lambda * lambda - 2 * P.x, p);  // x₃ = λ² - 2x
            long long y3 = mod(lambda * (P.x - x3) - P.y, p);  // y₃ = λ(x - x₃) - y
            Point R = {x3, y3, false};
            return R;
        }
    } else {
        // Cas général: P ≠ Q
        // Formule: λ = (y₂ - y₁) / (x₂ - x₁) mod p
        long long num = mod(Q.y - P.y, p);    // y₂ - y₁
        long long den = mod(Q.x - P.x, p);    // x₂ - x₁
        long long inv = modinv(den, p);       // Inverse de (x₂ - x₁)
        long long lambda = mod(num * inv, p); // λ
        long long x3 = mod(lambda * lambda - P.x - Q.x, p);  // x₃ = λ² - x₁ - x₂
        long long y3 = mod(lambda * (P.x - x3) - P.y, p);    // y₃ = λ(x₁ - x₃) - y₁
        Point R = {x3, y3, false};
        return R;
    }
}

// TP Question 3: Doublement d'un point : Point Point_double(Point P, int a, int p);
// Doublement d'un point = addition du point avec lui-même
// Utilise la fonction point_add pour éviter la duplication de code
Point point_double(Point P, long long a, long long p) {
    return point_add(P, P, a, p);
}

// TP Question 4: Multiplication scalaire : Point Scalar_mult(int k, Point P, int a, int p);
// Calcule k * P en utilisant l'algorithme binaire (double-and-add)
// Plus efficace que l'addition répétée simple
Point scalar_mult(long long k, Point P, long long a, long long p) {
    Point R = {0, 0, true}; // Initialisation avec le point à l'infini (élément neutre)
    Point Q = P;            // Copie du point de base
    while (k > 0) {
        if (k & 1) R = point_add(R, Q, a, p);  // Si bit de poids faible = 1, ajouter Q à R
        Q = point_double(Q, a, p);             // Doubler Q pour le prochain bit
        k >>= 1;                               // Décaler k vers la droite
    }
    return R;
}

// TP Chiffrement ElGamal ECC: Choisir un entier aléatoire k, Calculer C1 = k*G, C2 = M + k*QA
// Fonction de chiffrement ElGamal sur courbe elliptique
// Prend le message M (point), clé publique QA, générateur G, et k aléatoire
// Retourne le ciphertext (C1, C2)
void encrypt(Point M, long long k, Point G, Point QA, long long a, long long p, Point *C1, Point *C2) {
    *C1 = scalar_mult(k, G, a, p);     // C1 = k * G (partage publique)
    Point kQA = scalar_mult(k, QA, a, p);  // k * QA (masque)
    *C2 = point_add(M, kQA, a, p);     // C2 = M + k*QA (message masqué)
}

// Fonction utilitaire pour calculer l'opposé d'un point
// Nécessaire pour la soustraction dans le déchiffrement
Point negate(Point P, long long p) {
    if (P.infinity) return P;  // -∞ = ∞
    Point R = {P.x, mod(-P.y, p), false};  // (x, -y mod p)
    return R;
}

// TP Déchiffrement: Calculer M = C2 - dA*C1
// Fonction de déchiffrement ElGamal sur courbe elliptique
// Utilise la clé privée dA pour récupérer le message original
Point decrypt(Point C1, Point C2, long long dA, long long a, long long p) {
    Point dC1 = scalar_mult(dA, C1, a, p);      // dA * C1
    Point minus_dC1 = negate(dC1, p);           // -(dA * C1)
    return point_add(C2, minus_dC1, a, p);      // C2 + (-dA*C1) = M
}

// Fonction principale: démonstration du chiffrement ElGamal ECC
// Utilise l'exemple pédagogique du TP avec p=97, a=2, b=3, G=(3,6), dA=7, k=3
int main(void) {
    // Paramètres de la courbe elliptique (exemple pédagogique)
    long long p = 97, a = 2, b = 3;  // Corps premier p=97, courbe y² = x³ + 2x + 3 mod 97

    // Générateur de la courbe
    Point G = {3, 6, false};  // Point G = (3, 6)

    // Clé privée d'Alice
    long long dA = 7;

    // Calcul de la clé publique QA = dA * G
    Point QA = scalar_mult(dA, G, a, p);
    printf("QA = (%lld,%lld)%s\n", QA.x, QA.y, QA.infinity ? " INF" : "");

    // Message à chiffrer (doit être un point sur la courbe)
    Point M = {10, 20, false};

    // Vérification que les points sont sur la courbe
    if (!is_on_curve(G, a, b, p) || !is_on_curve(M, a, b, p)) {
        printf("Point not on curve\n");
        return 0;
    }

    // Valeur k aléatoire choisie par Bob (ici k=3 pour l'exemple)
    long long k = 3;

    // Chiffrement
    Point C1, C2;
    encrypt(M, k, G, QA, a, p, &C1, &C2);
    printf("C1 = (%lld,%lld)\n", C1.x, C1.y);
    printf("C2 = (%lld,%lld)\n", C2.x, C2.y);

    // Déchiffrement
    Point Mdec = decrypt(C1, C2, dA, a, p);
    printf("Mdec = (%lld,%lld)\n", Mdec.x, Mdec.y);

    // Vérification du déchiffrement
    if (Mdec.infinity == false && Mdec.x == M.x && Mdec.y == M.y) {
        printf("OK: Mdec == M\n");
    } else {
        printf("ERROR: Mdec != M\n");
    }
    return 0;
}