#pragma once
/**
	@file
	@brief parallel for
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <cybozu/exception.hpp>
#include <cybozu/thread.hpp>
#include <cybozu/array.hpp>

namespace cybozu {

namespace parallel_util {

template<class T, class N>
struct Thread : public cybozu::ThreadBase {
	T *t_;
	N begin_;
	N end_;
	int threadIdx_;
	std::string err_;
	Thread()
		: t_(0)
		, begin_(0)
		, end_(0)
		, threadIdx_(0)
	{
	}
	void init(T& t, N begin, N end, int threadIdx)
	{
		t_ = &t;
		begin_ = begin;
		end_ = end;
		threadIdx_ = threadIdx;
		err_.empty();
	}
	void threadEntry()
	{
		for (N i = begin_; i < end_; i++) {
			try {
				(*t_)(i, threadIdx_);
			} catch (std::exception& e) {
				err_ = e.what();
				break;
			}
		}
	}
	const std::string& getErr() const { return err_; }
};

} // parallel_util
/*
	void T::f(N i);
*/
template<class T, class N>
void parallel_for(T& target, N n, int threadNum)
{
	if (threadNum <= 0) throw cybozu::Exception("cybozu:parallel_for:bad threadNum") << threadNum;
	if ((N)threadNum > n) threadNum = (int)n;
	typedef parallel_util::Thread<T, N> Thread;
	{
		cybozu::ScopedArray<Thread> thread(threadNum);
		const N q = N(n / threadNum);
		const N r = N(n % threadNum);
		N begin = 0;
		for (int i = 0; i < threadNum; i++) {
			N end = begin + q;
			if (i < r) end++;
			thread[i].init(target, begin, end, i);
			begin = end;
		}
		for (int i = 0; i < threadNum; i++) {
			if (!thread[i].beginThread()) throw cybozu::Exception("cybozu:parallel_for:can't beginThread") << i;
		}
		for (int i = 0; i < threadNum; i++) {
			if (!thread[i].joinThread()) throw cybozu::Exception("cybozu:parallel_for:can't joinThread") << i;
		}
		for (int i = 0; i < threadNum; i++) {
			const std::string& err = thread[i].getErr();
			if (!err.empty()) throw cybozu::Exception("cybozu:paralell_for") << i << err;
		}
	}
}

} // cybozu
