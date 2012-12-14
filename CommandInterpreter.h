#ifndef COMMANDINTERPRETER_H
#define COMMANDINTERPRETER_H

class CommandInterpreter
{
public:
    struct Command
    {
        const char* mName;
        const char* mParam[4];
    };

    CommandInterpreter();
private:
    static Command mCmd[];
};

#endif // COMMANDINTERPRETER_H
