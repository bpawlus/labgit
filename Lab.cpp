/*
    Sklad sekcji:
    Benedykt Pawlus
    Daniel Wojdak
*/
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <chrono>
#include <thread>
#include <mutex>

//std::mutex m_scr;

/*
    Wyznaczanie dwoch liczb spelniajacych hipoteze goldbacha
    (wylaczenie odczytywanie z tablicy)

    n - liczba parzysta do weryfikacji
    mapa_pierwszych - referencja do tablicy liczb pierwszych
        (indeks oznacza liczbe, wartosc bool oznacza czy pierwsza czy nie)
*/
int* goldbach_seq(int n, bool* mapa_pierwszych) {
    for (int i = 1; i <= n; i++) {
        if (mapa_pierwszych[i] && mapa_pierwszych[n - i])
            return new int[2] {i, n - i};
    }
    return nullptr;
}



/*
    Znajdowanie liczb pierwszych sekwencyjnie

    n - koniec przedzialu
    mapa_pierwszych - referencja do tablicy liczb pierwszych
        (indeks oznacza liczbe, wartosc bool oznacza czy pierwsza czy nie)
*/
void znajdz_pierwsze_seq(bool* mapa_pierwszych, int n) {
    mapa_pierwszych[1] = true;

    for (int i = 2; i < n; i++) {
        mapa_pierwszych[i] = true;
        for (int j = sqrt(i); j > 1; j--) {
            if (i % j == 0) {
                mapa_pierwszych[i] = false;
                break;
            }
        }
    }
}



/*
    Znajdowanie liczb pierwszych w kilku watkach

    ind - indeksy kolejnych liczb pierwszych do znalezienia (0-nthr-1)
    nthr - liczba watkow
    max - przedzial
    mapa_pierwszych - referencja do tablicy liczb pierwszych 
        (indeks oznacza liczbe, wartosc bool oznacza czy pierwsza czy nie)
*/
void znajdz_pierwsze_par(int ind, int nthr, int max, bool* mapa_pierwszych) {
    //m_scr.lock();
    //std::cout << "Thread id: " << std::this_thread::get_id() << ": Min:" << min << " Max: " << max << std::endl;
    //m_scr.unlock();
    for (int i = 2+ind; i <= max; i+=nthr) {
        mapa_pierwszych[i] = true;
        for (int j = sqrt(i); j > 1; j--) {
            if (i % j == 0) {
                mapa_pierwszych[i] = false;
                break;
            }
        }
    }
}

/*
    Znajdowanie dwoch liczb spelniajacych hipoteze goldbacha w kilku watkach

    min, max - przedzial do przeszukania
    mapa_par - referencja do tablicy z liczbami potwierdzajacymi hipoteze
        (indeks oznacza liczbe, tablica int[] oznacza pare liczb potrwierdzajacych hipoteze)
    mapa_pierwszych - referencja do tablicy liczb pierwszych
        (indeks oznacza liczbe, wartosc bool oznacza czy pierwsza czy nie)
*/
int* goldbach_start_par(int min, int max, int**mapa_par, bool* mapa_pierwszych) {
    min = min % 2 == 1 ? min + 1 : min;

    //m_scr.lock();
    //std::cout << "Thread id: " << this_std::thread::get_id() << ": Min:" << min << " Max: " << max << " Mapa: " << mapa_par << std::endl;
    //m_scr.unlock();

    for (int i = min; i <= max; i += 2) {
        int* res = goldbach_seq(i, mapa_pierwszych);
        if (res != nullptr) {
            mapa_par[i][0] = res[0];
            mapa_par[i][1] = res[1];
        }
    }
    
    return nullptr;
}


