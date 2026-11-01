namespace c {
extern "C" void S_ExitSystem(char *message);
}

void S_ExitSystem(char *message)
{
    c::S_ExitSystem(message);
}
