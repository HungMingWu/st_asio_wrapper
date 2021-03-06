/*
 * stracked_executor.h
 *
 *  Created on: 2018-4-19
 *      Author: youngwolf
 *		email: mail2tao@163.com
 *		QQ: 676218192
 *		Community on QQ: 198941541
 *
 * the top class
 */

#ifndef _ST_ASIO_TRACKED_EXECUTOR_H_
#define _ST_ASIO_TRACKED_EXECUTOR_H_

#include "executor.h"

namespace st_asio_wrapper
{

#if 0 == ST_ASIO_DELAY_CLOSE
class tracked_executor
{
protected:
	virtual ~tracked_executor() {}
	tracked_executor(boost::asio::io_context& _io_context_) : io_context_(_io_context_), aci(std::make_shared<char>('\0')) {}

public:
	typedef std::function<void(const boost::system::error_code&)> handler_with_error;
	typedef std::function<void(const boost::system::error_code&, size_t)> handler_with_error_size;

	bool stopped() const {return io_context_.stopped();}

	#if BOOST_ASIO_VERSION >= 101100
	void post(const std::function<void()>& handler) {boost::asio::post(io_context_, handler);}
	void defer(const std::function<void()>& handler) {boost::asio::defer(io_context_, handler);}
	void dispatch(const std::function<void()>& handler) {boost::asio::dispatch(io_context_, handler);}
	void post_strand(boost::asio::io_context::strand& strand, const std::function<void()>& handler)
		{boost::asio::post(strand, handler);}
	void defer_strand(boost::asio::io_context::strand& strand, const std::function<void()>& handler)
		{boost::asio::defer(strand, handler);}
	void dispatch_strand(boost::asio::io_context::strand& strand, const std::function<void()>& handler)
		{boost::asio::dispatch(strand, handler);}
	#else
	void post(const std::function<void()>& handler) {io_context_.post(handler);}
	void dispatch(const std::function<void()>& handler) {io_context_.dispatch(handler);}
	void post_strand(boost::asio::io_context::strand& strand, const std::function<void()>& handler)
		{strand.post(handler);}
	void dispatch_strand(boost::asio::io_context::strand& strand, const std::function<void()>& handler)
		{strand.dispatch(handler);}
	#endif

	handler_with_error make_handler_error(const handler_with_error& handler) const {return handler;}
	handler_with_error_size make_handler_error_size(const handler_with_error_size& handler) const
		{return handler;}

	bool is_async_calling() const {return !aci.unique();}
	bool is_last_async_call() const {return aci.use_count() <= 2;} //can only be called in callbacks
	inline void set_async_calling(bool) {}

protected:
	boost::asio::io_context& io_context_;

private:
	std::shared_ptr<char> aci; //asynchronous calling indicator
};
#else
class tracked_executor : public executor
{
protected:
	tracked_executor(boost::asio::io_context& io_context_) : executor(io_context_), aci(false) {}

public:
	inline bool is_async_calling() const {return aci;}
	inline bool is_last_async_call() const {return true;}
	inline void set_async_calling(bool value) {aci = value;}

private:
	bool aci; //asynchronous calling indicator
};
#endif

} //namespace

#endif /* _ST_ASIO_TRACKED_EXECUTOR_H_ */
