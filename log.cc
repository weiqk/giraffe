/**
* @file log.cc
* @brief record listening ports log info to different log files
* @author ly
* @version 0.1.0
* @date 2013-11-29
*/
#include <log4cxx/logger.h>
#include <log4cxx/dailyrollingfileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/patternlayout.h>
#include "log.h"

using namespace log4cxx;

LoggerPtr Log::logger_(Logger::getLogger("log"));
//map<int,log4cpp::Category*> logmapping;
map<int, log4cxx::LoggerPtr> logmapping;

/**
* @brief thread running function
*/
void Log::RunThreadFunc()
{
//zmq version
{
    int n = ports_.size();
    zmq::context_t * context = context_;
	
	log4cxx::LogString pattern_str("%d:%p:%m%n"); 
	log4cxx::PatternLayoutPtr layout(new PatternLayout(pattern_str));
	
	vector<log4cxx::DailyRollingFileAppenderPtr> appender_array(n);
	vector<log4cxx::LoggerPtr> logger_array(n);
    for(int i=0;i<n;i++)
    {
        string logfile;
        stringstream ss;
        ss<<ports_[i];
        ss>>logfile;
        logfile += ".log";
		appender_array[i] = new DailyRollingFileAppender(layout, logfile, "'.'yyyy-MM-dd");
		appender_array[i]->setAppend(true);

		logger_array[i] = Logger::getLogger(logfile);
		logger_array[i]->addAppender(appender_array[i]);
		logger_array[i]->setAdditivity(false);
		logmapping.insert(make_pair(ports_[i], logger_array[i]));
    }	

	//vector<log4cpp::Appender *> appender(n);
   /* log4cpp::Appender *appender[n];*/
    //log4cpp::PatternLayout *playout = new log4cpp::PatternLayout();
    //playout->setConversionPattern("%d:%p:%m%n");
    //log4cpp::Category &root = log4cpp::Category::getRoot();
	//vector<log4cpp::Category *> logarray (n);
    //log4cpp::Category *logarray[n];
    //size_t maxfilesize = 1024*1024*1024; //1G

    //for(int i=0;i<n;i++)
    //{
    //    string logfile;
    //    stringstream ss;
    //    ss<<ports_[i];
    //    ss>>logfile;
	//	logfile += ".log";
    //    appender[i]=  new log4cpp::RollingFileAppender("default",logfile,maxfilesize,true);
    //    appender[i]->setLayout(playout);

    //    logarray[i] = &(root.getInstance(logfile));
    //    logarray[i]->addAppender(appender[i]);
    //    logarray[i]->setPriority(log4cpp::Priority::INFO);
    //    logmapping.insert(pair<int,log4cpp::Category *>(ports_[i],logarray[i]));
    //}
    bufelement info;
    //log4cpp::Category *log;
	LoggerPtr log;
    //map<int, log4cpp::Category *>::iterator it;
	map<int, LoggerPtr>::iterator it;
    zmq::socket_t socket_log (*context, this->zmqitems_[0].zmqpattern);
    //socket_log.setsockopt(ZMQ_RCVHWM, &ZMQ_RCVHWM_SIZE, sizeof(ZMQ_RCVHWM_SIZE));
    //cout<<"log:pattern:"<<this->zmqitems_[0].zmqpattern<<endl;
    //cout<<"log:socketaction:"<<this->zmqitems_[0].zmqsocketaction<<endl;
    //cout<<"log:socketaddr:"<<this->zmqitems_[0].zmqsocketaddr<<endl;
    if("bind" == this->zmqitems_[0].zmqsocketaction)
    {
        socket_log.bind(this->zmqitems_[0].zmqsocketaddr.c_str());
    }
    else if("connect" == this->zmqitems_[0].zmqsocketaction)
    {
        socket_log.connect(this->zmqitems_[0].zmqsocketaddr.c_str());
    }
    //zmq::pollitem_t items[] = {socket_log, 0, ZMQ_POLLIN, 0};

    while(true)
    {
        //wait for recv
        try
        {
            //int rc = zmq::poll(items, 1, 1000);//timeout = 1s
            //if(rc > 0)
            //{
            //    if(items[0].revents & ZMQ_POLLIN)
            //    {
                    zmq::message_t msg_rcv(sizeof(bufelement));
                    //socket_log.recv(&msg_rcv,ZMQ_NOBLOCK);
                    socket_log.recv(&msg_rcv);
					info = *(static_cast<bufelement *>(msg_rcv.data()));
                    it = logmapping.find(info.port_tag);
                    if(it != logmapping.end())
                    {
                        log = it->second;
						LOG4CXX_INFO(log, info.timestamp << \
									" len:" << info.len << \
									" " << info.iproto << \
									" " << info.saddrbyte1 << \
									"." << info.saddrbyte2 << \
									"." << info.saddrbyte3 << \
									"." << info.saddrbyte4 << \
									":" << info.sport << \
									" -> " << info.daddrbyte1 << \
									"." << info.daddrbyte2 << \
									"." << info.daddrbyte3 << \
									"." << info.daddrbyte4 << \
									":" << info.dport << \
									" flags:" << info.flags << \
									" dc_type:" << info.dctype << \
									" seq:" << info.seqtag);	

                        //log->info("%s len:%-5d %-6s %d.%d.%d.%d:%-6d -> %d.%d.%d.%d:%-6d flags:%-10s dc_type:%-16s seq:%-10lu",
                        //        info.timestamp,
                        //        info.len,
                        //        info.iproto,
                        //        info.saddrbyte1,
                        //        info.saddrbyte2,
                        //        info.saddrbyte3,
                        //        info.saddrbyte4,
                        //        info.sport,
                        //        info.daddrbyte1,
                        //        info.daddrbyte2,
                        //        info.daddrbyte3,
                        //        info.daddrbyte4,
                        //        info.dport,
                        //        info.flags,
                        //        info.dctype,
						//		info.seqtag);
                    }
                    else
                    {
                        //cout<<"can't find mapping!"<<endl;
						LOG4CXX_ERROR(logger_, "can't find mapping!");
                    }

            //    }
            //}
            //else if (rc ==0 )//timeout
            //{
            //    continue;
            //}
            //else
            //{
            //    cout<<"zmq poll fail"<<endl;
            //}
        }
        catch(zmq::error_t error)
        {
            //cout<<"zmq recv error:"<<error.what()<<endl;
			LOG4CXX_ERROR(logger_, "zmq recv error:" << error.what());
            continue;
        }
    }
}

//deque version
//{
//    int n = *((int *)(args));
//    log4cpp::Appender *appender[n];
//    log4cpp::PatternLayout *playout = new log4cpp::PatternLayout();
//    playout->setConversionPattern("%d:%p:%m%n");
//    log4cpp::Category &root = log4cpp::Category::getRoot();
//    log4cpp::Category *logarray[n];
//
//    for(int i=0;i<n;i++)
//    {
//        string logfile;
//        stringstream ss;
//        ss<<i;
//        ss>>logfile;
//		logfile += ".log";
//        appender[i]=  new log4cpp::FileAppender("default",logfile);
//        appender[i]->setLayout(playout);
//
//        logarray[i] = &(root.getInstance(logfile));
//        logarray[i]->addAppender(appender[i]);
//        logarray[i]->setPriority(log4cpp::Priority::INFO);
//        logmapping.insert(pair<int,log4cpp::Category *>(i,logarray[i]));
//    }
//
//    bufelement info;
//    log4cpp::Category *log;
//    map<int, log4cpp::Category *>::iterator it;
//
//    while(true)
//    {
//        //deque
//        if(!dbuf_log.empty())
//        {
//            pthread_mutex_lock(&mutex);
//            info = dbuf_log.front();
//            cout<<"deque size:"<<dbuf_log.size()<<endl;
//            cout<<"info filtertag:"<<info.filtertag<<endl;
//            it = logmapping.find(info.filtertag);
//            if(it != logmapping.end())
//            {
//                log = it->second;
//                log->info("%s len:%d %s %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d flags:%s dc_type:%s",
//                        info.timestamp,
//                        info.len,
//                        info.iproto,
//                        info.saddrbyte1,
//                        info.saddrbyte2,
//                        info.saddrbyte3,
//                        info.saddrbyte4,
//                        info.sport,
//                        info.daddrbyte1,
//                        info.daddrbyte2,
//                        info.daddrbyte3,
//                        info.daddrbyte4,
//                        info.dport,
//                        info.flags,
//                        info.dctype);
//
//                dbuf_log.pop_front();
//                pthread_mutex_unlock(&mutex);
//            }
//            else
//            {
//                cout<<"can't find mapping!"<<endl;
//            }
//        }
//
//    }
//}
//

}

