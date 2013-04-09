/**
 * @file echo.h
 *
 * @author jeff.daily@pnnl.gov
 */
#ifndef _ECHO_H_
#define _ECHO_H_

#include <fstream>
#include <iostream>
#include <streambuf>

class Echo
{
    public:
        explicit Echo(const string &filename,
                      ios_base::openmode mode=ios_base::out)
            : os(filename.c_str(), mode) { }

        explicit Echo(const char *filename,
                      ios_base::openmode mode=ios_base::out)
            : os(filename, mode) { }

        Echo& operator<< (bool val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (short val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (unsigned short val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (int val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (unsigned int val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (long val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (unsigned long val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (long long val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (unsigned long long val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (float val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (double val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (long double val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (void* val) {
            this->os << val;
            cout << val;
            return *this;
        }

        Echo& operator<< (streambuf* sb ) {
            this->os << sb;
            cout << sb;
            return *this;
        }

        Echo& operator<< (ostream& (*pf)(ostream&)) {
            pf(this->os);
            pf(cout);
            return *this;
        }

        Echo& operator<< (ios& (*pf)(ios&)) {
            pf(this->os);
            pf(cout);
            return *this;
        }

        Echo& operator<< (ios_base& (*pf)(ios_base&)) {
            pf(this->os);
            pf(cout);
            return *this;
        }

        Echo& operator<< (char c) {
            this->os << c;
            cout << c;
            return *this;
        }

        Echo& operator<< (signed char c) {
            this->os << c;
            cout << c;
            return *this;
        }

        Echo& operator<< (unsigned char c) {
            this->os << c;
            cout << c;
            return *this;
        }

        Echo& operator<< (const char* s) {
            this->os << s;
            cout << s;
            return *this;
        }

        Echo& operator<< (const signed char* s) {
            this->os << s;
            cout << s;
            return *this;
        }

        Echo& operator<< (const unsigned char* s) {
            this->os << s;
            cout << s;
            return *this;
        }

    private:
        ofstream os;
};


template<class charT, class traits, class T>
Echo& operator<< (Echo& os, const T& val)
{
    return os << val;
}

#endif /* _ECHO_H_ */

