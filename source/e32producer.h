#ifndef E32PRODUCER_H
#define E32PRODUCER_H

class E32ImageHeader;
class ParameterManager;

class E32Producer
{
    public:
        E32Producer(ParameterManager *args);
        ~E32Producer();

        void Run();
    private:
        void ReCompress();
        void MakeE32();
        void SaveE32(const char* s, size_t size);
    private:
        E32ImageHeader *iE32Hdr = nullptr;
        ParameterManager *iMan = nullptr;
};

#endif // E32PRODUCER_H
