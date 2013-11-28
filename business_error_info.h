#ifndef MONITOR_BUSINESS_ERROR_INFO_H
#define MONITOR_BUSINESS_ERROR_INFO_H

#include <zmq.hpp>
#include <log4cxx/logger.h>
#include <log4cxx/dailyrollingfileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/patternlayout.h>
//#include <curl/curl.h>
#include "basethread.h"

class HttpReturnContent
{
public:
	HttpReturnContent():pstr_(NULL),len_(0){};
	~HttpReturnContent()
	{
		if(NULL != pstr_)
		{
			delete [] pstr_;
			pstr_ = NULL;
		}
	};
	char *pstr_;
	int len_;
private:
};

class BusinessErrorInfo: public BaseThread
{
public:
	BusinessErrorInfo(zmq::context_t *context):context_(context)
	{
		sock_recv_ = NULL;
		sock_monitor_ = NULL;
	}	
	~BusinessErrorInfo()
	{
		if(NULL != sock_recv_)
		{
			sock_recv_->close();
			delete sock_recv_;
			sock_recv_ = NULL;
		}
		if(NULL != sock_monitor_)
		{
			sock_monitor_->close();
			delete sock_monitor_;
			sock_monitor_ = NULL;
		}
		if(NULL != pattern_layout_)
		{
			delete pattern_layout_;
			pattern_layout_ = 0;
		}
		if(NULL != appender_)
		{
			delete appender_;
			appender_ = 0;
		}
	};
	void Init();
	void RunThreadFunc();
private:
	void InitZMQ();
	void InitLog();
	//void InitCurl();
	void WriteToLog(const std::string &str);
	static size_t RecvDataCallback(void *ptr, size_t size, size_t nmemb, HttpReturnContent *);
	void SplitString(char *str, const char * separator);
	//void DispatchToWebServer(std::string &uri);
	void DispatchToWebServer(const void *data);

	zmq::context_t *context_;
	zmq::socket_t *sock_recv_;
	zmq::socket_t *sock_monitor_;
	static log4cxx::LoggerPtr logger_;
	log4cxx::LoggerPtr logger_business_error_;
	log4cxx::PatternLayoutPtr pattern_layout_;
	log4cxx::DailyRollingFileAppenderPtr appender_;
	//CURL * curl_;
	//CURLcode curl_res_code_;
	std::string market_id_;
	std::string dc_type_;
	std::string error_id_;
	std::string error_rank_;	
	std::string error_short_description_;
	std::string error_description_;
	//HttpReturnContent http_ret_content_;
};

#endif
