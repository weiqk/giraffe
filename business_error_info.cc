#include "business_error_info.h"
#include <zmq.hpp>
#include <iostream>
#include <log4cxx/logger.h>
#include <ctime>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>

using namespace log4cxx;

LoggerPtr BusinessErrorInfo::logger_(Logger::getLogger("business_errinfo"));

static const int FIX_TAG_LEN = 2;

static const char MONITOR_MSG_TIME[FIX_TAG_LEN + 1] = "01";
static const char MONITOR_MSG_EXCHANGE[FIX_TAG_LEN + 1] = "02";
static const char MONITOR_MSG_POSITION[FIX_TAG_LEN + 1] = "03";
static const char MONITOR_MSG_RANK[FIX_TAG_LEN + 1] = "04";
static const char MONITOR_MSG_SHORT_DESCRIPTION[FIX_TAG_LEN + 1] = "05";
static const char MONITOR_MSG_DESCRIPTION[FIX_TAG_LEN + 1] = "06";

void BusinessErrorInfo::Init()
{
	InitZMQ();
	LOG4CXX_INFO(logger_, "business thread: zmq has initialized!");
	InitLog();
	LOG4CXX_INFO(logger_, "business thread: log has initialized!");
	InitCurl();
	LOG4CXX_INFO(logger_, "business thead: curl has initialized!");
}

void BusinessErrorInfo::InitZMQ()
{
    sock_recv_ = new zmq::socket_t (*context_, this->zmqitems_[0].zmqpattern);
    if("bind" == this->zmqitems_[0].zmqsocketaction)
    {
        sock_recv_->bind(this->zmqitems_[0].zmqsocketaddr.c_str());
    }
    else if("connect" == this->zmqitems_[0].zmqsocketaction)
    {
        sock_recv_->connect(this->zmqitems_[0].zmqsocketaddr.c_str());
    }	
	else
	{
		LOG4CXX_INFO(logger_, "BusinessErrorInfo[0]:zmq socket bind/connect error!");
	}

	sock_monitor_ = new zmq::socket_t (*context_, this->zmqitems_[1].zmqpattern);
	if("bind" == this->zmqitems_[1].zmqsocketaction)
	{
		sock_monitor_->bind(this->zmqitems_[1].zmqsocketaction.c_str());
	}
	else if("connect" == this->zmqitems_[1].zmqsocketaction)
	{
		sock_monitor_->connect(this->zmqitems_[1].zmqsocketaddr.c_str());
	}
	else
	{
		LOG4CXX_INFO(logger_, "BusinessErrorInfo[1]:zmq socket bind/connect error!");
	}
}

void BusinessErrorInfo::InitLog()
{
	log4cxx::LogString pattern_format("%d:%p:%m%n");
	log4cxx::LogString date_pattern("'.'yyyy-MM-dd");
	log4cxx::LogString file_name("business_error.log");
	
	pattern_layout_ = new log4cxx::PatternLayout(pattern_format);

	appender_ = new log4cxx::DailyRollingFileAppender(pattern_layout_, file_name, date_pattern);
	appender_->setAppend(true);
	
	logger_business_error_ = Logger::getLogger("business_error");
	logger_business_error_->addAppender(appender_);
	logger_business_error_->setAdditivity(false);	
}

void BusinessErrorInfo::InitCurl()
{
	curl_ = curl_easy_init();
	if(!curl_)
	{
		LOG4CXX_ERROR(logger_, "curl init error!");
	}	
}

void BusinessErrorInfo::RunThreadFunc()
{
	zmq::message_t msg;
	while(true)
	{
		msg.rebuild();
		sock_recv_->recv(&msg);
		//std::cout<<"c++:"<<(char *)(msg.data())<<std::endl;
		
		//SplitString((char *)(msg.data()), ":");
		//std::string uri("http://10.15.63.121/Control/input.php");
		//DispatchToWebServer(uri);
		LOG4CXX_INFO(logger_, "recv");
		DispatchToWebServer(msg.data());
		LOG4CXX_ERROR(logger_business_error_, (char*)(msg.data()));
	}	
}

void BusinessErrorInfo::SplitString(char * str, const char * separator)
{
	assert(NULL != str && NULL != separator);
	char * pstr;
	vector <char *> str_vec;
	for(pstr = strtok(str, separator); NULL != pstr; )
	{
		str_vec.push_back(pstr);	
		pstr = strtok(NULL, separator);
	}	
	market_id_ = str_vec[0];
	dc_type_ = str_vec[1];
	error_id_ = str_vec[2];
	error_rank_= str_vec[3];
	error_short_description_ = str_vec[4];
	error_description_ = str_vec[5];
}