int main()
{
    const unsigned int przedz = 789789;
    const unsigned int nthr = 4;

    if (nthr < 2) return 0;

    bool* mapa_seq_pierwszych = new bool[przedz+1];
    int** mapa_seq = new int*[przedz+1];
    for (int i = 0; i < przedz+1; ++i)
        mapa_seq[i] = new int[2];
    auto start_seq = std::chrono::high_resolution_clock::now();

    znajdz_pierwsze_seq(mapa_seq_pierwszych, przedz);
    /*
        Znajdowanie dwoch liczb spelniajacych hipoteze goldbacha sekwencyjnie
    */
    for (int i = 4; i <= przedz; i += 2) {
        int* res = goldbach_seq(i, mapa_seq_pierwszych);
        if (res != nullptr) {
            mapa_seq[i][0] = res[0];
            mapa_seq[i][1] = res[1];
        }
    }
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::microseconds>(end_seq - start_seq);

    //for (int i = 4; i <= przedz; i+=2) {
    //    std::cout << i << ": " << mapa_seq[i][0] << "+" << mapa_seq[i][1] << std::endl;
    //}
    std::cout << "Czas seq: " << duration_seq.count() << "us" << std::endl;

    std::cout << "=====" << std::endl;

    bool* mapa_par_pierwszych = new bool[przedz + 1];
    int** mapa_par = new int* [przedz + 1];
    for (int i = 0; i < przedz + 1; ++i)
        mapa_par[i] = new int[2];
    int liczbnaprzedzial = przedz / nthr;
    auto start_par = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> thr;
    std::vector<std::thread> thr2;
    mapa_par_pierwszych[1] = true;

    for (int i = 0; i < nthr; i++) {
        thr.push_back(std::thread(znajdz_pierwsze_par, i, nthr, przedz, mapa_par_pierwszych));
    }

    for (auto& t : thr)
        t.join();
    thr.clear();




    thr.push_back(std::thread(goldbach_start_par, 4, liczbnaprzedzial, mapa_par, mapa_par_pierwszych));
    for (int i = 1; i < nthr - 1; i++) {
        thr.push_back(std::thread(goldbach_start_par, liczbnaprzedzial * i + 1, liczbnaprzedzial * (i + 1), mapa_par, mapa_par_pierwszych));
    }
    thr.push_back(std::thread(goldbach_start_par, liczbnaprzedzial * (nthr - 1) + 1, przedz, mapa_par, mapa_par_pierwszych));

    for (auto& t : thr)
        t.join();

    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::microseconds>(end_par - start_par);

    //for (int i = 4; i <= przedz; i += 2) {
    //    std::cout << i << ": " << mapa_par[i][0] << "+" << mapa_par[i][1] << std::endl;
    //}
    std::cout << "Czas par: " << duration_par.count() << "us" << std::endl;
    std::cout << "=====" << std::endl;
    std::cout << "Przyspieszenie: " << (float)(duration_seq.count()) / (float)(duration_par.count()) << " dla n=" << przedz << " i l.watkow=" << nthr << std::endl;
    std::cout << "=====" << std::endl;

    std::cout << "Sprawdzanie poprawnosci! Bledy/Zaprzecznie hipotezy: " << std::endl;
    for (int i = 4; i < przedz; i += 2) {
        if (!mapa_seq[i] || !mapa_par[i]) {
            std::cout << i << " nie spelnia hipotezy goldbacha?" << std::endl;
            continue;
        }

        if (mapa_seq[i][0] != mapa_par[i][0]) {
            std::cout << "BLAD!! " << mapa_seq[i][0] << ", " << mapa_seq[i][1] << " seq, " << mapa_par[i][0] << ", " << mapa_par[i][1] << " par!!" << std::endl;
        }
    }
    std::cout << "Koniec listy bledow, czysczenie srodowiska.. " << std::endl;

    delete mapa_seq_pierwszych;
    for (int i = 0; i < przedz; ++i)
        delete mapa_seq[i];
    delete mapa_seq;
 
    delete mapa_par_pierwszych;
    for (int i = 0; i < przedz; ++i)
        delete mapa_par[i];
    delete mapa_par;

    

    return 0;
}