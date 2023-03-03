#pragma once

class Singleton {
    private:
        static Singleton * pInstance;

        Singleton() {};
        Singleton( const Singleton& );
        Singleton& operator= ( Singleton& );
    public:
        static Singleton * getInstance() {
            if(!pInstance) {
                pInstance = new Singleton();
            }
            return pInstance;
        }
};
