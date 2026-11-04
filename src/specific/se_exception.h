#ifndef SE_EXCEPTION_H
#define SE_EXCEPTION_H

class SE_Exception {
public:
    SE_Exception() {}
    SE_Exception(SE_Exception &) {}
    SE_Exception(unsigned int code) : code(code) {}
    ~SE_Exception() {}
    unsigned int GetCode() { return code; }

private:
    unsigned int code;
};

#endif
