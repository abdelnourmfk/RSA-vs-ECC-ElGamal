 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * TP : CHIFFREMENT ELGAMAL ECC - VERSION INTERACTIVE
 * Permet de tester toutes les fonctionnalites mentionnees dans le PDF.
 */

typedef struct {
    int x;
    int y;
    bool is_infinity;
} Point;

const Point POINT_AT_INFINITY = {0, 0, true};

// --- Fonctions de base ---

int mod(long long a, int m) {
    long long result = a % m;
    if (result < 0) result += m;
    return (int)result;
}

int mod_inverse(int a, int m) {
    int m0 = m, y = 0, x = 1;
    if (m == 1) return 0;
    a = mod(a, m);
    while (a > 1) {
        if (m == 0) break;
        int q = a / m;
        int t = m;
        m = a % m; a = t;
        t = y;
        y = x - q * y; x = t;
    }
    if (x < 0) x += m0;
    return x;
}

void print_point(Point P) {
    if (P.is_infinity) printf("O (Infini)");
    else printf("(%d, %d)", P.x, P.y);
}

// --- Fonctions ECC demandees ---

// 1. Verification qu'un point appartient a la courbe
bool Is_on_curve(Point P, int a, int b, int p) {
    if (P.is_infinity) return true;
    long long y2 = (long long)P.y * P.y;
    long long rhs = (long long)P.x * P.x * P.x + (long long)a * P.x + b;
    return mod(y2, p) == mod(rhs, p);
}

// 3. Doublement d'un point
Point Point_double(Point P, int a, int p) {
    if (P.is_infinity || P.y == 0) return POINT_AT_INFINITY;
    int num = mod(3LL * P.x * P.x + a, p);
    int den = mod(2LL * P.y, p);
    int inv_den = mod_inverse(den, p);
    int lambda = mod((long long)num * inv_den, p);
    int x3 = mod((long long)lambda * lambda - 2LL * P.x, p);
    int y3 = mod((long long)lambda * (P.x - x3) - P.y, p);
    Point R = {x3, y3, false};
    return R;
}

// 2. Addition de deux points
Point Point_add(Point P, Point Q, int a, int p) {
    if (P.is_infinity) return Q;
    if (Q.is_infinity) return P;
    if (P.x == Q.x) {
        if (mod(P.y + Q.y, p) == 0) return POINT_AT_INFINITY;
        else return Point_double(P, a, p);
    }
    int num = mod(Q.y - P.y, p);
    int den = mod(Q.x - P.x, p);
    int inv_den = mod_inverse(den, p);
    int lambda = mod((long long)num * inv_den, p);
    int x3 = mod((long long)lambda * lambda - P.x - Q.x, p);
    int y3 = mod((long long)lambda * (P.x - x3) - P.y, p);
    Point R = {x3, y3, false};
    return R;
}

// 4. Multiplication scalaire
Point Scalar_mult(int k, Point P, int a, int p) {
    Point R = POINT_AT_INFINITY;
    Point Q = P;
    int k_abs = (k < 0) ? -k : k;
    while (k_abs > 0) {
        if (k_abs & 1) R = Point_add(R, Q, a, p);
        Q = Point_double(Q, a, p);
        k_abs >>= 1;
    }
    if (k < 0 && !R.is_infinity) R.y = mod(-R.y, p);
    return R;
}

// --- Menu et Interaction ---