void BusinessErrorInfo::DispatchToWebServer(const void * data)
{
	//std::string temp_str = "1:static:001:1:ha:haha";
	//SplitString((char *)temp_str.c_str(), ":");
	SplitString((char*)data, ":");

	time_t now_time = time(NULL);		
	char now_time_buf[20] = {0};
	sprintf(now_time_buf, "%llu", (unsigned long long)now_time);

	std::string msg_body;
	std::string equal = "=";
	std::string SOH = "\1";
	unsigned char msg_buf[2048] = {0};	
	msg_body = MONITOR_MSG_TIME + equal + now_time_buf + SOH + \
				MONITOR_MSG_EXCHANGE + equal + market_id_ + SOH + \
				MONITOR_MSG_POSITION + equal + error_id_ + SOH + \
				MONITOR_MSG_RANK + equal + error_rank_ + SOH + \
				MONITOR_MSG_SHORT_DESCRIPTION + equal + error_short_description_ + SOH + \
				MONITOR_MSG_DESCRIPTION + equal + error_description_ + SOH;		

	unsigned int body_len = msg_body.length();
	int total_len = 6 + body_len + 1;

	msg_buf[0] = '\02';
	msg_buf[1] = '\32';
	unsigned int body_len_nl = htonl(body_len);
	//LOG4CXX_INFO(logger_, body_len);
	//LOG4CXX_INFO(logger_, body_len_nl);
	memcpy((char*)(&msg_buf[2]), (char *)(&body_len_nl), 4);
	memcpy((char*)(&msg_buf[6]), msg_body.c_str(), body_len);
	msg_buf[6 + body_len] = '\03';

	LOG4CXX_INFO(logger_, "msg_body" <<msg_body);
	//LOG4CXX_INFO(logger_, "msg:" << msg_buf);

	zmq::message_t msg(total_len);
	memcpy(msg.data(), msg_buf, total_len);
	sock_monitor_->send(msg);
}

size_t BusinessErrorInfo::RecvDataCallback(void *ptr, size_t size, size_t nmemb, HttpReturnContent *pret_content)
{
	int len = size * nmemb;
	if(NULL == pret_content->pstr_)
	{
		pret_content->pstr_ = new char[len + 1];
		pret_content->len_ = len + 1;
	}
	else
	{
		if(pret_content->len_ < len)
		{
			delete [] pret_content->pstr_;
			pret_content->pstr_ = new char[len + 1];
			pret_content->len_ = len + 1;
		}
	}
	memcpy(pret_content->pstr_, (char *)ptr, len);
	pret_content->pstr_[len] = '\0';
	return len;
}

//void BusinessErrorInfo::DispatchToWebServer(std::string &uri)
//{
	//std::string curl_url_str;
	//time_t now_time = time(NULL);		
	//char now_time_buf[20] = {0};
	//sprintf(now_time_buf, "%llu", (unsigned long long)now_time);
	//curl_url_str = uri + "?"  
	//				+ "time=" + now_time_buf + "&" 
	//				+ "exchange=" + market_id_ + "&"
	//				+ "dc_type=" + dc_type_ + "&"
	//				+ "error=" + error_id_ + "&" 
	//				+ "type=" + type_id_;
	//LOG4CXX_INFO(logger_business_error_, curl_url_str);
	////curl_easy_setopt(curl_, CURLOPT_URL, "http://10.15.63.121/Control/input.php?time=1&exchange=2&error=3&type=1");
	//curl_easy_setopt(curl_, CURLOPT_URL, curl_url_str.c_str());
	//curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 3);
	//curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, RecvDataCallback);

	//curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &http_ret_content_);

	//curl_res_code_ = curl_easy_perform(curl_);
	//if(CURLE_OK == curl_res_code_)
	//{
	//	int ret_code;
	//	assert(NULL != http_ret_content_.pstr_);
	//	ret_code = atoi(http_ret_content_.pstr_);
	//	if(ret_code < 0)
	//	{
	//		LOG4CXX_ERROR(logger_, "http get error:" <<ret_code);
	//	}
	//	else
	//	{
	//		LOG4CXX_INFO(logger_, "http get success.");
	//	}
	//}	
	//else
	//{
	//	LOG4CXX_ERROR(logger_, "curl running error:" << curl_res_code_);
	//	curl_easy_cleanup(curl_);
	//}
//}
