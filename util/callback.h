/*
    Copyright (c) 2013, <copyright holder> <email>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef CALLBACK_H
#define CALLBACK_H

namespace sim_comm {

class empty {
};

class CallBackPtr {
public:
    virtual ~CallBackPtr() {};
};

template<typename R, typename T1,  typename T2, typename T3>
class CallBack : public CallBackPtr {
public:
    virtual ~CallBack() {};
    virtual R operator()() =0;
    virtual R operator()(T1 a) =0;
    virtual R operator()(T1 a,T2 b) =0;
    virtual R operator()(T1 a,T2 b, T3 c)=0;
};

template<typename T, typename R, typename T1=empty, typename T2=empty, typename T3=empty>
class CallBackNoParam : public CallBack<R,T1,T2,T3> {

private:
    T ptrToRealFunction;
public:
    CallBackNoParam(T const &ptrToFunction) {
        this->ptrToRealFunction = ptrToFunction;
    };

    virtual R operator()() {
        return ptrToRealFunction();
    }

    virtual R operator()(T1 a) {
        throw "Incorrect number of arguments";

    };

    virtual R operator()(T1 a,T2 b) {
        throw "Incorrect number of arguments";
    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        throw "Incorrect number of arguments";
    };


    virtual ~CallBackNoParam() {};
};

template<typename T, typename R, typename T1, typename T2=empty, typename T3=empty>
class CallBackOneParam : public CallBack<R,T1,T2,T3> {

private:
    T ptrToRealFunction;
public:
    CallBackOneParam(T const &ptrToFunction) {
        this->ptrToRealFunction = ptrToFunction;
    };

    virtual R operator()() {
        throw "Incorrect number of arguments";
    }

    virtual R operator()(T1 a) {
        return ptrToRealFunction(a);
    };

    virtual R operator()(T1 a,T2 b) {
        throw "Incorrect number of arguments";
    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        throw "Incorrect number of arguments";
    };


    virtual ~CallBackOneParam() {};
};

template<typename T, typename R, typename T1, typename T2, typename T3=empty>
class CallBackTwoParam : public CallBack<R,T1,T2,T3> {

private:
    T ptrToRealFunction;
public:
    CallBackTwoParam(T const &ptrToFunction) {
        this->ptrToRealFunction = ptrToFunction;
    };

    virtual R operator()() {
        throw "Incorrect number of arguments";
    }

    virtual R operator()(T1 a) {
        throw "Incorrect number of arguments";
    };

    virtual R operator()(T1 a,T2 b) {
        return ptrToRealFunction(a,b);

    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        throw "Incorrect number of arguments";
    };


    virtual ~CallBackTwoParam() {};
};

template<typename T, typename R, typename T1, typename T2, typename T3>
class CallBackThreeParam : public CallBack<R,T1,T2,T3> {

private:
    T ptrToRealFunction;
public:
    CallBackThreeParam(T const &ptrToFunction) {
        this->ptrToRealFunction = ptrToFunction;
    };

    virtual R operator()() {
        throw "Incorrect number of arguments";
    }

    virtual R operator()(T1 a) {
        throw "Incorrect number of arguments";
    };

    virtual R operator()(T1 a,T2 b) {
        throw "Incorrect number of arguments";


    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        return ptrToRealFunction(a,b,c);
    };


    virtual ~CallBackThreeParam() {};
};

template<typename R, typename T1=empty, typename T2=empty, typename T3=empty>
class CallBackNoParamImpl : public CallBack<R,T1,T2,T3> {

private:
    CallBack<R,T1,T2,T3>* impl;
public:
    template<typename T>
    CallBackNoParamImpl(T const &ptrToFunction) {
        impl=new CallBackNoParam<T,R,T1,T2,T3>(ptrToFunction);
    };

    virtual R operator()() {
        return impl->operator()();
    };

    virtual R operator()(T1 a) {
        return impl->operator()(a);
    };

    virtual R operator()(T1 a,T2 b) {
        return impl->operator()(a,b);
    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        return impl->operator()(a,b,c);
    };


    virtual ~CallBackNoParamImpl() {
        delete impl;
    };
};

template<typename R, typename T1, typename T2=empty, typename T3=empty>
class CallBackOneParamImpl : public CallBack<R,T1,T2,T3> {

private:
    CallBack<R,T1,T2,T3>* impl;
public:
    template<typename T>
    CallBackOneParamImpl(T const &ptrToFunction) {
        impl=new CallBackOneParam<T,R,T1,T2,T3>(ptrToFunction);
    };

    virtual R operator()() {
        return impl->operator()();
    };

    virtual R operator()(T1 a) {
        return impl->operator()(a);
    };

    virtual R operator()(T1 a,T2 b) {
        return impl->operator()(a,b);
    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        return impl->operator()(a,b,c);
    };


    virtual ~CallBackOneParamImpl() {
        delete impl;
    };
};

template<typename R, typename T1, typename T2, typename T3=empty>
class CallBackTwoParamImpl : public CallBack<R,T1,T2,T3> {

private:
    CallBack<R,T1,T2,T3>* impl;
public:
    template<typename T>
    CallBackTwoParamImpl(T const &ptrToFunction) {
        impl=new CallBackTwoParam<T,R,T1,T2,T3>(ptrToFunction);
    };

    virtual R operator()() {
        return impl->operator()();
    };

    virtual R operator()(T1 a) {
        return impl->operator()(a);
    };

    virtual R operator()(T1 a,T2 b) {
        return impl->operator()(a,b);
    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        return impl->operator()(a,b,c);
    };


    virtual ~CallBackTwoParamImpl() {
        delete impl;
    };
};

template<typename R, typename T1, typename T2, typename T3>
class CallBackThreeParamImpl : public CallBack<R,T1,T2,T3> {

private:
    CallBack<R,T1,T2,T3>* impl;
public:
    template<typename T>
    CallBackThreeParamImpl(T const &ptrToFunction) {
        impl=new CallBackThreeParam<T,R,T1,T2,T3>(ptrToFunction);
    };

    virtual R operator()() {
        return impl->operator()();
    };

    virtual R operator()(T1 a) {
        return impl->operator()(a);
    };

    virtual R operator()(T1 a,T2 b) {
        return impl->operator()(a,b);
    };

    virtual R operator()(T1 a,T2 b, T3 c) {
        return impl->operator()(a,b,c);
    };


    virtual ~CallBackThreeParamImpl() {
        delete impl;
    };
};

template <typename R>
CallBack<R, empty, empty, empty>* CreateCallback (R (*fnPtr)()) {
    return new CallBackNoParamImpl<R, empty,empty,empty>(fnPtr);
}

template <typename R, typename T1>
CallBack<R, T1, empty, empty>* CreateCallback (R (*fnPtr)(T1)) {
    return new CallBackOneParamImpl<R, T1,empty,empty>(fnPtr);
}

template <typename R, typename T1,typename T2>
CallBack<R, T1, T2,empty>* CreateCallback (R (*fnPtr)(T1, T2)) {
    return new CallBackTwoParamImpl<R, T1, T2,empty>(fnPtr);
}

template <typename R, typename T1,typename T2,typename T3>
CallBack<R, T1, T2,T3>* CreateCallback (R (*fnPtr)(T1, T2, T3)) {
    return new CallBackThreeParamImpl<R, T1, T2,T3>(fnPtr);
}

}

#endif // CALLBACK_H
