#ifndef DMA_H
#define DMA_H

#include <cstdint>

class Dma
{
public:
    class Stream
    {
    public:
        enum class StreamIndex { Stream0, Stream1, Stream2, Stream3, Stream4, Stream5, Stream6, Stream7 };
        enum class ChannelIndex { Channel0, Channel1, Channel2, Channel3, Channel4, Channel5, Channel6, Channel7 };

        Stream(Dma& dma, StreamIndex stream, ChannelIndex channel);
        ~Stream();

        void enable();
        void disable();
        void waitReady();

    private:
        Dma& mDma;
        uint8_t mStream;
        uint8_t mChannel;

    };

    Dma(unsigned int base);

private:
    enum InterruptFlag { FifoError = 0, DirectModeError = 2, TransferError = 3, HalfTransfer = 4, TransferComplete = 5 };

    struct __STREAM
    {
        struct __CR
        {
            uint32_t EN : 1;
            uint32_t DMEIE : 1;
            uint32_t TEIE : 1;
            uint32_t HTIE : 1;
            uint32_t TCIE : 1;
            uint32_t PFCTRL : 1;
            uint32_t DIR : 2;
            uint32_t CIRC : 1;
            uint32_t PINC : 1;
            uint32_t MINC : 1;
            uint32_t PSIZE : 2;
            uint32_t MSIZE : 2;
            uint32_t PINCOS : 1;
            uint32_t PL : 2;
            uint32_t DBM : 1;
            uint32_t CT : 1;
            uint32_t __RESERVED0 : 1;
            uint32_t PBURST : 2;
            uint32_t MBURST : 2;
            uint32_t CHSEL : 3;
            uint32_t __RESERVED1 : 4;
        }   CR;
        uint16_t NDTR;
        uint16_t __RESERVED0;
        uint32_t PAR;
        uint32_t M0AR;
        uint32_t M1AR;
        struct __FCR
        {
            uint8_t FTH : 2;
            uint8_t DMDIS : 1;
            uint8_t FS : 3;
            uint8_t __RESERVED0 : 1;
            uint8_t FEIE : 1;
            uint8_t __RESERVED1;
            uint16_t __RESERVED2;
        }   FCR;
    };
    struct DMA
    {
        uint16_t ISR[4];
        uint16_t IFCR[4];
        __STREAM STREAM[8];
    };

    volatile DMA* mBase;

    void clearInterrupt(uint8_t stream, uint8_t flag);
    bool checkInterrupt(uint8_t stream, uint8_t flag);
};

#endif // DMA_H
