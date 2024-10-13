// Test_RingBuffer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include<string>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std; // Bruker dette for � ikke nevne std:: hvergang

template<typename R> //bruker templates for klassen min, 
class MittRingBuffer // Lager en klasse med navn MittRingBuffer, som b�de har en private og public del
{
private:
    R* mitt_buffer; 
    int head;
    int tail;
    int size;
    int count;
    mutex LeserMitt_Buffer; //Bruker en mutex slik den ikke forstyrres
    mutex SkrivertilMitt_buffer; // Bruker ogs� en mutex, for � hindre � kj�re flere tr�der samtidig
    condition_variable a; // L�ser tr�d
    condition_variable b; // L�ser tr�d
public:
    MittRingBuffer(R innhold); // Her har vi konstrukt�r
    ~MittRingBuffer(); // Her har vi v�r destrukt�r
    void add(R verdi); // Dette er en funskjon, som legger til R verdi i v�rt buffer
    char get(); // Funksjon Get henter og tar inn verdi til v�rt buffer
    int getSize(); // Funksjon getSize gir oss st�rrlesen av v�rt buffer
};
template<typename R>
MittRingBuffer<R>::MittRingBuffer(R innhold)
{
    mitt_buffer = new R[innhold]; // Oppretter plass i minne heap
    head = tail = 0; // Starter � gi v�res verdier ulike variable
    size = innhold; // Setter size like det vi har puttet inn
    count = 0; // Starter � iterere
}
template<typename R>
MittRingBuffer<R>::~MittRingBuffer()
{
    delete[] mitt_buffer; // Frigj�r/Sletter plass i minne vi opprettet tidligere
}
template<typename R>
void MittRingBuffer<R>::add(R verdi)
{
    unique_lock <mutex> lock1(LeserMitt_Buffer); // Oppretter en unique_lock som kan l�se tr�der i v�rt mutex
        while (count == size) a.wait(lock1); // While som bruker venter til vi har klart � sende data
        mitt_buffer[head] = verdi; // Legger inn elementer i v�rt buffer
    head = (head + 1) % size;
    count++; // Operasjon som oppdaterer n�r vi setter inn elementer i buffret
    lock1.unlock(); // L�ser helt til det er lovlig
    b.notify_one(); // Kommuniserer med buffret og sier n�r buffret ikke er helt tomt
}

template<typename R>
char MittRingBuffer<R>::get()
{
    unique_lock <mutex> lock2(SkrivertilMitt_buffer); //
    while (count == 0)b.wait(lock2); // Tomt buffer, er 0 og venter til den f�r data
    
    char tmp_1 = mitt_buffer[tail]; //lager en char med navn tmp_1 i v�rt buffer
    tail = (tail + 1) % size; // Operasjon som er med p� � flytte tall
    count--; //Vi tar ut et element og m� oppdatere
    lock2.unlock(); //Den blir l�st til den f�r beskjed
    a.notify_one(); //Gir beskjed at bufferen ikke er tom
    return tmp_1;
}
template<typename R>
int MittRingBuffer<R>::getSize()
{
    return size; //F�r returnert st�rrelsen av v�rt buffer
}
template<typename R>
void LeserMitt_Buffer(MittRingBuffer<R>* mitt_buffer)
{
    while (true)
    {
        cout << mitt_buffer->get(); //Videre har vi en geter som f�r ut data i v�rt buffer, med whilel�kke
    }
}
template<typename R>
void mitt_bufferAdder(MittRingBuffer<R>* mitt_buffer)
{
    while (true)
    {
        string a;
        getline(cin, a); //F�r inn innput fra cin og leser deretter stringen
        for (int i = 0; i < a.size(); i++) // F�r inn stringen 
        {
        mitt_buffer->add(a[i]);
        this_thread::sleep_for(chrono::milliseconds(5));// F�r v�r tr�d til � hvile
        }
        mitt_buffer->add('\n');
    }
}
template<typename R>
void mitt_bufferGenerator(MittRingBuffer<R>* mitt_buffer)
{
    int n = 0;
    while (true) //Operasjon(generator) brukes for � generere buffer
    {
        mitt_buffer->add('O' + n);
        n = (n + 1) % 10;
        this_thread::sleep_for(chrono::milliseconds(450));
    }
}

int main()
{
    MittRingBuffer<int> mitt_buffer(100);
    thread reader(LeserMitt_Buffer<int>, & mitt_buffer); // Her er tr�d som leser fra tastatur
    thread adder(mitt_bufferAdder<int>, &mitt_buffer); // Her er en tr�d som f�r inn buffer
    thread generator(mitt_bufferGenerator<int>, &mitt_buffer); // Operasjon (genererer) teksten vi f�r skrevet inn
    reader.join();
    adder.join();
    generator.join();
}