void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    int p = 97, a = 2, b = 3;
    int choix;
    
    printf("=== TESTEUR INTERACTIF ELGAMAL ECC ===\n");
    
    do {
        printf("\n--- Menu Principal ---\n");
        printf("1. Configurer la courbe (p, a, b) [Actuel: p=%d, a=%d, b=%d]\n", p, a, b);
        printf("2. Verifier si un point est sur la courbe\n");
        printf("3. Addition de deux points\n");
        printf("4. Doublement d'un point\n");
        printf("5. Multiplication scalaire (k * P)\n");
        printf("6. Chiffrement ElGamal complet (Exemple ou Manuel)\n");
        printf("0. Quitter\n");
        printf("Choix : ");
        if (scanf("%d", &choix) != 1) { vider_buffer(); continue; }

        switch (choix) {
            case 1:
                printf("Entrez p (premier), a et b : ");
                scanf("%d %d %d", &p, &a, &b);
                long long disc = 4LL*a*a*a + 27LL*b*b;
                if (mod(disc, p) == 0) printf("Attention : Courbe singuliere !\n");
                else printf("Courbe valide.\n");
                break;

            case 2: {
                Point pt;
                printf("Entrez x et y du point : ");
                if (scanf("%d %d", &pt.x, &pt.y) != 2) { vider_buffer(); break; }
                vider_buffer();
                pt.is_infinity = false;
                printf("\nVerification du point "); print_point(pt); printf(" :\n");
                long long y2 = (long long)pt.y * pt.y;
                long long rhs = (long long)pt.x * pt.x * pt.x + (long long)a * pt.x + b;
                printf("y^2 = %lld (mod %d) = %d\n", y2, p, mod(y2, p));
                printf("x^3 + ax + b = %lld (mod %d) = %d\n", rhs, p, mod(rhs, p));
                if (Is_on_curve(pt, a, b, p)) {
                    printf("--> Le point APPARTIENT a la courbe !\n");
                } else {
                    printf("--> Le point N'APPARTIENT PAS a la courbe !\n");
                }
                break;
            }

            case 3: {
                Point p1, p2, res;
                printf("Point 1 (x y) : "); scanf("%d %d", &p1.x, &p1.y); p1.is_infinity = false;
                printf("Point 2 (x y) : "); scanf("%d %d", &p2.x, &p2.y); p2.is_infinity = false;
                res = Point_add(p1, p2, a, p);
                printf("Resultat : "); print_point(res); printf("\n");
                break;
            }

            case 4: {
                Point p1, res;
                printf("Point (x y) : "); scanf("%d %d", &p1.x, &p1.y); p1.is_infinity = false;
                res = Point_double(p1, a, p);
                printf("Resultat : "); print_point(res); printf("\n");
                break;
            }

            case 5: {
                Point p1, res;
                int k;
                printf("Point (x y) : "); scanf("%d %d", &p1.x, &p1.y); p1.is_infinity = false;
                printf("Scalaire k : "); scanf("%d", &k);
                res = Scalar_mult(k, p1, a, p);
                printf("Resultat %d * P : ", k); print_point(res); printf("\n");
                break;
            }

            case 6: {
                Point P, M, QA, C1, C2, M_dec;
                int dA, k;
                printf("\n--- Chiffrement ElGamal ---\n");
                printf("Point Generateur P (x y) : "); scanf("%d %d", &P.x, &P.y); P.is_infinity = false;
                printf("Cle privee Alice dA : "); scanf("%d", &dA);
                QA = Scalar_mult(dA, P, a, p);
                printf("Cle publique Alice QA : "); print_point(QA); printf("\n");
                
                printf("Message M (doit etre un point x y) : "); scanf("%d %d", &M.x, &M.y); M.is_infinity = false;
                printf("Aleatoire Bob k : "); scanf("%d", &k);
                
                // Chiffrement
                C1 = Scalar_mult(k, P, a, p);
                Point kQA = Scalar_mult(k, QA, a, p);
                C2 = Point_add(M, kQA, a, p);
                printf("Chiffre : C1="); print_point(C1); printf(", C2="); print_point(C2); printf("\n");
                
                // Dechiffrement
                Point dA_C1 = Scalar_mult(dA, C1, a, p);
                Point neg_dA_C1 = dA_C1; neg_dA_C1.y = mod(-neg_dA_C1.y, p);
                M_dec = Point_add(C2, neg_dA_C1, a, p);
                printf("Dechiffre : "); print_point(M_dec); printf("\n");
                break;
            }
        }
    } while (choix != 0);

    return 0;
}
