//operacje.h - wrappery

#ifndef OPERACJE_H
#define OPERACJE_H

class Semaphore {
    int sem_id;
public:
    void usun();
};

int alokujPamiec();
void zwolnijPamiec();

#endif