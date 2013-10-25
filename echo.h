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
#include <string>

class Echo
{
    public:
        explicit Echo()
            : os() { }

        explicit Echo(const string &filename,
                      ios_base::openmode mode=ios_base::out)
            : os(filename.c_str(), mode) { }

        explicit Echo(const char *filename,
                      ios_base::openmode mode=ios_base::out)
            : os(filename, mode) { }

	
        void open(const string &filename,
                  ios_base::openmode mode=ios_base::out) {
            this->os.open(filename.c_str(), mode);
        }

        void open(const char *filename,
                  ios_base::openmode mode=ios_base::out) {
            this->os.open(filename, mode);
        }

        void close() {
            this->os.close();
        }

        Echo& operator<< (bool val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (short val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (unsigned short val) {
            if (this->os.is_open()) {
                this->os << val;
               // cout << val;
            }
            return *this;
        }

        Echo& operator<< (int val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (unsigned int val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (long val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (unsigned long val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (long long val) {
            if (this->os.is_open()) {
                this->os << val;
               //cout << val;
            }
            return *this;
        }

        Echo& operator<< (unsigned long long val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (float val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (double val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (long double val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (void* val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (streambuf* val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (ostream& (*pf)(ostream&)) {
            if (this->os.is_open()) {
                pf(this->os);
                //pf(cout);
            }
            return *this;
        }

        Echo& operator<< (ios& (*pf)(ios&)) {
            if (this->os.is_open()) {
                pf(this->os);
                //pf(cout);
            }
            return *this;
        }

        Echo& operator<< (ios_base& (*pf)(ios_base&)) {
            if (this->os.is_open()) {
                pf(this->os);
                //pf(cout);
            }
            return *this;
        }

        Echo& operator<< (char val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (signed char val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (unsigned char val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (const char* val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (const signed char* val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        Echo& operator<< (const unsigned char* val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
            return *this;
        }

        template <typename T>
        Echo& operator<< (const T &val) {
            if (this->os.is_open()) {
                this->os << val;
                //cout << val;
            }
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

