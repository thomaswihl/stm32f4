/*
 * (c) 2012 Thomas Wihl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DMA_H
#define DMA_H

#include "System.h"
#include "InterruptController.h"

#include <cstdint>

class Dma
{
public:
    enum InterruptFlag { FifoError = 1, DirectModeError = 4, TransferError = 8, HalfTransfer = 16, TransferComplete = 32 };
    Dma(unsigned int base);

private:

    struct __STREAM
    {
        union __CR {
            struct
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
            };
            uint32_t CR;
        }   CR;
        uint32_t NDTR;
        uint32_t PAR;
        uint32_t M0AR;
        uint32_t M1AR;
        union __FCR
        {
            struct
            {
                uint32_t FTH : 2;
                uint32_t DMDIS : 1;
                uint32_t FS : 3;
                uint32_t __RESERVED0 : 1;
                uint32_t FEIE : 1;
                uint32_t __RESERVED1 : 24;
            };
            uint32_t FCR;
        }   FCR;
    };
    struct DMA
    {
        uint32_t ISR[2];
        uint32_t IFCR[2];
        __STREAM STREAM[8];
    };

    volatile DMA* mBase;

    void clearInterrupt(uint8_t stream, uint8_t flag);
    bool checkInterrupt(uint8_t stream, uint8_t flag);
    uint8_t getInterruptStatus(uint8_t stream);
    void clearInterruptStatus(uint8_t stream, uint8_t bits);
    unsigned int shiftFromIndex(uint32_t index);

public:
    class Stream : public InterruptController::Callback
    {
    public:
        enum class StreamIndex { Stream0, Stream1, Stream2, Stream3, Stream4, Stream5, Stream6, Stream7 };
        enum class ChannelIndex { Channel0, Channel1, Channel2, Channel3, Channel4, Channel5, Channel6, Channel7 };
        enum class Direction { PeripheralToMemory, MemoryToPeripheral, MemoryToMemory };
        enum class DataSize { Byte, HalfWord, Word };
        enum class Priority { Low, Medium, High, VeryHigh };
        enum class BurstLength { Single, Beats4, Beats8, Beats16 };
        enum class FlowControl { Dma, Sdio };
        enum class FifoThreshold { Quater = 0, Half = 1, ThreeQuater = 2, Full = 3, Disable = 4 };
        enum class End { Memory = 0, Peripheral = 1, MemoryToMemoryDestination = 0, MemoryToMemorySource = 1, Memory0 = 0, Memory1 = 2 };

        class Callback
        {
        public:
            enum class Reason { TransferComplete, TransferError, FifoError, DirectModeError };
            Callback() { }
            virtual ~Callback() { }
            virtual void dmaCallback(Stream* stream, Reason reason) = 0;
        };

        Stream(Dma& dma, StreamIndex stream, ChannelIndex channel, InterruptController::Line* interrupt);
        ~Stream();

        void start();
        void waitReady();

        void setBurstLength(End end, BurstLength burstLength);
        void setPriority(Priority priority);
        void setDataSize(End end, DataSize dataSize);
        void setIncrement(End end, bool increment);
        void setDirection(Direction direction);
        void setAddress(End end, System::BaseAddress address);
        void setCallback(Callback* callback);
        void setTransferCount(uint16_t count);
        uint16_t transferCount();
        void setFlowControl(FlowControl flowControl);
        void setCircular(bool circular);

        void config(Direction direction, bool peripheralIncrement, bool memoryIncrement, DataSize peripheralDataSize, DataSize memoryDataSize, BurstLength peripheralBurst, BurstLength memoryBurst);
        void configFifo(FifoThreshold threshold);

        virtual void interruptCallback(InterruptController::Index index);

        bool complete();

    private:
        Dma& mDma;
        uint8_t mStream;
        uint8_t mChannel;
        InterruptController::Line* mInterrupt;
        Callback* mCallback;

        System::BaseAddress mPeripheral;
        System::BaseAddress mMemory0;
        System::BaseAddress mMemory1;
        uint16_t mCount;
        Dma::__STREAM::__CR mStreamConfig;
        Dma::__STREAM::__FCR mFifoConfig;
    };

};

#endif // DMA_H
