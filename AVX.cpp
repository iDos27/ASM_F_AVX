#include <iostream>
using namespace std;

int main() {
    //Cierniaken Machen
    cout << "y = ax ^ 3 + bx ^ 2 + cx + d" << endl;
    double a = 2.0, b = 3.0, c = 4.0, d = 2.5, x = 2.0;
    //cout << "Podaj a, b, c, d, x: ";
    //cin >> a >> b >> c >> d >> x;

    double result;
    __asm {
        vmovsd xmm0, a;
        vmovsd xmm1, b;
        vmovsd xmm2, c;
        vmovsd xmm3, d;
        vmovsd xmm4, x;
        vmulsd xmm0, xmm0, xmm4;
        vmulsd xmm2, xmm2, xmm4;
        vmulsd xmm4, xmm4, xmm4;
        vmulsd xmm0, xmm4, xmm4;
        vmulsd xmm1, xmm1, xmm4;
        vaddsd xmm0, xmm0, xmm3;
        vaddsd xmm0, xmm0, xmm2;
        vaddsd xmm0, xmm0, xmm1;
        vmovsd result, xmm0;
    }
    cout << result << endl;

    cout << "Iloczyn skalarny dwoch wektorow" << endl;
    double aa[8] = { 1.0, 2.0, 3.0, 4.0, 1.5, 2.5, 3.5, 4.5 };
    double bb[8] = { 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0 };
    __asm {
        // rcx = a[], rdx = b[]
        vmovupd ymm0, ymmword ptr[rcx];         // a[0 - 3]
        vmovupd ymm1, ymmword ptr[rdx];         // b[0 - 3]
        vmulpd ymm0, ymm0, ymm1;                // ymm0 = a[0 - 3] * b[0 - 3]

        vmovupd ymm2, ymmword ptr[rcx + 32];    // a[4 - 7]
        vmovupd ymm3, ymmword ptr[rdx + 32];    // b[4 - 7]
        vmulpd ymm2, ymm2, ymm3;                // ymm2 = a[4 - 7] * b[4 - 7]

        vaddpd ymm0, ymm0, ymm2; ymm0 = suma obu części

        // Redukcja do skalara(sumowanie elementów)
        vextractf128 xmm1, ymm0, 1; wyciągnij wyższą połowę
        vaddpd xmm0, xmm0, xmm1;                // dodaj obie połowy : xmm0 = [a + b, c + d]

        // poziome dodanie
        vhaddpd xmm0, xmm0, xmm0; xmm0[0] = suma
    }

    // wersja dla każdej tablicy o długości n gdzie n%8 == 0
    __asm {
        mov rcx, 0;
        mov rsi, aa;
        mov rdi, bb;
    loop:
        cmp rcx, n;
        jge end;
        vmovupd ymm0, ymmword ptr[rsi + rcx * 8];
        ymovupd ymm1, ymmword ptr[rdi + rcx * 8];
        vmulpd ymm2, ymm0, ymm1;
        vaddpd ymm4, ymm4, ymm2;
        add rcx, 4;
        vmovupd ymm0, ymmword ptr[rsi + rcx * 8];
        ymovupd ymm1, ymmword ptr[rdi + rcx * 8];
        vmulpd ymm2, ymm0, ymm1;
        vaddpd ymm4, ymm4, ymm2;
        add rcx, 4;
        cmp rcx, n;
        jl loop;
    end:
        vextractf128 xmm1, ymm4, 1;
        vaddpd xmm0, ymm4, xmm1;
        vhaddpd xmm0, xmm0, xmm0;
        movsd result, xmm0
    }

    // wersja dla float
    __asm {
        mov rcx, 0;
        mov rsi, aa;
        mov rdi, bb;
    loop:
        cmp rcx, n;
        jge end;
        vmocups ymm0, ymmword ptr[rsi + rcx * 4];
        vmocups ymm1, ymmword ptr[rdi + rcx * 4];
        vmulps ymm2, ymm0, ymm1;
        vadd ymm4, ymm4, ymm2;
        add rcx, 8;
        cmp rcx, n;
        jl loop
    end:
        vextractf128 xmm1, ymm4, 1;
        vaddpd xmm0, ymm4, xmm1;
        vhaddps xmm0, xmm0, xmm0;
        vhaddps xmm0, xmm0, xmm0;
        movss result, xmm0
    }
    double wynik;

    cout << "Iloczyn skalarny = " << wynik << endl;


    // zmienne a i b typu float oraz tablica x liczb typu float.
    // Oblicz wartość y[i] = sqrt((x[i]-a)^2 + (x[i] - b)^2)
    __asm {
        mov             rcx, 0;
        mov             rsi, x;
        mov             rdi, y;
        vbroadcastss    ymm5, a;
        vbroadcastss    ymm6, b;
    loop:
        cmp rcx, n;
        jge end;
        vmovups ymm1, ymmword ptr[rsi + rcx * 4];
        vsubps  ymm2, ymm1, ymm5;
        vmulps  ymm2, ymm2, ymm2;                   // (x[i] - a)^2
        vsubps  ymm3, ymm1, ymm6;
        vmulps  ymm3, ymm3, ymm3;                   // (x[i] - b)^2
        vaddps  ymm4, ymm2, ymm3;                   // (x[i]-a)^2 + (x[i] - b)^2               
        vsqrtps ymm0, ymm4;                         // sqrt((x[i]-a)^2 + (x[i] - b)^2)
        vmovups ymmword ptr[rdi + rcx * 4], ymm0;   // zapis
        add rcx, 8;
        cmp rcx, n;
        jl loop;
    end:
        ret
    }

    // wersja dla double
    __asm {
        mov rcx, 0;
        mov rsi, x;
        mov rdi, y;
        vbroadcastsd ymm5, a;
        vbroadcastsd ymm6, b;

    loop:
        cmp rcx, n;
        jge end;
        vmovupd ymm1, ymmword ptr[rsi, +rcx * 8];
        vsubpd ymm2, ymm1, ymm5;
        vmulpd ymm2, ymm2, ymm2;
    }
    
//  zad 6. ax^3 + bx^2 + cx + d;
//  zad6(double a, double b, double c, double d, double x)
    _asm{
        vbroadcastsd ymm0, a;
        vbroadcastsd ymm1, b;
        vbroadcastsd ymm2, c;
        vbroadcastsd ymm3, d;
        vbroadcastsd ymm4, x;

        vmulsd ymm2, ymm2, ymm4            // ymm2 -> cx
        vaddsd ymm2, ymm2, ymm3            // ymm2 -> cx+d
        vmulsd ymm5, ymm4, ymm4            // ymm5 -> x^2   
        vmulsd ymm1, ymm1, ymm5            // ymm1 -> bx^2
        vmulsd ymm5, ymm5, ymm4            // ymm5 -> x^3
        vmulsd ymm0, ymm0, ymm5            // ymm0 -> ax^3
        vaddsd ymm0, ymm0, ymm1            // ax^3 + bx^2
        vaddsd ymm0, ymm0, ymm2            // wynik = ax^3 + bx^2 + cx+d
    }
// zad5(double a, double b, double x)
//   ymm0 -> a
//   ymm1 -> b
//   ymm2 -> x

    vmulsd ymm3 ,ymm2 ,ymm2             // ymm3 = x^2
    vmulsd ymm4, ymm3, ymm2             // ymm4 = x^3

    // a-bx^2
    vmulsd ymm5, ymm1, ymm3             // ymm5 = b*x^2
    vsubsd ymm5, ymm0, ymm5             // ymm5 = a-bx^2
    
    // abs(a-bx^2)
    vandpd ymm5, ymm5, [abs_mask];      // abs_mask = 0x7FFFFFFFFFFFFFFF

    // a*x^2 + b*x^3
    vmulsd ymm6, ymm0, ymm3             // ymm8 = a*x^2
    vmulsd ymm7, ymm1, ymm4             // ymm7 = b*x^3
    vaddsd ymm6, ymm6, ymm7             // ymm6 = a*x^2 + b*x^3
    
    // sqrt(a*x^2 + b*x^3)
    vsqrtsd ymm6, ymm6, ymm6            // ymm3 = sqrt(a*x^2 + b*x^3)
    
    // Wynik
    vmulsd ymm0, ymm5, ymm6             // ymm0 = wynik