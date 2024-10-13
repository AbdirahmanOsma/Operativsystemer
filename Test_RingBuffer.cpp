// Test_RingBuffer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include<string>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std; // Bruker dette for å ikke nevne std:: hvergang

template<typename R> //bruker templates for klassen min, 
class MittRingBuffer // Lager en klasse med navn MittRingBuffer, som både har en private og public del
{
private:
    R* mitt_buffer; 
    int head;
    int tail;
    int size;
    int count;
    mutex LeserMitt_Buffer; //Bruker en mutex slik den ikke forstyrres
    mutex SkrivertilMitt_buffer; // Bruker også en mutex, for å hindre å kjøre flere tråder samtidig
    condition_variable a; // Låser tråd
    condition_variable b; // Låser tråd
public:
    MittRingBuffer(R innhold); // Her har vi konstruktør
    ~MittRingBuffer(); // Her har vi vår destruktør
    void add(R verdi); // Dette er en funskjon, som legger til R verdi i vårt buffer
    char get(); // Funksjon Get henter og tar inn verdi til vårt buffer
    int getSize(); // Funksjon getSize gir oss størrlesen av vårt buffer
};
template<typename R>
MittRingBuffer<R>::MittRingBuffer(R innhold)
{
    mitt_buffer = new R[innhold]; // Oppretter plass i minne heap
    head = tail = 0; // Starter å gi våres verdier ulike variable
    size = innhold; // Setter size like det vi har puttet inn
    count = 0; // Starter å iterere
}
template<typename R>
MittRingBuffer<R>::~MittRingBuffer()
{
    delete[] mitt_buffer; // Frigjør/Sletter plass i minne vi opprettet tidligere
}
template<typename R>
void MittRingBuffer<R>::add(R verdi)
{
    unique_lock <mutex> lock1(LeserMitt_Buffer); // Oppretter en unique_lock som kan låse tråder i vårt mutex
        while (count == size) a.wait(lock1); // While som bruker venter til vi har klart å sende data
        mitt_buffer[head] = verdi; // Legger inn elementer i vårt buffer
    head = (head + 1) % size;
    count++; // Operasjon som oppdaterer når vi setter inn elementer i buffret
    lock1.unlock(); // Låser helt til det er lovlig
    b.notify_one(); // Kommuniserer med buffret og sier når buffret ikke er helt tomt
}

template<typename R>
char MittRingBuffer<R>::get()
{
    unique_lock <mutex> lock2(SkrivertilMitt_buffer); //
    while (count == 0)b.wait(lock2); // Tomt buffer, er 0 og venter til den får data
    
    char tmp_1 = mitt_buffer[tail]; //lager en char med navn tmp_1 i vårt buffer
    tail = (tail + 1) % size; // Operasjon som er med på å flytte tall
    count--; //Vi tar ut et element og må oppdatere
    lock2.unlock(); //Den blir låst til den får beskjed
    a.notify_one(); //Gir beskjed at bufferen ikke er tom
    return tmp_1;
}
template<typename R>
int MittRingBuffer<R>::getSize()
{
    return size; //Får returnert størrelsen av vårt buffer
}
template<typename R>
void LeserMitt_Buffer(MittRingBuffer<R>* mitt_buffer)
{
    while (true)
    {
        cout << mitt_buffer->get(); //Videre har vi en geter som får ut data i vårt buffer, med whileløkke
    }
}
template<typename R>
void mitt_bufferAdder(MittRingBuffer<R>* mitt_buffer)
{
    while (true)
    {
        string a;
        getline(cin, a); //Får inn innput fra cin og leser deretter stringen
        for (int i = 0; i < a.size(); i++) // Får inn stringen 
        {
        mitt_buffer->add(a[i]);
        this_thread::sleep_for(chrono::milliseconds(5));// Får vår tråd til å hvile
        }
        mitt_buffer->add('\n');
    }
}
template<typename R>
void mitt_bufferGenerator(MittRingBuffer<R>* mitt_buffer)
{
    int n = 0;
    while (true) //Operasjon(generator) brukes for å generere buffer
    {
        mitt_buffer->add('O' + n);
        n = (n + 1) % 10;
        this_thread::sleep_for(chrono::milliseconds(450));
    }
}

int main()
{
    MittRingBuffer<int> mitt_buffer(100);
    thread reader(LeserMitt_Buffer<int>, & mitt_buffer); // Her er tråd som leser fra tastatur
    thread adder(mitt_bufferAdder<int>, &mitt_buffer); // Her er en tråd som får inn buffer
    thread generator(mitt_bufferGenerator<int>, &mitt_buffer); // Operasjon (genererer) teksten vi får skrevet inn
    reader.join();
    adder.join();
    generator.join();
}