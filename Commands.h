#ifndef COMMANDS_H
#define COMMANDS_H

#include "CommandInterpreter.h"
#include "StmSystem.h"
#include "hw/lis302dl.h"
#include "hw/ws2801.h"
#include "System.h"
#include "sw/sdcard.h"
#include "hw/tlc5940.h"
#include "hw/hcsr04.h"

#include <cstdio>
#include <vector>

struct Command
{
    const char* mName;
    const char* mParam;
    const char* mHelp;
};

class CmdHelp : public CommandInterpreter::Command
{
public:
    CmdHelp();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Shows help for all commands, or the one given."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
};

class CmdInfo : public CommandInterpreter::Command
{
public:
    CmdInfo(StmSystem& system);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Shows system information."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    StmSystem& mSystem;
};

class CmdFunc : public CommandInterpreter::Command
{
public:
    CmdFunc(StmSystem& system);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Execute a function and show the result."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    StmSystem& mSystem;
};

class CmdRead : public CommandInterpreter::Command
{
public:
    CmdRead();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Read from memory (byte | halfword | word)."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];

    template<class T>
    void dump(T* address, unsigned int count);
};

class CmdWrite : public CommandInterpreter::Command
{
public:
    CmdWrite();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Write to memory (byte | halfword | word)."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
};

class CmdLis : public CommandInterpreter::Command, public System::Event::Callback
{
public:
    CmdLis(LIS302DL& lis);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Show LIS302DL info."; }
protected:
    virtual void eventCallback(System::Event* event);
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    LIS302DL& mLis;
    System::Event mEvent;
    bool mEnabled;
};

class CmdPin : public CommandInterpreter::Command
{
public:
    CmdPin(Gpio** gpio, unsigned int gpioCount);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Read or write GPIO pin."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    Gpio** mGpio;
    unsigned int mGpioCount;
};

class CmdMeasureClock : public CommandInterpreter::Command, public System::Event::Callback
{
public:
    CmdMeasureClock(ClockControl& clockControl, Timer& timer);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Measure external clock (HSE)."; }
protected:
    virtual void eventCallback(System::Event* event);
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    ClockControl& mClockControl;
    Timer& mTimer;
    System::Event mEvent;
    unsigned mCount;
};

class CmdRgb : public CommandInterpreter::Command
{
public:
    CmdRgb(Ws2801& ws);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Set RGB led values."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    Ws2801& mWs;
};

class CmdLightSensor : public CommandInterpreter::Command, public System::Event::Callback
{
public:
    CmdLightSensor(Gpio::Pin& led, Gpio::Pin& s2, Gpio::Pin& s3, Timer& timer, Ws2801 &ws);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Read RGB values."; }
protected:
    virtual void eventCallback(System::Event* event);
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    enum class Color { Red, Green, Blue, White };
    Gpio::Pin& mLed;
    Gpio::Pin& mS2;
    Gpio::Pin& mS3;
    Timer& mTimer;
    Ws2801& mWs;
    System::Event mEvent;

    void setColor(Color color);
    Color nextColor(Color color);
};

class CmdSdio : public CommandInterpreter::Command, public System::Event::Callback
{
public:
    CmdSdio(SdCard &sdCard);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Execute SD commands."; }
protected:
    virtual void eventCallback(System::Event* event);
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    SdCard& mSdCard;
    System::Event mEvent;
};

class CmdMotor : public CommandInterpreter::Command, public System::Event::Callback
{
public:
    CmdMotor();
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Control a motors speed."; }
    bool add(Timer& timer, Timer::CaptureCompareIndex m1, Timer::CaptureCompareIndex m2);
protected:
    virtual void eventCallback(System::Event* event);
private:
    struct Motor
    {
        Timer* mTimer;
        Timer::CaptureCompareIndex mPin1;
        Timer::CaptureCompareIndex mPin2;
    };

    static char const * const NAME[];
    static char const * const ARGV[];
    System::Event mEvent;
    unsigned mMotorCount;
    Motor mMotor[4];
};

class CmdLed : public CommandInterpreter::Command, public System::Event::Callback
{
public:
    CmdLed(Tlc5940& pwm);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Control LED intensity."; }
protected:
    virtual void eventCallback(System::Event* event);
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    System::Event mEvent;
    Tlc5940& mPwm;
};

class CmdDistance : public CommandInterpreter::Command
{
public:
    CmdDistance(HcSr04& hc);
    virtual bool execute(CommandInterpreter& interpreter, int argc, const CommandInterpreter::Argument* argv);
    virtual const char* helpText() const { return "Measure distance."; }
private:
    static char const * const NAME[];
    static char const * const ARGV[];
    HcSr04& mHc;
};

#endif // COMMANDS_H
